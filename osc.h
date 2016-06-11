/*
 * osc.h
 *
 *  Created on: Jun 4, 2016
 *      Author: koen
 */

#ifndef OSC_H_
#define OSC_H_

#include <stdint.h>
#include <stdlib.h>

struct osc_t
{
	double count;
	double pw;
	double reg;
	uint8_t gate;
	uint8_t shape;
};

float OscSample(struct osc_t *osc);

#endif /* OSC_H_ */
