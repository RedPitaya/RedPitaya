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
#include <stdlib.h>
#include <string.h>

#include "can.h"
#include "common.h"
#include "rp_hw_can.h"
#include "scpi-parser-ext.h"
#include "scpi/parser.h"

const scpi_choice_def_t scpi_CAN_Bool[] = {{"OFF", 0}, {"ON", 1}, SCPI_CHOICE_LIST_END};

const scpi_choice_def_t scpi_CAN_State[] = {{"ERROR_ACTIVE", RP_CAN_STATE_ERROR_ACTIVE},
                                            {"ERROR_WARNING", RP_CAN_STATE_ERROR_WARNING},
                                            {"ERROR_PASSIVE", RP_CAN_STATE_ERROR_PASSIVE},
                                            {"BUS_OFF", RP_CAN_STATE_BUS_OFF},
                                            {"STOPPED", RP_CAN_STATE_STOPPED},
                                            {"SLEEPING", RP_CAN_STATE_SLEEPING},
                                            SCPI_CHOICE_LIST_END};

const scpi_choice_def_t scpi_CAN_Mode[] = {{"LOOPBACK", RP_CAN_MODE_LOOPBACK}, {"BERR_REPORTING", RP_CAN_MODE_BERR_REPORTING}, SCPI_CHOICE_LIST_END};

auto parseInterface(scpi_t* context, rp_can_interface_t* interface) -> bool {
    int32_t cmd[1] = {0};

    if (!SCPI_CommandNumbers(context, cmd, 1, -1)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get parameters.")
        return false;
    }

    if (cmd[0] == -1) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get interface number.")
        return false;
    }
    *interface = (rp_can_interface_t)cmd[0];
    return true;
}

scpi_result_t RP_CAN_FpgaEnable(scpi_t* context) {
    int value;

    if (!SCPI_ParamChoice(context, scpi_CAN_Bool, &value, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }

    int result = rp_CanSetFPGAEnable(value);
    if (RP_HW_CAN_OK != result) {
        RP_LOG_CRIT("%s", rp_CanGetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG_INFO("%s", rp_CanGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_FpgaEnableQ(scpi_t* context) {
    bool value;
    int result = rp_CanGetFPGAEnable(&value);

    if (RP_HW_CAN_OK != result) {
        RP_LOG_CRIT("%s", rp_CanGetError(result));
        return SCPI_RES_ERR;
    }

    const char* _name;
    if (!SCPI_ChoiceToName(scpi_CAN_Bool, (int32_t)value, &_name)) {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed to parse bool value.")
        return SCPI_RES_ERR;
    }
    SCPI_ResultMnemonic(context, _name);

    RP_LOG_INFO("%s", rp_CanGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_Start(scpi_t* context) {
    auto itf = RP_CAN_0;
    if (!parseInterface(context, &itf)) {
        return SCPI_RES_ERR;
    }

    auto result = rp_CanStart(itf);

    if (RP_HW_CAN_OK != result) {
        RP_LOG_CRIT("%s", rp_CanGetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG_INFO("%s", rp_CanGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_Stop(scpi_t* context) {
    auto itf = RP_CAN_0;
    if (!parseInterface(context, &itf)) {
        return SCPI_RES_ERR;
    }

    auto result = rp_CanStop(itf);

    if (RP_HW_CAN_OK != result) {
        RP_LOG_CRIT("%s", rp_CanGetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG_INFO("%s", rp_CanGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_Restart(scpi_t* context) {
    auto itf = RP_CAN_0;
    if (!parseInterface(context, &itf)) {
        return SCPI_RES_ERR;
    }

    auto result = rp_CanRestart(itf);

    if (RP_HW_CAN_OK != result) {
        RP_LOG_CRIT("%s", rp_CanGetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG_INFO("%s", rp_CanGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_StateQ(scpi_t* context) {
    auto itf = RP_CAN_0;
    if (!parseInterface(context, &itf)) {
        return SCPI_RES_ERR;
    }
    rp_can_state_t state;
    auto result = rp_CanGetState(itf, &state);

    if (RP_HW_CAN_OK != result) {
        RP_LOG_CRIT("%s", rp_CanGetError(result));
        return SCPI_RES_ERR;
    }

    const char* _name;
    if (!SCPI_ChoiceToName(scpi_CAN_State, (int32_t)state, &_name)) {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed to parse state value.")
        return SCPI_RES_ERR;
    }
    SCPI_ResultMnemonic(context, _name);

    RP_LOG_INFO("%s", rp_CanGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_Bitrate(scpi_t* context) {
    auto itf = RP_CAN_0;
    if (!parseInterface(context, &itf)) {
        return SCPI_RES_ERR;
    }

    uint32_t value;

    if (!SCPI_ParamUInt32(context, &value, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }

    auto result = rp_CanSetBitrate(itf, value);

    if (RP_HW_CAN_OK != result) {
        RP_LOG_CRIT("%s", rp_CanGetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG_INFO("%s", rp_CanGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_BitrateSamplePoint(scpi_t* context) {
    auto itf = RP_CAN_0;
    if (!parseInterface(context, &itf)) {
        return SCPI_RES_ERR;
    }

    uint32_t bitrate;
    float sp;

    if (!SCPI_ParamUInt32(context, &bitrate, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamFloat(context, &sp, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing second parameter.");
        return SCPI_RES_ERR;
    }

    auto result = rp_CanSetBitrateAndSamplePoint(itf, bitrate, sp);

    if (RP_HW_CAN_OK != result) {
        RP_LOG_CRIT("%s", rp_CanGetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG_INFO("%s", rp_CanGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_BitrateSamplePointQ(scpi_t* context) {
    auto itf = RP_CAN_0;
    if (!parseInterface(context, &itf)) {
        return SCPI_RES_ERR;
    }

    uint32_t bitrate = 0;
    float sp = 0;

    auto result = rp_CanGetBitrateAndSamplePoint(itf, &bitrate, &sp);

    if (RP_HW_CAN_OK != result) {
        RP_LOG_INFO("%s", rp_CanGetError(result));
    }

    SCPI_ResultUInt32Base(context, bitrate, 10);
    SCPI_ResultFloat(context, sp);

    RP_LOG_INFO("%s", rp_CanGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_BitTiming(scpi_t* context) {
    auto itf = RP_CAN_0;
    if (!parseInterface(context, &itf)) {
        return SCPI_RES_ERR;
    }

    rp_can_bittiming_t bt;

    if (!SCPI_ParamUInt32(context, &bt.tq, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing tq parameter.");
        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamUInt32(context, &bt.prop_seg, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing prop_seg parameter.");
        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamUInt32(context, &bt.phase_seg1, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing phase_seg1 parameter.");
        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamUInt32(context, &bt.phase_seg2, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing phase_seg2 parameter.");
        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamUInt32(context, &bt.sjw, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing sjw parameter.");
        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamUInt32(context, &bt.brp, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing brp parameter.");
        return SCPI_RES_ERR;
    }

    auto result = rp_CanSetBitTiming(itf, bt);

    if (RP_HW_CAN_OK != result) {
        RP_LOG_CRIT("%s", rp_CanGetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG_INFO("%s", rp_CanGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_BitTimingQ(scpi_t* context) {
    auto itf = RP_CAN_0;
    if (!parseInterface(context, &itf)) {
        return SCPI_RES_ERR;
    }

    rp_can_bittiming_t bt;
    memset(&bt, 0, sizeof(rp_can_bittiming_t));

    auto result = rp_CanGetBitTiming(itf, &bt);

    if (RP_HW_CAN_OK != result) {
        RP_LOG_INFO("%s", rp_CanGetError(result));
    }

    SCPI_ResultUInt32Base(context, bt.tq, 10);
    SCPI_ResultUInt32Base(context, bt.prop_seg, 10);
    SCPI_ResultUInt32Base(context, bt.phase_seg1, 10);
    SCPI_ResultUInt32Base(context, bt.phase_seg2, 10);
    SCPI_ResultUInt32Base(context, bt.sjw, 10);
    SCPI_ResultUInt32Base(context, bt.brp, 10);

    RP_LOG_INFO("%s", rp_CanGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_BitTimingLimitsQ(scpi_t* context) {
    auto itf = RP_CAN_0;
    if (!parseInterface(context, &itf)) {
        return SCPI_RES_ERR;
    }

    rp_can_bittiming_limits_t bt;

    auto result = rp_CanGetBitTimingLimits(itf, &bt);

    if (RP_HW_CAN_OK != result) {
        RP_LOG_CRIT("%s", rp_CanGetError(result));
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

    RP_LOG_INFO("%s", rp_CanGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_ClockFreqQ(scpi_t* context) {
    auto itf = RP_CAN_0;
    if (!parseInterface(context, &itf)) {
        return SCPI_RES_ERR;
    }
    uint32_t freq;
    auto result = rp_CanGetClockFreq(itf, &freq);

    if (RP_HW_CAN_OK != result) {
        RP_LOG_CRIT("%s", rp_CanGetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultUInt32Base(context, freq, 10);

    RP_LOG_INFO("%s", rp_CanGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_BusErrorCountersQ(scpi_t* context) {
    auto itf = RP_CAN_0;
    if (!parseInterface(context, &itf)) {
        return SCPI_RES_ERR;
    }
    uint16_t tx, rx;
    auto result = rp_CanGetBusErrorCounters(itf, &tx, &rx);

    if (RP_HW_CAN_OK != result) {
        RP_LOG_CRIT("%s", rp_CanGetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultUInt32Base(context, tx, 10);
    SCPI_ResultUInt32Base(context, rx, 10);

    RP_LOG_INFO("%s", rp_CanGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_RestartTime(scpi_t* context) {
    auto itf = RP_CAN_0;
    if (!parseInterface(context, &itf)) {
        return SCPI_RES_ERR;
    }

    uint32_t value;

    if (!SCPI_ParamUInt32(context, &value, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }

    auto result = rp_CanSetRestartTime(itf, value);

    if (RP_HW_CAN_OK != result) {
        RP_LOG_CRIT("%s", rp_CanGetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG_INFO("%s", rp_CanGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_RestartTimeQ(scpi_t* context) {
    auto itf = RP_CAN_0;
    if (!parseInterface(context, &itf)) {
        return SCPI_RES_ERR;
    }

    uint32_t t;

    auto result = rp_CanGetRestartTime(itf, &t);

    if (RP_HW_CAN_OK != result) {
        RP_LOG_CRIT("%s", rp_CanGetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultUInt32Base(context, t, 10);

    RP_LOG_INFO("%s", rp_CanGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_ControllerMode(scpi_t* context) {
    auto itf = RP_CAN_0;
    if (!parseInterface(context, &itf)) {
        return SCPI_RES_ERR;
    }

    int32_t mode;
    int32_t state;

    if (!SCPI_ParamChoice(context, scpi_CAN_Mode, &mode, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamChoice(context, scpi_CAN_Bool, &state, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing second parameter.");
        return SCPI_RES_ERR;
    }

    auto result = rp_CanSetControllerMode(itf, (rp_can_mode_t)mode, state);

    if (RP_HW_CAN_OK != result) {
        RP_LOG_CRIT("%s", rp_CanGetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG_INFO("%s", rp_CanGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_ControllerModeQ(scpi_t* context) {
    auto itf = RP_CAN_0;
    if (!parseInterface(context, &itf)) {
        return SCPI_RES_ERR;
    }

    int32_t mode;
    bool state;

    if (!SCPI_ParamChoice(context, scpi_CAN_Mode, &mode, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }

    auto result = rp_CanGetControllerMode(itf, (rp_can_mode_t)mode, &state);

    if (RP_HW_CAN_OK != result) {
        RP_LOG_CRIT("%s", rp_CanGetError(result));
        return SCPI_RES_ERR;
    }

    const char* _name;
    if (!SCPI_ChoiceToName(scpi_CAN_Bool, (int32_t)state, &_name)) {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed to parse bool value.")
        return SCPI_RES_ERR;
    }
    SCPI_ResultMnemonic(context, _name);

    RP_LOG_INFO("%s", rp_CanGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_Open(scpi_t* context) {
    auto itf = RP_CAN_0;
    if (!parseInterface(context, &itf)) {
        return SCPI_RES_ERR;
    }

    auto result = rp_CanOpen(itf);

    if (RP_HW_CAN_OK != result) {
        RP_LOG_CRIT("%s", rp_CanGetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG_INFO("%s", rp_CanGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_Close(scpi_t* context) {
    auto itf = RP_CAN_0;
    if (!parseInterface(context, &itf)) {
        return SCPI_RES_ERR;
    }

    auto result = rp_CanClose(itf);

    if (RP_HW_CAN_OK != result) {
        RP_LOG_CRIT("%s", rp_CanGetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG_INFO("%s", rp_CanGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_Send(scpi_t* context) {
    bool extended = strstr(context->param_list.cmd_raw.data, ":E") != NULL;
    ;
    bool rtr = strstr(context->param_list.cmd_raw.data, ":RTR") != NULL;
    bool isTimeout = strstr(context->param_list.cmd_raw.data, ":T") != NULL;
    int paramCount = isTimeout ? 3 : 2;

    int32_t cmd[3] = {0, 0, 0};

    if (!SCPI_CommandNumbers(context, cmd, paramCount, -1)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get parameters.");
        return SCPI_RES_ERR;
    }

    if (cmd[0] == -1) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get interface number.");
        return SCPI_RES_ERR;
    }

    if (cmd[1] == -1) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get can id.");
        return SCPI_RES_ERR;
    }

    if (cmd[2] == -1 && isTimeout) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get timeout.");
        return SCPI_RES_ERR;
    }

    uint8_t buffer[8];
    uint32_t buf_size = 8;
    if (!SCPI_ParamBufferUInt8(context, buffer, &buf_size, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed get data.")
        return SCPI_RES_ERR;
    }

    auto interface = (rp_can_interface_t)cmd[0];
    uint32_t can_id = cmd[1];
    uint32_t timeout = isTimeout ? cmd[2] : 0;

    auto result = rp_CanSend(interface, can_id, buffer, buf_size, extended, rtr, timeout);

    if (RP_HW_CAN_OK != result) {
        RP_LOG_CRIT("%s", rp_CanGetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG_INFO("%s", rp_CanGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_ReadQ(scpi_t* context) {
    bool isTimeout = strstr(context->param_list.cmd_raw.data, ":T") != NULL;
    int paramCount = isTimeout ? 2 : 1;
    int32_t cmd[2] = {0, 0};
    bool error = false;

    if (!SCPI_CommandNumbers(context, cmd, paramCount, -1)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get parameters.");
        return SCPI_RES_ERR;
    }

    if (cmd[0] == -1) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get interface number.");
        return SCPI_RES_ERR;
    }

    if (cmd[1] == -1 && isTimeout) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get timeout.");
        return SCPI_RES_ERR;
    }

    auto interface = (rp_can_interface_t)cmd[0];
    uint32_t timeout = isTimeout ? cmd[1] : 0;

    rp_can_frame_t fm;
    auto result = rp_CanRead(interface, timeout, &fm);

    if (RP_HW_CAN_OK != result) {
        RP_LOG_CRIT("%s", rp_CanGetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultUInt32Base(context, fm.can_id, 10);
    SCPI_ResultUInt32Base(context, fm.can_id_raw, 10);
    SCPI_ResultBool(context, fm.is_extended_format);
    SCPI_ResultBool(context, fm.is_error_frame);
    SCPI_ResultBool(context, fm.is_remote_request);
    SCPI_ResultUInt32Base(context, fm.can_dlc, 10);
    SCPI_ResultBufferUInt8(context, fm.data, fm.can_dlc, &error);

    if (error) {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed to send data");
        return SCPI_RES_ERR;
    }

    RP_LOG_INFO("%s", rp_CanGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_AddFilter(scpi_t* context) {
    auto itf = RP_CAN_0;
    if (!parseInterface(context, &itf)) {
        return SCPI_RES_ERR;
    }

    uint32_t filter;
    uint32_t mask;

    if (!SCPI_ParamUInt32(context, &filter, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamUInt32(context, &mask, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing second parameter.");
        return SCPI_RES_ERR;
    }

    auto result = rp_CanAddFilter(itf, filter, mask);

    if (RP_HW_CAN_OK != result) {
        RP_LOG_CRIT("%s", rp_CanGetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG_INFO("%s", rp_CanGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_RemoveFilter(scpi_t* context) {
    auto itf = RP_CAN_0;
    if (!parseInterface(context, &itf)) {
        return SCPI_RES_ERR;
    }

    uint32_t filter;
    uint32_t mask;

    if (!SCPI_ParamUInt32(context, &filter, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamUInt32(context, &mask, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing second parameter.");
        return SCPI_RES_ERR;
    }

    auto result = rp_CanRemoveFilter(itf, filter, mask);

    if (RP_HW_CAN_OK != result) {
        RP_LOG_CRIT("%s", rp_CanGetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG_INFO("%s", rp_CanGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_ClearFilter(scpi_t* context) {
    auto itf = RP_CAN_0;
    if (!parseInterface(context, &itf)) {
        return SCPI_RES_ERR;
    }

    auto result = rp_CanClearFilter(itf);

    if (RP_HW_CAN_OK != result) {
        RP_LOG_CRIT("%s", rp_CanGetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG_INFO("%s", rp_CanGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_SetFilter(scpi_t* context) {
    auto itf = RP_CAN_0;
    if (!parseInterface(context, &itf)) {
        return SCPI_RES_ERR;
    }

    auto result = rp_CanSetFilter(itf, false);

    if (RP_HW_CAN_OK != result) {
        RP_LOG_CRIT("%s", rp_CanGetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG_INFO("%s", rp_CanGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_CAN_ShowErrorFrames(scpi_t* context) {
    int value;

    auto itf = RP_CAN_0;
    if (!parseInterface(context, &itf)) {
        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamChoice(context, scpi_CAN_Bool, &value, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }

    int result = rp_CanShowErrorFrames(itf, value);
    if (RP_HW_CAN_OK != result) {
        RP_LOG_CRIT("%s", rp_CanGetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG_INFO("%s", rp_CanGetError(result))
    return SCPI_RES_OK;
}