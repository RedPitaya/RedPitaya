#include <CustomParameters.h>
#include <DataManager.h>

#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "rp_hw_calib.h"

CBooleanParameter sw_window_show("SW_WIN_SHOW", CBaseParameter::RW, false, 0);
CIntParameter sw_window_x("SW_WIN_X", CBaseParameter::RW, 300, 0, 0, 65536);
CIntParameter sw_window_y("SW_WIN_Y", CBaseParameter::RW, 300, 0, 0, 65536);
CIntParameter sw_window_w("SW_WIN_W", CBaseParameter::RW, 600, 0, 0, 65536);
CIntParameter sw_window_h("SW_WIN_H", CBaseParameter::RW, 230, 0, 0, 65536);

CIntParameter sw_f_ver("SW_F_VER", CBaseParameter::RW, -1, 0, -1, 65536);
CIntParameter sw_f_count("SW_F_COUNT", CBaseParameter::RW, -1, 0, -1, 65536);
CIntParameter sw_u_ver("SW_U_VER", CBaseParameter::RW, -1, 0, -1, 65536);
CIntParameter sw_u_count("SW_U_COUNT", CBaseParameter::RW, -1, 0, -1, 65536);

auto updateEEPROMWindowSettings() -> void {
    if (IS_NEW(sw_window_show)) {
        sw_window_show.Update();
    }

    if (IS_NEW(sw_window_x)) {
        sw_window_x.Update();
    }

    if (IS_NEW(sw_window_y)) {
        sw_window_y.Update();
    }

    if (IS_NEW(sw_window_h)) {
        sw_window_h.Update();
    }

    if (IS_NEW(sw_window_w)) {
        sw_window_w.Update();
    }
}

auto updateEepromWindow() -> void {
    uint8_t* data = NULL;
    uint16_t size = 0;
    if (rp_CalibGetEEPROM(&data, &size, false) == RP_HW_CALIB_OK) {
        sw_u_ver.SendValue(data[0]);
        sw_u_count.SendValue(data[1]);
    } else {
        sw_u_ver.SendValue(-1);
        sw_u_count.SendValue(-1);
    }

    if (rp_CalibGetEEPROM(&data, &size, true) == RP_HW_CALIB_OK) {
        sw_f_ver.SendValue(data[0]);
        sw_f_count.SendValue(data[1]);
    } else {
        sw_f_ver.SendValue(-1);
        sw_f_count.SendValue(-1);
    }
}