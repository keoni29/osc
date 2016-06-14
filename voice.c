#include "osc.h"
#include "filter.h"
#include "voice.h"
#include "config.h"

struct voice_t* CreateVoice()
{
	int i;
	struct voice_t *v = (struct voice_t*)malloc(sizeof(struct voice_t));

	for (i = 0; i < VOICE_OPCOUNT; i++)
	{
		v->op[i].count = 0;
		v->op[i].pw = 0.5;
		v->op[i].inc = 0;
		v->op[i].mult = 1;
		v->op[i].shape = 3;
		v->opFMSource[i] = -1;
		v->mix[i] = 0;
	}

	v->mix[0] = 1;

	//filter_t filter[2];
	return v;
}

/** Quick voice mixer test */
double VoiceSample(struct voice_t* v)
{
	double s[VOICE_OPCOUNT] = {0};
	double sum = 0;
	int i;

	for (i = 0; i < VOICE_OPCOUNT; i++)
	{
		s[i] = OscSample(&v->op[i]);
		sum += s[i] * v->mix[i];
	}

	for (i = 0; i < VOICE_OPCOUNT; i++)
	{
		int src = v->opFMSource[i];
		if (src >= 0)
		{
			double b = v->opFMIndex[i];
			v->op[i].inc = (v->freq + b * s[src]) / sampleRate;
		}
	}

	return sum / VOICE_OPCOUNT;
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
