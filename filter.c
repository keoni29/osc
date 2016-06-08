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
