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

#include "common.h"
#include "rp_hw.h"
#include "scpi-parser-ext.h"
#include "scpi/parser.h"
#include "spi.h"

const scpi_choice_def_t scpi_mode[] = {{"LISL", RP_SPI_MODE_LISL},
                                       {"LIST", RP_SPI_MODE_LIST},
                                       {"HISL", RP_SPI_MODE_HISL},
                                       {"HIST", RP_SPI_MODE_HIST},
                                       SCPI_CHOICE_LIST_END};

const scpi_choice_def_t scpi_order_bit[] = {{"MSB", RP_SPI_ORDER_BIT_MSB}, {"LSB", RP_SPI_ORDER_BIT_LSB}, SCPI_CHOICE_LIST_END};

const scpi_choice_def_t scpi_cs_mode[] = {{"NORMAL", RP_SPI_CS_NORMAL}, {"HIGH", RP_SPI_CS_HIGH}, SCPI_CHOICE_LIST_END};

scpi_result_t RP_SPI_Init(scpi_t* context) {
    auto result = rp_SPI_Init();
    if (RP_HW_OK != result) {
        RP_LOG_CRIT("Failed to init Red Pitaya spi: %s", rp_HwGetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_InitDev(scpi_t* context) {
    char dev[255];
    const char* param = NULL;
    size_t param_len = 0;
    memset(dev, 0, 255);

    if (!SCPI_ParamCharacters(context, &param, &param_len, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }

    if (param_len == 0) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }

    strncpy(dev, param, param_len);
    auto result = rp_SPI_InitDevice(dev);
    if (RP_HW_OK != result) {
        RP_LOG_CRIT("Failed to init Red Pitaya spi: %s", rp_HwGetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_Release(scpi_t* context) {
    auto result = rp_SPI_Release();
    if (RP_HW_OK != result) {
        RP_LOG_CRIT("Failed to release spi: %s", rp_HwGetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_SetDefault(scpi_t* context) {
    auto result = rp_SPI_SetDefaultSettings();
    if (RP_HW_OK != result) {
        RP_LOG_CRIT("Failed to set default settings for spi: %s", rp_HwGetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_SetSettings(scpi_t* context) {
    auto result = rp_SPI_SetSettings();
    if (RP_HW_OK != result) {
        RP_LOG_CRIT("Failed to set settings for spi: %s", rp_HwGetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_GetSettings(scpi_t* context) {
    auto result = rp_SPI_GetSettings();
    if (RP_HW_OK != result) {
        RP_LOG_CRIT("Failed to get settings for spi: %s", rp_HwGetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_CreateMessage(scpi_t* context) {
    uint32_t value = 0;

    if (!SCPI_ParamUInt32(context, &value, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }

    auto result = rp_SPI_CreateMessage(value);
    if (RP_HW_OK != result) {
        RP_LOG_CRIT("Failed to create message: %s", rp_HwGetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_DestroyMessage(scpi_t* context) {
    int result = rp_SPI_DestoryMessage();
    if (RP_HW_OK != result) {
        RP_LOG_CRIT("Failed to delete message: %s", rp_HwGetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_GetMessageLenQ(scpi_t* context) {
    size_t len;
    auto result = rp_SPI_GetMessageLen(&len);

    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get spi message size: %s", rp_HwGetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultUInt32Base(context, len, 10);
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_GetRXBufferQ(scpi_t* context) {
    const uint8_t* buffer = 0;
    size_t index = 0;
    size_t size = 0;
    bool error = false;
    int32_t cmd[1] = {0};

    if (!SCPI_CommandNumbers(context, cmd, 1, -1)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get parameters.");
        return SCPI_RES_ERR;
    }

    if (cmd[0] == -1) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get index of message.")
        return SCPI_RES_ERR;
    }

    index = cmd[0];

    auto result = rp_SPI_GetRxBuffer(index, &buffer, &size);
    if (result != RP_HW_OK) {
        RP_LOG_CRIT("Failed rx buffer: %s", rp_HwGetError(result));
        return SCPI_RES_ERR;
    }

    if (!buffer) {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Buffer is null.")
        return SCPI_RES_ERR;
    }

    SCPI_ResultBufferUInt8(context, buffer, size, &error);

    if (error) {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed to send data");
        return SCPI_RES_ERR;
    }

    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_GetTXBufferQ(scpi_t* context) {
    const uint8_t* buffer = 0;
    size_t index = 0;
    size_t size = 0;
    bool error = false;
    int32_t cmd[1] = {0};

    if (!SCPI_CommandNumbers(context, cmd, 1, -1)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get parameters.");
        return SCPI_RES_ERR;
    }

    if (cmd[0] == -1) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get index of message.")
        return SCPI_RES_ERR;
    }

    index = cmd[0];

    auto result = rp_SPI_GetTxBuffer(index, &buffer, &size);
    if (result != RP_HW_OK) {
        RP_LOG_CRIT("Failed tx buffer: %s", rp_HwGetError(result));
        return SCPI_RES_ERR;
    }

    if (!buffer) {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Buffer is null.")
        return SCPI_RES_ERR;
    }

    SCPI_ResultBufferUInt8(context, buffer, size, &error);

    if (error) {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed to send data");
        return SCPI_RES_ERR;
    }

    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_GetCSChangeStateQ(scpi_t* context) {
    size_t index = 0;
    bool cs_state = false;
    int32_t cmd[1] = {0};

    if (!SCPI_CommandNumbers(context, cmd, 1, -1)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get parameters.");
        return SCPI_RES_ERR;
    }

    if (cmd[0] == -1) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get index of message.")
        return SCPI_RES_ERR;
    }

    index = cmd[0];

    auto result = rp_SPI_GetCSChangeState(index, &cs_state);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get cs state: %s", rp_HwGetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultBool(context, cs_state);
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_Set(scpi_t* context, bool cs_state, bool init_rx) {
    size_t index = 0;
    size_t size = 0;
    uint8_t* buffer = NULL;
    int32_t cmd[2] = {0, 0};

    if (!SCPI_CommandNumbers(context, cmd, 2, -1)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get parameters.");
        return SCPI_RES_ERR;
    }

    if (cmd[0] == -1) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get index of message.")
        return SCPI_RES_ERR;
    }

    if (cmd[1] == -1) {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed to get size of buffer.")
        return SCPI_RES_ERR;
    }

    index = cmd[0];
    size = cmd[1];

    buffer = (uint8_t*)malloc(size * sizeof(uint8_t));
    if (!buffer) {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed allocate buffer with size: %d.", size)
        return SCPI_RES_ERR;
    }
    size_t buf_size = size;
    if (!SCPI_ParamBufferUInt8(context, buffer, &buf_size, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed get data for buffer.")
        free(buffer);
        return SCPI_RES_ERR;
    }

    if (buf_size != size) {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Wrong data length.")
        free(buffer);
        return SCPI_RES_ERR;
    }

    auto result = rp_SPI_SetBufferForMessage(index, buffer, init_rx, buf_size, cs_state);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to set TX buffer: %s", rp_HwGetError(result));
        free(buffer);
        return SCPI_RES_ERR;
    }
    free(buffer);
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_SetTX(scpi_t* context) {
    return RP_SPI_Set(context, false, false);
}

scpi_result_t RP_SPI_SetTXCS(scpi_t* context) {
    return RP_SPI_Set(context, true, false);
}

scpi_result_t RP_SPI_SetTXRX(scpi_t* context) {
    return RP_SPI_Set(context, false, true);
}

scpi_result_t RP_SPI_SetTXRXCS(scpi_t* context) {
    return RP_SPI_Set(context, true, true);
}

scpi_result_t RP_SPI_SetRX2(scpi_t* context, bool cs_state) {
    size_t index = 0;
    size_t size = 0;
    int32_t cmd[2] = {0, 0};

    if (!SCPI_CommandNumbers(context, cmd, 2, -1)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get parameters.");
        return SCPI_RES_ERR;
    }

    if (cmd[0] == -1) {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed to get index of message.")
        return SCPI_RES_ERR;
    }

    if (cmd[1] == -1) {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed to get size of buffer.")
        return SCPI_RES_ERR;
    }

    index = cmd[0];
    size = cmd[1];

    auto result = rp_SPI_SetBufferForMessage(index, NULL, true, size, cs_state);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to set RX buffer: %s", rp_HwGetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_SetRX(scpi_t* context) {
    return RP_SPI_SetRX2(context, false);
}

scpi_result_t RP_SPI_SetRXCS(scpi_t* context) {
    return RP_SPI_SetRX2(context, true);
}

scpi_result_t RP_SPI_SetMode(scpi_t* context) {
    int32_t value;

    if (!SCPI_ParamChoice(context, scpi_mode, &value, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }

    rp_spi_mode_t par = (rp_spi_mode_t)value;

    auto result = rp_SPI_SetMode(par);
    if (RP_HW_OK != result) {
        RP_LOG_CRIT("Failed to set mode: %s", rp_HwGetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_GetModeQ(scpi_t* context) {
    const char* _name;

    rp_spi_mode_t value;
    auto result = rp_SPI_GetMode(&value);

    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get spi mode: %s", rp_HwGetError(result));
        return SCPI_RES_ERR;
    }

    if (!SCPI_ChoiceToName(scpi_mode, value, &_name)) {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed to parse mode.");
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultMnemonic(context, _name);
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_SetCSMode(scpi_t* context) {
    int32_t value;

    if (!SCPI_ParamChoice(context, scpi_cs_mode, &value, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }

    rp_spi_cs_mode_t par = (rp_spi_cs_mode_t)value;

    auto result = rp_SPI_SetCSMode(par);
    if (RP_HW_OK != result) {
        RP_LOG_CRIT("Failed to set cs mode: %s", rp_HwGetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_GetCSModeQ(scpi_t* context) {
    const char* _name;

    rp_spi_cs_mode_t value;
    auto result = rp_SPI_GetCSMode(&value);

    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get spi cs mode: %s", rp_HwGetError(result));
        return SCPI_RES_ERR;
    }

    if (!SCPI_ChoiceToName(scpi_cs_mode, value, &_name)) {
        RP_LOG_CRIT("Failed to parse cs mode.");
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultMnemonic(context, _name);
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_SetSpeed(scpi_t* context) {
    uint32_t value;

    if (!SCPI_ParamUInt32(context, &value, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }

    auto result = rp_SPI_SetSpeed(value);
    if (RP_HW_OK != result) {
        RP_LOG_CRIT("Failed to set spi speed: %s", rp_HwGetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_GetSpeedQ(scpi_t* context) {
    int32_t speed;
    auto result = rp_SPI_GetSpeed(&speed);

    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get spi speed: %s", rp_HwGetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultUInt32Base(context, speed, 10);
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_SetWord(scpi_t* context) {
    uint32_t value;

    if (!SCPI_ParamUInt32(context, &value, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }

    auto result = rp_SPI_SetWordLen(value);
    if (RP_HW_OK != result) {
        RP_LOG_CRIT("Failed to set word length: %s", rp_HwGetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_GetWordQ(scpi_t* context) {
    int32_t len;
    int result = rp_SPI_GetWordLen(&len);

    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get word length: %s", rp_HwGetError(result));
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultUInt32Base(context, len, 10);
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_Pass(scpi_t* context) {
    auto result = rp_SPI_ReadWrite();
    if (RP_HW_OK != result) {
        RP_LOG_CRIT("Failed pass message to spi: %s", rp_HwGetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}
