/*
 * voice.h
 *
 *  Created on: Jun 14, 2016
 *      Author: koen
 */

#ifndef VOICE_H_
#define VOICE_H_

#include <stdint.h>
#include <stdlib.h>
#include "filter.h"

/** The amount of operators per voice */
#define VOICE_OPCOUNT 2

struct osc_t
{
	double count;	/**< Phase accumulator (calculated)*/
	double pw;		/**< Pulsewidth (only for pwm mode) */
	double inc;		/**< Phase accumulator increment value */
	double mult;	/**< Frequency multiplier */
	uint8_t shape;	/**< Wave shape of the oscillator */
};

struct env_t
{
	uint8_t gate, dir;
	double a, d, s, r, amp;
};

struct voice_t
{
	double freq;						/**< Voice base frequency in Hz */
	uint8_t gate; 						/**< Start playing when 1, stop playing when 0 (not implemented) */
	struct osc_t op[VOICE_OPCOUNT];		/**< Array of operators */
	int opFMSource[VOICE_OPCOUNT];
	double opFMIndex[VOICE_OPCOUNT];
	struct filter_t filter[2];			/**< Two filters that can be configured as HPF, LPF, BPF or APF. */
	double mix[VOICE_OPCOUNT];
	struct env_t env[VOICE_OPCOUNT];	/**< Volume envelope for each operator */
};

int CreateVoice(struct voice_t **v);
double VoiceSample(struct voice_t* v);
void VoiceSetFreq(struct voice_t* v, double f);

double Pulse(double phase, double pw);
double Triangle(double phase);
double Sine(double phase);
double Saw(double phase);

void VoiceGateOn(struct voice_t *v);
void VoiceGateOff(struct voice_t *v);

#endif /* VOICE_H_ */
