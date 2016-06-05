#include <math.h>
#include "filter.h"

float LPF(float x, float rc)
{
	static double y, yn = 0, xn = 0;
	y = yn + rc * (xn - yn);
	xn = x;
	yn = y;
	return y;
}

float F2RC(float fc, float fs)
{
	return 2 * M_PI * fc / fs;
}
