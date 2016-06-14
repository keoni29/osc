#include <math.h>
#include "notes.h"

double Note2Freq(uint8_t note)
{
	return pow(2, (float)(note - 69) / 12) * 440;
}
