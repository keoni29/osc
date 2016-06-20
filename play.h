/*
 * play.h
 *
 *  Created on: Jun 16, 2016
 *      Author: koen
 */

#ifndef PLAY_H_
#define PLAY_H_


/** Callback for filling the audio stream buffer */
void PlayRenderSample(void* userdata, uint8_t* stream, int len);

int PlayInit();

int PlayNoteOn(int note, int key);

int PlayNoteOff(int note, int key);

/** Timer callback for playing midi files */
uint32_t Play(uint32_t interval, void *param);

#endif /* PLAY_H_ */
