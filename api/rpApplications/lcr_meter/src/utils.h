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

 #ifndef UTILS_H_
 #define UTILS_H_

#include "redpitaya/rp.h"

#define ECHECK_APP(x){ \
 		int retval = (x); \
 		if(retval != RP_OK) { \
 			printf("TODO: ADD ERROR\n"); \
 			return retval; \
 		} \
}

/* R_shunt constans */
static const uint32_t R_SHUNT_30	 = 30;
static const uint32_t R_SHUNT_75     = 75;
static const uint32_t R_SHUNT_300    = 300;
static const uint32_t R_SHUNT_750    = 750;
static const uint32_t R_SHUNT_3K     = 3000;
static const uint32_t R_SHUNT_7_5K   = 7500;
static const uint32_t R_SHUNT_30K    = 30000;
static const uint32_t R_SHUNT_80K    = 80000;
static const uint32_t R_SHUNT_430K   = 430000;
static const uint32_t R_SHUNT_3M     = 3000000;

float vectorMax(float *data, int size);
float vectorApprox(float *data, int size, float approx_val, bool min);
float trapezoidalApprox(float *data, float t, int size);
float vectorMean(float *data, int steps);
float **multiDimensionVector(int second_dimenson);
int set_IIC_Shunt(uint32_t shunt);

int lcr_switchRShunt(float z_ampl, uint32_t *r_shunt);

int lcr_getDecimationValue(float frequency,
						rp_acq_decimation_t *api_dec,
						int *dec_val);

#endif //UTILS_H_
