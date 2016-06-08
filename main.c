#include <math.h>
#include <pulse/simple.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filter.h"
#include "osc.h"

#if __FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define FORMAT PA_SAMPLE_FLOAT32LE
#elif __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__
#define FORMAT PA_SAMPLE_FLOAT32BE
#endif
/**********************************/

int main ()
{
	float *buff;
	float seconds = 4;
	int sampleRate = 192000;
	osc_t osc, lfo;
	pa_simple *s;
	uint64_t frames, i;

	frames = sampleRate * seconds;

	buff = (float *)malloc(frames *  sizeof(float));

	printf("Start!\r\n");

	osc.reg = 200.0f/sampleRate;
	osc.count = 0;
	osc.shape = 0;
	osc.pw = 0.5;

	lfo.reg = 5.0f /sampleRate;
	lfo.count = 0;
	lfo.shape = 1;

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
		static double amp = 1;
		amp -= 1.0f/frames;
		buff[i] = amp * LPF(OscFrame(&osc), F2RC(200, sampleRate));
		osc.reg = (500 + 10 * (OscFrame(&lfo) + 1)) / sampleRate;
	}

	printf("Begin playback!\r\n");

	pa_simple_write(s, buff, frames * sizeof(float), NULL);
	pa_simple_drain(s, NULL);

	pa_simple_free(s);

	printf("Done!\r\n");

	return 0;
}
