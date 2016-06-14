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
	double count;	/**< Phase accumulator (calculated)*/
	double pw;		/**< Pulsewidth (only for pwm mode) */
	double inc;		/**< Phase accumulator increment value */
	double mult;	/**< Frequency multiplier */
	uint8_t shape;	/**< Wave shape of the oscillator */
};

void OscSetFreq(struct osc_t *osc, double freq);
float OscSample(struct osc_t *osc);

#endif /* OSC_H_ */
