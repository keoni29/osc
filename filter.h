/*
 * filter.h
 *
 *  Created on: Jun 5, 2016
 *      Author: koen
 */

#ifndef FILTER_H_
#define FILTER_H_

#define F2RC(fc, fs) 2 * M_PI * fc / fs
float LPF(float x, float rc);


#endif /* FILTER_H_ */
