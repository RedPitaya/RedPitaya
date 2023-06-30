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
        RP_LOG(context,LOG_ERR, "*ACQ:AXI:DATA:UNITS Missing first parameter.");
        return SCPI_RES_ERR;
    }

    /* Set global units for acq scpi */
    axi_unit = (rp_scpi_acq_unit_t)choice;

    RP_LOG(context,LOG_INFO, "*ACQ:AXI:DATA:UNITS Successfully set scpi units.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqAxiScpiDataUnitsQ(scpi_t *context){

    const char *units;

    if(!SCPI_ChoiceToName(scpi_RpUnits, axi_unit, &units)){
        RP_LOG(context,LOG_ERR, "*ACQ:AXI:DATA:UNITS? Failed to get data units.");
        return SCPI_RES_ERR;
    }

    SCPI_ResultMnemonic(context, units);

    RP_LOG(context,LOG_INFO, "*ACQ:AXI:DATA:UNITS? Successfully returned data to client.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqAxiTriggerFillQ(scpi_t *context){
    
    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    int result;
    bool fillRes;

    result = rp_AcqAxiGetBufferFillState(channel,&fillRes);
    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*ACQ:AXI:SOUR#:TRIG:FILL? Failed to get trigger "
            "fill state: %s", rp_GetError(result));

        return SCPI_RES_ERR;
    }
    SCPI_ResultInt32(context, fillRes);

    RP_LOG(context,LOG_INFO, "*ACQ:AXI:SOUR#:TRIG:FILL? Successfully returned "
        "fill state value to client.");

    return SCPI_RES_OK;
}

scpi_result_t RP_AcqAxiDecimation(scpi_t *context) {

    uint32_t value;

    /* Read DECIMATION parameter */
    if (!SCPI_ParamUInt32(context, &value, true)) {
        RP_LOG(context,LOG_ERR, "*ACQ:AXI:DEC is missing first parameter.");
        return SCPI_RES_ERR;
    }

    // Now set the decimation
    int result = rp_AcqAxiSetDecimationFactor(value);
    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*ACQ:AXI:DEC Failed to set decimation: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*ACQ:AXI:DEC Successfully set decimation.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqAxiDecimationQ(scpi_t *context) {
    uint32_t decimation;
    int result = rp_AcqAxiGetDecimationFactor(&decimation);

    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*ACQ:AXI:DEC? Failed to get decimation: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultUInt32Base(context, decimation, 10);

    RP_LOG(context,LOG_INFO, "*ACQ:AXI:DEC? Successfully returned decimation.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqAxiStartQ(scpi_t *context) {
    uint32_t start,size;
    int result = rp_AcqAxiGetMemoryRegion(&start,&size);

    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*ACQ:AXI:START? Failed to get decimation: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultUInt32Base(context, start, 10);

    RP_LOG(context,LOG_INFO, "*ACQ:AXI:START? Successfully returned decimation.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqAxiEndQ(scpi_t *context) {
    uint32_t start,size;
    int result = rp_AcqAxiGetMemoryRegion(&start,&size);

    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*ACQ:AXI:SIZE? Failed to get decimation: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultUInt32Base(context, size, 10);

    RP_LOG(context,LOG_INFO, "*ACQ:AXI:SIZE? Successfully returned decimation.");
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

    // Now set the trigger delay
    int result = rp_AcqAxiSetTriggerDelay(channel,triggerDelay);

    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*ACQ:AXI:SOUR#:Trig:Dly Failed to set trigger delay: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*ACQ:AXI:SOUR#:Trig:Dly Successfully set trigger delay.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqAxiTriggerDelayQ(scpi_t *context) {

    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    // get trigger delay
    int32_t value;
    int result = rp_AcqAxiGetTriggerDelay(channel,&value);

    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*ACQ:AXI:SOUR#:Trig:Dly? Failed to get trigger delay: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultInt32(context, value);

    RP_LOG(context,LOG_INFO, "*ACQ:AXI:SOUR#:Trig:Dly? Successfully returned trigger delay.");
    return SCPI_RES_OK;
}


scpi_result_t RP_AcqAxiWritePointerQ(scpi_t *context) {
    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    // get write pointer
    uint32_t value;
    int result = rp_AcqAxiGetWritePointer(channel,&value);

    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*ACQ:AXI:SOUR#:Write:Pos? Failed to get writer "
            "position: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultUInt32Base(context, value, 10);

    RP_LOG(context,LOG_INFO, "*ACQ:AXI:SOUR#:Write:Pos? Successfully returned writer position.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqAxiWritePointerAtTrigQ(scpi_t *context) {
    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    // get write pointer at trigger
    uint32_t value;
    int result = rp_AcqAxiGetWritePointerAtTrig(channel,&value);

    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*ACQ:AXI:SOUR#:Trig:Pos? Failed to get writer position at trigger: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultUInt32Base(context, value, 10);

    RP_LOG(context,LOG_INFO, "*ACQ:AXI:SOUR#:Trig:Pos? Successfully returned writer position at trigger.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqAxiEnable(scpi_t *context) {

    int result;
    rp_channel_t channel;
    bool state_c;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }
    /* Parse first, STATE argument */
    if(!SCPI_ParamBool(context, &state_c, true)){
        RP_LOG(context,LOG_ERR, "*ACQ:AXI:SOUR#:ENable Missing first parameter.");
        return SCPI_RES_ERR;
    }

    result = rp_AcqAxiEnable(channel,state_c);

    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*ACQ:AXI:SOUR#:ENable Failed to start data capture: %s",
            rp_GetError(result));

        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*ACQ:AXI:SOUR#:ENable Successfully enabled data capture.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqAxiDataQ(scpi_t *context) {

    uint32_t start, size;
    int result;

    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    /* Parse START parameter */
    if(!SCPI_ParamUInt32(context, &start, true)){
        RP_LOG(context,LOG_ERR, "*ACQ:AXI:SOUR#:DATA:Start:N? is missing START parameter.");
        return SCPI_RES_ERR;
    }

    /* Parse SIZE parameter */
    if(!SCPI_ParamUInt32(context, &size, true)){
        RP_LOG(context,LOG_INFO, "*ACQ:AXI:SOUR#:DATA:Start:N? is missing SIZE parameter.");
        return SCPI_RES_ERR;
    }

    uint32_t size_buff;
    rp_AcqGetBufSize(&size_buff);
    if(axi_unit == RP_SCPI_VOLTS){
        float *buffer = nullptr;
        try{
            buffer = new float[size];
        }catch(const std::bad_alloc &)
        {
            RP_LOG(context,LOG_ERR, "*ACQ:AXI:SOUR#:DATA:Start:N? Failed allocate buffer");
            return SCPI_RES_ERR;
        };
        result = rp_AcqAxiGetDataV(channel, start, &size, buffer);
        if(result != RP_OK){
            RP_LOG(context,LOG_ERR, "*ACQ:AXI:SOUR#:DATA:Start:N? Failed to get "
            "data in volts: %s", rp_GetError(result));
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
            RP_LOG(context,LOG_ERR, "*ACQ:AXI:SOUR#:DATA:Start:N? Failed allocate buffer");
            return SCPI_RES_ERR;
        };
        result = rp_AcqAxiGetDataRaw(channel, start, &size, buffer);

        if(result != RP_OK){
            RP_LOG(context,LOG_ERR, "*ACQ:AXI:SOUR#:DATA:Start:N? Failed to get raw data: %s", rp_GetError(result));
            return SCPI_RES_ERR;
        }

        SCPI_ResultBufferInt16(context, buffer, size);
        delete[] buffer;
    }

    RP_LOG(context,LOG_INFO, "*ACQ:AXI:SOUR#:DATA:Start:N? Successfully returned data.");
    return SCPI_RES_OK;
}

scpi_result_t RP_AcqAxiSetAddres(scpi_t *context) {

    uint32_t start, size;
    int result;

    rp_channel_t channel;

    if (RP_ParseChArgvADC(context, &channel) != RP_OK){
        return SCPI_RES_ERR;
    }

    /* Parse START parameter */
    if(!SCPI_ParamUInt32(context, &start, true)){
        RP_LOG(context,LOG_ERR, "*ACQ:AXI:SOUR#:SET:Buffer is missing START parameter.");
        return SCPI_RES_ERR;
    }

    /* Parse SIZE parameter */
    if(!SCPI_ParamUInt32(context, &size, true)){
        RP_LOG(context,LOG_INFO, "*ACQ:AXI:SOUR#:SET:Buffer is missing SIZE parameter.");
        return SCPI_RES_ERR;
    }
    
    result = rp_AcqAxiSetBufferBytes(channel,start,size);

    if(result != RP_OK){
        RP_LOG(context,LOG_ERR, "*ACQ:AXI:SOUR#:SET:Buffer Failed to set write addresses: %s",
            rp_GetError(result));

        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*ACQ:AXI:SOUR#:SET:Buffer Successfully set addresses.");
    return SCPI_RES_OK;
}