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

SDL_mutex *mut;

const uint8_t song[] = {
		60,62,64,65,67,65,64,62
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
		VoiceSetFreq(v, Note2Freq(song[p]));

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
		}
		SDL_mutexV(mut);
	}
}


int main()
{
	struct voice_t *v = NULL;

	/* Setup a patch for this voice */
	v = CreateVoice();
	v->op[1].mult = 3;
	v->op[1].shape = 0;
	v->opFMSource[0] = 1;
	v->opFMIndex[0] = 500;

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
