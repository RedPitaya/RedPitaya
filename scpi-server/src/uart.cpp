/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server SCPI commands implementation
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "rp_hw.h"
#include "scpi-parser-ext.h"
#include "scpi/parser.h"
#include "uart.h"

const scpi_choice_def_t scpi_BIT_Size[] = {{"CS6", RP_UART_CS6}, {"CS7", RP_UART_CS7}, {"CS8", RP_UART_CS8}, SCPI_CHOICE_LIST_END};

const scpi_choice_def_t scpi_STOP_bit[] = {{"STOP1", RP_UART_STOP1}, {"STOP2", RP_UART_STOP2}, SCPI_CHOICE_LIST_END};

const scpi_choice_def_t scpi_PARITY[] = {{"NONE", RP_UART_NONE}, {"EVEN", RP_UART_EVEN},   {"ODD", RP_UART_ODD},
                                         {"MARK", RP_UART_MARK}, {"SPACE", RP_UART_SPACE}, SCPI_CHOICE_LIST_END};

scpi_result_t RP_Uart_Init(scpi_t* context) {
    auto result = rp_UartInit();
    if (RP_HW_OK != result) {
        RP_LOG_CRIT("Failed to init Red Pitaya uart: %s", rp_HwGetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_Uart_Release(scpi_t* context) {
    auto result = rp_UartRelease();
    if (RP_HW_OK != result) {
        RP_LOG_CRIT("Failed to release uart: %s", rp_HwGetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_Uart_SetSettings(scpi_t* context) {
    auto result = rp_UartSetSettings();
    if (RP_HW_OK != result) {
        RP_LOG_CRIT("Failed set settings uart: %s", rp_HwGetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_Uart_BIT_Size(scpi_t* context) {
    int32_t value = 0;
    if (!SCPI_ParamChoice(context, scpi_BIT_Size, &value, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }
    rp_uart_bits_size_t par = (rp_uart_bits_size_t)value;
    auto result = rp_UartSetBits(par);
    if (RP_HW_OK != result) {
        RP_LOG_CRIT("Failed to set character size: %s", rp_HwGetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_Uart_BIT_SizeQ(scpi_t* context) {
    const char* _name = nullptr;
    rp_uart_bits_size_t value;
    auto result = rp_UartGetBits(&value);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get character size: %s", rp_HwGetError(result));
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    if (!SCPI_ChoiceToName(scpi_BIT_Size, value, &_name)) {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed to parse character size.")
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    // Return back result
    SCPI_ResultMnemonic(context, _name);
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_Uart_Speed(scpi_t* context) {
    uint32_t value = 0;
    if (!SCPI_ParamUInt32(context, &value, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }
    auto result = rp_UartSetSpeed(value);
    if (RP_HW_OK != result) {
        RP_LOG_CRIT("Failed to set uart speed: %s", rp_HwGetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_Uart_SpeedQ(scpi_t* context) {
    int32_t speed = 0;
    auto result = rp_UartGetSpeed(&speed);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get uart speed: %s", rp_HwGetError(result));
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    // Return back result
    SCPI_ResultUInt32Base(context, speed, 10);
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_Uart_STOP_Bit(scpi_t* context) {
    int32_t value = 0;
    if (!SCPI_ParamChoice(context, scpi_STOP_bit, &value, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }
    rp_uart_stop_bits_t par = (rp_uart_stop_bits_t)value;
    auto result = rp_UartSetStopBits(par);
    if (RP_HW_OK != result) {
        RP_LOG_CRIT("Failed to set stop bit size: %s", rp_HwGetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_Uart_STOP_BitQ(scpi_t* context) {
    const char* _name = nullptr;
    rp_uart_stop_bits_t value = RP_UART_STOP1;
    auto result = rp_UartGetStopBits(&value);
    if (RP_HW_OK != result) {
        RP_LOG_CRIT("Failed to get stop bit size: %s", rp_HwGetError(result));
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    if (!SCPI_ChoiceToName(scpi_STOP_bit, value, &_name)) {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed to get stop bit size.")
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    // Return back result
    SCPI_ResultMnemonic(context, _name);
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_Uart_PARITY(scpi_t* context) {
    int32_t value = 0;
    if (!SCPI_ParamChoice(context, scpi_PARITY, &value, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }
    rp_uart_parity_t par = (rp_uart_parity_t)value;
    auto result = rp_UartSetParityMode(par);
    if (RP_HW_OK != result) {
        RP_LOG_CRIT("Failed to set parity mode: %s", rp_HwGetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_Uart_PARITYQ(scpi_t* context) {
    const char* _name = nullptr;
    rp_uart_parity_t value = RP_UART_NONE;
    auto result = rp_UartGetParityMode(&value);
    if (RP_HW_OK != result) {
        RP_LOG_CRIT("Failed to get parity mode: %s", rp_HwGetError(result));
        return SCPI_RES_ERR;
    }
    if (!SCPI_ChoiceToName(scpi_PARITY, value, &_name)) {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed to get parity mode.")
        return SCPI_RES_ERR;
    }
    // Return back result
    SCPI_ResultMnemonic(context, _name);
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_Uart_Timeout(scpi_t* context) {
    uint8_t value = 0;
    if (!SCPI_ParamUInt8(context, &value, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }
    auto result = rp_UartSetTimeout(value);
    if (RP_HW_OK != result) {
        RP_LOG_CRIT("Failed to set uart timeout: %s", rp_HwGetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_Uart_TimeoutQ(scpi_t* context) {
    uint8_t speed = 0;
    auto result = rp_UartGetTimeout(&speed);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get uart timeout: %s", rp_HwGetError(result));
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    // Return back result
    SCPI_ResultUInt32Base(context, speed, 10);
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_Uart_SendBuffer(scpi_t* context) {
    uint8_t* buffer = nullptr;
    size_t size = 0;
    uint32_t buf_size = 0;
    int32_t cmd[1] = {0};
    if (!SCPI_CommandNumbers(context, cmd, 1, -1)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get parameters.")
        return SCPI_RES_ERR;
    }
    if (cmd[0] == -1) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get size.")
        return SCPI_RES_ERR;
    }
    size = cmd[0];
    buffer = (uint8_t*)malloc(size * sizeof(uint8_t));
    if (!buffer) {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed allocate buffer with size: %d", size)
        return SCPI_RES_ERR;
    }
    buf_size = size;
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
    auto result = rp_UartWrite(buffer, buf_size);
    if (result != RP_HW_OK) {
        RP_LOG_CRIT("Failed send data: %s", rp_HwGetError(result));
        free(buffer);
        return SCPI_RES_ERR;
    }
    free(buffer);
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_Uart_ReadBufferQ(scpi_t* context) {
    uint8_t* buffer = 0;
    size_t size = 0;
    bool error = false;
    int32_t read_size = 0;
    int32_t cmd[1] = {0};
    if (!SCPI_CommandNumbers(context, cmd, 1, -1)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get parameters.")
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    if (cmd[0] == -1) {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed to get size.")
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    size = cmd[0];
    buffer = (uint8_t*)malloc(size * sizeof(uint8_t));
    if (!buffer) {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed allocate buffer with size: %d.", size)
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    read_size = size;
    auto result = rp_UartRead(buffer, &read_size);
    if (result != RP_HW_OK) {
        RP_LOG_CRIT("Failed read data: %s", rp_HwGetError(result));
        free(buffer);
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    SCPI_ResultBufferUInt8(context, buffer, read_size, &error);
    free(buffer);
    if (error) {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed to send data");
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }

    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}