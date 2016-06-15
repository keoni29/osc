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

static SDL_mutex *mut;

const uint8_t song[] = {
		60,0,62,0,64,0,65,0,67,0,65,0,64,0,62,0
};

static int Code2Note(SDL_Scancode s);

uint32_t RasterInterrupt(uint32_t interval, void *param)
{
	struct voice_t *v = (struct voice_t *)param;
	static int p = 0;
	if (SDL_mutexP(mut) == -1)
	{
		fprintf(stderr, "Could not lock mutex.");
	}
	else
	{
		if (song[p])
		{
			VoiceSetFreq(v, Note2Freq(song[p]));
			v->gate = 1;
		}
		else
		{
			v->gate = 0;
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
		fprintf(stderr, "Could not lock mutex.");
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


int main(int argc, char **argv)
{
	struct voice_t *v = NULL;
	/* Setup a patch for this voice */
	CreateVoice(&v);
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

	v->opFMSource[0] = 1;
	v->op[0].shape = 0;
	v->op[0].pw = 0.25;
	v->mix[0] = 0.2;
	v->op[1].mult = 0.25;
	v->op[1].shape = 0;
	v->opFMIndex[0] = 1;
	v->env[1].r = 1.5;
	v->env[1].a = 0.3;
	v->env[0].r = 2;

	if (argc >= 2)
	{
		v->op[1].mult = strtod(argv[1], NULL);
	}

	if (argc >= 3)
	{
		v->opFMIndex[0] = strtod(argv[2], NULL);
	}

	if (argc >= 4)
	{
		v->opFMSource[1] = 2;
		v->opFMIndex[1] = strtod(argv[3], NULL);
	}

	if (argc >= 5)
	{
		v->op[2].mult = strtod(argv[4], NULL);;
	}

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
		const int oct = 5;
		static double freq = 0;
		while(SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT)
			{
				running = 0;
				break;
			}

			if (e.type == SDL_KEYDOWN)
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
						VoiceSetFreq(v, freq);
						v->gate = 1;
						SDL_mutexV(mut);
					}
				}
			}
			else if (e.type == SDL_KEYUP)
			{
				if (SDL_mutexP(mut) == -1)
				{
					fprintf(stderr, "Could not lock mutex.\r\n");
				}
				else
				{
					v->gate = 0;
					SDL_mutexV(mut);
				}
			}

		}

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 1);
		SDL_RenderClear(renderer);

		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 1);
		int i;
		for (i = 0; i < 640; i++)
		{
			float t = i * freq / 3000;
			float amp = 0;
			if (SDL_mutexP(mut) == -1)
			{
				fprintf(stderr, "Could not lock mutex.\r\n");
			}
			else
			{
				amp = v->env[0].amp;
				SDL_mutexV(mut);
			}
			SDL_RenderDrawPoint(renderer, i, 120 + amp * 50 * Sine(t / 640.0 + 2 * Sine(t / 160.0)));
		}

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
