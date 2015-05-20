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
#include "rpApp.h"

#define ADC_BUFFER_SIZE             (16*1024)
#define MILLI_TO_NANO               1000000.0

#define ECHECK_APP(x) { \
        int retval = (x); \
        if (retval != RP_OK) { \
            fprintf(stderr, "Runtime error: %s returned \"%s\" at %s:%d\n", #x, rpApp_GetError(retval), __FILE__, __LINE__); \
        } \
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

#define STOP_THREAD(THREAD_X) \
if ((THREAD_X) != -1) { \
	pthread_cancel(THREAD_X); \
    (THREAD_X) = (pthread_t) -1; \
}

#define START_THREAD(THREAD_X, THREAD_FUN) \
if ((THREAD_X) == -1) { \
    if (pthread_create(&(THREAD_X), NULL, &(THREAD_FUN), NULL)) { \
        return RP_APP_EST; \
    } \
}

#define EXECUTE_ATOMICALLY(MUTEX, ACTION) \
pthread_mutex_lock(&(MUTEX));\
(ACTION); \
pthread_mutex_unlock(&(MUTEX));


int cmn_Init();
int cmn_Release();

int intCmp(const void *a, const void *b);

float indexToTime(int64_t index);
int64_t timeToIndex(float time);

#endif /* COMMON_APP_H_ */
