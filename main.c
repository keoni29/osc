#include <math.h>
#include <pulse/simple.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filter.h"
#include "osc.h"
#include "config.h"
/**********************************/

int main ()
{
	float *buff;
	float seconds = 1;
	pa_sample_spec ss;
	pa_simple *s = NULL;
	struct filter_t f;
	struct osc_t osc;
	uint64_t frames, i;

	ss.format = PA_SAMPLE_FLOAT32NE;
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
	CreateFilter(&f, 1000);

	printf("Start!\r\n");

	osc.reg = 400.0f/sampleRate;
	osc.count = 0;
	osc.shape = 0;
	osc.pw = 0.5;

	frames = sampleRate * seconds;
	buff = (float *)malloc(frames *  sizeof(float));
	for (i = 0; i < frames; i++)
	{
		buff[i] = ProcessLPF(OscSample(&osc), &f);
	}

	printf("Begin playback!\r\n");

	pa_simple_write(s, buff, frames * sizeof(float), NULL);
	pa_simple_drain(s, NULL);

	pa_simple_free(s);

	printf("Done!\r\n");

	return 0;
}
