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
	double reg;		/**< Phase accumulator increment value (calculated) */
	double freq;	/**< Oscillator frequency in Hz */
	uint8_t gate;	/**< Oscillator gate bit (not implemented.) 0 halts the oscillator, 1 resumes it*/
	uint8_t shape;	/**< Wave shape of the oscillator */
};

void OscSetFreq(struct osc_t *osc, double freq);
float OscSample(struct osc_t *osc);

#endif /* OSC_H_ */
