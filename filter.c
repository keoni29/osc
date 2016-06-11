#include <math.h>
#include "filter.h"
#include "config.h"

/** Macros and defines **/
/** Calculate filter time constant */
/************************/
void CreateFilter(struct filter_t *f, double fc)
{
	f->yn = 0;
	f->xn = 0;
	f->k = 2 * M_PI * fc / sampleRate;
}

double ProcessLPF(double x, struct filter_t *lpf)
{
	double y;
	y = lpf->yn + lpf->k * (lpf->xn - lpf->yn);
	lpf->xn = x;
	lpf->yn = y;
	return y;
}

double ProcessHPF(double x, struct filter_t *hpf)
{
	double y;
	y = x - hpf->k * hpf->yn;
	hpf->yn = y;
	return y;
}
