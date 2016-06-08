#include <math.h>
#include "osc.h"

/* OscFrame = Generate new sample (frame)
 *
 * osc = oscillator
 * returns a single sample (frame)
 *
 * Samples are in the range of -1 to 1. */
float OscFrame(osc_t *osc)
{
	double oscVal = 0;
	switch (osc->shape)
	{
	case 1:
		/* Triangle waveform. */
		oscVal = osc->count;
		if (oscVal >= 0.5)
		{
			/* Mirror second half of saw to get triangle. */
			oscVal = 1 - oscVal;
		}
		else if (oscVal <= -0.5)
		{
			oscVal = -1 - oscVal;
		}
		/* Triangle is half the resolution of sawtooth. */
		oscVal = oscVal * 2;
		break;
	case 2:
		/* Pulse waveform */
		oscVal = -1;
		if (osc->count > osc->pw)
		{
			oscVal = 1;
		}
		break;
	case 3:
		/* Sine waveform */
		oscVal = sin(osc->count * M_PI);
		break;
	case 0:
	default:
		/* Sawtooth waveform (default) */
		oscVal = osc->count;
		break;
	}

	/* Update phase. */
	osc->count += osc->reg * 2;
	while(osc->count >= 1)
	{
		osc->count -= 2;
	}

	return oscVal;
}

