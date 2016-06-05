#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <pulse/simple.h>
#include <math.h>
#include "osc.h"
#include "filter.h"

/* N seconds of sound at 44.1KHz. */
#define _SECONDS 4
#define _RATE 192000
//44100
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
	float rc[1];
	int sampleRate = _RATE;
	pa_simple *s;
	float *buff = (float *)malloc(frames *  sizeof(float));

	printf("Start!\r\n");

	osc[0].reg = 200.0f/sampleRate;
	osc[0].count = 0;
	osc[0].shape = 0;
	osc[0].pw = 0.5;

	osc[1].reg = 5.0f /sampleRate;
	osc[1].count = 0;
	osc[1].shape = 1;

	rc[0] = F2RC(2, sampleRate);

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


	float decay = 1.0f/frames;

	for (i = 0; i < frames; i++)
	{
		static double amp = 1;
		amp -= decay;
		buff[i] = amp * LPF(OscFrame(&osc[0]), rc[0]);
		osc[0].reg = (500 + 10 * (OscFrame(&osc[1]) + 1))/sampleRate;
		rc[0] += F2RC(0.02, sampleRate);
	}

	printf("Begin playback!\r\n");
	pa_simple_write(s, buff, frames * sizeof(float), NULL);
	pa_simple_drain(s, NULL);

	pa_simple_free(s);

	printf("Done!\r\n");

	return 0;
}
