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

// context (copy of EEPROM data)

// register set
static volatile regset_calib_t *regset = NULL;

int calib_Init() {
    cmn_Map(RP_CALIB_BASE_SIZE, RP_CALIB_BASE_ADDR, (void**) &regset);
    int range[2] = {0,0};
    rp_calib_params_t context;
    rp_CalibReadParams(&context);
    rp_CalibSetParams(&context, range);
    return RP_OK;
}

int calib_Release() {
    cmn_Unmap(RP_CALIB_BASE_SIZE, (void**) &regset);
    return RP_OK;
}

/**
 * @brief Reset calibration parameters to default values and write them into regset.
 *
 * Default values for calibration are based on an ideal hardware.
 *
 * @param[out]   calib_params  Pointer to destination buffer.
 * @retval       RP_OK - Success
 */
int rp_CalibrationReset() {
    int range[2] = {0,0};
    rp_calib_params_t context;
    for (int range=0; range<2; range++) {
        for (int ch=0; ch<2; ch++) {
            context.acq[ch][range].offset = 0.0;
            context.acq[ch][range].gain   = 1.0;
        }
    }
    for (int ch=0; ch<2; ch++) {
        context.gen[ch].offset = 0.0;
        context.gen[ch].gain   = 1.0;
    }
    rp_CalibSetParams(&context, range);
    return RP_OK;
}

/**
 * @brief Read calibration parameters from EEPROM device.
 *
 * Function reads calibration parameters from EEPROM device and stores them to the
 * specified structure.
 *
 * @param[out]   context  Pointer to destination buffer.
 * @retval       RP_OK - Success
 * @retval       else  - Failure
 */
int rp_CalibReadParams(rp_calib_params_t *context) {
    FILE   *fp;
    size_t  size;
    /* sanity check */
    if (context == NULL) {
        return RP_UIA;
    }
    /* open EEPROM device */
    fp = fopen(RP_CALIB_EEPROM_PATH, "r");
    if (fp == NULL) {
        return RP_EOED;
    }
    /* ...and seek to the appropriate storage offset */
    if (fseek(fp, RP_CALIB_EEPROM_ADDR, SEEK_SET) < 0) {
        fclose(fp);
        return RP_FCA;
    }
    /* read data from EEPROM component and store it to the specified buffer */
    size = fread(context, sizeof(char), sizeof(rp_calib_params_t), fp);
    if(size != sizeof(rp_calib_params_t)) {
        fclose(fp);
        return RP_RCA;
    }
    fclose(fp);
    return RP_OK;
}

/**
 * @brief Write calibration parameters to EEPROM device.
 *
 * Function writes calibration parameters from specified structure to EEPROM devic.
 *
 * @param[out]   context  Pointer to destination buffer.
 * @retval       RP_OK - Success
 * @retval       else  - Failure
 */
int rp_CalibWriteParams(rp_calib_params_t *context) {
    FILE   *fp;
    size_t  size;
    /* open EEPROM device */
    fp = fopen(RP_CALIB_EEPROM_PATH, "w+");
    if (fp == NULL) {
        return RP_EOED;
    }
    /* ...and seek to the appropriate storage offset */
    if (fseek(fp, RP_CALIB_EEPROM_ADDR, SEEK_SET) < 0) {
        fclose(fp);
        return RP_FCA;
    }
    /* write data to EEPROM component */
    size = fwrite(context, sizeof(char), sizeof(rp_calib_params_t), fp);
    if (size != sizeof(rp_calib_params_t)) {
        fclose(fp);
        return RP_RCA;
    }
    fclose(fp);
    return RP_OK;
}

/**
 * @return RP_OK
 */
int rp_CalibGetParams(rp_calib_params_t *context, int range[2]) {
    const int ratio_dwm = 1 << (RP_CALIB_DWM-2);
    const int ratio_dws = 1 << (RP_CALIB_DWS-1);
    for (int ch=0; ch<2; ch++) {
        float scale = range[ch] ? 20 : 1;
        context->acq[ch][range[ch]].offset = ((float) ioread32 (&regset->acq[ch].sum)) * scale / ratio_dws;
        context->acq[ch][range[ch]].gain   = ((float) ioread32 (&regset->acq[ch].mul)) * scale / ratio_dwm;
    }
    for (int ch=0; ch<2; ch++) {
        context->gen[ch].offset            = ((float) ioread32 (&regset->gen[ch].sum))         / ratio_dws;
        context->gen[ch].gain              = ((float) ioread32 (&regset->gen[ch].mul))         / ratio_dwm;
    }
    return RP_OK;
}

/**
 * @return RP_OK
 */
int rp_CalibSetParams(rp_calib_params_t *context, int range[2]) {
    const int ratio_dwm = 1 << (RP_CALIB_DWM-2);
    const int ratio_dws = 1 << (RP_CALIB_DWS-1);
    for (int ch=0; ch<2; ch++) {
        float scale = range[ch] ? 20 : 1;
        iowrite32 ((int32_t) (context->acq[ch][range[ch]].offset / scale * ratio_dws), &regset->acq[ch].sum);
        iowrite32 ((int32_t) (context->acq[ch][range[ch]].gain   / scale * ratio_dwm), &regset->acq[ch].mul);
    }
    for (int ch=0; ch<2; ch++) {
        iowrite32 ((int32_t) (context->gen[ch].offset                    * ratio_dws), &regset->gen[ch].sum);
        iowrite32 ((int32_t) (context->gen[ch].gain                      * ratio_dwm), &regset->gen[ch].mul);
    }
    return RP_OK;
}
