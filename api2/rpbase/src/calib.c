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
#include "calib.h"

// Cached parameter values.
static rp_calib_params_t calib;

int calib_Init() {
    calib_ReadParams(&calib);
    calib_SetParams(&calib);
    return RP_OK;
}

int calib_Release() {
    return RP_OK;
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
    if (calib_params == NULL) {
        return RP_UIA;
    }
    /* open EEPROM device */
    fp = fopen(eeprom_device, "r");
    if (fp == NULL) {
        return RP_EOED;
    }
    /* ...and seek to the appropriate storage offset */
    if (fseek(fp, eeprom_calib_off, SEEK_SET) < 0) {
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
    return RP_OK;
}

int calib_WriteParams(rp_calib_params_t *calib_params) {
    FILE   *fp;
    size_t  size;
    /* open EEPROM device */
    fp = fopen(eeprom_device, "w+");
    if (fp == NULL) {
        return RP_EOED;
    }
    /* ...and seek to the appropriate storage offset */
    if (fseek(fp, eeprom_calib_off, SEEK_SET) < 0) {
        fclose(fp);
        return RP_FCA;
    }
    /* write data to EEPROM component */
    size = fwrite(calib_params, sizeof(char), sizeof(rp_calib_params_t), fp);
    if (size != sizeof(rp_calib_params_t)) {
        fclose(fp);
        return RP_RCA;
    }
    fclose(fp);
    return RP_OK;
}

int calib_Reset() {
    for (int range=0; range<2; range++) {
        for (int ch=0; ch<2; ch++) {
            calib.acq[ch][range].offset = 0.0;
            calib.acq[ch][range].gain   = 1.0;
        }
    }
    for (int ch=0; ch<2; ch++) {
        calib.gen[ch].offset = 0.0;
        calib.gen[ch].gain   = 1.0;
    }
    calib_SetParams(&calib);
    return RP_OK;
}

/**
 * @return RP_OK
 */
int calib_GetParams(rp_calib_params_t *calib_params) {
    return RP_OK;
}

/**
 * @return RP_OK
 */
int calib_SetParams(rp_calib_params_t *calib_params) {
    return RP_OK;
}

