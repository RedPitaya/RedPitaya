/**
 * $Id: $
 *
 * @brief Red Pitaya Calibration Module.
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <stdlib.h>
#include <unistd.h>
#include "redpitaya/rp1.h"
#include "common.h"
#include "calib.h"
#include "gen.h"

#define CALIB_MAGIC 0xAABBCCDD

static const char eeprom_device[]="/sys/bus/i2c/devices/0-0050/eeprom";
static const int  eeprom_calib_off=0x0008;

// Cached parameter values.
static rp_calib_params_t calib, failsafe_params;

int calib_Init() {
    calib_ReadParams(&calib);
    return RP_OK;
}

int calib_Release() {
    return RP_OK;
}

/**
 * Returns cached parameter values
 * @return Cached parameters.
 */
rp_calib_params_t calib_GetParams() {
    return calib;
}

/**
 * @brief Read calibration parameters from EEPROM device.
 *
 * Function reads calibration parameters from EEPROM device and stores them to the
 * specified buffer. Communication to the EEPROM device is taken place through
 * appropriate system driver accessed through the file system device
 * /sys/bus/i2c/devices/0-0050/eeprom.
 *
 * @param[out]   calib_params  Pointer to destination buffer.
 * @retval       0 Success
 * @retval       >0 Failure
 *
 */
int calib_ReadParams(rp_calib_params_t *calib_params) {
    FILE   *fp;
    size_t  size;

    /* sanity check */
    if(calib_params == NULL)
        return RP_UIA;
    /* open EEPROM device */

    fp = fopen(eeprom_device, "r");
    if(fp == NULL)
        return RP_EOED;

    /* ...and seek to the appropriate storage offset */
    if(fseek(fp, eeprom_calib_off, SEEK_SET) < 0) {
        fclose(fp);
        return RP_FCA;
    }

    /* read data from EEPROM component and store it to the specified buffer */
    size = fread(calib_params, sizeof(char), sizeof(rp_calib_params_t), fp);
    if(size != sizeof(rp_calib_params_t)) {
        fclose(fp);
        return RP_RCA;
    }
    fclose(fp);

    if (calib_params->magic != CALIB_MAGIC) {
        calib_params->fe_hi_offs[0] = calib_params->fe_lo_offs[0];
        calib_params->fe_hi_offs[1] = calib_params->fe_lo_offs[1];
    }
    return 0;
}

int calib_WriteParams(rp_calib_params_t calib_params) {
    FILE   *fp;
    size_t  size;

    /* open EEPROM device */
    fp = fopen(eeprom_device, "w+");
    if(fp == NULL)
        return RP_EOED;

    /* ...and seek to the appropriate storage offset */
    if(fseek(fp, eeprom_calib_off, SEEK_SET) < 0) {
        fclose(fp);
        return RP_FCA;
    }

    /* write data to EEPROM component */
    calib_params.magic = CALIB_MAGIC;
    size = fwrite(&calib_params, sizeof(char), sizeof(rp_calib_params_t), fp);
    if(size != sizeof(rp_calib_params_t)) {
        fclose(fp);
        return RP_RCA;
    }
    fclose(fp);
    return RP_OK;
}

void calib_SetToZero() {
    for (int unsigned ch=0; ch<2; ch++) {
        calib.be_dc_offs[ch] = 0;
        calib.fe_lo_offs[ch] = 0;
        calib.fe_hi_offs[ch] = 0;
        calib.be_fs     [ch] = calib_FullScaleFromVoltage(1);
        calib.fe_fs_g[1][ch] = calib_FullScaleFromVoltage(20);
        calib.fe_fs_g[0][ch] = calib_FullScaleFromVoltage(1);
    }
}

/* saturation */
int32_t calib_Saturate(int unsigned bits, int32_t cnt) {
    int32_t max =  (1 << (bits - 1)) - 1;
    int32_t min = -(1 << (bits - 1))    ;
    if      (cnt > max)  return max;
    else if (cnt < min)  return min;
    else                 return cnt;
}

/**
* @brief Converts calibration Full scale to volts. Scale is usually read from EPROM calibration parameters.
* If parameter is 0, a factor 1 is returned -> no scaling.
*
* @param[in] fullScaleGain value of full voltage scale
* @retval Scale in volts
*/
float calib_FullScaleToVoltage(uint32_t cnt) {
    return (cnt ? (float)cnt  * 100.0 / (float)((uint64_t)1<<32)
                : 1.0);
}

/**
* @brief Converts scale voltage to calibration Full scale. Result is usually written to EPROM calibration parameters.
*
* @param[in] voltageScale Scale value in voltage
* @retval Scale in volts
*/
int32_t calib_FullScaleFromVoltage(float voltage) {
    return (int32_t) (voltage / 100.0 * (float)((uint64_t)1<<32));
}

int32_t calib_GetAcqOffset(int unsigned channel, int unsigned gain) {
    return (gain == 1 ? calib.fe_hi_offs[channel]
                      : calib.fe_lo_offs[channel]);
}

float calib_GetAcqScale(int unsigned channel, int unsigned gain) {
    return (calib_FullScaleToVoltage(calib.fe_fs_g[gain][channel]) * GAIN_V(gain));
}

int32_t calib_GetGenOffset(int unsigned channel) {
    return (calib.be_dc_offs[channel]);
}

float calib_GetGenScale(int unsigned channel) {
    return (calib_FullScaleToVoltage(calib.be_fs[channel]));
}

int calib_SetAcqOffset(int unsigned channel, int unsigned gain, rp_calib_params_t* out_params) {
    rp_calib_params_t params;
    calib_ReadParams(&params);
    failsafe_params = params;

    /* Reset current calibration parameters*/
    if (gain == 0)  params.fe_lo_offs[channel] = 0;
    else            params.fe_hi_offs[channel] = 0;
    /* Acquire uses this calibration parameters - reset them */
    calib = params;

    if (gain == 0)  params.fe_lo_offs[channel] = calib_GetDataMedian(channel, 0);
    else            params.fe_hi_offs[channel] = calib_GetDataMedian(channel, 1);

    /* Set new local parameter */
    if (out_params) {
        // *out_params = params;
        if (gain == 0)  out_params->fe_lo_offs[channel] = params.fe_lo_offs[channel];
        else            out_params->fe_hi_offs[channel] = params.fe_hi_offs[channel];
    } else
        calib_WriteParams(params);
    return calib_Init();
}

int calib_SetAcqScale(int unsigned channel, int unsigned gain, float referentialVoltage, rp_calib_params_t* out_params) {
    rp_calib_params_t params;
    calib_ReadParams(&params);
    failsafe_params = params;

    /* Reset current calibration parameters*/
     params.fe_fs_g[gain][channel] = calib_FullScaleFromVoltage(GAIN_V(gain));
    /* Acquire uses this calibration parameters - reset them */
    calib = params;

    /* Calculate real max adc voltage */
    //float value = calib_GetDataMedianFloat(channel, 0); TODO
    float value = calib_GetDataMedianFloat(channel, gain);
    uint32_t calibValue = calib_FullScaleFromVoltage(GAIN_V(gain) * referentialVoltage / value);

    params.fe_fs_g[gain][channel] = calibValue;

    /* Set new local parameter */
    if (out_params) {
        out_params->fe_fs_g[gain][channel] = params.fe_fs_g[gain][channel];
    } else
        calib_WriteParams(params);
    return calib_Init();
}

int calib_SetGenOffset(int unsigned channel) {
    rp_calib_params_t params;
    calib_ReadParams(&params);

    failsafe_params = params;
    /* Reset current calibration parameters*/
    params.be_dc_offs[channel] = 0;
    /* Generate uses this calibration parameters - reset them */
    calib = params;

    /* Generate zero signal */
    rp_GenReset();
    rp_GenWaveform(channel, RP_WAVEFORM_SINE);
    rp_GenAmp(channel, 0);
    rp_GenOffset(channel, 0);
    rp_GenOutEnable(channel);

    params.be_dc_offs[channel] = -calib_GetDataMedian(channel, 0),

    /* Set new local parameter */
    calib_WriteParams(params);
    return calib_Init();
}

int calib_SetGenScale(int unsigned channel) {
    rp_calib_params_t params;
    calib_ReadParams(&params);
    failsafe_params = params;

    /* Reset current calibration parameters*/
    params.be_fs[channel] = calib_FullScaleFromVoltage(1);
    /* Generate uses this calibration parameters - reset them */
    calib = params;

    /* Generate constant signal signal */
    rp_GenReset();
    rp_GenWaveform(channel, RP_WAVEFORM_PWM);
    rp_GenDutyCycle(channel, 1);
    rp_GenAmp(channel, CONSTANT_SIGNAL_AMPLITUDE);
    rp_GenOffset(channel, 0);
    rp_GenOutEnable(channel);

    /* Calculate real max adc voltage */
    float value = calib_GetDataMedianFloat(channel, 0);
    uint32_t calibValue = calib_FullScaleFromVoltage((float) (value / CONSTANT_SIGNAL_AMPLITUDE));

    params.be_fs[channel] = calibValue;

    /* Set new local parameter */
    calib_WriteParams(params);
    return calib_Init();
}

static int getGenAmp(int unsigned channel, float amp, float* min, float* max) {
    rp_GenReset();
    rp_GenWaveform(channel, RP_WAVEFORM_SINE);
    rp_GenAmp(channel, amp);
    rp_GenOffset(channel, 0);
    rp_GenOutEnable(channel);

    return calib_GetDataMinMaxFloat(channel, 0, min, max);
}

static int getGenDC_int(int unsigned channel, float dc) {
    rp_GenReset();
    rp_GenWaveform(channel, RP_WAVEFORM_DC);
    rp_GenAmp(channel, 0);
    rp_GenOffset(channel, dc);
    rp_GenOutEnable(channel);

    return calib_GetDataMedian(channel, 0);
}

int calib_CalibrateGen(int unsigned channel, rp_calib_params_t* out_params) {
    rp_calib_params_t params;
    calib_ReadParams(&params);

    /* Reset current calibration parameters*/
    params.be_fs[channel] = calib_FullScaleFromVoltage(1);

    params.be_dc_offs[channel] = 0;

    /* Generate uses this calibration parameters - reset them */
    calib = params;

    float value1, value2;
    getGenAmp(channel, CONSTANT_SIGNAL_AMPLITUDE, &value1, &value2);
    float scale = (value2 - value1) / (2.f * CONSTANT_SIGNAL_AMPLITUDE);
    fprintf(stderr, "v1: %f, v2: %f, scale: %f\n", value1, value2, scale);

    int off1 = getGenDC_int(channel, -CONSTANT_SIGNAL_AMPLITUDE);
    int off2 = getGenDC_int(channel, 0);
    int off3 = getGenDC_int(channel, CONSTANT_SIGNAL_AMPLITUDE);
    int offset = -(off1 + off2 + off3) / 3;

    fprintf(stderr, "off1: %d, off2: %d, off3: %d, off: %d\n", off1, off2, off3, offset);
    /* Generate constant signal signal */
    uint32_t calibValue = calib_FullScaleFromVoltage(scale);

    params.be_fs[channel] = calibValue;

    params.be_dc_offs[channel] = offset;

    /* Set new local parameter */
    if (out_params) {
        // *out_params = params;
        out_params->be_fs[channel] = params.be_fs[channel];
        out_params->be_dc_offs[channel] = params.be_dc_offs[channel];
    } else
        calib_WriteParams(params);
    return calib_Init();
}

int calib_Reset() {
    calib_SetToZero();
    calib_WriteParams(calib);
    return calib_Init();
}

int32_t calib_GetDataMedian(int unsigned channel, int unsigned gain) {
    /* Acquire data */
    rp_AcqReset();
    rp_AcqSetGain(channel, gain);
    rp_AcqSetDecimationFactor(64);
    rp_AcqStart();
    rp_AcqSetTriggerSrc(RP_TRIG_SRC_NOW);
    usleep(1000000);
    rp_AcqStop();

    int16_t data[BUFFER_LENGTH];
    uint32_t bufferSize = (uint32_t) BUFFER_LENGTH;
    rp_AcqGetDataRaw(channel, 0, &bufferSize, data);

    long long avg = 0;
    for(int i = 0; i < BUFFER_LENGTH; ++i)
        avg += data[i];

    avg /= BUFFER_LENGTH;
    fprintf(stderr, "\ncalib_GetDataMedian: avg = %d\n", (int32_t)avg);
    return avg;
}

float calib_GetDataMedianFloat(int unsigned channel, int unsigned gain) {
    rp_AcqReset();
    rp_AcqSetGain(channel, gain);
    rp_AcqSetDecimationFactor(64);
    rp_AcqStart();
    rp_AcqSetTriggerSrc(RP_TRIG_SRC_NOW);
    usleep(1000000);
    int BUF_SIZE = BUFFER_LENGTH;

    rp_AcqStop();

    float data[BUF_SIZE];
    uint32_t bufferSize = (uint32_t) BUF_SIZE;
    rp_AcqGetDataV(channel, 0, &bufferSize, data);

    double avg = 0;
    for(int i = 0; i < BUFFER_LENGTH; ++i)
        avg += data[i];

    avg /= BUFFER_LENGTH;
    fprintf(stderr, "\ncalib_GetDataMedianFloat: avg = %f\n", (float)avg);
    return avg;
}

int calib_GetDataMinMaxFloat(int unsigned channel, int unsigned gain, float* min, float* max) {
    rp_AcqReset();
    rp_AcqSetGain(channel, gain);
    rp_AcqSetDecimationFactor(64);
    rp_AcqStart();
    rp_AcqSetTriggerSrc(RP_TRIG_SRC_NOW);
    usleep(1000000);
    int BUF_SIZE = BUFFER_LENGTH;

    rp_AcqStop();

    float data[BUF_SIZE];
    uint32_t bufferSize = (uint32_t) BUF_SIZE;
    rp_AcqGetDataV(channel, 0, &bufferSize, data);

    float _min = data[0];
    float _max = data[0];
    for(int i = 1; i < BUFFER_LENGTH; ++i) {
        _min = (_min > data[i]) ? data[i] : _min;
        _max = (_max < data[i]) ? data[i] : _max;
    }

    fprintf(stderr, "\ncalib_GetDataMinMaxFloat: min = %f, max = %f\n", _min, _max);
    *min = _min;
    *max = _max;
    return RP_OK;
}

int calib_setCachedParams() {
    fprintf(stderr, "write FAILSAFE PARAMS\n");
    calib_WriteParams(failsafe_params);
    calib = failsafe_params;

    return 0;
}
