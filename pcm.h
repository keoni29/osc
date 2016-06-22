/*
 * pcm.h
 *
 *  Created on: Jun 21, 2016
 *      Author: koen
 */

#ifndef PCM_H_
#define PCM_H_

#include <stdint.h>

struct pcm_t
{
	uint32_t count;
	uint32_t length;
	uint32_t playing;
	int16_t *data;
};

float PCMSample(struct pcm_t *p);
int PCMCreate(const char *fname, struct pcm_t *p);
int PCMIsPlaying(struct pcm_t *p);
void PCMCreateEmpty(struct pcm_t *p);
void PCMFree(struct pcm_t *p);
void PCMStart(struct pcm_t *p);
void PCMStop(struct pcm_t *p);


#endif /* PCM_H_ */
