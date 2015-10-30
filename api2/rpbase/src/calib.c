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
#include "redpitaya/rp.h"
#include "common.h"
#include "generate.h"
#include "calib.h"

#define CALIB_MAGIC 0xAABBCCDD

int calib_ReadParams(rp_calib_params_t *calib_params);

static const char eeprom_device[]="/sys/bus/i2c/devices/0-0050/eeprom";
static const int  eeprom_calib_off=0x0008;

// Cached parameter values.
static rp_calib_params_t calib, failsafa_params;

int calib_Init()
{
    ECHECK(calib_ReadParams(&calib));
    return RP_OK;
}

int calib_Release()
{
    return RP_OK;
}

/**
 * Returns cached parameter values
 * @return Cached parameters.
 */
rp_calib_params_t calib_GetParams()
{
    return calib;
}

/**
* @brief Converts scale voltage to calibration Full scale. Result is usually written to EPROM calibration parameters.
*
* @param[in] voltageScale Scale value in voltage
* @retval Scale in volts
*/
static uint32_t cmn_CalibFullScaleFromVoltage(float voltageScale) {
    return (uint32_t) (voltageScale / 100.0 * ((uint64_t)1<<32));
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
int calib_ReadParams(rp_calib_params_t *calib_params)
{
    FILE   *fp;
    size_t  size;

    /* sanity check */
    if(calib_params == NULL) {
        return RP_UIA;
    }

    /* open EEPROM device */
    fp = fopen(eeprom_device, "r");
    if(fp == NULL) {
        return RP_EOED;
    }

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
		calib_params->fe_ch1_hi_offs = calib_params->fe_ch1_lo_offs;
		calib_params->fe_ch2_hi_offs = calib_params->fe_ch2_lo_offs;
	}

    return 0;
}


/*
 * Initialize calibration parameters to default values.

int calib_GetDefaultParams(rp_calib_params_t *calib_params)
{
    calib_params->fe_ch1_fs_g_hi = 28101971; // 0.6543 [V]
    calib_params->fe_ch2_fs_g_hi = 28101971; // 0.6543 [V]
    calib_params->fe_ch1_fs_g_lo = 625682246; // 14.56 [V]
    calib_params->fe_ch2_fs_g_lo = 625682246; // 14.56 [V]
    calib_params->fe_ch1_lo_offs = 585;
    calib_params->fe_ch2_lo_offs = 585;
    calib_params->fe_ch1_hi_offs = 585;
    calib_params->fe_ch2_hi_offs = 585;
    calib_params->be_ch1_fs = 42949673; // 1 [V]
    calib_params->be_ch2_fs = 42949673; // 1 [V]
    calib_params->be_ch1_dc_offs = 0x3eac;
    calib_params->be_ch2_dc_offs = 0x3eac;

    return 0;
}
 */

int calib_WriteParams(rp_calib_params_t calib_params) {
    FILE   *fp;
    size_t  size;

    /* open EEPROM device */
    fp = fopen(eeprom_device, "w+");
    if(fp == NULL) {
        return RP_EOED;
    }

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
    calib.be_ch1_dc_offs = 0;
    calib.be_ch2_dc_offs = 0;
    calib.fe_ch1_lo_offs = 0;
    calib.fe_ch2_lo_offs = 0;
    calib.fe_ch1_hi_offs = 0;
    calib.fe_ch2_hi_offs = 0;

    calib.be_ch1_fs      = cmn_CalibFullScaleFromVoltage(1);
    calib.be_ch2_fs      = cmn_CalibFullScaleFromVoltage(1);
    calib.fe_ch1_fs_g_lo = cmn_CalibFullScaleFromVoltage(20);
    calib.fe_ch1_fs_g_hi = cmn_CalibFullScaleFromVoltage(1);
    calib.fe_ch2_fs_g_lo = cmn_CalibFullScaleFromVoltage(20);
    calib.fe_ch2_fs_g_hi = cmn_CalibFullScaleFromVoltage(1);
}

int calib_Reset() {
    calib_SetToZero();
    ECHECK(calib_WriteParams(calib));
    return calib_Init();
}

int32_t calib_GetDataMedian(rp_channel_t channel, rp_pinState_t gain) {
    /* Acquire data */
    ECHECK(rp_AcqReset());
    ECHECK(rp_AcqSetGain(channel, gain));
    ECHECK(rp_AcqSetDecimation(RP_DEC_64));
    ECHECK(rp_AcqStart());
    ECHECK(rp_AcqSetTriggerSrc(RP_TRIG_SRC_NOW));
    usleep(1000000);
    ECHECK(rp_AcqStop());

    int16_t data[BUFFER_LENGTH];
    uint32_t bufferSize = (uint32_t) BUFFER_LENGTH;
    ECHECK(rp_AcqGetDataRaw(channel, 0, &bufferSize, data));

    long long avg = 0;
    for(int i = 0; i < BUFFER_LENGTH; ++i)
        avg += data[i];

    avg /= BUFFER_LENGTH;
    fprintf(stderr, "\ncalib_GetDataMedian: avg = %d\n", (int32_t)avg);
    return avg;
}

float calib_GetDataMedianFloat(rp_channel_t channel, rp_pinState_t gain) {
    ECHECK(rp_AcqReset());
    ECHECK(rp_AcqSetGain(channel, gain));
    ECHECK(rp_AcqSetDecimation(RP_DEC_64));
    ECHECK(rp_AcqStart());
    ECHECK(rp_AcqSetTriggerSrc(RP_TRIG_SRC_NOW));
    usleep(1000000);
    int BUF_SIZE = BUFFER_LENGTH;

    ECHECK(rp_AcqStop());

    float data[BUF_SIZE];
    uint32_t bufferSize = (uint32_t) BUF_SIZE;
    ECHECK(rp_AcqGetDataV(channel, 0, &bufferSize, data));

    double avg = 0;
    for(int i = 0; i < BUFFER_LENGTH; ++i)
        avg += data[i];

    avg /= BUFFER_LENGTH;
    fprintf(stderr, "\ncalib_GetDataMedianFloat: avg = %f\n", (float)avg);
    return avg;
}

int calib_GetDataMinMaxFloat(rp_channel_t channel, rp_pinState_t gain, float* min, float* max) {
    ECHECK(rp_AcqReset());
    ECHECK(rp_AcqSetGain(channel, gain));
    ECHECK(rp_AcqSetDecimation(RP_DEC_64));
    ECHECK(rp_AcqStart());
    ECHECK(rp_AcqSetTriggerSrc(RP_TRIG_SRC_NOW));
    usleep(1000000);
    int BUF_SIZE = BUFFER_LENGTH;

    ECHECK(rp_AcqStop());

    float data[BUF_SIZE];
    uint32_t bufferSize = (uint32_t) BUF_SIZE;
    ECHECK(rp_AcqGetDataV(channel, 0, &bufferSize, data));

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
    ECHECK(calib_WriteParams(failsafa_params));
    calib = failsafa_params;

    return 0;
}
