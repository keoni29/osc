#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdint.h>
#include <libconfig.h>
#include "notes.h"
#include "voice.h"
#include "pcm.h"
#include "channel.h"

#define CH_DRUMS 9

extern SDL_mutex *mut;

/** Create a new midi channel
 * 	Returns a negative value on failure.
 * 	Voices are dynamically allocated.
 * 	Todo: make channel number to drums better...
 */
int CreateChannel(struct channel_t *c, uint16_t number)
{
	int i;

	c->number = number;

	if (c->number == CH_DRUMS)
	{
		c->voiceCount = 0;
		/* Todo make special PCM voices */
		for (i = 0; i < 128; i++)
		{
			char str[20];
			sprintf(str, "samples/drums/%d.wav", i);
			if (PCMCreate(str, &c->drum[i]) < 0)
			{
				fprintf(stderr, "Could not load sample %d\n", i);
				PCMCreateEmpty(&c->drum[i]);
			}
		}
	}
	else
	{
		c->voiceCount = MAX_POLYPHONY;

		for(i = 0; i < c->voiceCount; i++)
		{
			if (CreateVoice(&c->voice[i]) < 0)
			{
				fprintf(stderr, "Ch %02d, voice %d could not be created\n", c->number, i);
				break;
			}
			c->vgate[i] = -1;
		}
	}

	return 0;
}

float ChannelSample(struct channel_t *c)
{
	float result = 0;
	int i;

	if (c->number == CH_DRUMS)
	{
		for (i = 35; i < 81; i++)
		{
			result += PCMSample(&c->drum[i]);
		}
	}
	else
	{
		for (i = 0; i < c->voiceCount; i++)
		{
			result+= VoiceSample(c->voice[i]) / 8;	/* Todo change this arbitrary constant */
		}
	}

	return result;
}

int ChannelEvent(struct channel_t *c, struct SMF_event *e)
{
	/* Todo process every event */
	switch (e->status)
	{
	case SMF_NoteOff:
		if (c->number != CH_DRUMS)
		{
			ChannelNoteOff(c, e->p1, e->p1);
		}
		else
		{
			/* Todo perhaps volume envelope for pcm? */
			//PCMStop(&drum[e.p1]);
		}
		break;

	case SMF_NoteOn:
		if (c->number != CH_DRUMS)
		{
			if (e->p2 == 0)
			{
				ChannelNoteOff(c, e->p1, e->p1);
			}
			else
			{
				ChannelNoteOn(c, e->p1, e->p1, e->p2);
			}
		}
		else
		{
			/* Todo remove this printf */
#ifdef DEBUG
			printf("Drum %d\n", e->p1);
#endif
			if (e->p2 == 0)
			{
				PCMStop(&c->drum[e->p1]);
			}
			else
			{
				PCMStart(&c->drum[e->p1]);
			}
		}
		break;

	case SMF_KeyPressure:

		break;

	case SMF_ControlChange:

		break;

	case SMF_ChannelPressure:

		break;

	case SMF_PitchBend:

		break;

	case SMF_Unhandled:
	default:
		/* Todo handle unhandled events */
		break;
	}
	return 0;
}

/* Todo note history stack? */

/** Play a note
 *	note : The midi note value to be played
 *	key : The key on the keyboard associated with this note
 *	channel : The midi channel number
 */
int ChannelNoteOn(struct channel_t *c, int note, int key, uint8_t vel)
{
	float freq;
	int i;
	uint8_t r = 0;
	if (key >= 0 && note >= 0)
	{
		for (i = 0; i < c->voiceCount; i++)
		{
			if (c->vgate[i] == -1)
			{
				/* Todo put this back */
				//if (c->voice[i]->env[0].amp < 0.3)
				//{
					c->vgate[i] = key;
					r = 1;
				//}
				if (r)
				{
					break;
				}
			}
		}

		if (!r)
		{

#ifdef DEBUG
			fprintf(stderr, "Not enough voices for channel %d\n", c->number);
#endif
			return -1;
		}

		freq = Note2Freq(note);
		VoiceSetFreq(c->voice[i], freq);
		VoiceGateOn(c->voice[i]);
		c->voice[i]->volume = (float)vel / 128;
	}

	return key;
}

/* Todo get rid of note parameter as it is unused */
int ChannelNoteOff(struct channel_t *c, int note, int key)
{
	int i;
	uint8_t r = 0;
	if (key >= 0 && note >= 0)
	{
		for (i = 0; i < c->voiceCount; i++)
		{
			if (c->vgate[i] == key)
			{
				c->vgate[i] = -1;
				r = 1;
				break;
			}
		}

		if (!r)
		{
			return -1;
		}

		VoiceGateOff(c->voice[i]);
	}
	return key;
}

/* Todo: Make patch shared between voices instead of loading per voice */
/** Returns -1 on failure, 0 on success */
int ChannelLoadPatch(struct channel_t *c, const char *fname, const char *patch)
{
	config_t config;
	struct voice_t *v;
	int i, j;

	/* Read the configuration file */
	config_init(&config);

	if (config_read_file(&config, fname) == CONFIG_FALSE)
	{
		fprintf(stderr, "%s\n", config_error_text(&config));
		return -1;
	}
	else
	{
		config_set_auto_convert(&config, CONFIG_OPTION_AUTOCONVERT);
	}

	for(i = 0; i < c->voiceCount; i++)
	{
		char path[100];
		double f;
		int d;

		v = c->voice[i];

		if (SDL_mutexP(mut) == -1)
		{
			fprintf(stderr, "Could not lock mutex.\r\n");
			return -1;
		}
		else
		{
			/* Todo: when float fails try integer */
			for(j = 0; j < VOICE_OPCOUNT; j++)
			{
				sprintf(path,"%s.op.[%d].mult", patch, j);
				config_lookup_float(&config, path, &f);
				v->op[j].mult = f;
				sprintf(path, "%s.op.[%d].shape", patch, j);
				config_lookup_int(&config, path, &d);
				v->op[j].shape = d;
				sprintf(path, "%s.op.[%d].pw", patch, j);
				config_lookup_float(&config, path, &f);
				v->op[j].pw = f;
				sprintf(path, "%s.op.[%d].a", patch, j);
				config_lookup_float(&config, path, &f);
				v->env[j].a = f;
				sprintf(path, "%s.op.[%d].d", patch, j);
				config_lookup_float(&config, path, &f);
				v->env[j].d = f;
				sprintf(path, "%s.op.[%d].s", patch, j);
				config_lookup_float(&config, path, &f);
				v->env[j].s = f;
				sprintf(path, "%s.op.[%d].r", patch, j);
				config_lookup_float(&config, path, &f);
				v->env[j].r = f;
				sprintf(path, "%s.op.[%d].mix", patch, j);
				config_lookup_float(&config, path, &f);
				v->mix[j] = f;
				sprintf(path, "%s.op.[%d].fmsrc", patch, j);
				config_lookup_int(&config, path, &d);
				v->opFMSource[j] = d;
				sprintf(path, "%s.op.[%d].fmi", patch, j);
				config_lookup_float(&config, path, &f);
				v->opFMIndex[j] = f;
			}
			SDL_mutexV(mut);
		}

		ChannelNoteOff(c, i, i);
	}

	config_destroy(&config);
	return 0;
}
