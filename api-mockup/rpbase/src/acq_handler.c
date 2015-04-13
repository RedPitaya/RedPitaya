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

/* @brief ADC buffer size is 16 k samples. */
static const uint32_t ADC_BUFFER_SIZE = 16 * 1024;

/* @brief Trig. reg. value offset when set to 0 */
static const int32_t TRIG_DELAY_ZERO_OFFSET = 8192;

/* @brief Sampling period (non-decimated) - 8 [ns]. */
static const uint64_t ADC_SAMPLE_PERIOD = 8;

/* @brief Number of ADC acquisition bits. */
static const int ADC_BITS = 14;

/* @brief Currently set Gain state */
static rp_pinState_t gain_ch_a = RP_LOW;
static rp_pinState_t gain_ch_b = RP_LOW;

/* @brief Determines whether TriggerDelay was set in time or sample units */
static bool triggerDelayInNs = false;


/* @brief Default filter equalization coefficients */
static const uint32_t GAIN_HI_CHA_FILT_AA = 0x7D93;
static const uint32_t GAIN_HI_CHA_FILT_BB = 0x437C7;
static const uint32_t GAIN_HI_CHA_FILT_PP = 0x2666;
static const uint32_t GAIN_HI_CHA_FILT_KK = 0xd9999a;
static const uint32_t GAIN_HI_CHB_FILT_AA = 0x7D93;
static const uint32_t GAIN_HI_CHB_FILT_BB = 0x437C7;
static const uint32_t GAIN_HI_CHB_FILT_PP = 0x2666;
static const uint32_t GAIN_HI_CHB_FILT_KK = 0xd9999a;
static const uint32_t GAIN_LO_CHA_FILT_AA = 0x4C5F;
static const uint32_t GAIN_LO_CHA_FILT_BB = 0x2F38B;
static const uint32_t GAIN_LO_CHA_FILT_PP = 0x2666;
static const uint32_t GAIN_LO_CHA_FILT_KK = 0xd9999a;
static const uint32_t GAIN_LO_CHB_FILT_AA = 0x4C5F;
static const uint32_t GAIN_LO_CHB_FILT_BB = 0x2F38B;
static const uint32_t GAIN_LO_CHB_FILT_PP = 0x2666;
static const uint32_t GAIN_LO_CHB_FILT_KK = 0xd9999a;

/*----------------------------------------------------------------------------*/
/**
 * @brief Converts time in [ns] to ADC samples
 *
 *
 * @param[in] time time, specified in [ns]
 * @retval int number of ADC samples
 */
static uint32_t cnvTimeToSmpls(int64_t time_ns)
{
    /* Calculate sampling period (including decimation) */

    uint32_t decimation;
    ECHECK(acq_GetDecimationFactor(&decimation));

    int64_t smpl_p = (ADC_SAMPLE_PERIOD * (int64_t)decimation);
    return (int32_t)round((double)time_ns / smpl_p);
}

/*----------------------------------------------------------------------------*/
/**
 * @brief Converts ADC samples to time in [ns]
 *
 *
 * @param[in] samples, number of ADC samples
 * @retval int time, specified in [ns]
 */
static int64_t cnvSmplsToTime(int32_t samples)
{
    /* Calculate time (including decimation) */

    uint32_t decimation;
    ECHECK(acq_GetDecimationFactor(&decimation));

    return (int64_t)samples * ADC_SAMPLE_PERIOD * (int32_t)decimation;
}

/**
 * Sets equalization filter with default coefficients per channel
 * @param channel Channel A or B
 * @return 0 when successful
 */
static int setEqFilters(rp_channel_t channel)
{
    rp_pinState_t gain;
    ECHECK(acq_GetGain(channel, &gain));

    // Update equalization filter with default coefficients
    if (channel == RP_CH_1)
    {
        if (gain == RP_HIGH)
        {
            return osc_SetEqFiltersChA(
                    GAIN_HI_CHA_FILT_AA,
                    GAIN_HI_CHA_FILT_BB,
                    GAIN_HI_CHA_FILT_KK,
                    GAIN_HI_CHA_FILT_PP);
        }
        else
        {
            return osc_SetEqFiltersChA(
                    GAIN_LO_CHA_FILT_AA,
                    GAIN_LO_CHA_FILT_BB,
                    GAIN_LO_CHA_FILT_KK,
                    GAIN_LO_CHA_FILT_PP);
        }
    }
    else
    {
        if (gain == RP_HIGH)
        {
            return osc_SetEqFiltersChB(
                    GAIN_HI_CHB_FILT_AA,
                    GAIN_HI_CHB_FILT_BB,
                    GAIN_HI_CHB_FILT_KK,
                    GAIN_HI_CHB_FILT_PP);
        }
        else
        {
            return osc_SetEqFiltersChB(
                    GAIN_LO_CHB_FILT_AA,
                    GAIN_LO_CHB_FILT_BB,
                    GAIN_LO_CHB_FILT_KK,
                    GAIN_LO_CHB_FILT_PP);
        }
    }
}

/*----------------------------------------------------------------------------*/

int acq_SetGain(rp_channel_t channel, rp_pinState_t state)
{

    rp_pinState_t *gain = NULL;

    if (channel == RP_CH_1) {
        gain = &gain_ch_a;
    }
    else {
        gain = &gain_ch_b;
    }

    // Gain is already at the same state. Nothing to do...
    if (*gain == state) {
        return RP_OK;
    }

    // Read old values which are dependent on the gain...
    rp_pinState_t old_gain;
    float ch_thr, ch_hyst;
    old_gain = *gain;
    ECHECK(acq_GetChannelThreshold(channel, &ch_thr));
    ECHECK(acq_GetChannelThresholdHyst(channel, &ch_hyst));

    // Now update the gain
    *gain = state;

    // And recalculate new values...
    int status = RP_OK;
    status = acq_SetChannelThreshold(channel, ch_thr);
    if (status == RP_OK) {
        status = acq_SetChannelThresholdHyst(channel, ch_hyst);
    }

    // In case of an error, put old values back and report the error
    if (status != RP_OK) {
        *gain = old_gain;
        acq_SetChannelThreshold(channel, ch_thr);
        acq_SetChannelThresholdHyst(channel, ch_hyst);
    }
    // At the end if everything is ok, update also equalization filters based on the new gain.
    // Updating eq filters should never fail...
    else {
        status = setEqFilters(channel);
    }

    return status;
}

int acq_GetGain(rp_channel_t channel, rp_pinState_t* state)
{
    if (channel == RP_CH_1) {
        *state = gain_ch_a;
    }
    else {
        *state = gain_ch_b;
    }
    return RP_OK;
}

/**
 * Returns currently set gain in Volts
 * @param state
 * @return
 */
int acq_GetGainV(rp_channel_t channel, float* voltage)
{
    rp_pinState_t *gain = NULL;

    if (channel == RP_CH_1) {
        gain = &gain_ch_a;
    }
    else {
        gain = &gain_ch_b;
    }

    if (*gain == RP_LOW) {
        *voltage = 1.0;
    }
    else {
        *voltage = 20.0;
    }
    return RP_OK;
}

int acq_SetDecimation(rp_acq_decimation_t decimation)
{
    int64_t time_ns = 0;

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
    float max_rate = 125000000.0;

    rp_acq_decimation_t decimation;
    ECHECK(acq_GetDecimation(&decimation));

    switch(decimation){
        case RP_DEC_1:
            *sampling_rate = max_rate / 1;
            break;
        case RP_DEC_8:
            *sampling_rate = max_rate / 8;
            break;
        case RP_DEC_64:
            *sampling_rate = max_rate / 64;
            break;
        case RP_DEC_1024:
            *sampling_rate = max_rate / 1024;
            break;
        case RP_DEC_8192:
            *sampling_rate = max_rate / 8192;
            break;
        case RP_DEC_65536:
            *sampling_rate = max_rate / 65536;
            break;
    }
    
    return RP_OK;
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

int acq_GetTriggerState(rp_acq_trig_state_t* state)
{
    rp_acq_trig_src_t source;
    ECHECK(osc_GetTriggerSource(&source));

    if (source == RP_TRIG_SRC_DISABLED) {
        *state=RP_TRIG_STATE_TRIGGERED;
    }
    else{
    	*state=RP_TRIG_STATE_WAITING;
    }

    return RP_OK;
}

int acq_SetTriggerDelay(int32_t decimated_data_num, bool updateMaxValue)
{
	int32_t trig_dly;
	if(decimated_data_num < -TRIG_DELAY_ZERO_OFFSET){
    		trig_dly=0;
	}
	else{
		trig_dly = decimated_data_num + TRIG_DELAY_ZERO_OFFSET;
	}

	/*
    if (trig_dly > ADC_BUFFER_SIZE) {
        if (updateMaxValue) {
        	trig_dly = ADC_BUFFER_SIZE;
        }
        else {
            return RP_EOOR;
        }
    }
    */
    ECHECK(osc_SetTriggerDelay(trig_dly));
    triggerDelayInNs = false;
    return RP_OK;
}

int acq_SetTriggerDelayNs(int64_t time_ns, bool updateMaxValue)
{
    int32_t samples = cnvTimeToSmpls(time_ns);
    ECHECK(acq_SetTriggerDelay(samples, updateMaxValue));
    triggerDelayInNs = true;
    return RP_OK;
}

int acq_GetTriggerDelay(int32_t* decimated_data_num)
{
	uint32_t trig_dly;
	int r=osc_GetTriggerDelay(&trig_dly);
	*decimated_data_num=(int32_t)trig_dly-TRIG_DELAY_ZERO_OFFSET;
    return r;
}

int acq_GetTriggerDelayNs(int64_t* time_ns)
{
    int32_t samples;
    ECHECK(acq_GetTriggerDelay(&samples));
    *time_ns=cnvSmplsToTime(samples);
    return RP_OK;
}

int acq_GetWritePointer(uint32_t* pos)
{
    return osc_GetWritePointer(pos);
}

int acq_GetWritePointerAtTrig(uint32_t* pos)
{
    return osc_GetWritePointerAtTrig(pos);
}

int acq_SetTriggerLevel(float voltage)
{
    ECHECK(acq_SetChannelThreshold(RP_CH_1, voltage));
    return acq_SetChannelThreshold(RP_CH_2, voltage);
}

int acq_GetTriggerLevel(float *voltage)
{
    float v1, v2;
    ECHECK(acq_GetChannelThreshold(RP_CH_1, &v1));
    ECHECK(acq_GetChannelThreshold(RP_CH_2, &v2));

    if (fabs(v1 - v2) > FLOAT_EPS) {
        return RP_EOOR;
    }

    *voltage = v1;

    return RP_OK;
}

int acq_SetChannelThreshold(rp_channel_t channel, float voltage)
{
    float gain;

    ECHECK(acq_GetGainV(channel, &gain));

    if (fabs(voltage) - fabs(gain) > FLOAT_EPS) {
        return RP_EOOR;
    }

    rp_calib_params_t calib = calib_GetParams();
    int32_t dc_offs = (channel == RP_CH_1 ? calib.fe_ch1_dc_offs : calib.fe_ch2_dc_offs);
    uint32_t cnt = cmn_CnvVToCnt(ADC_BITS, voltage, gain, dc_offs, 0.0);
    if (channel == RP_CH_1) {
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

    if (channel == RP_CH_1) {
        ECHECK(osc_GetThresholdChA(&cnts));
    }
    else {
        ECHECK(osc_GetThresholdChB(&cnts));
    }

    ECHECK(acq_GetGainV(channel, &gain));
    rp_calib_params_t calib = calib_GetParams();

    int32_t dc_offs = (channel == RP_CH_1 ? calib.fe_ch1_dc_offs : calib.fe_ch2_dc_offs);
    *voltage = cmn_CnvCntToV(ADC_BITS, cnts, gain, dc_offs, 0.0);

    return RP_OK;
}

int acq_SetTriggerHyst(float voltage)
{
    ECHECK(acq_SetChannelThresholdHyst(RP_CH_1, voltage));
    return acq_SetChannelThresholdHyst(RP_CH_2, voltage);
}

int acq_GetTriggerHyst(float *voltage)
{
    float v1, v2;
    ECHECK(acq_GetChannelThresholdHyst(RP_CH_1, &v1));
    ECHECK(acq_GetChannelThresholdHyst(RP_CH_2, &v2));

    if (fabs(v1 - v2) > FLOAT_EPS) {
        return RP_EOOR;
    }

    return RP_OK;
}


int acq_SetChannelThresholdHyst(rp_channel_t channel, float voltage)
{
    float gain;

    ECHECK(acq_GetGainV(channel, &gain));

    if (fabs(voltage) - fabs(gain) > FLOAT_EPS) {
        return RP_EOOR;
    }

    rp_calib_params_t calib = calib_GetParams();
    int32_t dc_offs = (channel == RP_CH_1 ? calib.fe_ch1_dc_offs : calib.fe_ch2_dc_offs);
    uint32_t cnt = cmn_CnvVToCnt(ADC_BITS, voltage, gain, dc_offs, 0.0);
    if (channel == RP_CH_1) {
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

    if (channel == RP_CH_1) {
        ECHECK(osc_GetHysteresisChA(&cnts));
    }
    else {
        ECHECK(osc_GetHysteresisChB(&cnts));
    }

    ECHECK(acq_GetGainV(channel, &gain));
    rp_calib_params_t calib = calib_GetParams();

    int32_t dc_offs = (channel == RP_CH_1 ? calib.fe_ch1_dc_offs : calib.fe_ch2_dc_offs);
    *voltage = cmn_CnvCntToV(ADC_BITS, cnts, gain, dc_offs, 0.0);

    return RP_OK;
}

int acq_Start()
{
    return osc_WriteDataIntoMemory(true);
}

int acq_Reset()
{
    ECHECK(acq_SetDefault());
    return osc_ResetWriteStateMachine();
}

static const volatile uint32_t* getRawBuffer(rp_channel_t channel)
{
    if (channel == RP_CH_1) {
        return osc_GetDataBufferChA();
    }
    else {
        return osc_GetDataBufferChB();
    }
}

static uint32_t getSizeFromStartEndPos(uint32_t start_pos, uint32_t end_pos)
{

    end_pos = acq_GetNormalizedDataPos(end_pos);
    start_pos = acq_GetNormalizedDataPos(start_pos);

    if (end_pos < start_pos) {
        end_pos += ADC_BUFFER_SIZE;
    }

    return end_pos - start_pos + 1;
}

uint32_t acq_GetNormalizedDataPos(uint32_t pos)
{
    return (pos % ADC_BUFFER_SIZE);
}

int acq_GetDataRaw(rp_channel_t channel, uint32_t pos, uint32_t* size, int16_t* buffer)
{

    *size = MIN(*size, ADC_BUFFER_SIZE);

    uint32_t cnts;

    const volatile uint32_t* raw_buffer = getRawBuffer(channel);

    rp_calib_params_t calib = calib_GetParams();
    int32_t dc_offs = (channel == RP_CH_1 ? calib.fe_ch1_dc_offs : calib.fe_ch2_dc_offs);

    for (uint32_t i = 0; i < (*size); ++i) {
        cnts = (raw_buffer[(pos + i) % ADC_BUFFER_SIZE]) & ADC_BITS;

        buffer[i] = cmn_CalibCnts(ADC_BITS, cnts, dc_offs);
    }

    return RP_OK;
}

int acq_GetDataPosRaw(rp_channel_t channel, uint32_t start_pos, uint32_t end_pos, int16_t* buffer, uint32_t *buffer_size)
{
    uint32_t size = getSizeFromStartEndPos(start_pos, end_pos);

    if (size > *buffer_size) {
        return RP_BTS;
    }

    *buffer_size = size;
    return acq_GetDataRaw(channel, start_pos, buffer_size, buffer);
}

/**
 * Use only when write pointer has stopped...
 */
int acq_GetOldestDataRaw(rp_channel_t channel, uint32_t* size, int16_t* buffer)
{
    uint32_t pos;

    ECHECK(acq_GetWritePointer(&pos));
    pos++;

    return acq_GetDataRaw(channel, pos, size, buffer);
}

int acq_GetLatestDataRaw(rp_channel_t channel, uint32_t* size, int16_t* buffer)
{
    *size = MIN(*size, ADC_BUFFER_SIZE);

    uint32_t pos;
    ECHECK(acq_GetWritePointer(&pos));

    pos++;

    if ((*size) > pos) {
        pos += ADC_BUFFER_SIZE;
    }
    pos -= (*size);

    return acq_GetDataRaw(channel, pos, size, buffer);
}

int acq_GetDataV(rp_channel_t channel,  uint32_t pos, uint32_t* size, float* buffer)
{
    *size = MIN(*size, ADC_BUFFER_SIZE);

    float gain;
    ECHECK(acq_GetGainV(channel, &gain));

    rp_calib_params_t calib = calib_GetParams();
    int32_t dc_offs = (channel == RP_CH_1 ? calib.fe_ch1_dc_offs : calib.fe_ch2_dc_offs);

    const volatile uint32_t* raw_buffer = getRawBuffer(channel);

    uint32_t cnts;
    for (uint32_t i = 0; i < (*size); ++i) {
        cnts = raw_buffer[(pos + i) % ADC_BUFFER_SIZE];
        buffer[i] = cmn_CnvCntToV(ADC_BITS, cnts, gain, dc_offs, 0.0);
    }

    return RP_OK;
}

int acq_GetDataPosV(rp_channel_t channel,  uint32_t start_pos, uint32_t end_pos, float* buffer, uint32_t *buffer_size)
{
    uint32_t size = getSizeFromStartEndPos(start_pos, end_pos);
    if (size > *buffer_size) {
        return RP_BTS;
    }
    *buffer_size = size;
    return acq_GetDataV(channel, start_pos, buffer_size, buffer);
}

/**
 * Use only when write pointer has stopped...
 */
int acq_GetOldestDataV(rp_channel_t channel, uint32_t* size, float* buffer)
{
    uint32_t pos;

    ECHECK(acq_GetWritePointer(&pos));
    pos++;

    return acq_GetDataV(channel, pos, size, buffer);
}

int acq_GetLatestDataV(rp_channel_t channel, uint32_t* size, float* buffer)
{
    *size = MIN(*size, ADC_BUFFER_SIZE);

    uint32_t pos;
    ECHECK(acq_GetWritePointer(&pos));

    pos++;

    if ((*size) > pos) {
        pos += ADC_BUFFER_SIZE;
    }

    pos -= (*size);
    return acq_GetDataV(channel, pos, size, buffer);
}


int acq_GetBufferSize(uint32_t *size) {
    *size = ADC_BUFFER_SIZE;
    return RP_OK;
}

/**
 * Sets default configuration
 * @return
 */
int acq_SetDefault() {
    ECHECK(acq_SetGain(RP_CH_1, RP_LOW));
    ECHECK(acq_SetGain(RP_CH_2, RP_LOW));
    ECHECK(acq_SetChannelThresholdHyst(RP_CH_1, 0.0));
    ECHECK(acq_SetChannelThresholdHyst(RP_CH_2, 0.0));
    ECHECK(acq_SetDecimation(RP_DEC_1));
    ECHECK(acq_SetSamplingRate(RP_SMP_125M));
    ECHECK(acq_SetAveraging(true));
    ECHECK(acq_SetTriggerSrc(RP_TRIG_SRC_DISABLED));
    ECHECK(acq_SetTriggerDelay(0, false));
    ECHECK(acq_SetTriggerDelayNs(0, false));
    ECHECK(acq_SetTriggerLevel(0));

    return RP_OK;
}
