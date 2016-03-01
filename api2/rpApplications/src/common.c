/**
 * $Id: $
 *
 * @brief Red Pitaya Applications library common module implementation
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */


#include <math.h>
#include "common.h"


int cmn_Init() {
    return RP_OK;
}

int cmn_Release() {
    return RP_OK;
}

int intCmp(const void *a, const void *b) {
    const int *ia = a, *ib = b;
    return (*ia < *ib) ? -1 : (*ia > *ib);
}

// Returns time in milliseconds
float indexToTime(int64_t index) {
    float samplingRate;
    ECHECK_APP(rp_AcqGetSamplingRateHz(&samplingRate));
    return (float) (index * 1000.0 / samplingRate);
}

// Parameter time is in milliseconds
int64_t timeToIndex(float time) {
    float samplingRate;
    ECHECK_APP(rp_AcqGetSamplingRateHz(&samplingRate));
    return (int64_t) round(samplingRate * time / 1000.0);
}