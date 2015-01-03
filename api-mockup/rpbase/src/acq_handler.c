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

/* @brief Number of ADC acquisition bits. */
static const int ADC_BITS = 14;

/* @brief Currently set Gain state */
static rp_pinState_t gain = RP_LOW;



/*----------------------------------------------------------------------------*/
/**
* @brief Converts ADC counts to voltage [V]
*
* Function is used to publish captured signal data to external world in user units.
* Calculation is based on maximal voltage, which can be applied on ADC inputs and
* calibrated and user defined DC offsets.
*
* @param[in] cnts Captured Signal Value, expressed in ADC counts
* @param[in] adc_max_v Maximal ADC voltage, specified in [V]
* @param[in] calib_dc_off Calibrated DC offset, specified in ADC counts
* @param[in] user_dc_off User specified DC offset, specified in [V]
* @retval float Signal Value, expressed in user units [V]
*/

static float cnvCntToV(uint32_t cnts, float adc_max_v, int calib_dc_off, float user_dc_off)
{
	int m;
	float ret_val;
	/* check sign */
	if(cnts & (1 << (ADC_BITS - 1))) {
		/* negative number */
		m = -1 *((cnts ^ ((1 << ADC_BITS) - 1)) + 1);
	} else {
		/* positive number */
		m = cnts;
	}

	/* adopt ADC count with calibrated DC offset */
	m += calib_dc_off;

	/* map ADC counts into user units */
	if(m < (-1 * (1 << (ADC_BITS - 1))))
		m = (-1 * (1 << (ADC_BITS - 1)));
	else if(m > (1 << (ADC_BITS - 1)))
		m = (1 << (ADC_BITS - 1));

	ret_val = (m * adc_max_v / (float)(1 << (ADC_BITS - 1)));

	/* and adopt the calculation with user specified DC offset */
	ret_val += user_dc_off;

	return ret_val;
}

/**
* @brief Converts voltage in [V] to ADC counts
*
* Function is used for setting up trigger threshold value, which is written into
* appropriate FPGA register. This value needs to be specified in ADC counts, while
* user specifies this information in Voltage. The resulting value is based on the
* specified threshold voltage, maximal ADC voltage, calibrated and user specified
* DC offsets.
*
* @param[in] voltage Voltage, specified in [V]
* @param[in] adc_max_v Maximal ADC voltage, specified in [V]
* @param[in] calib_dc_off Calibrated DC offset, specified in ADC counts
* @param[in] user_dc_off User specified DC offset, , specified in [V]
* @retval int ADC counts
*/
static uint32_t cnvVToCnt(float voltage, float adc_max_v, int calib_dc_off, float user_dc_off)
{
	int adc_cnts = 0;

	/* check and limit the specified voltage arguments towards */
	/* maximal voltages which can be applied on ADC inputs */
	if(voltage > adc_max_v)
		voltage = adc_max_v;
	else if(voltage < -adc_max_v)
		voltage = -adc_max_v;

	/* adopt the specified voltage with user defined DC offset */
	voltage -= user_dc_off;

	/* map voltage units into FPGA adc counts */
	adc_cnts = (int)round(voltage * (float)((int)(1 << ADC_BITS)) / (2 * adc_max_v));

	/* clip to the highest value (we are dealing with 14 bits only) */
	if((voltage > 0) && (adc_cnts & (1 << (ADC_BITS - 1))))
		adc_cnts = (1 << (ADC_BITS - 1)) - 1;
	else
		adc_cnts = adc_cnts & ((1 << (ADC_BITS)) - 1);

	/* adopt calculated ADC counts with calibration DC offset */
	adc_cnts -= calib_dc_off;

	return (uint32_t)adc_cnts;
}


/*----------------------------------------------------------------------------*/

int acq_SetGain(rp_pinState_t state)
{
	gain = state;
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

int acq_SetTriggerDelay(uint32_t decimated_data_num)
{
	if (decimated_data_num > ADC_BUFFER_SIZE) {
		return RP_EOOR;
	}
	return osc_SetTriggerDelay(decimated_data_num);
}

int acq_GetTriggerDelay(uint32_t* decimated_data_num)
{
	return osc_GetTriggerDelay(decimated_data_num);
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
	uint32_t cnt = cnvVToCnt(voltage, gain, dc_offs, 0.0);
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
	*voltage = cnvCntToV(cnts, gain, dc_offs, 0.0);

	return RP_OK;
}


int acq_SetChannelThresholdHyst(rp_channel_t channel, float voltage)
{
	float gain;

	ECHECK(acq_GetGainV(&gain));
	rp_calib_params_t calib = calib_GetParams();
	int32_t dc_offs = (channel == RP_CH_A ? calib.fe_ch1_dc_offs : calib.fe_ch2_dc_offs);
	uint32_t cnt = cnvVToCnt(voltage, gain, dc_offs, 0.0);
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
	*voltage = cnvCntToV(cnts, gain, dc_offs, 0.0);

	return RP_OK;
}
