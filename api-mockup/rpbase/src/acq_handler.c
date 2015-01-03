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
#include <math.h>

#include "version.h"
#include "common.h"
#include "calib.h"
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

/* @brief ADC buffer size is 16 k samples. */
static const uint32_t ADC_BUFFER_SIZE = 16 * 1024;

/* @brief Sampling period (non-decimated) - 8 [ns]. */
static const uint64_t ADC_SAMPLE_PERIOD = 8;

/* @brief Number of ADC acquisition bits. */
static const int ADC_BITS = 14;

/* @brief Currently set Gain state */
static rp_pinState_t gain = RP_LOW;

/* @brief Determines whether TriggerDelay was set in time or sample units */
static bool triggerDelayInNs = false;

/*----------------------------------------------------------------------------*/
/**
* @brief Converts time in [ns] to ADC samples
*
*
* @param[in] time time, specified in [ns]
* @retval int number of ADC samples
*/
uint32_t cnvTimeToSmpls(uint64_t time_ns)
{
	/* Calculate sampling period (including decimation) */

	uint32_t decimation;
	ECHECK(acq_GetDecimationFactor(&decimation));

	uint64_t smpl_p = (ADC_SAMPLE_PERIOD * decimation);
	return (uint32_t)round((double)time_ns / smpl_p);
}

/*----------------------------------------------------------------------------*/
/**
* @brief Converts ADC samples to time in [ns]
*
*
* @param[in] samples, number of ADC samples
* @retval int time, specified in [ns]
*/
uint64_t cnvSmplsToTime(uint32_t samples)
{
	/* Calculate time (including decimation) */

	uint32_t decimation;
	ECHECK(acq_GetDecimationFactor(&decimation));

	return (uint64_t)samples * ADC_SAMPLE_PERIOD * decimation;
}


/*----------------------------------------------------------------------------*/

int acq_SetGain(rp_pinState_t state)
{
	gain = state;

	// TODO: Update channel treshold and channel treshold hyst if it was set in volts

	return RP_OK;
}

int acq_GetGain(rp_pinState_t* state)
{
	*state = gain;
	return RP_OK;
}

/**
 * Returns currently set gain in Volts
 * @param state
 * @return
 */
int acq_GetGainV(float* gain)
{
	if (gain == RP_LOW) {
		*gain = 1.0;
	}
	else {
		*gain = 20.0;
	}
	return RP_OK;
}

int acq_SetDecimation(rp_acq_decimation_t decimation)
{
	uint64_t time_ns = 0;

	if (triggerDelayInNs) {
		ECHECK(acq_GetTriggerDelayNs(&time_ns));
	}

	switch (decimation) {
	case RP_DEC_1:
		ECHECK(osc_SetDecimation(DEC_1));
		break;
	case RP_DEC_8:
		ECHECK(osc_SetDecimation(DEC_8));
		break;
	case RP_DEC_64:
		ECHECK(osc_SetDecimation(DEC_64));
		break;
	case RP_DEC_1024:
		ECHECK(osc_SetDecimation(DEC_1024));
		break;
	case RP_DEC_8192:
		ECHECK(osc_SetDecimation(DEC_8192));
		break;
	case RP_DEC_65536:
		ECHECK(osc_SetDecimation(DEC_65536));
		break;
	default:
		return RP_EOOR;
	}

	// Now update trigger delay based on new decimation
	if (triggerDelayInNs) {
		ECHECK(acq_SetTriggerDelayNs(time_ns, true));
	}

	return RP_OK;
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

int acq_GetDecimationFactor(uint32_t* decimation)
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

int acq_SetTriggerDelay(uint32_t decimated_data_num, bool updateMaxValue)
{
	if (decimated_data_num > ADC_BUFFER_SIZE) {
		if (updateMaxValue) {
			decimated_data_num = ADC_BUFFER_SIZE;
		}
		else {
			return RP_EOOR;
		}
	}
	ECHECK(osc_SetTriggerDelay(decimated_data_num));
	triggerDelayInNs = false;
	return RP_OK;
}

int acq_SetTriggerDelayNs(uint64_t time_ns, bool updateMaxValue)
{
	uint32_t samples = cnvTimeToSmpls(time_ns);
	ECHECK(acq_SetTriggerDelay(samples, updateMaxValue));

	triggerDelayInNs = true;
	return RP_OK;
}

int acq_GetTriggerDelay(uint32_t* decimated_data_num)
{
	return osc_GetTriggerDelay(decimated_data_num);
}

int acq_GetTriggerDelayNs(uint64_t* time_ns)
{
	uint32_t samples;
	ECHECK(acq_GetTriggerDelay(&samples));
	return cnvSmplsToTime(samples);
}

int acq_GetWritePointer(uint32_t* pos)
{
	return osc_GetWritePointer(pos);
}

int acq_GetWritePointerAtTrig(uint32_t* pos)
{
	return osc_GetWritePointerAtTrig(pos);
}

int acq_Start()
{
	return osc_WriteDataIntoMemory(true);
}

int acq_Stop()
{
	return osc_WriteDataIntoMemory(false);
}

int acq_SetChannelThreshold(rp_channel_t channel, float voltage)
{
	float gain;

	ECHECK(acq_GetGainV(&gain));
	rp_calib_params_t calib = calib_GetParams();
	int32_t dc_offs = (channel == RP_CH_A ? calib.fe_ch1_dc_offs : calib.fe_ch2_dc_offs);
	uint32_t cnt = cmn_CnvVToCnt(ADC_BITS, voltage, gain, dc_offs, 0.0);
	if (channel == RP_CH_A) {
		return osc_SetThresholdChA(cnt);
	}
	else {
		return osc_SetThresholdChB(cnt);
	}
}

int acq_GetChannelThreshold(rp_channel_t channel, float* voltage)
{
	float gain;
	uint32_t cnts;

	if (channel == RP_CH_A) {
		ECHECK(osc_GetThresholdChA(&cnts));
	}
	else {
		ECHECK(osc_GetThresholdChB(&cnts));
	}

	ECHECK(acq_GetGainV(&gain));
	rp_calib_params_t calib = calib_GetParams();

	int32_t dc_offs = (channel == RP_CH_A ? calib.fe_ch1_dc_offs : calib.fe_ch2_dc_offs);
	*voltage = cmn_CnvCntToV(ADC_BITS, cnts, gain, dc_offs, 0.0);

	return RP_OK;
}


int acq_SetChannelThresholdHyst(rp_channel_t channel, float voltage)
{
	float gain;

	ECHECK(acq_GetGainV(&gain));
	rp_calib_params_t calib = calib_GetParams();
	int32_t dc_offs = (channel == RP_CH_A ? calib.fe_ch1_dc_offs : calib.fe_ch2_dc_offs);
	uint32_t cnt = cmn_CnvVToCnt(ADC_BITS, voltage, gain, dc_offs, 0.0);
	if (channel == RP_CH_A) {
		return osc_SetHysteresisChA(cnt);
	}
	else {
		return osc_SetHysteresisChB(cnt);
	}
}

int acq_GetChannelThresholdHyst(rp_channel_t channel, float* voltage)
{
	float gain;
	uint32_t cnts;

	if (channel == RP_CH_A) {
		ECHECK(osc_GetHysteresisChA(&cnts));
	}
	else {
		ECHECK(osc_GetHysteresisChB(&cnts));
	}

	ECHECK(acq_GetGainV(&gain));
	rp_calib_params_t calib = calib_GetParams();

	int32_t dc_offs = (channel == RP_CH_A ? calib.fe_ch1_dc_offs : calib.fe_ch2_dc_offs);
	*voltage = cmn_CnvCntToV(ADC_BITS, cnts, gain, dc_offs, 0.0);

	return RP_OK;
}
