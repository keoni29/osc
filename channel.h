/*
 * channel.h
 *
 *  Created on: Jun 24, 2016
 *      Author: koen
 */

#ifndef CHANNEL_H_
#define CHANNEL_H_

#define MAX_POLYPHONY 4

#include "voice.h"
#include "pcm.h"
#include "smf.h"

struct channel_t
{
	uint8_t number;				/**< Channel number (1-16 for midi) */
	uint8_t voiceCount;			/**< The number of voices currently allocated */
	struct voice_t *voice[MAX_POLYPHONY];
	struct pcm_t drum[128];	/**< All samples are loaded when the channel is created */
	int vgate[MAX_POLYPHONY];	/**< Voice gate. For keeping track of which notes are on. */
};

int ChannelEvent(struct channel_t *c, struct SMF_event *e);

int ChannelNoteOn(struct channel_t *c, int note, int key);

int ChannelNoteOff(struct channel_t *c, int note, int key);

int ChannelLoadPatch(struct channel_t *c, const char *fname, const char *patch);

int CreateChannel(struct channel_t *c, uint16_t number);

float ChannelSample(struct channel_t *c);

#endif /* CHANNEL_H_ */
