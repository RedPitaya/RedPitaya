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
#include "rp.h"
#include "common.h"
#include "generate.h"
#include "calib.h"

int calib_ReadParams(rp_calib_params_t *calib_params);

static const char eeprom_device[]="/sys/bus/i2c/devices/0-0050/eeprom";
static const int  eeprom_calib_off=0x0008;

// Cached parameter values.
static rp_calib_params_t calib;

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
    calib_params->fe_ch1_dc_offs = 585;
    calib_params->fe_ch2_dc_offs = 585;
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
    size = fwrite(&calib_params, sizeof(char), sizeof(rp_calib_params_t), fp);
    if(size != sizeof(rp_calib_params_t)) {
        fclose(fp);
        return RP_RCA;
    }
    fclose(fp);

    return RP_OK;
}

uint32_t calib_GetFrontEndScale(rp_channel_t channel, rp_pinState_t gain) {
    if (gain == RP_HIGH) {
        return (channel == RP_CH_1 ? calib.fe_ch1_fs_g_hi : calib.fe_ch2_fs_g_hi);
    }
    else {
        return (channel == RP_CH_1 ? calib.fe_ch1_fs_g_lo : calib.fe_ch2_fs_g_lo);
    }
}

int calib_SetFrontEndOffset(rp_channel_t channel) {
    rp_calib_params_t params;
    ECHECK(calib_ReadParams(&params));

    /* Reset current calibration parameters*/
    CHECK_OUTPUT(params.fe_ch1_dc_offs = 0,
                 params.fe_ch2_dc_offs = 0)
    /* Acquire uses this calibration parameters - reset them */
    calib = params;

    CHECK_OUTPUT(params.fe_ch1_dc_offs = -calib_GetDataMedian(channel),
                 params.fe_ch2_dc_offs = -calib_GetDataMedian(channel))

    /* Set new local parameter */
    ECHECK(calib_WriteParams(params));
    return calib_Init();
}

int calib_SetFrontEndScaleLV(rp_channel_t channel, float referentialVoltage) {
    rp_calib_params_t params;
    ECHECK(calib_ReadParams(&params));

    /* Reset current calibration parameters*/
    CHECK_OUTPUT(params.fe_ch1_fs_g_lo = cmn_CalibFullScaleFromVoltage(1),
            params.fe_ch2_fs_g_lo = cmn_CalibFullScaleFromVoltage(1))
    /* Acquire uses this calibration parameters - reset them */
    calib = params;

    /* Calculate real max adc voltage */
    float value = calib_GetDataMedianFloat(channel, RP_LOW);
    uint32_t calibValue = cmn_CalibFullScaleFromVoltage(referentialVoltage / value);

    CHECK_OUTPUT(params.fe_ch1_fs_g_lo = calibValue,
                 params.fe_ch2_fs_g_lo = calibValue )

    /* Set new local parameter */
    ECHECK(calib_WriteParams(params));
    return calib_Init();
}

int calib_SetFrontEndScaleHV(rp_channel_t channel, float referentialVoltage) {
    rp_calib_params_t params;
    ECHECK(calib_ReadParams(&params));
    /* Reset current calibration parameters*/
    CHECK_OUTPUT(params.fe_ch1_fs_g_hi = cmn_CalibFullScaleFromVoltage(1),
                 params.fe_ch2_fs_g_hi = cmn_CalibFullScaleFromVoltage(1))
    /* Acquire uses this calibration parameters - reset them */
    calib = params;

    /* Calculate real max adc voltage */
    float value = calib_GetDataMedianFloat(channel, RP_HIGH);
    uint32_t calibValue = cmn_CalibFullScaleFromVoltage(referentialVoltage / value);

    CHECK_OUTPUT(params.fe_ch1_fs_g_hi = calibValue,
                 params.fe_ch2_fs_g_hi = calibValue )

    /* Set new local parameter */
    ECHECK(calib_WriteParams(params));
    return calib_Init();
}

int calib_SetBackEndOffset(rp_channel_t channel) {
    rp_calib_params_t params;
    ECHECK(calib_ReadParams(&params));

    /* Reset current calibration parameters*/
    CHECK_OUTPUT(params.be_ch1_dc_offs = 0,
                 params.be_ch2_dc_offs = 0)
    /* Generate uses this calibration parameters - reset them */
    calib = params;

    /* Generate zero signal */
    ECHECK(rp_GenReset());
    ECHECK(rp_GenWaveform(channel, RP_WAVEFORM_SINE));
    ECHECK(rp_GenAmp(channel, 0));
    ECHECK(rp_GenOutEnable(channel));

    CHECK_OUTPUT(params.be_ch1_dc_offs = -calib_GetDataMedian(channel),
                 params.be_ch2_dc_offs = -calib_GetDataMedian(channel))

    /* Set new local parameter */
    ECHECK(calib_WriteParams(params));
    return calib_Init();
}

int calib_SetBackEndScale(rp_channel_t channel) {
    rp_calib_params_t params;
    ECHECK(calib_ReadParams(&params));

    /* Reset current calibration parameters*/
    CHECK_OUTPUT(params.be_ch1_fs = 0,
                 params.be_ch2_fs = 0)
    /* Generate uses this calibration parameters - reset them */
    calib = params;

    /* Generate constant signal signal */
    ECHECK(rp_GenReset());
    ECHECK(rp_GenWaveform(channel, RP_WAVEFORM_PWM));
    ECHECK(rp_GenDutyCycle(channel, 1));
    ECHECK(rp_GenAmp(channel, CONSTANT_SIGNAL_AMPLITUDE));
    ECHECK(rp_GenOutEnable(channel));

    /* Calculate real max adc voltage */
    float value = calib_GetDataMedianFloat(channel, RP_LOW);
    uint32_t calibValue = cmn_CalibFullScaleFromVoltage((float) (CONSTANT_SIGNAL_AMPLITUDE / value));

    CHECK_OUTPUT(params.be_ch1_fs = calibValue,
                 params.be_ch2_fs = calibValue)

    /* Set new local parameter */
    ECHECK(calib_WriteParams(params));
    return calib_Init();
}

int32_t calib_GetDataMedian(rp_channel_t channel) {
    /* Acquire data */
    ECHECK(rp_AcqReset());
    ECHECK(rp_AcqStart());

    int16_t data[BUFFER_LENGTH];
    uint32_t bufferSize = (uint32_t) BUFFER_LENGTH;
    ECHECK(rp_AcqGetDataRaw(channel, 0, &bufferSize, data));

    ECHECK(rp_AcqStop());

    /* Calculate median value */
    qsort(data, BUFFER_LENGTH, sizeof(int16_t), int16cmp);
    return data[BUFFER_LENGTH/2];
}

float calib_GetDataMedianFloat(rp_channel_t channel, rp_pinState_t gain) {
    ECHECK(rp_AcqReset());
    ECHECK(rp_AcqSetGain(channel, gain));
    ECHECK(rp_AcqStart());
    usleep(10000);
    int BUF_SIZE = BUFFER_LENGTH;

    float data[BUF_SIZE];
    uint32_t bufferSize = (uint32_t) BUF_SIZE;
    ECHECK(rp_AcqGetDataV(channel, 0, &bufferSize, data));

    ECHECK(rp_AcqStop());

    /* Calculate median value */
    qsort(data, (size_t) BUF_SIZE, sizeof(float), floatCmp);
    return data[BUF_SIZE/2];
}