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
#include "oscilloscope.h"
#include "acq_handler.h"
#include "analog_mixed_signals.h"
#include "apin_handler.h"
#include "health.h"
#include "health_handler.h"
#include "calib.h"
#include "rp.h"

static char version[50];

/**
 * Global methods
 */

int rp_Init()
{
	ECHECK(calib_Init());
	ECHECK(hk_Init());
	ECHECK(ams_Init());
	ECHECK(health_Init());
	ECHECK(osc_Init());
    // TODO: Place other module initializations here
	return RP_OK;
}

int rp_Release()
{
	ECHECK(osc_Release());
	ECHECK(health_Release());
	ECHECK(ams_Release());
	ECHECK(hk_Release());
	ECHECK(calib_Release());

    // TODO: Place other module releasing here (in reverse order)
	return RP_OK;
}

const char* rp_GetVersion()
{
	sprintf(version, "%s (%s)", VERSION_STR, REVISION_STR);
	return version;
}

rp_calib_params_t rp_GetCalibrationSettings()
{
	return calib_GetParams();
}

const char* rp_GetError(int errorCode)
{
	switch (errorCode) {
		case RP_OK:
			return "OK";
		case RP_EOED:
			return "Failed to Open EEPROM Device.";
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
        case RP_UIA:
        	return "Uninitialized Input Argument.";
        case RP_FCA:
        	return "Failed to Find Calibration Parameters.";
        case RP_RCA:
        	return "Failed to Read Calibration Parameters.";
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


/**
 * Acquire methods
 */

int rp_AcqSetDecimation(rp_acq_decimation_t decimation)
{
	return acq_SetDecimation(decimation);
}

int rp_AcqGetDecimation(rp_acq_decimation_t* decimation)
{
	return acq_GetDecimation(decimation);
}

int rp_AcqGetDecimationFactor(uint32_t* decimation)
{
	return acq_GetDecimationFactor(decimation);
}

int rp_AcqSetSamplingRate(rp_acq_sampling_rate_t sampling_rate)
{
	return acq_SetSamplingRate(sampling_rate);
}

int rp_AcqGetSamplingRate(rp_acq_sampling_rate_t* sampling_rate)
{
	return acq_GetSamplingRate(sampling_rate);
}

int rp_AcqGetSamplingRateHz(float* sampling_rate)
{
	return acq_GetSamplingRateHz(sampling_rate);
}

int rp_AcqSetAveraging(bool enabled)
{
	return acq_SetAveraging(enabled);
}

int rp_AcqGetAveraging(bool *enabled)
{
	return acq_GetAveraging(enabled);
}

int rp_AcqSetTriggerSrc(rp_acq_trig_src_t source)
{
	return acq_SetTriggerSrc(source);
}

int rp_AcqGetTriggerSrc(rp_acq_trig_src_t* source)
{
	return acq_GetTriggerSrc(source);
}

int rp_AcqSetTriggerDelay(uint32_t decimated_data_num)
{
	return acq_SetTriggerDelay(decimated_data_num, false);
}

int rp_AcqGetTriggerDelay(uint32_t* decimated_data_num)
{
	return acq_GetTriggerDelay(decimated_data_num);
}

int rp_AcqSetTriggerDelayNs(uint64_t time_ns)
{
	return acq_SetTriggerDelayNs(time_ns, false);
}

int rp_AcqGetTriggerDelayNs(uint64_t* time_ns)
{
	return acq_GetTriggerDelayNs(time_ns);
}

int rp_AcqGetGain(rp_pinState_t* state)
{
	return acq_GetGain(state);
}

int rp_AcqGetGainV(float* voltage)
{
	return acq_GetGainV(voltage);
}

int rp_AcqSetGain(rp_pinState_t state)
{
	return acq_SetGain(state);
}

int rp_AcqGetChannelThreshold(rp_channel_t channel, float* voltage)
{
	return acq_GetChannelThreshold(channel, voltage);
}

int rp_AcqSetChannelThreshold(rp_channel_t channel, float voltage)
{
	return acq_SetChannelThreshold(channel, voltage);
}

int rp_AcqGetChannelThresholdHyst(rp_channel_t channel, float* voltage)
{
	return acq_GetChannelThresholdHyst(channel, voltage);
}

int rp_AcqSetChannelThresholdHyst(rp_channel_t channel, float voltage)
{
	return acq_SetChannelThresholdHyst(channel, voltage);
}

int rp_AcqGetWritePointer(uint32_t* pos)
{
	return acq_GetWritePointer(pos);
}

int rp_AcqGetWritePointerAtTrig(uint32_t* pos)
{
	return acq_GetWritePointerAtTrig(pos);
}

int rp_AcqStart()
{
	return acq_Start();
}

int rp_AcqStop()
{
	return acq_Stop();
}


/**
 * Health methods
 */

int rp_HealthGetValue(rp_health_t sensor, float* value)
{
	return health_GetValue(sensor, value);
}
