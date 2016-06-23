#include <math.h>
#include <SDL2/SDL.h>
#include "SDL2/SDL_thread.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "voice.h"
#include "notes.h"
#include "play.h"
#include "smf.h"
#include "pcm.h"
/**********************************/

static int Code2Note(SDL_Scancode s);

int main(int argc, char **argv)
{
	/* Start SDL with audio support */
	if(SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)==-1) {
	    printf("SDL_Init: %s\n", SDL_GetError());
	    exit(1);
	}

	char patch[100];
	SDL_Renderer *renderer = NULL;
	SDL_Window *window = NULL;
	window = SDL_CreateWindow("FM synth demo", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 240, 0);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	sprintf(patch, "%s", "grand");
	if (argc >= 2)
	{
		sprintf(patch, "%s", argv[1]);
	}

	if (argc >= 3)
	{
		if (SMF_Load(argv[2]) < 0)
		{
			fprintf(stderr, "Error loading midi file.\r\n");
			exit(1);
		}
	}

	PlayInit();
	PlayLoadConfig("voice.cfg", patch);

	printf("Start... \r\n");

	SDL_Event e;
	uint8_t running = 1;
	while (running)
	{
		int note;
		static int oct = 5;
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
					PlayNoteOn(note + oct * 12, note);
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
					else if (e.key.keysym.scancode == SDL_SCANCODE_1)
					{
						PlayLoadConfig("voice.cfg", "grand");
					}
					else if (e.key.keysym.scancode == SDL_SCANCODE_2)
					{
						PlayLoadConfig("voice.cfg", "organ1");
					}
					else if (e.key.keysym.scancode == SDL_SCANCODE_3)
					{
						PlayLoadConfig("voice.cfg", "bell1");
					}
					else if (e.key.keysym.scancode == SDL_SCANCODE_4)
					{
						PlayLoadConfig("voice.cfg", "bell2");
					}
				}
			}
			else if (e.type == SDL_KEYUP && e.key.repeat == 0)
			{
				note = Code2Note(e.key.keysym.scancode);
				PlayNoteOff(note + oct * 12, note);
			}

		}

		/*SDL_SetRenderDrawColor(renderer, 0, 0, 0, 1);
		SDL_RenderClear(renderer);*/
		SDL_RenderPresent(renderer);
	}

	/* Stop playing sound */
	SDL_CloseAudio();
	printf("Stop... \r\n");

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	exit(0);
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
