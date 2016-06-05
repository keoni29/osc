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

typedef struct
{
	float reg;
	float count;
	float pw;
	uint8_t shape;
	uint8_t gate;
} osc_t;

float OscFrame(osc_t *osc);

#endif /* OSC_H_ */
