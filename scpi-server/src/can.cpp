/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server SCPI commands implementation
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
#include <string.h>
#include <stdlib.h>

#include "common.h"
#include "scpi/parser.h"
#include "rp_hw_can.h"
#include "can.h"

const scpi_choice_def_t scpi_CAN_Bool[] = {
    {"OFF", 0},
    {"ON", 1},
    SCPI_CHOICE_LIST_END
};

const scpi_choice_def_t scpi_CAN_State[] = {
    {"ERROR_ACTIVE", RP_CAN_STATE_ERROR_ACTIVE},
    {"ERROR_WARNING", RP_CAN_STATE_ERROR_WARNING},
    {"ERROR_PASSIVE", RP_CAN_STATE_ERROR_PASSIVE},
    {"BUS_OFF", RP_CAN_STATE_BUS_OFF},
    {"STOPPED", RP_CAN_STATE_STOPPED},
    {"SLEEPING", RP_CAN_STATE_SLEEPING},
    SCPI_CHOICE_LIST_END
};

const scpi_choice_def_t scpi_CAN_Mode[] = {
    {"LOOPBACK", RP_CAN_MODE_LOOPBACK},
    {"LISTENONLY", RP_CAN_MODE_LISTENONLY},
    {"3_SAMPLES", RP_CAN_MODE_3_SAMPLES},
    {"ONE_SHOT", RP_CAN_MODE_ONE_SHOT},
    {"BERR_REPORTING", RP_CAN_MODE_BERR_REPORTING},
    SCPI_CHOICE_LIST_END
};

auto parseInterface(scpi_t * context,rp_can_interface_t *interface,const char *func) -> bool{
    int32_t cmd[1] = {0};
    
    if (!SCPI_CommandNumbers(context,cmd,1,-1)){
        RP_LOG(context,LOG_ERR, "*%s Failed to get parameters.",func);
        return false;
    }
 
    if (cmd[0] == -1){
        RP_LOG(context,LOG_ERR, "*%s Failed to get interface number",func);
        return false;
    }
    *interface = (rp_can_interface_t)cmd[0];
    return true;
} 


scpi_result_t RP_CAN_FpgaEnable(scpi_t * context){
    RP_F_NAME(const char func[] = "CAN:FPGA";)
    int value;

    if (!SCPI_ParamChoice(context, scpi_CAN_Bool, &value, true)) {
        RP_LOG(context,LOG_ERR, "*%s is missing first parameter.",func);
        return SCPI_RES_ERR;
    }

    int result = rp_CanSetFPGAEnable(value);
    if (RP_HW_CAN_OK != result) {
        RP_LOG(context,LOG_ERR, "*%s %s (%d)",func, rp_CanGetError(result), result);
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*%s %s (%d)",func, rp_CanGetError(result), result);
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_FpgaEnableQ(scpi_t * context){
    RP_F_NAME(const char func[] = "CAN:FPGA?";)
    bool value;
    int result = rp_CanGetFPGAEnable(&value);

    if (RP_HW_CAN_OK != result) {
        RP_LOG(context,LOG_ERR, "*%s %s (%d)",func, rp_CanGetError(result), result);
        return SCPI_RES_ERR;
    }

    const char *_name;
    if(!SCPI_ChoiceToName(scpi_CAN_Bool, (int32_t)value, &_name)){
        RP_LOG(context,LOG_ERR, "*%s Failed to parse bool value.",func);
        return SCPI_RES_ERR;
    }
    SCPI_ResultMnemonic(context, _name);

    RP_LOG(context,LOG_INFO, "*%s %s (%d)",func, rp_CanGetError(result), result);
    return SCPI_RES_OK;
}


scpi_result_t RP_CAN_Start(scpi_t * context){
    RP_F_NAME(const char func[] = "CAN#:START";)

    auto itf = RP_CAN_0;
    if (!parseInterface(context,&itf,func)){
        return SCPI_RES_ERR;
    } 

    auto result = rp_CanStart(itf);

    if (RP_HW_CAN_OK != result) {
        RP_LOG(context,LOG_ERR, "*%s %s (%d)",func, rp_CanGetError(result), result);
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*%s %s (%d)",func, rp_CanGetError(result), result);
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_Stop(scpi_t * context){
    RP_F_NAME(const char func[] = "CAN#:STOP";)

    auto itf = RP_CAN_0;
    if (!parseInterface(context,&itf,func)){
        return SCPI_RES_ERR;
    } 

    auto result = rp_CanStop(itf);

    if (RP_HW_CAN_OK != result) {
        RP_LOG(context,LOG_ERR, "*%s %s (%d)",func, rp_CanGetError(result), result);
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*%s %s (%d)",func, rp_CanGetError(result), result);
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_Restart(scpi_t * context){
    RP_F_NAME(const char func[] = "CAN#:RESTART";)

    auto itf = RP_CAN_0;
    if (!parseInterface(context,&itf,func)){
        return SCPI_RES_ERR;
    } 

    auto result = rp_CanRestart(itf);

    if (RP_HW_CAN_OK != result) {
        RP_LOG(context,LOG_ERR, "*%s %s (%d)",func, rp_CanGetError(result), result);
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*%s %s (%d)",func, rp_CanGetError(result), result);
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_StateQ(scpi_t * context){
    RP_F_NAME(const char func[] = "CAN#:STATE?";)

    auto itf = RP_CAN_0;
    if (!parseInterface(context,&itf,func)){
        return SCPI_RES_ERR;
    } 
    rp_can_state_t state;
    auto result = rp_CanGetState(itf,&state);

    if (RP_HW_CAN_OK != result) {
        RP_LOG(context,LOG_ERR, "*%s %s (%d)",func, rp_CanGetError(result), result);
        return SCPI_RES_ERR;
    }

    const char *_name;
    if(!SCPI_ChoiceToName(scpi_CAN_State, (int32_t)state, &_name)){
        RP_LOG(context,LOG_ERR, "*%s Failed to parse state value.",func);
        return SCPI_RES_ERR;
    }
    SCPI_ResultMnemonic(context, _name);

    RP_LOG(context,LOG_INFO, "*%s %s (%d)",func, rp_CanGetError(result), result);
    return SCPI_RES_OK;
}


scpi_result_t RP_CAN_Bitrate(scpi_t * context){
    RP_F_NAME(const char func[] = "CAN#:BitRate";)

    auto itf = RP_CAN_0;
    if (!parseInterface(context,&itf,func)){
        return SCPI_RES_ERR;
    }

    uint32_t value;

    if (!SCPI_ParamUInt32(context, &value, true)) {
        RP_LOG(context,LOG_ERR, "*%s is missing first parameter.",func);
        return SCPI_RES_ERR;
    }

    auto result = rp_CanSetBitrate(itf,value);

    if (RP_HW_CAN_OK != result) {
        RP_LOG(context,LOG_ERR, "*%s %s (%d)",func, rp_CanGetError(result), result);
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*%s %s (%d)",func, rp_CanGetError(result), result);
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_BitrateSamplePoint(scpi_t * context){
    RP_F_NAME(const char func[] = "CAN#:BitRate:SamplePoint";)

    auto itf = RP_CAN_0;
    if (!parseInterface(context,&itf,func)){
        return SCPI_RES_ERR;
    }

    uint32_t bitrate;
    float sp;

    if (!SCPI_ParamUInt32(context, &bitrate, true)) {
        RP_LOG(context,LOG_ERR, "*%s is missing first parameter.",func);
        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamFloat(context, &sp, true)) {
        RP_LOG(context,LOG_ERR, "*%s is missing second parameter.",func);
        return SCPI_RES_ERR;
    }

    auto result = rp_CanSetBitrateAndSamplePoint(itf,bitrate,sp);

    if (RP_HW_CAN_OK != result) {
        RP_LOG(context,LOG_ERR, "*%s %s (%d)",func, rp_CanGetError(result), result);
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*%s %s (%d)",func, rp_CanGetError(result), result);
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_BitrateSamplePointQ(scpi_t * context){
    RP_F_NAME(const char func[] = "CAN#:BitRate:SamplePoint?";)

    auto itf = RP_CAN_0;
    if (!parseInterface(context,&itf,func)){
        return SCPI_RES_ERR;
    }

    uint32_t bitrate;
    float sp;

    auto result = rp_CanGetBitrateAndSamplePoint(itf,&bitrate,&sp);

    if (RP_HW_CAN_OK != result) {
        RP_LOG(context,LOG_ERR, "*%s %s (%d)",func, rp_CanGetError(result), result);
        return SCPI_RES_ERR;
    }

    SCPI_ResultUInt32Base(context, bitrate, 10);
    SCPI_ResultFloat(context, sp);

    RP_LOG(context,LOG_INFO, "*%s %s (%d)",func, rp_CanGetError(result), result);
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_BitTiming(scpi_t * context){
    RP_F_NAME(const char func[] = "CAN#:BitTiming";)

    auto itf = RP_CAN_0;
    if (!parseInterface(context,&itf,func)){
        return SCPI_RES_ERR;
    }

    rp_can_bittiming_t bt;

    if (!SCPI_ParamUInt32(context, &bt.tq, true)) {
        RP_LOG(context,LOG_ERR, "*%s is missing tq parameter.",func);
        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamUInt32(context, &bt.prop_seg, true)) {
        RP_LOG(context,LOG_ERR, "*%s is missing prop_seg parameter.",func);
        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamUInt32(context, &bt.phase_seg1, true)) {
        RP_LOG(context,LOG_ERR, "*%s is missing phase_seg1 parameter.",func);
        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamUInt32(context, &bt.phase_seg2, true)) {
        RP_LOG(context,LOG_ERR, "*%s is missing phase_seg2 parameter.",func);
        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamUInt32(context, &bt.sjw, true)) {
        RP_LOG(context,LOG_ERR, "*%s is missing sjw parameter.",func);
        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamUInt32(context, &bt.brp, true)) {
        RP_LOG(context,LOG_ERR, "*%s is missing brp parameter.",func);
        return SCPI_RES_ERR;
    }

    auto result = rp_CanSetBitTiming(itf,bt);

    if (RP_HW_CAN_OK != result) {
        RP_LOG(context,LOG_ERR, "*%s %s (%d)",func, rp_CanGetError(result), result);
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*%s %s (%d)",func, rp_CanGetError(result), result);
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_BitTimingQ(scpi_t * context){
    RP_F_NAME(const char func[] = "CAN#:BitTiming?";)

    auto itf = RP_CAN_0;
    if (!parseInterface(context,&itf,func)){
        return SCPI_RES_ERR;
    }

    rp_can_bittiming_t bt;

    auto result = rp_CanGetBitTiming(itf,&bt);

    if (RP_HW_CAN_OK != result) {
        RP_LOG(context,LOG_ERR, "*%s %s (%d)",func, rp_CanGetError(result), result);
        return SCPI_RES_ERR;
    }

    SCPI_ResultUInt32Base(context, bt.tq, 10);
    SCPI_ResultUInt32Base(context, bt.prop_seg, 10);
    SCPI_ResultUInt32Base(context, bt.phase_seg1, 10);
    SCPI_ResultUInt32Base(context, bt.phase_seg2, 10);
    SCPI_ResultUInt32Base(context, bt.sjw, 10);
    SCPI_ResultUInt32Base(context, bt.brp, 10);

    RP_LOG(context,LOG_INFO, "*%s %s (%d)",func, rp_CanGetError(result), result);
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_BitTimingLimitsQ(scpi_t * context){
    RP_F_NAME(const char func[] = "CAN#:BitTimingLimits?";)

    auto itf = RP_CAN_0;
    if (!parseInterface(context,&itf,func)){
        return SCPI_RES_ERR;
    }

    rp_can_bittiming_limits_t bt;

    auto result = rp_CanGetBitTimingLimits(itf,&bt);

    if (RP_HW_CAN_OK != result) {
        RP_LOG(context,LOG_ERR, "*%s %s (%d)",func, rp_CanGetError(result), result);
        return SCPI_RES_ERR;
    }

    SCPI_ResultUInt32Base(context, bt.tseg1_min, 10);
    SCPI_ResultUInt32Base(context, bt.tseg1_max, 10);
    SCPI_ResultUInt32Base(context, bt.tseg2_min, 10);
    SCPI_ResultUInt32Base(context, bt.tseg2_max, 10);
    SCPI_ResultUInt32Base(context, bt.sjw_max, 10);
    SCPI_ResultUInt32Base(context, bt.brp_min, 10);
    SCPI_ResultUInt32Base(context, bt.brp_max, 10);
    SCPI_ResultUInt32Base(context, bt.brp_inc, 10);

    RP_LOG(context,LOG_INFO, "*%s %s (%d)",func, rp_CanGetError(result), result);
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_ClockFreqQ(scpi_t * context){
    RP_F_NAME(const char func[] = "CAN#:CLOCK?";)

    auto itf = RP_CAN_0;
    if (!parseInterface(context,&itf,func)){
        return SCPI_RES_ERR;
    } 
    uint32_t freq;
    auto result = rp_CanGetClockFreq(itf,&freq);

    if (RP_HW_CAN_OK != result) {
        RP_LOG(context,LOG_ERR, "*%s %s (%d)",func, rp_CanGetError(result), result);
        return SCPI_RES_ERR;
    }

    SCPI_ResultUInt32Base(context, freq, 10);

    RP_LOG(context,LOG_INFO, "*%s %s (%d)",func, rp_CanGetError(result), result);
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_BusErrorCountersQ(scpi_t * context){
    RP_F_NAME(const char func[] = "CAN#:BUS:ERROR?";)

    auto itf = RP_CAN_0;
    if (!parseInterface(context,&itf,func)){
        return SCPI_RES_ERR;
    } 
    uint16_t tx,rx;
    auto result = rp_CanGetBusErrorCounters(itf,&tx,&rx);

    if (RP_HW_CAN_OK != result) {
        RP_LOG(context,LOG_ERR, "*%s %s (%d)",func, rp_CanGetError(result), result);
        return SCPI_RES_ERR;
    }

    SCPI_ResultUInt32Base(context, tx, 10);
    SCPI_ResultUInt32Base(context, rx, 10);

    RP_LOG(context,LOG_INFO, "*%s %s (%d)",func, rp_CanGetError(result), result);
    return SCPI_RES_OK;
}


scpi_result_t RP_CAN_RestartTime(scpi_t * context){
    RP_F_NAME(const char func[] = "CAN#:RestartTime";)

    auto itf = RP_CAN_0;
    if (!parseInterface(context,&itf,func)){
        return SCPI_RES_ERR;
    }

    uint32_t value;

    if (!SCPI_ParamUInt32(context, &value, true)) {
        RP_LOG(context,LOG_ERR, "*%s is missing first parameter.",func);
        return SCPI_RES_ERR;
    }

    auto result = rp_CanSetRestartTime(itf,value);

    if (RP_HW_CAN_OK != result) {
        RP_LOG(context,LOG_ERR, "*%s %s (%d)",func, rp_CanGetError(result), result);
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*%s %s (%d)",func, rp_CanGetError(result), result);
    return SCPI_RES_OK;
}


scpi_result_t RP_CAN_RestartTimeQ(scpi_t * context){
    RP_F_NAME(const char func[] = "CAN#:RestartTime?";)

    auto itf = RP_CAN_0;
    if (!parseInterface(context,&itf,func)){
        return SCPI_RES_ERR;
    }

    uint32_t t;

    auto result = rp_CanGetRestartTime(itf,&t);

    if (RP_HW_CAN_OK != result) {
        RP_LOG(context,LOG_ERR, "*%s %s (%d)",func, rp_CanGetError(result), result);
        return SCPI_RES_ERR;
    }

    SCPI_ResultUInt32Base(context, t, 10);

    RP_LOG(context,LOG_INFO, "*%s %s (%d)",func, rp_CanGetError(result), result);
    return SCPI_RES_OK;
}


scpi_result_t RP_CAN_ControllerMode(scpi_t * context){
    RP_F_NAME(const char func[] = "CAN#:MODE";)

    auto itf = RP_CAN_0;
    if (!parseInterface(context,&itf,func)){
        return SCPI_RES_ERR;
    }

    int32_t mode;
    int32_t state;

    if (!SCPI_ParamChoice(context, scpi_CAN_Mode, &mode, true)) {
        RP_LOG(context,LOG_ERR, "*%s is missing first parameter.");
        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamChoice(context, scpi_CAN_Bool, &state, true)) {
        RP_LOG(context,LOG_ERR, "*%s is missing second parameter.");
        return SCPI_RES_ERR;
    }

    auto result = rp_CanSetControllerMode(itf,(rp_can_mode_t)mode,state);

    if (RP_HW_CAN_OK != result) {
        RP_LOG(context,LOG_ERR, "*%s %s (%d)",func, rp_CanGetError(result), result);
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*%s %s (%d)",func, rp_CanGetError(result), result);
    return SCPI_RES_OK;
}


scpi_result_t RP_CAN_ControllerModeQ(scpi_t * context){
    RP_F_NAME(const char func[] = "CAN#:MODE?";)

    auto itf = RP_CAN_0;
    if (!parseInterface(context,&itf,func)){
        return SCPI_RES_ERR;
    }

    int32_t mode;
    bool state;

    if (!SCPI_ParamChoice(context, scpi_CAN_Mode, &mode, true)) {
        RP_LOG(context,LOG_ERR, "*%s is missing first parameter.");
        return SCPI_RES_ERR;
    }

    auto result = rp_CanGetControllerMode(itf,(rp_can_mode_t)mode,&state);

    if (RP_HW_CAN_OK != result) {
        RP_LOG(context,LOG_ERR, "*%s %s (%d)",func, rp_CanGetError(result), result);
        return SCPI_RES_ERR;
    }

    const char *_name;
    if(!SCPI_ChoiceToName(scpi_CAN_Bool, (int32_t)state, &_name)){
        RP_LOG(context,LOG_ERR, "*%s Failed to parse bool value.",func);
        return SCPI_RES_ERR;
    }
    SCPI_ResultMnemonic(context, _name);

    RP_LOG(context,LOG_INFO, "*%s %s (%d)",func, rp_CanGetError(result), result);
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_Open(scpi_t * context){
    RP_F_NAME(const char func[] = "CAN#:OPEN";)

    auto itf = RP_CAN_0;
    if (!parseInterface(context,&itf,func)){
        return SCPI_RES_ERR;
    } 

    auto result = rp_CanOpen(itf);

    if (RP_HW_CAN_OK != result) {
        RP_LOG(context,LOG_ERR, "*%s %s (%d)",func, rp_CanGetError(result), result);
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*%s %s (%d)",func, rp_CanGetError(result), result);
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_Close(scpi_t * context){
    RP_F_NAME(const char func[] = "CAN#:CLOSE";)

    auto itf = RP_CAN_0;
    if (!parseInterface(context,&itf,func)){
        return SCPI_RES_ERR;
    } 

    auto result = rp_CanClose(itf);

    if (RP_HW_CAN_OK != result) {
        RP_LOG(context,LOG_ERR, "*%s %s (%d)",func, rp_CanGetError(result), result);
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*%s %s (%d)",func, rp_CanGetError(result), result);
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_SendEx(scpi_t * context,bool extended, bool rtr, const char *func){

    int32_t cmd[2] = {0,0};
    
    if (!SCPI_CommandNumbers(context,cmd,2,-1)){
        RP_LOG(context,LOG_ERR, "*%s Failed to get parameters.",func);
        return SCPI_RES_ERR;
    }
 
    if (cmd[0] == -1){
        RP_LOG(context,LOG_ERR, "*%s Failed to get interface number",func);
        return SCPI_RES_ERR;
    }

    if (cmd[1] == -1){
        RP_LOG(context,LOG_ERR, "*%s Failed to get can id",func);
        return SCPI_RES_ERR;
    }

    uint8_t buffer[8];
    uint32_t buf_size = 8;
    if(!SCPI_ParamBufferUInt8(context, buffer, &buf_size, true)){
        RP_LOG(context,LOG_ERR, "*%s Failed get data.",func);
        return SCPI_RES_ERR;
    }


    auto interface = (rp_can_interface_t)cmd[0];
    uint32_t can_id = cmd[1];

    auto result = rp_CanSend(interface,can_id,buffer,buf_size,extended,rtr,0);

    if (RP_HW_CAN_OK != result) {
        RP_LOG(context,LOG_ERR, "*%s %s (%d)",func, rp_CanGetError(result), result);
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*%s %s (%d)",func, rp_CanGetError(result), result);
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_SendTimeoutEx(scpi_t * context,bool extended, bool rtr, const char *func){

    int32_t cmd[3] = {0,0,0};
    
    if (!SCPI_CommandNumbers(context,cmd,3,-1)){
        RP_LOG(context,LOG_ERR, "*%s Failed to get parameters.",func);
        return SCPI_RES_ERR;
    }
 
    if (cmd[0] == -1){
        RP_LOG(context,LOG_ERR, "*%s Failed to get interface number",func);
        return SCPI_RES_ERR;
    }

    if (cmd[1] == -1){
        RP_LOG(context,LOG_ERR, "*%s Failed to get can id",func);
        return SCPI_RES_ERR;
    }

    if (cmd[2] == -1){
        RP_LOG(context,LOG_ERR, "*%s Failed to get timeout",func);
        return SCPI_RES_ERR;
    }

    uint8_t buffer[8];
    uint32_t buf_size = 8;
    if(!SCPI_ParamBufferUInt8(context, buffer, &buf_size, true)){
        RP_LOG(context,LOG_ERR, "*%s Failed get data.",func);
        return SCPI_RES_ERR;
    }


    auto interface = (rp_can_interface_t)cmd[0];
    uint32_t can_id = cmd[1];
    uint32_t timeout = cmd[2];

    auto result = rp_CanSend(interface,can_id,buffer,buf_size,extended,rtr,timeout);

    if (RP_HW_CAN_OK != result) {
        RP_LOG(context,LOG_ERR, "*%s %s (%d)",func, rp_CanGetError(result), result);
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*%s %s (%d)",func, rp_CanGetError(result), result);
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_Send(scpi_t * context){
    RP_F_NAME(const char func[] = "CAN#:Send#";)
    return RP_CAN_SendEx(context,false,false,func);
}

scpi_result_t RP_CAN_SendTimeout(scpi_t * context){
    RP_F_NAME(const char func[] = "CAN#:Send#:Timeout#";)
    return RP_CAN_SendTimeoutEx(context,false,false,func);
}

scpi_result_t RP_CAN_SendExtended(scpi_t * context){
    RP_F_NAME(const char func[] = "CAN#:Send#:E";)
    return RP_CAN_SendEx(context,true,false,func);
}

scpi_result_t RP_CAN_SendExtendedTimeout(scpi_t * context){
    RP_F_NAME(const char func[] = "CAN#:Send#:Timeout#:E";)
    return RP_CAN_SendTimeoutEx(context,true,false,func);
}

scpi_result_t RP_CAN_SendRTR(scpi_t * context){
    RP_F_NAME(const char func[] = "CAN#:Send#:RTR";)
    return RP_CAN_SendEx(context,false,true,func);
}

scpi_result_t RP_CAN_SendTimeoutRTR(scpi_t * context){
    RP_F_NAME(const char func[] = "CAN#:Send#:Timeout#:RTR";)
    return RP_CAN_SendTimeoutEx(context,false,true,func);
}

scpi_result_t RP_CAN_SendExtendedRTR(scpi_t * context){
    RP_F_NAME(const char func[] = "CAN#:Send#:E:RTR";)
    return RP_CAN_SendEx(context,true,true,func);
}

scpi_result_t RP_CAN_SendExtendedTimeoutRTR(scpi_t * context){
    RP_F_NAME(const char func[] = "CAN#:Send#:Timeout#:E:RTR";)
    return RP_CAN_SendTimeoutEx(context,true,true,func);
}

scpi_result_t RP_CAN_Read(scpi_t * context){
    RP_F_NAME(const char func[] = "CAN#:Read";)
    int32_t cmd[1] = {0};
    
    if (!SCPI_CommandNumbers(context,cmd,1,-1)){
        RP_LOG(context,LOG_ERR, "*%s Failed to get parameters.",func);
        return SCPI_RES_ERR;
    }
 
    if (cmd[0] == -1){
        RP_LOG(context,LOG_ERR, "*%s Failed to get interface number",func);
        return SCPI_RES_ERR;
    }

    auto interface = (rp_can_interface_t)cmd[0];
    
    rp_can_frame_t fm;
    auto result = rp_CanRead(interface,0,&fm);

    if (RP_HW_CAN_OK != result) {
        RP_LOG(context,LOG_ERR, "*%s %s (%d)",func, rp_CanGetError(result), result);
        return SCPI_RES_ERR;
    }

    SCPI_ResultUInt32Base(context, fm.can_id, 10);
    SCPI_ResultUInt32Base(context, fm.can_id_raw, 10);
    SCPI_ResultBool(context, fm.is_extended_format);
    SCPI_ResultBool(context, fm.is_error_frame);
    SCPI_ResultBool(context, fm.is_remote_request);
    SCPI_ResultUInt32Base(context, fm.can_dlc, 10);
    SCPI_ResultBufferUInt8(context, fm.data, fm.can_dlc);

    RP_LOG(context,LOG_INFO, "*%s %s (%d)",func, rp_CanGetError(result), result);
    return SCPI_RES_OK;
}


scpi_result_t RP_CAN_ReadTimeout(scpi_t * context){
    RP_F_NAME(const char func[] = "CAN#:Read:Timeout#";)
    int32_t cmd[2] = {0,0};
    
    if (!SCPI_CommandNumbers(context,cmd,1,-1)){
        RP_LOG(context,LOG_ERR, "*%s Failed to get parameters.",func);
        return SCPI_RES_ERR;
    }
 
    if (cmd[0] == -1){
        RP_LOG(context,LOG_ERR, "*%s Failed to get interface number",func);
        return SCPI_RES_ERR;
    }

    if (cmd[1] == -1){
        RP_LOG(context,LOG_ERR, "*%s Failed to get timeout",func);
        return SCPI_RES_ERR;
    }

    auto interface = (rp_can_interface_t)cmd[0];
    uint32_t timeout = cmd[1];
    
    rp_can_frame_t fm;
    auto result = rp_CanRead(interface,timeout,&fm);

    if (RP_HW_CAN_OK != result) {
        RP_LOG(context,LOG_ERR, "*%s %s (%d)",func, rp_CanGetError(result), result);
        return SCPI_RES_ERR;
    }

    SCPI_ResultUInt32Base(context, fm.can_id, 10);
    SCPI_ResultUInt32Base(context, fm.can_id_raw, 10);
    SCPI_ResultBool(context, fm.is_extended_format);
    SCPI_ResultBool(context, fm.is_error_frame);
    SCPI_ResultBool(context, fm.is_remote_request);
    SCPI_ResultUInt32Base(context, fm.can_dlc, 10);
    SCPI_ResultBufferUInt8(context, fm.data, fm.can_dlc);

    RP_LOG(context,LOG_INFO, "*%s %s (%d)",func, rp_CanGetError(result), result);
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_AddFilter(scpi_t * context){
    RP_F_NAME(const char func[] = "CAN#:Filter:Add";)

    auto itf = RP_CAN_0;
    if (!parseInterface(context,&itf,func)){
        return SCPI_RES_ERR;
    }

    uint32_t filter;
    uint32_t mask;

    if (!SCPI_ParamUInt32(context, &filter, true)) {
        RP_LOG(context,LOG_ERR, "*%s is missing first parameter.",func);
        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamUInt32(context, &mask, true)) {
        RP_LOG(context,LOG_ERR, "*%s is missing second parameter.",func);
        return SCPI_RES_ERR;
    }

    auto result = rp_CanAddFilter(itf,filter,mask);

    if (RP_HW_CAN_OK != result) {
        RP_LOG(context,LOG_ERR, "*%s %s (%d)",func, rp_CanGetError(result), result);
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*%s %s (%d)",func, rp_CanGetError(result), result);
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_RemoveFilter(scpi_t * context){
    RP_F_NAME(const char func[] = "CAN#:Filter:Remove";)

    auto itf = RP_CAN_0;
    if (!parseInterface(context,&itf,func)){
        return SCPI_RES_ERR;
    }

    uint32_t filter;
    uint32_t mask;

    if (!SCPI_ParamUInt32(context, &filter, true)) {
        RP_LOG(context,LOG_ERR, "*%s is missing first parameter.",func);
        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamUInt32(context, &mask, true)) {
        RP_LOG(context,LOG_ERR, "*%s is missing second parameter.",func);
        return SCPI_RES_ERR;
    }

    auto result = rp_CanRemoveFilter(itf,filter,mask);

    if (RP_HW_CAN_OK != result) {
        RP_LOG(context,LOG_ERR, "*%s %s (%d)",func, rp_CanGetError(result), result);
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*%s %s (%d)",func, rp_CanGetError(result), result);
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_ClearFilter(scpi_t * context){
    RP_F_NAME(const char func[] = "CAN#:Filter:Clear";)

    auto itf = RP_CAN_0;
    if (!parseInterface(context,&itf,func)){
        return SCPI_RES_ERR;
    }


    auto result = rp_CanClearFilter(itf);

    if (RP_HW_CAN_OK != result) {
        RP_LOG(context,LOG_ERR, "*%s %s (%d)",func, rp_CanGetError(result), result);
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*%s %s (%d)",func, rp_CanGetError(result), result);
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_SetFilter(scpi_t * context){
    RP_F_NAME(const char func[] = "CAN#:Filter:Set";)

    auto itf = RP_CAN_0;
    if (!parseInterface(context,&itf,func)){
        return SCPI_RES_ERR;
    }

    auto result = rp_CanSetFilter(itf,false);

    if (RP_HW_CAN_OK != result) {
        RP_LOG(context,LOG_ERR, "*%s %s (%d)",func, rp_CanGetError(result), result);
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*%s %s (%d)",func, rp_CanGetError(result), result);
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_ShowErrorFrames(scpi_t * context){
    RP_F_NAME(const char func[] = "CAN#:SHOW:ERROR";)
    int value;

    auto itf = RP_CAN_0;
    if (!parseInterface(context,&itf,func)){
        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamChoice(context, scpi_CAN_Bool, &value, true)) {
        RP_LOG(context,LOG_ERR, "*%s is missing first parameter.",func);
        return SCPI_RES_ERR;
    }

    int result = rp_CanShowErrorFrames(itf,value);
    if (RP_HW_CAN_OK != result) {
        RP_LOG(context,LOG_ERR, "*%s %s (%d)",func, rp_CanGetError(result), result);
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*%s %s (%d)",func, rp_CanGetError(result), result);
    return SCPI_RES_OK;
}