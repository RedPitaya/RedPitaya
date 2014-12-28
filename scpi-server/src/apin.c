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
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include "scpi/scpi.h"
#include "rp.h"

#include "utils.h"
#include "apin.h"
#include "../3rdparty/libs/scpi-parser/libscpi/inc/scpi/parser.h"
#include "../3rdparty/libs/scpi-parser/libscpi/inc/scpi/types.h"
#include "../../api-mockup/rpbase/src/rp.h"


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

    float value;
	char port[15];
    char value_string[15];

    // read first parameter PORT (RP_AOUT0, RP_AIN0, ...)
    if (!SCPI_ParamString(context, &param, &param_len, true)) {
    	syslog(LOG_ERR, "*ANALOG:PIN is missing first parameter.");
    	return SCPI_RES_ERR;
    }
    strncpy(port, param, param_len);
    port[param_len] = '\0';

    // read second parameter VALUE (2.45)
    if (!SCPI_ParamString(context, &param, &param_len, true)) {
        syslog(LOG_ERR, "*ANALOG:PIN is missing second parameter.");
        return SCPI_RES_ERR;
    }
    strncpy(value_string, param, param_len);
    value_string[param_len] = '\0';

    // Convert port into pin id
    rp_apin_t pin;
    if (getRpApin(port, &pin)) {
    	syslog(LOG_ERR, "*ANALOG:PIN parameter port is invalid.");
    	return SCPI_RES_ERR;
    }

    char *endptr;
    errno = 0;    /* To distinguish success/failure after call */
    value = strtof(value_string, &endptr);
    if ((errno == ERANGE && (value == LONG_MAX || value == LONG_MIN)) || (errno != 0 && value == 0) || endptr == value_string) {
        syslog(LOG_ERR, "*ANALOG:PIN parameter value is invalid.");
        return SCPI_RES_ERR;
    }

    // Now set the pin state
    int result = rp_ApinSetValue(pin, value);

    if (RP_OK != result)
	{
		syslog(LOG_ERR, "*ANALOG:PIN Failed to set pin value: %s", rp_GetError(result));
		return SCPI_RES_ERR;
	}

	syslog(LOG_INFO, "*ANALOG:PIN Successfully set port %s to value %.3f.", port, value);

	SET_OK(context);
}