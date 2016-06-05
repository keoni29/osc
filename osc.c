#include <pulse/simple.h>
#include "osc.h"

typedef struct
{
	uint16_t reg;
	uint32_t count;
	uint16_t pw;
	uint8_t shape;
	uint8_t gate;
} osc_t;

#define PAL50

/* Estimated clock multipliers for 44.1KHz sampling rate. */
#ifdef NTSC60
#define CLK_MUL 23
#else
/* Default clock multiplier */
#define CLK_MUL 22
#endif

#define N_OSC 3
static const uint8_t nOsc = N_OSC;
static osc_t osc[N_OSC];
static float ampMaster = 1.00f / (N_OSC * 128);

#if __FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define FORMAT PA_SAMPLE_FLOAT32LE
#elif __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__
#define FORMAT PA_SAMPLE_FLOAT32BE
#endif

static pa_simple *s;

void OscInit()
{
	pa_sample_spec ss;
	ss.format = FORMAT;
	ss.channels = 1;
	ss.rate = 44100;
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


	SetOscReg(0, 4000, 0, 0);
	SetOscReg(1, 3214, 1, 0);
	SetOscReg(2, 6000, 1, 32000);
}

void OscFree()
{
	pa_simple_free(s);
}

void OscGenerateFrames(float *buff, uint64_t frames)
{
	uint64_t i;
	for(i = 0; i < frames; i++)
	{
		buff[i] = RenderFrame();
	}
}

void OscPlay(float *buff, uint64_t frames)
{
	pa_simple_write(s, buff, frames * sizeof(float), NULL);
	pa_simple_drain(s, NULL);
}

uint8_t UpdateOsc(uint8_t id)
{
	uint8_t oscVal = 0;
	if (id < nOsc)
	{
		osc[id].count += osc[id].reg * CLK_MUL;
		uint32_t oscPol = osc[id].count & (1 << 23);

		switch (osc[id].shape)
		{
		case 1:
			/* Triangle waveform. */
			oscVal = (uint8_t)(osc[id].count >> 16);
			if (oscPol)
			{
				/* Mirror second half of saw to get triangle. */
				oscVal = -oscVal - 1;
			}
			/* Triangle is half the resolution of sawtooth. */
			oscVal = oscVal << 1;
			break;
		case 2:
			/* Pulse waveform */
			if ((uint16_t)(osc[id].count >> 8) > osc[id].pw)
			{
				oscVal = 255;
			}
			break;
		case 3:
			/* Noise waveform */
			/* Not implemented yet */
			break;
		case 0:
		default:
			/* Sawtooth waveform (default) */
			oscVal = (uint8_t)(osc[id].count >> 16);
			break;
		}
	}

	return oscVal;
}

uint8_t SetOscReg(uint8_t id, uint16_t reg, uint8_t shape, uint16_t pw)
{
	if (id < nOsc)
	{
		osc[id].reg = reg;
		osc[id].shape = shape;
		osc[id].pw = pw;
		return 1;
	}
	else
	{
		return 0;
	}
}

uint8_t IncOscPitch(uint8_t id)
{
	if (id < nOsc)
	{
		osc[id].reg += 10;
		return 1;
	}
	else
	{
		return 0;
	}
}

float RenderFrame()
{
	int i;
	float result = 0;
	for(i = 0; i < nOsc; i++)
	{
		result += (float)UpdateOsc(i);
	}
	result = result * ampMaster - 1;
	return result;
}


