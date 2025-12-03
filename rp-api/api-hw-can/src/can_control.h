/**
 * $Id: $
 *
 * @brief Red Pitaya CAN control
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#ifndef CAN_CONTROL_H
#define CAN_CONTROL_H



auto control_Start(const char *_name) -> int;
auto control_Stop(const char *_name) -> int;
auto control_Restart(const char *_name) -> int;
auto control_SetBitrateAndSamplePoint(const char *_name, uint32_t _bitRateHz, float _samplePoint) -> int;
auto control_GetBitrateAndSamplePoint(const char *_name, uint32_t *_bitRateHz, float *_samplePoint) -> int;
auto control_GetBitTiming(const char *_name, rp_can_bittiming_t *_bitTiming) -> int;
auto control_SetBitTiming(const char *_name, rp_can_bittiming_t _bitTiming) -> int;
auto control_GetBitTimingLimits(const char *_name, rp_can_bittiming_limits_t *_bitTimingLimits) -> int;
auto control_GetClockFreq(const char *_name, uint32_t *freq) -> int;
auto control_GetBusErrorCounters(const char *_name, uint16_t *_tx, uint16_t *_rx) -> int;
auto control_SetRestartMs(const char *_name, uint32_t _ms) -> int;
auto control_GetRestartMs(const char *_name, uint32_t *_ms) -> int;
auto control_GetCanState(const char *_name, rp_can_state_t *_state) -> int;
auto control_SetMode(const char *_name, rp_can_mode_t _mode, bool _state) -> int;
auto control_GetMode(const char *_name, rp_can_mode_t _mode, bool *_state) -> int;

#endif // CAN_CONTROL_H