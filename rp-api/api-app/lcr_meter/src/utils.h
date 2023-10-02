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


#include <sys/syslog.h>
#include "rp.h"

#define RP_LOG(...){ \
	syslog(__VA_ARGS__); \
}

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define ABS(x) (((x) < 0) ? (-(x)) : (x))

#define EXEC_CHECK(x){ \
 		int retval = (x); \
 		if(retval != RP_OK) { \
 			RP_LOG(LOG_INFO, "Execution error.\n"); \
 			return retval; \
 		} \
}

#define ECHECK_APP_MUTEX(MUTEX, x) { \
    int retval = (x); \
    if (retval != RP_OK) { \
        RP_LOG(LOG_INFO, "Execution error.\n");  \
        pthread_mutex_unlock(&MUTEX); \
        return retval; \
    } \
}

#define ECHECK(X) { \
    int retval = (X); \
    if (retval != RP_OK) { \
    fprintf(stderr, "Runtime error: %s returned \"%s\" at %s:%d\n", #X, rp_GetError(retval), __FILE__, __LINE__); \
    return retval; \
    } \
    }

#define CHECK_CHANNEL(X) \
    uint8_t channels_rp_HPGetFastADCChannelsCount = 0; \
    if (rp_HPGetFastADCChannelsCount(&channels_rp_HPGetFastADCChannelsCount) != RP_HP_OK){ \
        fprintf(stderr,"[Error:%s] Can't get fast ADC channels count\n",X); \
        return RP_NOTS; \
    } \
    if (channel >= channels_rp_HPGetFastADCChannelsCount){ \
        fprintf(stderr,"[Error:%s] Channel is larger than allowed\n",X); \
        return RP_NOTS; \
    }

float vectorMax(float *data, int size);
float vectorApprox(float *data, int size, float approx_val, bool min);
float trapezoidalApprox(double *data, float t, int size);
void FirRectangleWindow(float *data, int data_size, int window_size);
float vectorMean(float *data, int steps);
float **multiDimensionVector(int second_dimenson);
int delMultiDimensionVector(float** data);
int set_IIC_Shunt(int k);
bool isSineTester(float **data, uint32_t size, double T);

void lcr_getDecimationValue(float frequency,rp_acq_decimation_t *api_dec,int *dec_val,uint32_t adc_rate);
int checkExtensionModuleConnection();

void Fir(double *data, int data_size);
void Normalize(double *data, int data_size);

#endif //UTILS_H_
