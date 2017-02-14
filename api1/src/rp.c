/**
 * $Id: $
 *
 * @brief Red Pitaya library API interface implementation
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

#include "common.h"
#include "housekeeping.h"
#include "acq.h"
#include "gen.h"
#include "calib.h"

// include source files
#include "acq.c"
#include "gen.c"

/**
 * Global methods
 */

int rp_Init() {
    cmn_Init();
    calib_Init();
    hk_Init();
    gen_Init();
    osc_Init();
    // Set default configuration per handler
    rp_Reset();
    return RP_OK;
}

int rp_Release() {
    osc_Release();
    gen_Release();
    hk_Release();
    calib_Release();
    cmn_Release();
    return RP_OK;
}

int rp_Reset() {
    rp_GenReset();
    rp_AcqReset();
    return 0;
}

const char* rp_GetError(int errorCode) {
    switch (errorCode) {
        case RP_OK:    return "OK";
        case RP_EOED:  return "Failed to Open EEPROM Device.";
        case RP_EOMD:  return "Failed to open memory device.";
        case RP_ECMD:  return "Failed to close memory device.";
        case RP_EMMD:  return "Failed to map memory device.";
        case RP_EUMD:  return "Failed to unmap memory device.";
        case RP_EOOR:  return "Value out of range.";
        case RP_EMRO:  return "Modifying read only filed is not allowed.";
        case RP_EWIP:  return "Writing to input pin is not valid.";
        case RP_EPN:   return "Invalid Pin number.";
        case RP_UIA:   return "Uninitialized Input Argument.";
        case RP_FCA:   return "Failed to Find Calibration Parameters.";
        case RP_RCA:   return "Failed to Read Calibration Parameters.";
        case RP_BTS:   return "Buffer too small";
        case RP_EIPV:  return "Invalid parameter value";
        case RP_EUF:   return "Unsupported Feature";
        case RP_ENN:   return "Data not normalized";
        case RP_EFOB:  return "Failed to open bus";
        case RP_EFCB:  return "Failed to close bus";
        case RP_EABA:  return "Failed to acquire bus access";
        case RP_EFRB:  return "Failed to read from the bus";
        case RP_EFWB:  return "Failed to write to the bus";
        default:       return "Unknown error";
    }
}

/**
 * Calibrate methods
 */

rp_calib_params_t rp_GetCalibrationSettings()
{
    return calib_GetParams();
}

int rp_CalibrateAcqOffset(rp_channel_t channel, rp_pinState_t gain, rp_calib_params_t* out_params) {
    return calib_SetAcqOffset(channel, gain, out_params);
}

int rp_CalibrateAcqScaleLV(rp_channel_t channel, float referentialVoltage, rp_calib_params_t* out_params) {
    return calib_SetAcqScale(channel, 0, referentialVoltage, out_params);
}

int rp_CalibrateAcqScaleHV(rp_channel_t channel, float referentialVoltage, rp_calib_params_t* out_params) {
    return calib_SetAcqScale(channel, 1, referentialVoltage, out_params);
}

int rp_CalibrateGenOffset(rp_channel_t channel) {
    return calib_SetGenOffset(channel);
}

int rp_CalibrateGenScale(rp_channel_t channel) {
    return calib_SetGenScale(channel);
}

int rp_CalibrateGen(rp_channel_t channel, rp_calib_params_t* out_params) {
    return calib_CalibrateGen(channel, out_params);
}

int rp_CalibrationReset() {
    return calib_Reset();
}

int rp_CalibrationSetCachedParams() {
    return calib_setCachedParams();
}

int rp_CalibrationWriteParams(rp_calib_params_t calib_params) {
    return calib_WriteParams(calib_params);
}

/**
 * Identification
 */

int rp_IdGetID(uint32_t *id) {
    *id = ioread32(&hk->id);
    return RP_OK;
}

int rp_IdGetDNA(uint64_t *dna) {
    *dna = ((uint64_t) ioread32(&hk->dna_hi) << 32)
         | ((uint64_t) ioread32(&hk->dna_lo) <<  0);
    return RP_OK;
}

/**
 * Digital loop
 */

int rp_EnableDigitalLoop(bool enable) {
    iowrite32((uint32_t) enable, &hk->digital_loop);
    return RP_OK;
}


