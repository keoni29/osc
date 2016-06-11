#include <math.h>
#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filter.h"
#include "osc.h"
#include "config.h"
/**********************************/

struct chip_t
{
	uint8_t ch;
	struct osc_t *osc[2];
	struct filter_t *filter[2];
};

/** Callback for filling the audio stream buffer */
void RenderSample(void* userdata, uint8_t* stream, int len)
{
	int i, c;
	struct chip_t *chip = (struct chip_t *)userdata;
	len = len / 4;
	for (i = 0; i < len; i++)
	{
		float* ptr = (float *)stream + i;
		*ptr = 0;
		/* Simple filter example */
		/*for(c = 0; c < chip->ch; c++)
		{
			*ptr += ProcessFilter(OscSample(chip->osc[c]), chip->filter[c]) / chip->ch;
		}*/
		/* Simple frequency modulation example */
		OscSetFreq(chip->osc[0], 600 * OscSample(chip->osc[1]) + 800);
		*ptr += OscSample(chip->osc[0]);
	}
}


int main()
{
	struct osc_t osc0 = {	.count = 0,
							.shape = 3,
							.pw = 0.5 };
	struct osc_t osc1 = {	.count = 0,
							.shape = 3,
							.pw = 0.5 };
	struct filter_t f0, f1;
	struct chip_t chip;

	OscSetFreq(&osc0, 100);
	OscSetFreq(&osc1, 250);

	/* Create an all-pass filter */
	CreateFilter(&f0, 1000, FILTER_NONE);
	CreateFilter(&f1, 2000, FILTER_NONE);

	/* Pass pointers to components to chip structure */
	chip.ch = 2;
	chip.osc[0] = &osc0;
	chip.filter[0] = &f0;
	chip.osc[1] = &osc1;
	chip.filter[1] = &f1;

	/* Desired audio settings */
	SDL_AudioSpec have = {
		.freq = sampleRate,
		.format = AUDIO_F32SYS,
		.channels = 1,
		.samples = 4096,
		.callback = RenderSample,
		.userdata = (void *)&chip
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

	printf("Start... \r\n");

	/* Start playing sound */
	SDL_PauseAudio(0);

	/* Do some fancy frequency modulation */
	OscSetFreq(&osc1, 25);
	SDL_Delay(500);
	OscSetFreq(&osc1, 50);
	SDL_Delay(500);
	OscSetFreq(&osc1, 100);
	SDL_Delay(500);
	OscSetFreq(&osc1, 200);
	SDL_Delay(500);
	OscSetFreq(&osc1, 400);
	SDL_Delay(500);
	OscSetFreq(&osc1, 800);
	SDL_Delay(500);
	OscSetFreq(&osc1, 1600);
	SDL_Delay(500);
	OscSetFreq(&osc1, 3200);
	SDL_Delay(500);

	/* Stop playing sound */
	SDL_CloseAudio();

	printf("Stop... \r\n");

	return 0;
}
