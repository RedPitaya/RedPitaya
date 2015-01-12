/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server apin SCPI commands implementation
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <syslog.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"
#include "apin.h"
#include "../3rdparty/libs/scpi-parser/libscpi/inc/scpi/parser.h"
#include "../../api-mockup/rpbase/src/common.h"


int RP_ApinSetDefaultValues() {
    // Setting default parameter
    rp_apin_t pin;
    for (pin = RP_AOUT0; pin <= RP_AOUT3; pin++) {
        ECHECK(rp_ApinSetValue(pin, 0));
    }

    return RP_OK;
}

/**
 * Returns Analog Pin value in volts to SCPI context
 * @param context SCPI context
 * @return success or failure
 */
scpi_result_t RP_AnalogPinGetValue(scpi_t * context) {
    const char * param;
    size_t param_len;

	char port[15];

    // read first parameter PORT (RP_AOUT0, RP_AIN0, ...)
    if (!SCPI_ParamString(context, &param, &param_len, true)) {
    	syslog(LOG_ERR, "*ANALOG:PIN? is missing first parameter.");
    	return SCPI_RES_ERR;
    }
    strncpy(port, param, param_len);
    port[param_len] = '\0';

    // Convert port into pin id
    rp_apin_t pin;
    if (getRpApin(port, &pin)) {
    	syslog(LOG_ERR, "*ANALOG:PIN? parameter port is invalid.");
    	return SCPI_RES_ERR;
    }

    // Now get the pin value
    float value;
    int result = rp_ApinGetValue(pin, &value);

    if (RP_OK != result)
    {
    	syslog(LOG_ERR, "*ANALOG:PIN? Failed to get pin value: %s", rp_GetError(result));
    	return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultDouble(context, value);

	syslog(LOG_INFO, "*ANALOG:PIN? Successfully returned port %s value %.3f.", port, value);

    return SCPI_RES_OK;
}

/**
 * Sets Analog Pin value in volts
 * @param context SCPI context
 * @return success or failure
 */
scpi_result_t RP_AnalogPinSetValue(scpi_t * context) {
    const char * param;
    size_t param_len;

    double value;
	char port[15];

    // read first parameter PORT (RP_AOUT0, RP_AIN0, ...)
    if (!SCPI_ParamString(context, &param, &param_len, true)) {
    	syslog(LOG_ERR, "*ANALOG:PIN is missing first parameter.");
    	return SCPI_RES_ERR;
    }
    strncpy(port, param, param_len);
    port[param_len] = '\0';

    // read second parameter VALUE (2.45)
    if (!SCPI_ParamDouble(context, &value, true)) {
        syslog(LOG_ERR, "*ANALOG:PIN is missing second parameter.");
        return SCPI_RES_ERR;
    }
    // Convert port into pin id
    rp_apin_t pin;
    if (getRpApin(port, &pin)) {
    	syslog(LOG_ERR, "*ANALOG:PIN parameter port is invalid.");
    	return SCPI_RES_ERR;
    }

    // Now set the pin state
    int result = rp_ApinSetValue(pin, (float) value);

    if (RP_OK != result)
	{
		syslog(LOG_ERR, "*ANALOG:PIN Failed to set pin value: %s", rp_GetError(result));
		return SCPI_RES_ERR;
	}

	syslog(LOG_INFO, "*ANALOG:PIN Successfully set port %s to value %.3f.", port, value);

	return SCPI_RES_OK;
}