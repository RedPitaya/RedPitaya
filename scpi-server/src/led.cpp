/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server SCPI commands implementation
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "common.h"
#include "led.h"
#include "rp_hw.h"
#include "scpi/parser.h"

const scpi_choice_def_t scpi_LED_state[] = {{"OFF", 0}, {"ON", 1}, SCPI_CHOICE_LIST_END};

#define E3_EXPANDER_ADDR 0x10

// The E3 extension module resets the board when the heartbeat line stops toggling.
static bool isE3ModuleConnected() {
    if (!rp_HPGetIsE3PresentOrDefault())
        return false;
    int fd = open("/dev/i2c-0", O_RDWR);
    if (fd < 0)
        return false;
    char buf;
    bool connected = ioctl(fd, I2C_SLAVE_FORCE, E3_EXPANDER_ADDR) >= 0 && read(fd, &buf, 1) >= 0;
    close(fd);
    return connected;
}

scpi_result_t RP_LED_MMC(scpi_t* context) {
    int32_t value = 0;
    if (!SCPI_ParamChoice(context, scpi_LED_state, &value, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }
    auto result = rp_SetLEDMMCState((bool)value);
    if (RP_HW_OK != result) {
        RP_LOG_ERR("Failed to set MMC led: %s", rp_HwGetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_LED_MMCQ(scpi_t* context) {
    const char* _name = nullptr;
    bool value = false;
    auto result = rp_GetLEDMMCState(&value);
    if (RP_OK != result) {
        RP_LOG_ERR("Failed get LED state: %d", result);
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    if (!SCPI_ChoiceToName(scpi_LED_state, (int32_t)value, &_name)) {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed to parse LED state.")
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    // Return back result
    SCPI_ResultMnemonic(context, _name);
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_LED_HB(scpi_t* context) {
    int32_t value = 0;
    if (!SCPI_ParamChoice(context, scpi_LED_state, &value, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }
    if (!value && isE3ModuleConnected()) {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Turning off the heartbeat LED reboots the board with the E3 extension module connected.")
        return SCPI_RES_ERR;
    }
    auto result = rp_SetLEDHeartBeatState((bool)value);
    if (RP_HW_OK != result) {
        RP_LOG_ERR("Failed to set HB led: %s", rp_HwGetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_LED_HBQ(scpi_t* context) {
    const char* _name = nullptr;
    bool value = false;
    auto result = rp_GetLEDHeartBeatState(&value);
    if (RP_OK != result) {
        RP_LOG_ERR("Failed get LED state: %s", rp_HwGetError(result));
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    if (!SCPI_ChoiceToName(scpi_LED_state, (int32_t)value, &_name)) {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed to parse LED state.")
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    // Return back result
    SCPI_ResultMnemonic(context, _name);
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_LED_ETH(scpi_t* context) {
    int32_t value = 0;
    if (!SCPI_ParamChoice(context, scpi_LED_state, &value, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }
    auto result = rp_SetLEDEthState((bool)value);
    if (RP_HW_OK != result) {
        RP_LOG_ERR("Failed to set ETH led: %s", rp_HwGetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_LED_ETHQ(scpi_t* context) {
    const char* _name = nullptr;
    bool value = false;
    auto result = rp_GetLEDEthState(&value);
    if (RP_OK != result) {
        RP_LOG_ERR("Failed get LED state: %s", rp_HwGetError(result));
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    if (!SCPI_ChoiceToName(scpi_LED_state, (int32_t)value, &_name)) {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed to parse LED state.")
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    // Return back result
    SCPI_ResultMnemonic(context, _name);
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}
