/**
 * $Id: $
 *
 * @brief Red Pitaya Applications library common module interface
 *
 * @Author Luka Golinar, RedPitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

 #ifndef COMMON_LCR_H_
 #define COMMON_LCR_H_

#include "../../../rpbase/src/rp.h"

#define ECHECK_APP(x){ \
 		int retval = (x); \
 		if(retval != RP_OK) { \
 			printf("TODO: ADD ERROR\n"); \
 			return retval; \
 		} \
}

float vectorMax(float *data, int view_size);
float trapezoidalApprox(float *data, float t, int view_size);
float meanVector(float *data, int view_size);
float **multiDimensionVector(int first_dimension, int second_dimenson);

#endif /* COMMON_LCR_H_ */