/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server acquire SCPI commands implementation
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
#include <stdlib.h>
#include <string.h>
#include <new>

#include "acquire.h"
#include "common.h"

#include "scpi/parser.h"
#include "scpi/units.h"


rp_scpi_acq_unit_t axi_unit     = RP_SCPI_VOLTS;        // default value

scpi_result_t RP_AcqAxiScpiDataUnits(scpi_t *context) {

    int32_t choice;

    /* Read UNITS parameters */
    if(!SCPI_ParamChoice(context, scpi_RpUnits, &choice, true)){
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"Missing first parameter.");
        return SCPI_RES_ERR;
    }

    /* Set global units for acq scpi */
    axi_unit = (rp_scpi_acq_unit_t)choice;

    RP_LOG_INFO("%s",rp_GetError(RP_OK))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqAxiScpiDataUnitsQ(scpi_t *context){

    const char *units;

    if(!SCPI_ChoiceToName(scpi_RpUnits, axi_unit, &units)){
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR,"Failed to get data units.");
        return SCPI_RES_ERR;
    }

    SCPI_ResultMnemonic(context, units);

    RP_LOG_INFO("%s",rp_GetError(RP_OK))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqAxiTriggerFillQ(scpi_t *context){

    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }
    bool fillRes;
    auto result = rp_AcqAxiGetBufferFillState(channel,&fillRes);
    if(result != RP_OK){
        RP_LOG_CRIT("Failed to get trigger fill state: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    SCPI_ResultInt32(context, fillRes);

    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqAxiDecimation(scpi_t *context) {
    uint32_t value;

    /* Read DECIMATION parameter */
    if (!SCPI_ParamUInt32(context, &value, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"Missing first parameter.");
        return SCPI_RES_ERR;
    }

    auto result = rp_AcqAxiSetDecimationFactor(value);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to set decimation: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqAxiDecimationCh(scpi_t *context) {

    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    uint32_t value;

    /* Read DECIMATION parameter */
    if (!SCPI_ParamUInt32(context, &value, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"Missing first parameter.");
        return SCPI_RES_ERR;
    }

    auto result = rp_AcqAxiSetDecimationFactorCh(channel,value);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to set decimation: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqAxiDecimationQ(scpi_t *context) {

    uint32_t decimation;

    auto result = rp_AcqAxiGetDecimationFactor(&decimation);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get decimation: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    SCPI_ResultUInt32Base(context, decimation, 10);

    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqAxiDecimationChQ(scpi_t *context) {

    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    uint32_t decimation;

    auto result = rp_AcqAxiGetDecimationFactorCh(channel,&decimation);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get decimation: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    SCPI_ResultUInt32Base(context, decimation, 10);

    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}


scpi_result_t RP_AcqAxiStartQ(scpi_t *context) {
    uint32_t start,size;
    int result = rp_AcqAxiGetMemoryRegion(&start,&size);

    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get decimation: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    SCPI_ResultUInt32Base(context, start, 10);
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqAxiEndQ(scpi_t *context) {
    uint32_t start,size;
    auto result = rp_AcqAxiGetMemoryRegion(&start,&size);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get decimation: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    SCPI_ResultUInt32Base(context, size, 10);
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqAxiTriggerDelay(scpi_t *context) {
    rp_channel_t channel;
    if (RP_ParseChArgvADC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    int32_t triggerDelay;

    // read first parameter TRIGGER DELAY (value in samples)
    if (!SCPI_ParamInt32(context, &triggerDelay, false)) {
        triggerDelay = 0;
    }

    auto result = rp_AcqAxiSetTriggerDelay(channel,triggerDelay);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to set trigger delay: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqAxiTriggerDelayQ(scpi_t *context) {

    rp_channel_t channel;
    if (RP_ParseChArgvADC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    // get trigger delay
    int32_t value;
    auto result = rp_AcqAxiGetTriggerDelay(channel,&value);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get trigger delay: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultInt32(context, value);
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}


scpi_result_t RP_AcqAxiWritePointerQ(scpi_t *context) {
    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    // get write pointer
    uint32_t value;
    auto result = rp_AcqAxiGetWritePointer(channel,&value);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get writer position: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultUInt32Base(context, value, 10);
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqAxiWritePointerAtTrigQ(scpi_t *context) {
    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    // get write pointer at trigger
    uint32_t value;
    auto result = rp_AcqAxiGetWritePointerAtTrig(channel,&value);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get writer position at trigger: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultUInt32Base(context, value, 10);
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqAxiEnable(scpi_t *context) {

    rp_channel_t channel;
    bool state_c;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }
    /* Parse first, STATE argument */
    if(!SCPI_ParamBool(context, &state_c, true)){
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"Missing first parameter.");
        return SCPI_RES_ERR;
    }

    auto result = rp_AcqAxiEnable(channel,state_c);

    if(result != RP_OK){
        RP_LOG_CRIT("Failed to start data capture: %s",rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqAxiDataQ(scpi_t *context) {

    uint32_t start, size;
    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    /* Parse START parameter */
    if(!SCPI_ParamUInt32(context, &start, true)){
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"Missing START parameter.");
        return SCPI_RES_ERR;
    }

    /* Parse SIZE parameter */
    if(!SCPI_ParamUInt32(context, &size, true)){
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"Missing SIZE parameter.");
        return SCPI_RES_ERR;
    }
    auto result = 0;
    uint32_t size_buff;
    rp_AcqGetBufSize(&size_buff);
    if(axi_unit == RP_SCPI_VOLTS){
        float *buffer = nullptr;
        try{
            buffer = new float[size];
        }catch(const std::bad_alloc &)
        {
            SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR,"Failed allocate buffer");
            return SCPI_RES_ERR;
        };
        result = rp_AcqAxiGetDataV(channel, start, &size, buffer);
        if(result != RP_OK){
            RP_LOG_CRIT("Failed to get data in volts: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }

        SCPI_ResultBufferFloat(context, buffer, size);
        delete[] buffer;
    }else{
        int16_t *buffer = nullptr;
        try{
            buffer = new int16_t[size];
        }catch(const std::bad_alloc &)
        {
            SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR,"Failed allocate buffer");
            return SCPI_RES_ERR;
        };
        result = rp_AcqAxiGetDataRaw(channel, start, &size, buffer);

        if(result != RP_OK){
            RP_LOG_CRIT("Failed to get raw data: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }

        SCPI_ResultBufferInt16(context, buffer, size);
        delete[] buffer;
    }
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqAxiSetAddres(scpi_t *context) {

    uint32_t start, size;
    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    /* Parse START parameter */
    if(!SCPI_ParamUInt32(context, &start, true)){
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"Missing START parameter.");
        return SCPI_RES_ERR;
    }

    /* Parse SIZE parameter */
    if(!SCPI_ParamUInt32(context, &size, true)){
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER,"Missing SIZE parameter.");
        return SCPI_RES_ERR;
    }

    auto result = rp_AcqAxiSetBufferBytes(channel,start,size);

    if(result != RP_OK){
        RP_LOG_CRIT("Failed to set write addresses: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s",rp_GetError(result))
    return SCPI_RES_OK;
}