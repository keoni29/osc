#include <math.h>
#include <SDL2/SDL.h>
#include "SDL2/SDL_thread.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filter.h"
#include "config.h"
#include "voice.h"
#include "notes.h"
/**********************************/

#define VOICE_COUNT 17

static SDL_mutex *mut;

static int Code2Note(SDL_Scancode s);

/*uint32_t RasterInterrupt(uint32_t interval, void *param)
{
	struct voice_t *v = (struct voice_t *)param;
	if (SDL_mutexP(mut) == -1)
	{
		fprintf(stderr, "Could not lock mutex.");
	}
	else
	{
		...
		SDL_mutexV(mut);
	}
	return interval;
}*/

/** Callback for filling the audio stream buffer */
void RenderSample(void* userdata, uint8_t* stream, int len)
{
	int i, j;
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
			for (j = 0; j < VOICE_COUNT; j++)
			{
				struct voice_t *v = *(e + j);
				*ptr += VoiceSample(v) / 4;
			}
		}
		SDL_mutexV(mut);
	}
}


int main(int argc, char **argv)
{
	struct voice_t *v;
	struct voice_t *ensemble[VOICE_COUNT];
	int i, j;
	/* Setup a patch for this voice */
	for(i = 0; i < VOICE_COUNT; i++)
	{
		CreateVoice(&v);
		ensemble[i] = v;
		v->opFMIndex[0] = 5;
		v->opFMSource[0] = 1;
		v->env[0].r = 0.5;

		v->env[1].a = 1;
		v->op[1].mult = 0.25;
		v->env[1].r = 0.8;

		v->op[2].mult = 2;
		v->mix[2] = 1;
		for (j = 0; j < VOICE_OPCOUNT - 1; j++)
		{
			;
		}
	}

	/* Desired audio settings */
	SDL_AudioSpec have = {
		.freq = sampleRate,
		.format = AUDIO_F32SYS,
		.channels = 1,
		.samples = 4096,
		.callback = RenderSample,
		.userdata = (void *)&ensemble
	};
	SDL_AudioSpec want;

	/* Start SDL with audio support */
	if(SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)==-1) {
	    printf("SDL_Init: %s\n", SDL_GetError());
	    exit(1);
	}

	if(SDL_OpenAudio(&have, &want)==-1) {
	    printf("SDL_OpenAudio: %s\n", SDL_GetError());
	    exit(2);
	}

	SDL_Renderer *renderer = NULL;
	SDL_Window *window = NULL;
	window = SDL_CreateWindow("FM synth demo", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 240, 0);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);


	mut = SDL_CreateMutex();
	//SDL_TimerID tid = SDL_AddTimer(500, RasterInterrupt, v);

	printf("Start... \r\n");

	/* Start playing sound */
	SDL_PauseAudio(0);

	SDL_Event e;
	uint8_t running = 1;
	while (running)
	{
		int note;
		static int oct = 5;
		static double freq = 0;
		while(SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT)
			{
				running = 0;
				break;
			}

			if (e.type == SDL_KEYDOWN && e.key.repeat == 0)
			{
				note = Code2Note(e.key.keysym.scancode);
				if (note >= 0)
				{
					if (SDL_mutexP(mut) == -1)
					{
						fprintf(stderr, "Could not lock mutex.\r\n");
					}
					else
					{
						freq = Note2Freq(note + 12*oct);
						v = *(ensemble + note);
						VoiceSetFreq(v, freq);
						VoiceGateOn(v);
						SDL_mutexV(mut);
					}
				}
				else
				{
					if (e.key.keysym.scancode == SDL_SCANCODE_EQUALS)
					{
						oct +=1;
					}
					else if (e.key.keysym.scancode == SDL_SCANCODE_MINUS)
					{
						oct -=1;
					}
				}
			}
			else if (e.type == SDL_KEYUP && e.key.repeat == 0)
			{
				if (SDL_mutexP(mut) == -1)
				{
					fprintf(stderr, "Could not lock mutex.\r\n");
				}
				else
				{
					note = Code2Note(e.key.keysym.scancode);
					if (note >= 0)
					{
						v = *(ensemble + note);
						VoiceGateOff(v);
					}
					SDL_mutexV(mut);
				}
			}

		}

		/*SDL_SetRenderDrawColor(renderer, 0, 0, 0, 1);
		SDL_RenderClear(renderer);*/
		SDL_RenderPresent(renderer);
	}

	/* Stop playing sound */
	SDL_CloseAudio();
	//SDL_RemoveTimer(tid);
	printf("Stop... \r\n");

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

int Code2Note(SDL_Scancode s)
{
	const uint16_t conv[] = {
			0x1d, 0x16, 0x1b, 0x07, 0x06, 0x19, 0x0a, 0x05,
			0x0b, 0x11, 0x0d, 0x10, 0x36, 0x0f, 0x37, 0x33,
			0x38
	};
	int i, e = (sizeof(conv) / sizeof(conv[0]));

	for(i = 0; i < e; i++)
	{
		if (s == conv[i])
		{
			return i;
		}
	}

	return -1;
}
