#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <pulse/simple.h>
#include "osc.h"

/* N seconds of sound at 44.1KHz. */
#define _SECONDS 10
#define _RATE 44100
#define _FRAMES _RATE * _SECONDS

#if __FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define FORMAT PA_SAMPLE_FLOAT32LE
#elif __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__
#define FORMAT PA_SAMPLE_FLOAT32BE
#endif
/**********************************/

int main ()
{
	uint64_t i;
	uint64_t frames = _FRAMES;
	osc_t osc[3];
	int sampleRate = _RATE;
	pa_simple *s;
	float *buff = (float *)malloc(frames *  sizeof(float));

	printf("Start!\r\n");

	osc[0].reg = 200/44100.0f;
	osc[0].count = 0;
	osc[0].shape = 3;
	osc[0].pw = 0.5;

	osc[1].reg = 50 /44100.0f;
	osc[1].count = 0;
	osc[1].shape = 2;

	pa_sample_spec ss;
	ss.format = FORMAT;
	ss.channels = 1;
	ss.rate = sampleRate;
	s = pa_simple_new(NULL,
					  "Oscillator Demo",
					  PA_STREAM_PLAYBACK,
					  NULL,
					  "Chiptune",
					  &ss,
					  NULL,
					  NULL,
					  NULL
					  );



	for (i = 0; i < frames; i++)
	{
		buff[i] = OscFrame(&osc[0]);
		osc[0].reg = (500 + 10 * (OscFrame(&osc[1]) + 1))/44100.0f;
	}

	pa_simple_write(s, buff, frames * sizeof(float), NULL);
	pa_simple_drain(s, NULL);

	pa_simple_free(s);

	printf("Done!\r\n");

	return 0;
}
