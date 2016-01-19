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

#include "version.h"
#include "common.h"
#include "analog.h"
#include "housekeeping.h"
#include "acquire.h"
#include "analog.h"
#include "calib.h"
#include "generate.h"

static char version[50];

/**
 * Global methods
 */

int rp_Init()
{
    cmn_Init();
    analog_Init();
    for (int unsigned i=0; i<RP_MNA; i++)
        acq_Init(i);
    // TODO: Place other module initializations here

    // Set default configuration per handler
    rp_Reset();

    return RP_OK;
}

int rp_Release()
{
    analog_Release();
    for (int unsigned i=0; i<RP_MNA; i++)
        acq_Release(i);
    hk_Release();
    // TODO: Place other module releasing here (in reverse order)
    cmn_Release();
    return RP_OK;
}

int rp_Reset()
{
    ECHECK(rp_AOpinReset());
    for (int unsigned i=0; i<RP_MNA; i++)
        rp_AcqReset(i);
    // TODO: Place other module resetting here (in reverse order)
    return 0;
}

const char* rp_GetVersion()
{
    sprintf(version, "%s (%s)", VERSION_STR, REVISION_STR);
    return version;
}

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

