#include <SDL2/SDL.h>
#include "pcm.h"
#include "config.h"

float PCMSample(struct pcm_t *p)
{
	float result = 0;
	if (p->playing && p->count < p->length)
	{
		++p->count;
		result = (float)p->data[p->count] / 32768;
	}
	else
	{
		p->playing = 0;
	}
	return result;
}

/** Create new pcm object and load pcm data from file. */
int PCMCreate(const char *fname, struct pcm_t *p)
{
	SDL_AudioSpec wav_spec = {
			.freq = sampleRate,
			.format = AUDIO_S16SYS,
			.channels = 1,
			.samples = 4096,
			.callback = NULL,
			.userdata = NULL
		};
	p->count = 0;
	if (SDL_LoadWAV(fname, &wav_spec, (uint8_t **)&p->data, &p->length) == NULL) {
		return -1;
	}
	p->length = p->length / sizeof(p->data[0]);
	return 0;
}

void PCMCreateEmpty(struct pcm_t *p)
{
	p->length = 0;
	p->count = 0;
}

void PCMFree(struct pcm_t *p)
{
	SDL_FreeWAV((uint8_t *)p->data);
}

void PCMStart(struct pcm_t *p)
{
	p->count = 0;
	p->playing = 1;
}

void PCMStop(struct pcm_t *p)
{
	p->playing = 0;
}

int PCMIsPlaying(struct pcm_t *p)
{
	return p->playing;
}
