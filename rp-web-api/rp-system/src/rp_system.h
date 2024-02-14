/**
 * $Id$
 *
 * @brief Red Pitaya Web module
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */


#ifndef __RP_SYSTEM_H__
#define __RP_SYSTEM_H__

#include <stdint.h>


typedef enum {
    RP_WS_NONE          = 0,
    RP_WS_CPU           = 0x1,
    RP_WS_TEMPERATURE   = 0x2,
    RP_WS_RAM           = 0x4,
    RP_WS_SLOW_DAC      = 0x8,
    RP_WS_SENSOR_VOLT   = 0x16,
    RP_WS_DISK_SIZE     = 0x32,
    RP_WS_ALL           = 0xFF
} rp_system_mode_t;

void rp_WS_Init();
void rp_WS_SetMode(rp_system_mode_t mode);
void rp_WS_SetInterval(rp_system_mode_t mode,uint32_t ms);
void rp_WS_UpdateParameters(bool force);
void rp_WS_PauseSend(bool state);
#endif

