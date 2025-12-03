/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server apin SCPI commands implementation
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#include <stdio.h>
#include <string.h>

#include "apin.h"
#include "common.h"
#include "scpi/parser.h"
//#include "../../api/src/common.h"

/* Apin choice def */
const scpi_choice_def_t scpi_RpApin[] = {{"AOUT0", 0},  //!< Analog output 0
                                         {"AOUT1", 1},  //!< Analog output 1
                                         {"AOUT2", 2},  //!< Analog output 2
                                         {"AOUT3", 3},  //!< Analog output 3
                                         {"AIN0", 4},   //!< Analog input 0
                                         {"AIN1", 5},   //!< Analog input 1
                                         {"AIN2", 6},   //!< Analog input 2
                                         {"AIN3", 7},   //!< Analog input 3
                                         SCPI_CHOICE_LIST_END};

scpi_result_t RP_AnalogPinReset(scpi_t* context) {
    auto result = rp_ApinReset();
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to reset Red Pitaya analog resources: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

/**
 * Returns Analog Pin value in volts to SCPI context
 * @param context SCPI context
 * @return success or failure
 */
scpi_result_t RP_AnalogPinValueQ(scpi_t* context) {
    int32_t choice = 0;
    /* Read first parameter - APIN */
    if (!SCPI_ParamChoice(context, scpi_RpApin, &choice, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    // Convert port into pin id
    rp_apin_t pin = (rp_apin_t)choice;
    // Now get the pin value
    float value = 0;
    uint32_t raw = 0;
    auto result = rp_ApinGetValue(pin, &value, &raw);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get pin value: %s", rp_GetError(result));
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    // Return back result
    SCPI_ResultFloat(context, value);
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

/**
 * Sets Analog Pin value in volts
 * @param context SCPI context
 * @return success or failure
 */
scpi_result_t RP_AnalogPinValue(scpi_t* context) {
    int32_t choice = 0;
    double value = 0;
    /* Read first parameter - APIN */
    if (!SCPI_ParamChoice(context, scpi_RpApin, &choice, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }
    /* Read second parameter - VALUE */
    if (!SCPI_ParamDouble(context, &value, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing second parameter.");
        return SCPI_RES_ERR;
    }
    // Convert port into pin id
    rp_apin_t pin = (rp_apin_t)choice;
    /* Set pin value */
    auto result = rp_ApinSetValue(pin, (float)value);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to set pin value: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}
