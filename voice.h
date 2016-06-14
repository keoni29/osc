/*
 * voice.h
 *
 *  Created on: Jun 14, 2016
 *      Author: koen
 */

#ifndef VOICE_H_
#define VOICE_H_

/** The amount of operators per voice */
#define VOICE_OPCOUNT 4

struct voice_t
{
	double freq;						/**< Voice base frequency in Hz */
	uint8_t gate; 						/**< Start playing when 1, stop playing when 0 (not implemented) */
	struct osc_t op[VOICE_OPCOUNT];		/**< Array of operators */
	int opFMSource[VOICE_OPCOUNT];
	double opFMIndex[VOICE_OPCOUNT];
	struct filter_t filter[2];			/**< Two filters that can be configured as HPF, LPF, BPF or APF. */
	double mix[VOICE_OPCOUNT];
};



struct voice_t* CreateVoice();
double VoiceSample(struct voice_t* v);
void VoiceSetFreq(struct voice_t* v, double f);

#endif /* VOICE_H_ */
