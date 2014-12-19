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
#include "housekeeping.h"
#include "dpin_handler.h"
#include "analog_mixed_signals.h"
#include "apin_handler.h"
#include "rp.h"

static char version[50];


/**
 * Global methods
 */

int rp_Init()
{
	ECHECK(hk_Init());
    ECHECK(ams_Init());
    // TODO: Place other module initializations here
	return RP_OK;
}

int rp_Release()
{
	ECHECK(hk_Release());
    ECHECK(ams_Release());
    // TODO: Place other module releasing here
	return RP_OK;
}

const char* rp_GetVersion()
{
	sprintf(version, "%s (%s)", VERSION_STR, REVISION_STR);
	return version;
}

const char* rp_GetError(int errorCode)
{
	switch (errorCode) {
		case RP_OK:
			return "OK";
		case RP_EOMD:
			return "Failed to open memory device.";
		case RP_ECMD:
			return "Failed to close memory device.";
		case RP_EMMD:
			return "Failed to map memory device.";
		case RP_EUMD:
			return "Failed to unmap memory device.";
		case RP_EOOR:
			return "Value out of range.";
		case RP_ELID:
			return "LED input direction is not valid.";
		case RP_EMRO:
			return "Modifying read only filed is not allowed.";
		case RP_EWIP:
			return "Writing to input pin is not valid.";
        case RP_EPN:
            return "Invalid Pin number.";
		case RP_EOR:
			return "Value out of range";
		default:
			return "Unknown error";
	}
}


/**
 * Digital Pin Input Output methods
 */

int rp_DpinSetDirection(rp_dpin_t pin, rp_pinDirection_t direction)
{
	return dpin_SetDirection(pin, direction);
}

int rp_DpinGetDirection(rp_dpin_t pin, rp_pinDirection_t* direction)
{
	return dpin_GetDirection(pin, direction);
}

int rp_DpinSetState(rp_dpin_t pin, rp_pinState_t state)
{
	return dpin_SetState(pin, state);
}

int rp_DpinGetState(rp_dpin_t pin, rp_pinState_t* state)
{
	return dpin_GetState(pin, state);
}

/**
 * Analog In Output methods
 */

int rp_ApinSetValue(rp_apin_t pin, float value)
{
    return apin_SetValue(pin, value);
}

int rp_ApinGetValue(rp_apin_t pin, float* value)
{
    return apin_GetValue(pin, value);
}

int rp_ApinSetValueRaw(rp_apin_t pin, uint32_t value)
{
    return apin_SetValueRaw(pin, value);
}

int rp_ApinGetValueRaw(rp_apin_t pin, uint32_t* value)
{
    return apin_GetValueRaw(pin, value);
}

int rp_ApinGetRange(rp_apin_t pin, float* min_val,  float* max_val)
{
    return apin_GetRange(pin, min_val, max_val);
}
