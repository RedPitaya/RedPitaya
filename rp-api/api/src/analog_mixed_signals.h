/**
 * $Id: $
 *
 * @brief Red Pitaya library Analog Mixed Signals (AMS) module interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#ifndef __ANALOG_MIXED_SIGNALS_H
#define __ANALOG_MIXED_SIGNALS_H

#include <string>
#include "common.h"
#include "rp_hw-profiles.h"

// Base Analog Mixed Signals address
static const int ANALOG_MIXED_SIGNALS_BASE_ADDR = 0x00400000;
static const int ANALOG_MIXED_SIGNALS_BASE_SIZE = 0x30;

typedef struct analog_mixed_signals_control_s {
    uint32_t aif[4];
    uint32_t reserved[4];
    uint32_t dac[4];
} analog_mixed_signals_control_t;

static volatile analog_mixed_signals_control_t* ams = NULL;

static int ams_Init() {
    return cmn_Map(ANALOG_MIXED_SIGNALS_BASE_SIZE, ANALOG_MIXED_SIGNALS_BASE_ADDR, (void**)&ams);
}

static int ams_Release() {
    return cmn_Unmap(ANALOG_MIXED_SIGNALS_BASE_SIZE, (void**)&ams);
}

static int ams_printRegset() {

    auto strWithCh = [](const char* fmt, int ch) -> std::string {
        char buff[255];
        sprintf(buff, fmt, ch);
        return buff;
    };

    volatile analog_mixed_signals_control_t* ams = NULL;

    int fd1 = -1;
    int ret = cmn_InitMap(ANALOG_MIXED_SIGNALS_BASE_SIZE, ANALOG_MIXED_SIGNALS_BASE_ADDR, (void**)&ams, &fd1);
    if (ret != RP_OK) {
        return ret;
    }

    for (auto i = 0; i < 4; i++) {
        printReg("%-25s\t0x%X = 0x%08X (%d)\n", strWithCh("Channel %d aif", i + 1).c_str(),
                 ANALOG_MIXED_SIGNALS_BASE_ADDR + offsetof(analog_mixed_signals_control_t, aif) + sizeof(uint32_t) * i, ams->aif[i]);
    }

    for (auto i = 0; i < 4; i++) {
        printReg("%-25s\t0x%X = 0x%08X (%d)\n", strWithCh("Channel %d reserved", i + 1).c_str(),
                 ANALOG_MIXED_SIGNALS_BASE_ADDR + offsetof(analog_mixed_signals_control_t, reserved) + sizeof(uint32_t) * i, ams->reserved[i]);
    }

    for (auto i = 0; i < 4; i++) {
        printReg("%-25s\t0x%X = 0x%08X (%d)\n", strWithCh("Channel %d dac", i + 1).c_str(),
                 ANALOG_MIXED_SIGNALS_BASE_ADDR + offsetof(analog_mixed_signals_control_t, dac) + sizeof(uint32_t) * i, ams->dac[i]);
    }
    return cmn_ReleaseClose(fd1, ANALOG_MIXED_SIGNALS_BASE_SIZE, (void**)&ams);
}

#endif  //__ANALOG_MIXED_SIGNALS_H
