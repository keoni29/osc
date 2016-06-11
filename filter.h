/*
 * filter.h
 *
 *  Created on: Jun 5, 2016
 *      Author: koen
 */

#ifndef FILTER_H_
#define FILTER_H_

#include <stdint.h>

struct filter_t
{
	double k, yn, xn;
};

void CreateFilter(struct filter_t *f, double fc);
double ProcessLPF(double x, struct filter_t *lpf);
double ProcessHPF(double x, struct filter_t *hpf);

#endif /* FILTER_H_ */
