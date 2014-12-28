/**
 * $Id: $
 *
 * @brief Red Pitaya library Acquire signal handler implementation
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <stdio.h>
#include <stdint.h>

#include "version.h"
#include "common.h"
#include "oscilloscope.h"
#include "acq_handler.h"


// Decimation constants
static const uint32_t DEC_1     = 1;
static const uint32_t DEC_8     = 8;
static const uint32_t DEC_64    = 64;
static const uint32_t DEC_1024  = 1024;
static const uint32_t DEC_8192  = 8192;
static const uint32_t DEC_65536 = 65536;


int acq_SetSamplingRate(rp_acq_sampling_rate_t sampling_rate)
{
	switch (sampling_rate) {
	case RP_SMP_125M:
		return osc_SetDecimation(DEC_1);
	case RP_SMP_15_625M:
		return osc_SetDecimation(DEC_8);
	case RP_SMP_1_953M:
		return osc_SetDecimation(DEC_64);
	case RP_SMP_122_070K:
		return osc_SetDecimation(DEC_1024);
	case RP_SMP_15_258K:
		return osc_SetDecimation(DEC_8192);
	case RP_SMP_1_907K:
		return osc_SetDecimation(DEC_65536);
	default:
		return RP_EOOR;
	}
}

int acq_GetSamplingRate(rp_acq_sampling_rate_t* sampling_rate)
{
	uint32_t decimation;
	ECHECK(osc_GetDecimation(&decimation));

	if (decimation == DEC_1) {
		*sampling_rate = RP_SMP_125M;
		return RP_OK;
	}
	else if (decimation == DEC_8) {
		*sampling_rate = RP_SMP_15_625M;
		return RP_OK;
	}
	else if (decimation == DEC_64) {
		*sampling_rate = RP_SMP_1_953M;
		return RP_OK;
	}
	else if (decimation == DEC_1024) {
		*sampling_rate = RP_SMP_122_070K;
		return RP_OK;
	}
	else if (decimation == DEC_8192) {
		*sampling_rate = RP_SMP_15_258K;
		return RP_OK;
	}
	else if (decimation == DEC_65536) {
		*sampling_rate = RP_SMP_1_907K;
		return RP_OK;
	}
	else {
		return RP_EOOR;
	}
}

int acq_GetSamplingRateNum(float* sampling_rate)
{
	rp_acq_sampling_rate_t rate;
	ECHECK(acq_GetSamplingRate(&rate));

	switch (rate) {
		case RP_SMP_125M:
			*sampling_rate = 125.0 * 1024 * 1024;
			return RP_OK;
		case RP_SMP_15_625M:
			*sampling_rate =  15.625 * 1024 * 1024;
			return RP_OK;
		case RP_SMP_1_953M:
			*sampling_rate =  1.953 * 1024 * 1024;
			return RP_OK;
		case RP_SMP_122_070K:
			*sampling_rate =  122.070 * 1024;
			return RP_OK;
		case RP_SMP_15_258K:
			*sampling_rate =  15.258 * 1024;
			return RP_OK;
		case RP_SMP_1_907K:
			*sampling_rate =  1.907 * 1024;
			return RP_OK;
		default:
			return RP_EOOR;
	}
}
