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

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <stdlib.h>
#include "common.h"
#include "calib.h"

int rp_CalibOpen(char *dev, rp_handle_uio_t *handle) {
    handle->length = RP_CALIB_BASE_SIZE;
    int status = common_Open (dev, handle);
    if (status != RP_OK) {
        return status;
    }

    // allocate local context
    handle->context = (rp_calib_context_t *) malloc(sizeof(rp_calib_context_t));
    // initialization
    rp_calib_context_t *context = (rp_calib_context_t *) handle->context;
    context->chn = 2;
    for (int unsigned i=0; i<context->chn; i++) {
      context->range [i] = 1;
    }
    rp_CalibReadParams(context);
    rp_CalibSetParams(handle);
    return RP_OK;
}

int rp_CalibClose(rp_handle_uio_t *handle) {
    int status = common_Close (handle); 
    if (status != RP_OK) {
        return status;
    }
    // free context
    free(handle->context);
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
int rp_CalibrationReset(rp_handle_uio_t *handle) {
    rp_calib_context_t *context = (rp_calib_context_t *) handle->context;
    for (int range=0; range<2; range++) {
        for (int ch=0; ch<2; ch++) {
            context->acq[ch][range].offset = 0.0;
            context->acq[ch][range].gain   = 1.0;
        }
    }
    for (int ch=0; ch<2; ch++) {
        context->gen[ch].offset = 0.0;
        context->gen[ch].gain   = 1.0;
    }
    rp_CalibSetParams(handle);
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
int rp_CalibReadParams(rp_calib_context_t *context) {
    int    fp;
    size_t size;
    /* sanity check */
    if (context == NULL) {
        return RP_UIA;
    }
    /* open EEPROM device */
    fp = open(RP_CALIB_EEPROM_PATH, O_RDONLY);
    if (!fp) {
        return RP_EOED;
    }
    /* ...and seek to the appropriate storage offset */
    if (lseek(fp, RP_CALIB_EEPROM_ADDR, SEEK_SET) < 0) {
        close(fp);
        return RP_FCA;
    }
    /* read data from EEPROM component and store it to the specified buffer */
    size = read(fp, context, sizeof(rp_calib_context_t));
    if (size != sizeof(rp_calib_context_t)) {
        close(fp);
        return RP_RCA;
    }
    close(fp);
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
int rp_CalibWriteParams(rp_calib_context_t *context) {
    int    fp;
    size_t size;
    /* open EEPROM device */
    fp = open(RP_CALIB_EEPROM_PATH, O_WRONLY);
    if (!fp) {
        return RP_EOED;
    }
    /* ...and seek to the appropriate storage offset */
    if (lseek(fp, RP_CALIB_EEPROM_ADDR, SEEK_SET) < 0) {
        close(fp);
        return RP_FCA;
    }
    /* write data to EEPROM component */
    size = write(fp, context, sizeof(rp_calib_context_t));
    if (size != sizeof(rp_calib_context_t)) {
        close(fp);
        return RP_RCA;
    }
    close(fp);
    return RP_OK;
}

/**
 * @return RP_OK
 */
int rp_CalibGetParams(rp_handle_uio_t *handle) {
    rp_calib_context_t *context = (rp_calib_context_t *) handle->context;
    rp_calib_regset_t  *regset  = (rp_calib_regset_t  *) handle->regset ;
    const int ratio_dwm = 1 << (RP_CALIB_DWM-2);
    const int ratio_dws = 1 << (RP_CALIB_DWS-1);
    for (int ch=0; ch<context->chn; ch++) {
        float scale = context->range[ch] ? 20 : 1;
        context->acq[ch][context->range[ch]].offset = ((float) ioread32 (&regset->acq[ch].sum)) * scale / ratio_dws;
        context->acq[ch][context->range[ch]].gain   = ((float) ioread32 (&regset->acq[ch].mul)) * scale / ratio_dwm;
    }
    for (int ch=0; ch<context->chn; ch++) {
        context->gen[ch].offset                     = ((float) ioread32 (&regset->gen[ch].sum))         / ratio_dws;
        context->gen[ch].gain                       = ((float) ioread32 (&regset->gen[ch].mul))         / ratio_dwm;
    }
    return RP_OK;
}

/**
 * @return RP_OK
 */
int rp_CalibSetParams(rp_handle_uio_t *handle) {
    rp_calib_context_t *context = (rp_calib_context_t *) handle->context;
    rp_calib_regset_t  *regset  = (rp_calib_regset_t  *) handle->regset ;
    const int ratio_dwm = 1 << (RP_CALIB_DWM-2);
    const int ratio_dws = 1 << (RP_CALIB_DWS-1);
    for (int ch=0; ch<context->chn; ch++) {
        float scale = context->range[ch] ? 20 : 1;
        iowrite32 ((int32_t) (context->acq[ch][context->range[ch]].offset / scale * ratio_dws), &regset->acq[ch].sum);
        iowrite32 ((int32_t) (context->acq[ch][context->range[ch]].gain   / scale * ratio_dwm), &regset->acq[ch].mul);
    }
    for (int ch=0; ch<context->chn; ch++) {
        iowrite32 ((int32_t) (context->gen[ch].offset                             * ratio_dws), &regset->gen[ch].sum);
        iowrite32 ((int32_t) (context->gen[ch].gain                               * ratio_dwm), &regset->gen[ch].mul);
    }
    return RP_OK;
}
