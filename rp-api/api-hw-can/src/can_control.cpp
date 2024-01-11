#include <stdio.h>
#include "rp_hw_can.h"
#include "can_control.h"
#include "common.h"
#include <libsocketcan.h>

auto control_Start(const char *_name) -> int {
    if (can_do_start(_name) < 0) {
        return RP_HW_CAN_ESI;
	}
    return RP_HW_CAN_OK;
}

auto control_Stop(const char *_name) -> int {
    if (can_do_stop(_name) < 0) {
        return RP_HW_CAN_ESI;
	}
    return RP_HW_CAN_OK;
}

auto control_Restart(const char *_name) -> int {
    if (can_do_restart(_name) < 0) {
        return RP_HW_CAN_ESI;
	}
    return RP_HW_CAN_OK;
}


auto control_SetBitrateAndSamplePoint(const char *_name, uint32_t _bitRateHz, float _samplePoint) -> int {
    auto isSP = _samplePoint != -1;
    uint32_t iSP = _samplePoint * 1000;
	int err;    
    if (isSP){
        if (_samplePoint < 0 || _samplePoint > 0.999) 
            return RP_HW_CAN_EBS;
    	err = can_set_bitrate_samplepoint(_name, _bitRateHz, iSP);
    }
	else{
		err = can_set_bitrate(_name, _bitRateHz);
    }
    if (err < 0) {
        return RP_HW_CAN_EBS;
    }
    return RP_HW_CAN_OK;
}

auto control_GetBitrateAndSamplePoint(const char *_name, uint32_t *_bitRateHz, float *_samplePoint) -> int{
    struct can_bittiming bt;
	if (can_get_bittiming(_name, &bt) < 0) {
        return RP_HW_CAN_EBS;
	}
    *_bitRateHz = bt.bitrate;
    *_samplePoint = (float)((float)bt.sample_point / 1000);
    return RP_HW_CAN_OK;
}

auto control_GetBitTiming(const char *_name, rp_can_bittiming_t *_bitTiming) -> int{
    struct can_bittiming bt;
	if (can_get_bittiming(_name, &bt) < 0) {
        return RP_HW_CAN_EBT;
	}
    _bitTiming->tq = bt.tq;
    _bitTiming->prop_seg = bt.prop_seg;
    _bitTiming->phase_seg1 = bt.phase_seg1;
    _bitTiming->phase_seg2 = bt.phase_seg2;
    _bitTiming->sjw = bt.sjw;
    _bitTiming->brp = bt.brp;
    return RP_HW_CAN_OK;
}

auto control_SetBitTiming(const char *_name, rp_can_bittiming_t _bitTiming) -> int{
    struct can_bittiming bt;
    memset(&bt, 0, sizeof(bt));
    bt.tq = _bitTiming.tq;
    bt.prop_seg = _bitTiming.prop_seg;
    bt.phase_seg1 = _bitTiming.phase_seg1;
    bt.phase_seg2 = _bitTiming.phase_seg2;
    bt.sjw = _bitTiming.sjw;
    bt.brp = _bitTiming.brp;

	if (can_set_bittiming(_name, &bt) < 0) {
        return RP_HW_CAN_EBT;
	}
    return RP_HW_CAN_OK;
}

auto control_GetBitTimingLimits(const char *_name, rp_can_bittiming_limits_t *_bitTimingLimits) -> int{
    struct can_bittiming_const bt;
	if (can_get_bittiming_const(_name, &bt) < 0) {
        return RP_HW_CAN_EBT;
	}
    _bitTimingLimits->tseg1_min = bt.tseg1_min;
    _bitTimingLimits->tseg1_max = bt.tseg1_max;
    _bitTimingLimits->tseg2_min = bt.tseg2_min;
    _bitTimingLimits->tseg2_max = bt.tseg2_max;
    _bitTimingLimits->sjw_max = bt.sjw_max;
    _bitTimingLimits->brp_min = bt.brp_min;
    _bitTimingLimits->brp_max = bt.brp_max;
    _bitTimingLimits->brp_inc = bt.brp_inc;
    return RP_HW_CAN_OK;
}

auto control_GetClockFreq(const char *_name, uint32_t *_freq) -> int{
    struct can_clock clock;

	memset(&clock, 0, sizeof(struct can_clock));
	if (can_get_clock(_name, &clock) < 0) {
        return RP_HW_CAN_EGF;
    }
    *_freq = clock.freq;
    return RP_HW_CAN_OK;
}

auto control_GetBusErrorCounters(const char *_name, uint16_t *_tx, uint16_t *_rx) -> int{
    struct can_berr_counter bc;

	memset(&bc, 0, sizeof(struct can_berr_counter));

	if (can_get_berr_counter(_name, &bc) < 0) {
		return RP_HW_CAN_EGE;
	}
    *_tx = bc.txerr;
    *_rx = bc.rxerr;
    return RP_HW_CAN_OK;
}

auto control_SetRestartMs(const char *_name, uint32_t _ms) -> int{
    if (can_set_restart_ms(_name,_ms) < 0) {
        return RP_HW_CAN_ERT;
	}
    return RP_HW_CAN_OK;  
}

auto control_GetRestartMs(const char *_name, uint32_t *_ms) -> int{
    if (can_get_restart_ms(_name,_ms) < 0) {
        return RP_HW_CAN_ERT;
	}
    return RP_HW_CAN_OK;  
}

auto control_GetCanState(const char *_name, rp_can_state_t *_state) -> int{
    int state;
	if (can_get_state(_name, &state) < 0) {
        return RP_HW_CAN_EGS;
	}
    if (state > RP_CAN_STATE_SLEEPING)
        return RP_HW_CAN_EGS;
    *_state = static_cast<rp_can_state_t>(state);
    return RP_HW_CAN_OK;
}

auto control_SetMode(const char *_name, rp_can_mode_t _mode, bool _state) -> int{
    struct can_ctrlmode cm;
    auto mode = (uint32_t)_mode;
	if (can_get_ctrlmode(_name, &cm) < 0) {
        return RP_HW_CAN_ECM;
	}
    // Checking for supported modes may be implemented in future versions of the kernel.
    // if (!(cm.mask & _mode)){
    //     return RP_HW_CAN_ECU;
    // }
    auto f = cm.flags & ~mode;
    f = f | (_state ? mode : 0);
    cm.flags = f;
    cm.mask = mode;
    if (can_set_ctrlmode(_name, &cm) < 0) {
		return RP_HW_CAN_ECM;
	}
    return RP_HW_CAN_OK; 
}

auto control_GetMode(const char *_name, rp_can_mode_t _mode, bool *_state) -> int{
    struct can_ctrlmode cm;
    auto mode = (uint32_t)_mode;
	if (can_get_ctrlmode(_name, &cm) < 0) {
        return RP_HW_CAN_ECM;
	}
    *_state = cm.flags & mode;
    // Checking for supported modes may be implemented in future versions of the kernel.
    // if (!(cm.mask & _mode)){
    //     return RP_HW_CAN_ECU;
    // }
    return RP_HW_CAN_OK;
}
