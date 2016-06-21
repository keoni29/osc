/*
 * filter.h
 *
 *  Created on: Jun 5, 2016
 *      Author: koen
 */

#ifndef FILTER_H_
#define FILTER_H_

#include <stdint.h>

enum filter_type
{
	FILTER_LPF,
	FILTER_HPF,
	FILTER_NONE
};

struct filter_t
{
	float	k,	/**< Filter time constant (calculated) */
			yn,	/**< Previous output value (calculated) */
			xn;	/**< Previous input value (calculated) */
	enum filter_type type; /**< Specifies the which kind of filter operation to perform */
};

/** Calculates the time constant based on the provided cutoff frequency fc */
void CreateFilter(struct filter_t *f, float fc, enum filter_type type);
/** Perform a filtering operation on input x. The filter state is stored in f. */
float ProcessFilter(float x, struct filter_t *f);

#endif /* FILTER_H_ */
