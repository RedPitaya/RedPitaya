#include <CustomParameters.h>
#include <DataManager.h>

#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "rp_hw_calib.h"

CBooleanParameter sw_window_show("SW_WIN_SHOW", CBaseParameter::RW, false, 0);
CIntParameter sw_window_x("SW_WIN_X", CBaseParameter::RW, 300, 0, 0, 65536);
CIntParameter sw_window_y("SW_WIN_Y", CBaseParameter::RW, 300, 0, 0, 65536);
CIntParameter sw_window_w("SW_WIN_W", CBaseParameter::RW, 650, 0, 0, 65536);
CIntParameter sw_window_h("SW_WIN_H", CBaseParameter::RW, 240, 0, 0, 65536);

CIntParameter sw_f_ver("SW_F_VER", CBaseParameter::RW, -1, 0, -1, 65536);
CIntParameter sw_f_count("SW_F_COUNT", CBaseParameter::RW, -1, 0, -1, 65536);
CIntParameter sw_u_ver("SW_U_VER", CBaseParameter::RW, -1, 0, -1, 65536);
CIntParameter sw_u_count("SW_U_COUNT", CBaseParameter::RW, -1, 0, -1, 65536);

CStringParameter eeprom_calib_value("EEPROM_CALIB_VALUE", CBaseParameter::RW, "", 0);

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

auto sendEepromFactoryZone() -> void {
    rp_eepromUniData_t* data = nullptr;
    uint8_t* eeprom_buffer = nullptr;
    uint16_t size = 0;
    if (rp_CalibGetEEPROM(&eeprom_buffer, &size, true) == RP_HW_CALIB_OK) {
        data = reinterpret_cast<rp_eepromUniData_t*>(eeprom_buffer);
        if (data->dataStructureId == RP_HW_PACK_ID_V5 || data->dataStructureId == RP_HW_PACK_ID_V6) {
            std::string value = "";
            for (int i = 0; i < data->count; ++i) {
                std::string name;
                rp_GetNameOfUniversalId(data->item[i].id, &name);
                value += name + "\t" + std::to_string(data->item[i].id) + "\t" + std::to_string(data->item[i].value) + "\n";
            }
            eeprom_calib_value.SendValue(value);
        } else {
            eeprom_calib_value.SendValue("");
        }
    } else {
        eeprom_calib_value.SendValue("");
    }
    free(eeprom_buffer);
}

auto sendEepromUserZone() -> void {
    rp_eepromUniData_t* data = nullptr;
    uint8_t* eeprom_buffer = nullptr;
    uint16_t size = 0;
    if (rp_CalibGetEEPROM(&eeprom_buffer, &size, false) == RP_HW_CALIB_OK) {
        data = reinterpret_cast<rp_eepromUniData_t*>(eeprom_buffer);
        if (data->dataStructureId == RP_HW_PACK_ID_V5 || data->dataStructureId == RP_HW_PACK_ID_V6) {
            std::string value = "";
            for (int i = 0; i < data->count; ++i) {
                std::string name;
                rp_GetNameOfUniversalId(data->item[i].id, &name);
                value += name + "\t" + std::to_string(data->item[i].id) + "\t" + std::to_string(data->item[i].value) + "\n";
            }
            eeprom_calib_value.SendValue(value);
        } else {
            eeprom_calib_value.SendValue("");
        }
    } else {
        eeprom_calib_value.SendValue("");
    }
    free(eeprom_buffer);
}