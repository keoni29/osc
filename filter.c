#include <math.h>
#include "filter.h"
#include "config.h"

static float ProcessLPF(float x, struct filter_t *lpf);
static float ProcessHPF(float x, struct filter_t *hpf);

void CreateFilter(struct filter_t *f, float fc, enum filter_type type)
{
	f->yn = 0;
	f->xn = 0;
	f->k = 2 * M_PI * fc / sampleRate;
	f->type = type;
}

float ProcessFilter(float x, struct filter_t *f)
{
	switch (f->type)
	{
	case FILTER_LPF:
		return ProcessLPF(x, f);
	case FILTER_HPF:
		return ProcessHPF(x, f);
	case FILTER_NONE:
	default:
		return x;
	}
}

static float ProcessLPF(float x, struct filter_t *lpf)
{
	float y;
	y = lpf->yn + lpf->k * (lpf->xn - lpf->yn);
	lpf->xn = x;
	lpf->yn = y;
	return y;
}

static float ProcessHPF(float x, struct filter_t *hpf)
{
	float y;
	y = x - hpf->k * hpf->yn;
	hpf->yn = y;
	return y;
}
