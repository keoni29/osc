#include <math.h>
#include "filter.h"
#include "config.h"

static double ProcessLPF(double x, struct filter_t *lpf);
static double ProcessHPF(double x, struct filter_t *hpf);

void CreateFilter(struct filter_t *f, double fc, enum filter_type type)
{
	f->yn = 0;
	f->xn = 0;
	f->k = 2 * M_PI * fc / sampleRate;
	f->type = type;
}

double ProcessFilter(double x, struct filter_t *f)
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

static double ProcessLPF(double x, struct filter_t *lpf)
{
	double y;
	y = lpf->yn + lpf->k * (lpf->xn - lpf->yn);
	lpf->xn = x;
	lpf->yn = y;
	return y;
}

static double ProcessHPF(double x, struct filter_t *hpf)
{
	double y;
	y = x - hpf->k * hpf->yn;
	hpf->yn = y;
	return y;
}
