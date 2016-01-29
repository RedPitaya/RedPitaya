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
#include "pdm.h"
#include "id.h"
#include "acquire.h"
#include "calib.h"
#include "generate.h"
#include "rp_api.h"

const char* rp_GetError(int errorCode) {
    switch (errorCode) {
        case RP_OK  :  return "OK";
        case RP_EOED:  return "Failed to Open EEPROM Device.";
        case RP_EOMD:  return "Failed to open memory device.";
        case RP_ECMD:  return "Failed to close memory device.";
        case RP_EMMD:  return "Failed to map memory device.";
        case RP_EUMD:  return "Failed to unmap memory device.";
        case RP_EOOR:  return "Value out of range.";
        case RP_ELID:  return "LED input direction is not valid.";
        case RP_EMRO:  return "Modifying read only filed is not allowed.";
        case RP_EWIP:  return "Writing to input pin is not valid.";
        case RP_EPN :  return "Invalid Pin number.";
        case RP_UIA :  return "Uninitialized Input Argument.";
        case RP_FCA :  return "Failed to Find Calibration Parameters.";
        case RP_RCA :  return "Failed to Read Calibration Parameters.";
        case RP_BTS :  return "Buffer too small";
        case RP_EIPV:  return "Invalid parameter value";
        case RP_EUF :  return "Unsupported Feature";
        case RP_ENN :  return "Data not normalized";
        case RP_EFOB:  return "Failed to open bus";
        case RP_EFCB:  return "Failed to close bus";
        case RP_EABA:  return "Failed to acquire bus access";
        case RP_EFRB:  return "Failed to read from the bus";
        case RP_EFWB:  return "Failed to write to the bus";
        default:       return "Unknown error";
    }
}

