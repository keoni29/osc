#include <math.h>
#include "filter.h"
#include "voice.h"
#include "config.h"

static void DefaultVoice(struct voice_t *v);
static double UpdateEnv(struct env_t *env, uint8_t gate);
static double OscSample(struct osc_t *osc);

int CreateVoice(struct voice_t** v)
{
	*v = NULL;
	*v = (struct voice_t*)malloc(sizeof(struct voice_t));

	if (*v == NULL)
	{
		return 0;
	}

	DefaultVoice(*v);

	//filter_t filter[2];
	return 1;
}

static void DefaultVoice(struct voice_t *v)
{
	int i;
	for (i = 0; i < VOICE_OPCOUNT; i++)
	{
		v->op[i].count = 0;
		v->op[i].pw = 0.5;
		v->op[i].inc = 0;
		v->op[i].mult = 1;
		v->op[i].shape = 2;
		v->opFMSource[i] = -1;
		v->opFMIndex[i] = 0;

		v->env[i].amp = 0;
		v->env[i].a = 0.001;
		v->env[i].d = 0.5;
		v->env[i].s = 0.5;
		v->env[i].r = 0.001;
		v->env[i].gate = 0;
		v->env[i].dir = 0;

		v->mix[i] = 0;
	}

	v->mix[0] = 1;
}

/** Quick voice mixer test */
double VoiceSample(struct voice_t* v)
{
	double s[VOICE_OPCOUNT] = {0};
	double sum = 0;
	int i;

	for (i = 0; i < VOICE_OPCOUNT; i++)
	{
		double amp = UpdateEnv(&v->env[i], v->gate);

		if (amp)
		{
			switch(v->op[i].shape)
			{
			case 1:
				s[i] = Triangle(OscSample(&v->op[i])) * amp;
				break;
			case 2:
				s[i] = Saw(OscSample(&v->op[i])) * amp;
				break;
			case 3:
				s[i] = Pulse(OscSample(&v->op[i]), v->op[i].pw) * amp;
				break;
			case 0:
			default:
				s[i] = Sine(OscSample(&v->op[i])) * amp;
				break;
			}

			if (v->mix[i])
			{
				sum += s[i] * v->mix[i];
			}
		}
	}

	for (i = 0; i < VOICE_OPCOUNT; i++)
	{
		int src = v->opFMSource[i];
		if (src >= 0)
		{
			double b = v->opFMIndex[i] * v->freq;
			v->op[i].inc = (v->freq + b * s[src]) / sampleRate;
		}
	}

	return sum / VOICE_OPCOUNT;
}

static double UpdateEnv(struct env_t *env, uint8_t gate)
{
	if (!env->gate && gate)
	{
		/* Reset envelope and start attack */
		env->amp = 0;
		env->dir = 1;
	}
	else if (env->gate && !gate)
	{
		/* Start release */
		env->dir = 0;
	}

	if (gate)
	{
		if (env->dir)
		{
			env->amp += 1/(sampleRate * env->a);
			if (env->amp > 1)
			{
				env->amp = 1;
				/* Start decay */
				env->dir = 0;
			}
		}
		else
		{
			if (env->amp > env->s)
			{
				env->amp -= 1/(sampleRate * env->d);
			}
		}
	}
	else
	{
		env->amp -= 1/(sampleRate * env->r);
		if (env->amp < 0)
		{
			env->amp = 0;
		}
	}

	env->gate = gate;

	return env->amp;
}

void VoiceSetFreq(struct voice_t* v, double f)
{
	int i;
	v->freq = f;
	for (i = 0; i < VOICE_OPCOUNT; i++)
	{
		v->op[i].inc = f / sampleRate;
	}
}

/** OscFrame = Generate new sample (frame)
 *
 * osc = oscillator
 * returns a single sample
 *
 * Sample values are in the range of -1 to 1.
 */
static double OscSample(struct osc_t *osc)
{
	/* Update phase. */
	osc->count = fmod(osc->count + osc->inc * osc->mult, 1);

	return osc->count;
}

/* Pulse waveform */
double Pulse(double phase, double pw)
{
	double result = -1;
	phase = fmod(phase, 1);
	if (phase > pw)
	{
		result = 1;
	}

	return result;
}

double Triangle(double phase)
{
	double result;
	phase = fmod(phase, 1);
	result = phase;
	if (phase >= 0.5)
	{
		/* Mirror second half of saw to get triangle. */
		result = 1 - phase;
	}
	/* Triangle is half the resolution of sawtooth. */
	return result * 4 - 1;
}

double Sine(double phase)
{
	phase = fmod(phase, 1);
	return sin(phase * 2 * M_PI);
}

double Saw(double phase)
{
	phase = fmod(phase, 1);
	return phase * 2 - 1;
}

/** Set gate bit and reset operator phase */
void VoiceGateOn(struct voice_t *v)
{
	int i;
	v->gate = 1;
	for(i = 0; i < VOICE_OPCOUNT; i++)
	{
		v->op[i].count = 0;
	}
}

/** Clear gate bit */
void VoiceGateOff(struct voice_t *v)
{
	v->gate = 0;
}
