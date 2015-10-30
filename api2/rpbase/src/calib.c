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

int calib_setCachedParams() {
	fprintf(stderr, "write FAILSAFE PARAMS\n");
    ECHECK(calib_WriteParams(failsafa_params));
    calib = failsafa_params;

    return 0;
}
