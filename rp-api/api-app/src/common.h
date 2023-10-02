/**
 * $Id: $
 *
 * @brief Red Pitaya Applications library common module interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef COMMON_APP_H_
#define COMMON_APP_H_

/* @brief ADC buffer size is 16 k samples. */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string>

#include "rpApp.h"
#include "rp_hw-calib.h"

#define MILLI_TO_NANO               1000000.0

#define MAX_ADC_CHANNELS 4
#define MAX_DAC_CHANNELS 2

#define ECHECK_APP(x) { \
    int retval = (x); \
    if (retval != RP_OK) { \
        fprintf(stderr, "Runtime error: %s returned \"%s\" at %s:%d\n", #x, rpApp_GetError(retval), __FILE__, __LINE__); \
        return retval; \
    } \
}


#define ECHECK_APP_NO_RET(x) { \
    int retval = (x); \
    if (retval != RP_OK) { \
         fprintf(stderr, "Runtime error: %s returned \"%s\" at %s:%d\n", #x, rpApp_GetError(retval), __FILE__, __LINE__); \
    } \
}

#define CHANNEL_ACTION(CHANNEL, CHANNEL_1_ACTION, CHANNEL_2_ACTION) \
if ((CHANNEL) == RP_CH_1) { \
    CHANNEL_1_ACTION; \
} \
else if ((CHANNEL) == RP_CH_2) { \
    CHANNEL_2_ACTION; \
} \
else { \
    return RP_EPN; \
}

#define CHANNEL_ACTION_4CH(CHANNEL, CHANNEL_1_ACTION, CHANNEL_2_ACTION, CHANNEL_3_ACTION, CHANNEL_4_ACTION) \
if ((CHANNEL) == RP_CH_1) { \
    CHANNEL_1_ACTION; \
} \
else if ((CHANNEL) == RP_CH_2) { \
    CHANNEL_2_ACTION; \
} \
else if ((CHANNEL) == RP_CH_3) { \
    CHANNEL_3_ACTION; \
} \
else if ((CHANNEL) == RP_CH_4) { \
    CHANNEL_4_ACTION; \
} \
else { \
    return RP_EPN; \
}

#define SOURCE_ACTION(SOURCE, SOURCE_1_ACTION, SOURCE_2_ACTION, SOURCE_3_ACTION) \
switch ((SOURCE)) { \
    case RPAPP_OSC_SOUR_CH1: \
        (SOURCE_1_ACTION); \
        break; \
    case RPAPP_OSC_SOUR_CH2: \
        (SOURCE_2_ACTION); \
        break; \
    case RPAPP_OSC_SOUR_MATH: \
        (SOURCE_3_ACTION); \
        break; \
    default: \
        return RP_EPN; \
        break; \
}

#define SOURCE_ACTION_4CH(SOURCE, SOURCE_1_ACTION, SOURCE_2_ACTION, SOURCE_3_ACTION,SOURCE_4_ACTION,SOURCE_5_ACTION) \
switch ((SOURCE)) { \
    case RPAPP_OSC_SOUR_CH1: \
        (SOURCE_1_ACTION); \
        break; \
    case RPAPP_OSC_SOUR_CH2: \
        (SOURCE_2_ACTION); \
        break; \
    case RPAPP_OSC_SOUR_CH3: \
        (SOURCE_3_ACTION); \
        break; \
    case RPAPP_OSC_SOUR_CH4: \
        (SOURCE_4_ACTION); \
        break; \
    case RPAPP_OSC_SOUR_MATH: \
        (SOURCE_5_ACTION); \
        break; \
    default: \
        return RP_EPN; \
        break; \
}

#define STOP_THREAD(THREAD_X) \
if ((THREAD_X) != (pthread_t)-1) { \
	pthread_cancel(THREAD_X); \
	int ret = pthread_join(THREAD_X, NULL); \
	if(ret != 0){ fprintf(stderr, "pthread_join() failed: thread id: %d", (unsigned int)THREAD_X); fflush(stderr);} \
    (THREAD_X) = (pthread_t) -1; \
}

#define START_THREAD(THREAD_X, THREAD_FUN) \
if ((THREAD_X) == (pthread_t)-1) { \
    if (pthread_create(&(THREAD_X), NULL, &(THREAD_FUN), NULL)) { \
        return RP_APP_EST; \
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

#define EXECUTE_ATOMICALLY(MUTEX, ACTION) \
    (MUTEX).lock();\
    (ACTION); \
    (MUTEX).unlock();


int cmn_Init();
int cmn_Release();

int intCmp(const void *a, const void *b);

float indexToTime(int64_t index);
int64_t timeToIndex(float time);


auto getADCChannels() -> uint8_t;
auto getDACChannels() -> uint8_t;
auto getDACRate() -> uint32_t;
auto getADCRate() -> uint32_t;
auto getModel() -> rp_HPeModels_t;
auto getDACDevider() -> double;
auto getModelName() -> std::string;

auto convertToVoltSigned(uint32_t cnts, uint8_t bits, float fullScale, uint32_t gain, uint32_t base, int32_t offset) -> float;
auto calibCntsSigned(uint32_t cnts, uint8_t bits, uint32_t gain, uint32_t base, int32_t offset) -> int32_t;
auto getADCSamplePeriod(double *value) -> int;

auto convertCh(rp_channel_t ch) -> rp_channel_calib_t;
auto convertChFromIndex(uint8_t index) -> rp_channel_t;
auto convertPower(rp_acq_ac_dc_mode_t ch) -> rp_acq_ac_dc_mode_calib_t;

#endif /* COMMON_APP_H_ */
