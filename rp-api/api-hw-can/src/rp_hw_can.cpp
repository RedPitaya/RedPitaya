/**
 * $Id: $
 *
 * @brief Red Pitaya library API Hardware interface implementation
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

#include "rp_hw_can.h"
#include "rp.h"
#include "common/version.h"
#include "common.h"
#include "can_control.h"
#include "can_socket.h"

static char version[100];

const char *can_states[RP_CAN_STATE_SLEEPING + 1] = {
	"ERROR-ACTIVE",
	"ERROR-WARNING",
	"ERROR-PASSIVE",
	"BUS-OFF",
	"STOPPED",
	"SLEEPING"
};

int rp_CanSetFPGAEnable(bool _enable){
    ECHECK_APP_RP(rp_InitReset(false))
    ECHECK_APP_RP(rp_SetCANModeEnable(_enable));
    ECHECK_APP_RP(rp_Release())
    return RP_HW_CAN_OK;
}

int rp_CanGetFPGAEnable(bool *_state){
    ECHECK_APP_RP(rp_InitReset(false))
    ECHECK_APP_RP(rp_GetCANModeEnable(_state));
    ECHECK_APP_RP(rp_Release())
    return RP_HW_CAN_OK;
}

const char* rp_CanGetVersion()
{
    sprintf(version, "%s (%s)", VERSION_STR, REVISION_STR);
    return version;
}

const char* rp_CanGetError(int errorCode) {
    switch (errorCode) {
        case RP_HW_CAN_OK:    return "OK";
        case RP_HW_CAN_ESI:    return "Failed start interface";
        case RP_HW_CAN_EST:    return "Failed stop interface";
        case RP_HW_CAN_ERI:    return "Failed restart interface";
        case RP_HW_CAN_EUI:    return "Unknown interface";
        case RP_HW_CAN_EBS:    return "Failed set or get bitrate and sample point";
        case RP_HW_CAN_EBT:    return "Failed set or get bittiming";
        case RP_HW_CAN_EGF:    return "Failed get clock parameters";
        case RP_HW_CAN_EGE:    return "Failed get error counters";
        case RP_HW_CAN_ERT:    return "Failed set or get restart time";
        case RP_HW_CAN_EGS:    return "Failed get current interface state";
        case RP_HW_CAN_ECM:    return "Failed get or set controller mode";
        case RP_HW_CAN_ECU:    return "Controller mode not supported";
        case RP_HW_CAN_ESO:    return "Failed open socket";
        case RP_HW_CAN_ESC:    return "Failed close socket";
        case RP_HW_CAN_ESA:    return "Failed. Socket already open";
        case RP_HW_CAN_ESB:    return "Failed bind socket";
        case RP_HW_CAN_ESN:    return "Failed. Socket not opened";
        case RP_HW_CAN_ESD:    return "Failed. Missing data";
        case RP_HW_CAN_ESBO:   return "Failed. Send buffer overflow";
        case RP_HW_CAN_ESTE:   return "Failed. Timeout reached";
        case RP_HW_CAN_ESPE:   return "Failed. Poll error";
        case RP_HW_CAN_ESE:    return "Failed. Send error";
        case RP_HW_CAN_ESFA:   return "Failed add filter. Filter already present in list";
        case RP_HW_CAN_ESFS:   return "Failed apply filter";
        case RP_HW_CAN_ESEF:   return "Failed to set error handling";
        case RP_HW_CAN_ESR:    return "Failed read frame from socket";

        default:       return "Unknown error";
    }
}

int rp_CanStart(rp_can_interface_t _interface){
    auto ifname = getInterfaceName(_interface);
    if (!strcmp(ifname,""))
        return RP_HW_CAN_EUI;
    return control_Start(ifname);
}

int rp_CanStop(rp_can_interface_t _interface){
    auto ifname = getInterfaceName(_interface);
    if (!strcmp(ifname,""))
        return RP_HW_CAN_EUI;
    return control_Stop(ifname);
}

int rp_CanRestart(rp_can_interface_t _interface){
    auto ifname = getInterfaceName(_interface);
    if (!strcmp(ifname,""))
        return RP_HW_CAN_EUI;
    return control_Restart(ifname);
}

int rp_CanGetState(rp_can_interface_t _interface,rp_can_state_t *_state){
    auto ifname = getInterfaceName(_interface);
    if (!strcmp(ifname,""))
        return RP_HW_CAN_EUI;
    return control_GetCanState(ifname,_state);
}

const char * rp_CanGetStateName(rp_can_state_t state){
    return can_states[state];
}


int rp_CanSetBitrate(rp_can_interface_t _interface,uint32_t _bitRate){
    auto ifname = getInterfaceName(_interface);
    if (!strcmp(ifname,""))
        return RP_HW_CAN_EUI;
    return control_SetBitrateAndSamplePoint(ifname,_bitRate,-1);
}

int rp_CanSetBitrateAndSamplePoint(rp_can_interface_t _interface,uint32_t _bitRate,float _samplePoint){
    auto ifname = getInterfaceName(_interface);
    if (!strcmp(ifname,""))
        return RP_HW_CAN_EUI;
    return control_SetBitrateAndSamplePoint(ifname,_bitRate,_samplePoint);
}

int rp_CanGetBitrateAndSamplePoint(rp_can_interface_t _interface,uint32_t *_bitRate,float *_samplePoint){
    auto ifname = getInterfaceName(_interface);
    if (!strcmp(ifname,""))
        return RP_HW_CAN_EUI;
    return control_GetBitrateAndSamplePoint(ifname,_bitRate,_samplePoint);
}

int rp_CanGetBitTiming(rp_can_interface_t _interface, rp_can_bittiming_t *_bitTiming){
    auto ifname = getInterfaceName(_interface);
    if (!strcmp(ifname,""))
        return RP_HW_CAN_EUI;
    return control_GetBitTiming(ifname,_bitTiming);
}

int rp_CanSetBitTiming(rp_can_interface_t _interface, rp_can_bittiming_t _bitTiming){
    auto ifname = getInterfaceName(_interface);
    if (!strcmp(ifname,""))
        return RP_HW_CAN_EUI;
    return control_SetBitTiming(ifname,_bitTiming);
}

int rp_CanGetBitTimingLimits(rp_can_interface_t _interface, rp_can_bittiming_limits_t *_bitTimingLimits){
    auto ifname = getInterfaceName(_interface);
    if (!strcmp(ifname,""))
        return RP_HW_CAN_EUI;
    return control_GetBitTimingLimits(ifname,_bitTimingLimits);
}

int rp_CanGetClockFreq(rp_can_interface_t _interface,uint32_t *_freq){
    auto ifname = getInterfaceName(_interface);
    if (!strcmp(ifname,""))
        return RP_HW_CAN_EUI;
    return control_GetClockFreq(ifname,_freq);
}

int rp_CanGetBusErrorCounters(rp_can_interface_t _interface, uint16_t *_tx, uint16_t *_rx){
    auto ifname = getInterfaceName(_interface);
    if (!strcmp(ifname,""))
        return RP_HW_CAN_EUI;
    return control_GetBusErrorCounters(ifname,_tx,_rx);
}

int rp_CanSetRestartTime(rp_can_interface_t _interface, uint32_t _ms){
    auto ifname = getInterfaceName(_interface);
    if (!strcmp(ifname,""))
        return RP_HW_CAN_EUI;
    return control_SetRestartMs(ifname,_ms);
}

int rp_CanGetRestartTime(rp_can_interface_t _interface, uint32_t *_ms){
    auto ifname = getInterfaceName(_interface);
    if (!strcmp(ifname,""))
        return RP_HW_CAN_EUI;
    return control_GetRestartMs(ifname,_ms);
}

int rp_CanSetControllerMode(rp_can_interface_t _interface, rp_can_mode_t _mode, bool _state){
    auto ifname = getInterfaceName(_interface);
    if (!strcmp(ifname,""))
        return RP_HW_CAN_EUI;
    return control_SetMode(ifname,_mode,_state);
}

int rp_CanGetControllerMode(rp_can_interface_t _interface, rp_can_mode_t _mode, bool *_state){
    auto ifname = getInterfaceName(_interface);
    if (!strcmp(ifname,""))
        return RP_HW_CAN_EUI;
    return control_GetMode(ifname,_mode,_state);
}

int rp_CanOpen(rp_can_interface_t _interface){
    return socket_Open(_interface);
}

int rp_CanClose(rp_can_interface_t _interface){
    return socket_Close(_interface);
}

int rp_CanSend(rp_can_interface_t _interface, uint32_t _canId, unsigned char *_data, uint8_t _dataSize, bool _isExtended, bool _rtr, uint32_t _timeout){
    return socket_Send(_interface,_canId,_data,_dataSize,_isExtended,_rtr,_timeout);
}

int rp_CanRead(rp_can_interface_t _interface, uint32_t _timeout, rp_can_frame_t *_frame){
    return socket_Read(_interface,_timeout,_frame);
}

int rp_CanAddFilter(rp_can_interface_t _interface, uint32_t _filter, uint32_t _mask){
    return socket_AddFilter(_interface,_filter,_mask);
}

int rp_CanRemoveFilter(rp_can_interface_t _interface, uint32_t _filter, uint32_t _mask){
    return socket_RemoveFilter(_interface,_filter,_mask);
}

int rp_CanClearFilter(rp_can_interface_t _interface){
    return socket_ClearFilter(_interface);
}

int rp_CanSetFilter(rp_can_interface_t _interface, bool _isJoinFilter){
    return socket_SetFilter(_interface,_isJoinFilter);
}

int rp_CanShowErrorFrames(rp_can_interface_t _interface, bool _enable){
    return socket_ShowErrorFrames(_interface,_enable);
}