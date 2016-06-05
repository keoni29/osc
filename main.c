#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "osc.h"

/* N seconds of sound at 44.1KHz. */
#define _SECONDS 1
#define _FRAMES 44100 * _SECONDS

int main ()
{
	uint64_t frames = _FRAMES;
	float *buff = (float *)malloc(frames *  sizeof(float));

	OscInit();
	OscGenerateFrames(buff, frames);
	OscPlay(buff, frames);
	OscFree();

	return 0;
}
