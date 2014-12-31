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

// Sampling rate constants
static const uint32_t SR_125MHZ = 125.0 * 1024 * 1024;
static const uint32_t SR_15_625MHZ = 15.625 * 1024 * 1024;
static const uint32_t SR_1_953MHZ = 1.953 * 1024 * 1024;
static const uint32_t SR_122_070KHZ = 122.070 * 1024;
static const uint32_t SR_15_258KHZ = 15.258 * 1024;
static const uint32_t SR_1_907KHZ = 1.907 * 1024;


int acq_SetDecimation(rp_acq_decimation_t decimation)
{
	switch (decimation) {
	case RP_DEC_1:
		return osc_SetDecimation(DEC_1);
	case RP_DEC_8:
		return osc_SetDecimation(DEC_8);
	case RP_DEC_64:
		return osc_SetDecimation(DEC_64);
	case RP_DEC_1024:
		return osc_SetDecimation(DEC_1024);
	case RP_DEC_8192:
		return osc_SetDecimation(DEC_8192);
	case RP_DEC_65536:
		return osc_SetDecimation(DEC_65536);
	default:
		return RP_EOOR;
	}
}

int acq_GetDecimation(rp_acq_decimation_t* decimation)
{
	uint32_t decimationVal;
	ECHECK(osc_GetDecimation(&decimationVal));

	if (decimationVal == DEC_1) {
		*decimation = RP_DEC_1;
		return RP_OK;
	}
	else if (decimationVal == DEC_8) {
		*decimation = RP_DEC_8;
		return RP_OK;
	}
	else if (decimationVal == DEC_64) {
		*decimation = RP_DEC_64;
		return RP_OK;
	}
	else if (decimationVal == DEC_1024) {
		*decimation = RP_DEC_1024;
		return RP_OK;
	}
	else if (decimationVal == DEC_8192) {
		*decimation = RP_DEC_8192;
		return RP_OK;
	}
	else if (decimationVal == DEC_65536) {
		*decimation = RP_DEC_65536;
		return RP_OK;
	}
	else {
		return RP_EOOR;
	}
}

int acq_GetDecimationNum(uint32_t* decimation)
{
	rp_acq_decimation_t decimationVal;
	ECHECK(acq_GetDecimation(&decimationVal));

	switch (decimationVal) {
		case RP_DEC_1:
			*decimation = DEC_1;
			return RP_OK;
		case RP_DEC_8:
			*decimation = DEC_8;
			return RP_OK;
		case RP_DEC_64:
			*decimation = DEC_64;
			return RP_OK;
		case RP_DEC_1024:
			*decimation = DEC_1024;
			return RP_OK;
		case RP_DEC_8192:
			*decimation = DEC_8192;
			return RP_OK;
		case RP_DEC_65536:
			*decimation = DEC_65536;
			return RP_OK;
		default:
			return RP_EOOR;
	}
}


int acq_SetSamplingRate(rp_acq_sampling_rate_t sampling_rate)
{
	switch (sampling_rate) {
		case RP_SMP_125M:
			return acq_SetDecimation(RP_DEC_1);
		case RP_SMP_15_625M:
			return acq_SetDecimation(RP_DEC_8);
		case RP_SMP_1_953M:
			return acq_SetDecimation(RP_DEC_64);
		case RP_SMP_122_070K:
			return acq_SetDecimation(RP_DEC_1024);
		case RP_SMP_15_258K:
			return acq_SetDecimation(RP_DEC_8192);
		case RP_SMP_1_907K:
			return acq_SetDecimation(RP_DEC_65536);
		default:
			return RP_EOOR;
	}
}

int acq_GetSamplingRate(rp_acq_sampling_rate_t* sampling_rate)
{
	rp_acq_decimation_t decimation;
	ECHECK(acq_GetDecimation(&decimation));

	switch (decimation) {
		case RP_DEC_1:
			*sampling_rate = RP_SMP_125M;
			return RP_OK;
		case RP_DEC_8:
			*sampling_rate = RP_SMP_15_625M;
			return RP_OK;
		case RP_DEC_64:
			*sampling_rate = RP_SMP_1_953M;
			return RP_OK;
		case RP_DEC_1024:
			*sampling_rate = RP_SMP_122_070K;
			return RP_OK;
		case RP_DEC_8192:
			*sampling_rate = RP_SMP_15_258K;
			return RP_OK;
		case RP_DEC_65536:
			*sampling_rate = RP_SMP_1_907K;
			return RP_OK;
		default:
			return RP_EOOR;
	}
}

int acq_GetSamplingRateHz(float* sampling_rate)
{
	rp_acq_sampling_rate_t rate;
	ECHECK(acq_GetSamplingRate(&rate));

	switch (rate) {
		case RP_SMP_125M:
			*sampling_rate = SR_125MHZ;
			return RP_OK;
		case RP_SMP_15_625M:
			*sampling_rate =  SR_15_625MHZ;
			return RP_OK;
		case RP_SMP_1_953M:
			*sampling_rate =  SR_1_953MHZ;
			return RP_OK;
		case RP_SMP_122_070K:
			*sampling_rate =  SR_122_070KHZ;
			return RP_OK;
		case RP_SMP_15_258K:
			*sampling_rate =  SR_15_258KHZ;
			return RP_OK;
		case RP_SMP_1_907K:
			*sampling_rate =  SR_1_907KHZ;
			return RP_OK;
		default:
			return RP_EOOR;
	}
}

int acq_SetAveraging(bool enable)
{
	return osc_SetAveraging(enable);
}

int acq_GetAveraging(bool* enable)
{
	return osc_GetAveraging(enable);
}

int acq_SetTriggerSrc(rp_acq_trig_src_t source)
{
	return osc_SetTriggerSource(source);
}

int acq_GetTriggerSrc(rp_acq_trig_src_t* source)
{
	return osc_GetTriggerSource(source);
}
