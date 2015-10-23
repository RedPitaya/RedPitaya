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

 #ifndef COMMON_IMP_H_
 #define COMMON_IMP_H_

#include "redpitaya/rp.h"

#define ECHECK_APP(x){ \
 		int retval = (x); \
 		if(retval != RP_OK) { \
 			printf("TODO: ADD ERROR\n"); \
 			return retval; \
 		} \
}

float vectorMax(float *data, int size);
float vectorApprox(float *data, int size, float approx_val, bool min);
float trapezoidalApprox(float *data, float t, int size);
float vectorMean(float *data, int steps);
float **multiDimensionVector(int second_dimenson);
int set_IIC_Shunt(uint32_t shunt);

/* Directry creation functions */
int createPath(char *path);
int createSingle(char *path);

#endif /* COMMON_IMP_H_ */
