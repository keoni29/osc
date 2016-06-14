#include <math.h>
#include <SDL2/SDL.h>
#include "SDL2/SDL_thread.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filter.h"
#include "osc.h"
#include "config.h"
#include "voice.h"
#include "notes.h"
/**********************************/

static SDL_mutex *mut;
static int gate = 0;
static double env, attack = 0.1, release = 0.5;

const uint8_t song[] = {
		60,0,62,0,64,0,65,0,67,0,65,0,64,0,62,0
};

uint32_t RasterInterrupt(uint32_t interval, void *param)
{
	struct voice_t *v = (struct voice_t *)param;
	static int p = 0;
	if (SDL_mutexP(mut) == -1)
	{
		printf("Could not lock mutex.");
	}
	else
	{
		printf("Frame %d\r\n", p);
		if (song[p])
		{
			VoiceSetFreq(v, Note2Freq(song[p]));
			gate = 1;
		}
		else
		{
			gate = 0;
		}

		p ++;
		if (p == sizeof(song) / sizeof(song[0]))
		{
			p = 0;
		}

		SDL_mutexV(mut);
	}
	return interval;
}

/** Callback for filling the audio stream buffer */
void RenderSample(void* userdata, uint8_t* stream, int len)
{
	int i;
	struct voice_t *v = (struct voice_t *)userdata;
	len = len / sizeof(float);
	if (SDL_mutexP(mut) == -1)
	{
		printf("Could not lock mutex.");
	}
	else
	{
		for (i = 0; i < len; i++)
		{
			float* ptr = (float *)stream + i;
			*ptr = VoiceSample(v);

			v->opFMIndex[0] = env * 1000;

			if (gate)
			{
				env += 1/(sampleRate * attack);
				if (env > 1)
				{
					env = 1;
				}
			}
			else
			{
				env -= 1/(sampleRate * release);
				if (env < 0)
				{
					env = 0;
				}
			}
		}
		SDL_mutexV(mut);
	}
}


int main()
{
	struct voice_t *v = NULL;

	/* Setup a patch for this voice */
	v = CreateVoice();
	v->op[0].shape = 3;
	v->op[0].pw = 0.125;
	v->mix[0] = 0.2;
	v->op[1].mult = 0.2;
	v->op[1].shape = 3;
	v->opFMSource[0] = 1;


	/* Todo: grouping voices to create polyphonic synth */


	/* Desired audio settings */
	SDL_AudioSpec have = {
		.freq = sampleRate,
		.format = AUDIO_F32SYS,
		.channels = 1,
		.samples = 4096,
		.callback = RenderSample,
		.userdata = (void *)v
	};
	SDL_AudioSpec want;

	/* Start SDL with audio support */
	if(SDL_Init(SDL_INIT_AUDIO)==-1) {
	    printf("SDL_Init: %s\n", SDL_GetError());
	    exit(1);
	}

	if(SDL_OpenAudio(&have, &want)==-1) {
	    printf("SDL_OpenAudio: %s\n", SDL_GetError());
	    exit(2);
	}

	mut = SDL_CreateMutex();
	SDL_TimerID tid = SDL_AddTimer(500, RasterInterrupt, v);

	printf("Start... \r\n");

	/* Start playing sound */
	SDL_PauseAudio(0);

	//SDL_Delay(2000);
	getchar();
	/* Stop playing sound */
	SDL_CloseAudio();
	SDL_RemoveTimer(tid);
	printf("Stop... \r\n");


	return 0;
}
