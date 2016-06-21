#include <SDL2/SDL.h>
#include "SDL2/SDL_thread.h"
#include "notes.h"
#include "voice.h"
#include "config.h"
#include "play.h"
#include "smf.h"
#include "pcm.h"

#define VOICE_COUNT 15

static SDL_mutex *mut;
static struct voice_t *ensemble[VOICE_COUNT];
static int vgate[VOICE_COUNT];

static struct pcm_t drum[128];

#ifdef DEBUG
FILE *export;
#endif

/** Play SMF frame-by-frame.
 * 	Returns amount of frames before next event. */
uint32_t PlaySMF()
{
	int result;
	uint32_t frames = 0;
	uint32_t interval;
	struct SMF_event e;
	while (1)
	{
		result = SMF_PollEvent(&e, &interval);

		if (result == SMF_EVENT_READY)
		{
			/* Todo process every event */
			switch (e.status)
			{
			case SMF_NoteOff:
				if (e.channel != 10)
				{
					PlayNoteOff(e.p1, e.p1);
				}
				else
				{
					/* Todo perhaps volume envelope for pcm? */
					//PCMStop(&drum[e.p1]);
				}
				break;

			case SMF_NoteOn:
				if (e.channel != 10)
				{
					if (e.p2 == 0)
					{
						PlayNoteOff(e.p1, e.p1);
					}
					else
					{
						PlayNoteOn(e.p1, e.p1);
					}
				}
				else
				{
					/* Todo remove this printf */
#ifdef DEBUG
					printf("Drum %d\n", e.p1);
#endif
					if (e.p2 == 0)
					{
						PCMStop(&drum[e.p1]);
					}
					else
					{
						PCMStart(&drum[e.p1]);
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
		}
		else if (result == SMF_STOPPED)
		{
			return 0;
		}

		frames = (interval * sampleRate) / 1000;
		if (frames)
		{
			break;
		}
	}

	return frames;
}

/** Callback for filling the audio stream buffer */
void PlayRenderSample(void* userdata, uint8_t* stream, int len)
{
	int i, j;
	static uint8_t playing = 1;
	static uint32_t frames = 0;
	struct voice_t **e = (struct voice_t **)userdata;
	len = len / sizeof(float);
	if (SDL_mutexP(mut) == -1)
	{
		fprintf(stderr, "Could not lock mutex.");
	}
	else
	{

		for (i = 0; i < len; i++)
		{
			float* ptr = (float *)stream + i;
			*ptr = 0;

			for (j = 35; j < 81; j++)
			{
				*ptr += PCMSample(&drum[j]);
			}

			for (j = 0; j < VOICE_COUNT; j++)
			{
				struct voice_t *v = *(e + j);
				*ptr += VoiceSample(v) / 10;	/* Todo change this arbitrary constant */
			}

			if (playing && frames == 0)
			{
				frames = PlaySMF();
				if (frames == 0)
				{
					playing = 0;
				}
			}
			else
			{
				--frames;
			}
		}
#ifdef DEBUG
		fwrite(stream, len, sizeof(float), export);
#endif
		SDL_mutexV(mut);
	}
}

int PlayInit()
{
	struct voice_t *v;
	int i, j;
#ifdef DEBUG
	export = fopen("text.raw", "w");
#endif

	/* First create empty pcm samples */
	for (i = 0; i < 128; i++)
	{
		char str[20];
		sprintf(str, "samples/drums/%d.wav", i);
		if (PCMCreate(str, &drum[i]) < 0)
		{
			fprintf(stderr, "Could not load sample %d\n", i);
			PCMCreateEmpty(&drum[i]);
		}
	}

	/* Setup a patch for this voice */
	for(i = 0; i < VOICE_COUNT; i++)
	{
		CreateVoice(&v);
		ensemble[i] = v;
		v->opFMIndex[0] = 5;
		v->opFMSource[0] = 1;
		v->env[0].r = 0.5;


		v->env[1].a = 0.001;
		v->env[1].d = 0.02;
		v->env[1].s = 0.7;
		v->op[1].mult = 2;
		v->env[1].r = 0.8;

		v->op[1].mult = 2;

		for (j = 0; j < VOICE_OPCOUNT - 1; j++)
		{
			;
		}

		vgate[i] = -1;
	}

	/* Desired audio settings */
	SDL_AudioSpec have = {
		.freq = sampleRate,
		.format = AUDIO_F32SYS,
		.channels = 1,
		.samples = 4096,
		.callback = PlayRenderSample,
		.userdata = (void *)&ensemble
	};
	SDL_AudioSpec want;

	if(SDL_OpenAudio(&have, &want)==-1)
	{
		fprintf(stderr, "SDL_OpenAudio: %s\n", SDL_GetError());
		exit(2);
	}

	mut = SDL_CreateMutex();
	/* Start playing sound */
	SDL_PauseAudio(0);

	return 0;
}

int PlayNoteOn(int note, int key)
{
	float freq;
	int i;
	uint8_t r = 0;
	if (key >= 0 && note >= 0)
	{
		for (i = 0; i < VOICE_COUNT; i++)
		{
			if (vgate[i] == -1)
			{
				vgate[i] = key;
				r = 1;
				break;
			}
		}

		if (!r)
		{
			return -1;
		}

		if (SDL_mutexP(mut) == -1)
		{
			fprintf(stderr, "Could not lock mutex.\r\n");
		}
		else
		{
			struct voice_t *v;
			freq = Note2Freq(note);
			v = *(ensemble + i);
			VoiceSetFreq(v, freq);
			VoiceGateOn(v);
			SDL_mutexV(mut);
		}
	}

	return key;
}

int PlayNoteOff(int note, int key)
{
	int i;
	uint8_t r = 0;
	if (key >= 0 && note >= 0)
	{
		for (i = 0; i < VOICE_COUNT; i++)
		{
			if (vgate[i] == key)
			{
				vgate[i] = -1;
				r = 1;
				break;
			}
		}

		if (!r)
		{
			return -1;
		}

		struct voice_t *v;
		v = *(ensemble + i);

		if (SDL_mutexP(mut) == -1)
		{
			fprintf(stderr, "Could not lock mutex.\r\n");
		}
		else
		{
			VoiceGateOff(v);
			SDL_mutexV(mut);
		}
	}
	return key;
}
