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

uint8_t UpdateOsc(uint8_t id);
uint8_t SetOscReg(uint8_t id, uint16_t reg, uint8_t shape, uint16_t pw);
uint8_t IncOscPitch(uint8_t id);
float RenderFrame(void);
void OscFree();
void OscInit();
void OscGenerateFrames(float *buff, uint64_t frames);
void OscPlay(float *buff, uint64_t frames);
#endif /* OSC_H_ */
