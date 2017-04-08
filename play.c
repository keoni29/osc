#include <SDL2/SDL.h>
#include "SDL2/SDL_thread.h"
#include "config.h"
#include "play.h"
#include "smf.h"
#include "channel.h"

/* Defines and macros */
#ifdef DEBUG
#define RECORD
#endif

#ifdef RECORD
FILE *export;
#endif
/**********************/

SDL_mutex *mut;

#define MIDI_CHANNELS 16

struct channel_t channel[MIDI_CHANNELS];

/** Play SMF frame-by-frame.
 * 	Returns amount of frames before next event. */
uint32_t PlaySMF()
{
	int result;
	uint32_t frames;
	uint32_t interval;
	struct SMF_event e;
	while (1)
	{
		result = SMF_PollEvent(&e, &interval);

		if (result == SMF_EVENT_READY)
		{
			ChannelEvent(&channel[e.channel], &e);
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

			for (j = 0; j < MIDI_CHANNELS; j++)
			{
				*ptr += ChannelSample(&channel[j]);
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
		SDL_mutexV(mut);
#ifdef DEBUG
		fwrite(stream, len, sizeof(float), export);
#endif
	}
}

int PlayInit()
{
	int i;
	/* Desired audio settings */
	SDL_AudioSpec have = {
		.freq = sampleRate,
		.format = AUDIO_F32SYS,
		.channels = 1,
		.samples = 4096,
		.callback = PlayRenderSample,
		.userdata = (void *)&channel
	};
	SDL_AudioSpec want;

#ifdef DEBUG
	export = fopen("text.raw", "w");
#endif

	mut = SDL_CreateMutex();

	/* Create all midi channels. */
	for (i = 0; i < MIDI_CHANNELS; i++)
	{
		if (CreateChannel(&channel[i], i) == -1)
		{
			fprintf(stderr, "Could not create channel %02d\n", i);
		}
		else
		{
			/* Todo make patch module & make voices share patches & make patch struct */
			ChannelLoadPatch(&channel[i], "voice.cfg", "organ1");
		}
	}

	if(SDL_OpenAudio(&have, &want) == -1)
	{
		fprintf(stderr, "SDL_OpenAudio: %s\n", SDL_GetError());
		exit(2);
	}

	/* Start playing sound */
	SDL_PauseAudio(0);

	return 0;
}
