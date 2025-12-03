/**
 * $Id: $
 *
 * @brief Red Pitaya Led System Module.
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#ifndef LED_SYSTEM_H
#define LED_SYSTEM_H

#include "rp_hw.h"

int led_GetMMCState(bool *_enable);
int led_SetMMCState(bool _enable);

int led_GetHeartBeatState(bool *_enable);
int led_SetHeartBeatState(bool _enable);

int led_GetEthState(bool *_state);
int led_SetEthState(bool _state);

#endif