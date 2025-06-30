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
#include "i2c.h"
#include "rp_hw.h"
#include "scpi-parser-ext.h"
#include "scpi/parser.h"

scpi_result_t RP_I2C_Dev(scpi_t* context) {
    int32_t cmd[1] = {0};
    uint8_t dev_address = 0;
    if (!SCPI_CommandNumbers(context, cmd, 1, -1)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get parameters.")
        return SCPI_RES_ERR;
    }
    if (cmd[0] == -1) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get address of device.")
        return SCPI_RES_ERR;
    }
    dev_address = cmd[0];
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
    auto result = rp_I2C_InitDevice(dev, dev_address);
    if (RP_HW_OK != result) {
        RP_LOG_CRIT("Failed to init Red Pitaya spi: %s", rp_HwGetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_I2C_DevQ(scpi_t* context) {
    int32_t address = 0;
    auto result = rp_I2C_getDevAddress(&address);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get i2c device address: %s", rp_HwGetError(result));
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    // Return back result
    SCPI_ResultUInt32Base(context, address, 10);
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_I2C_ForceMode(scpi_t* context) {
    scpi_bool_t value;
    if (!SCPI_ParamBool(context, &value, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }
    auto result = rp_I2C_setForceMode(value);
    if (RP_HW_OK != result) {
        RP_LOG_CRIT("Failed to set i2c force mode: %s", rp_HwGetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_I2C_ForceModeQ(scpi_t* context) {
    bool value = false;
    auto result = rp_I2C_getForceMode(&value);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get i2c force mode: %s", rp_HwGetError(result));
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    // Return back result
    SCPI_ResultBool(context, value);
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_I2C_SMBUS_ReadQ(scpi_t* context) {
    int32_t cmd[1] = {0};
    uint8_t reg = 0;
    if (!SCPI_CommandNumbers(context, cmd, 1, -1)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get parameters.")
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    if (cmd[0] == -1) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get register for read.")
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    reg = cmd[0];
    uint8_t value = 0;
    auto result = rp_I2C_SMBUS_Read(reg, &value);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to read byte from i2c: %s", rp_HwGetError(result));
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    // Return back result
    SCPI_ResultUInt32Base(context, value, 10);
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_I2C_SMBUS_ReadWordQ(scpi_t* context) {
    int32_t cmd[1] = {0};
    uint8_t reg = 0;
    if (!SCPI_CommandNumbers(context, cmd, 1, -1)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get parameters.")
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    if (cmd[0] == -1) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get register for read.")
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    reg = cmd[0];
    uint16_t value = 0;
    auto result = rp_I2C_SMBUS_ReadWord(reg, &value);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to read word from i2c: %s", rp_HwGetError(result));
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    // Return back result
    SCPI_ResultUInt32Base(context, value, 10);
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_I2C_SMBUS_ReadBufferQ(scpi_t* context) {
    size_t reg = 0;
    size_t size = 0;
    uint8_t* buffer = NULL;
    bool error = false;
    int32_t cmd[2] = {0, 0};
    if (!SCPI_CommandNumbers(context, cmd, 2, -1)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get parameters.")
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    if (cmd[0] == -1) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get register for read.")
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    if (cmd[1] == -1) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get size of buffer.")
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    reg = cmd[0];
    size = cmd[1];
    buffer = (uint8_t*)malloc(size * sizeof(uint8_t));
    if (!buffer) {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed allocate buffer with size: %d.", size)
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }

    int read_size = size;
    auto result = rp_I2C_SMBUS_ReadBuffer(reg, buffer, &read_size);
    if (result != RP_HW_OK) {
        RP_LOG_CRIT("Failed read buffer: %s", rp_HwGetError(result));
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

scpi_result_t RP_I2C_IOCTL_ReadBufferQ(scpi_t* context) {
    size_t size = 0;
    bool error = false;
    uint8_t* buffer = NULL;
    int32_t cmd[1] = {0};
    if (!SCPI_CommandNumbers(context, cmd, 1, -1)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get parameters.")
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    if (cmd[0] == -1) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get size of buffer.")
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    size = cmd[0];
    buffer = (uint8_t*)malloc(size * sizeof(uint8_t));
    if (!buffer) {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed allocate buffer with size: %d.", size)
        return SCPI_RES_ERR;
    }

    int read_size = size;
    auto result = rp_I2C_IOCTL_ReadBuffer(buffer, read_size);
    if (result != RP_HW_OK) {
        RP_LOG_CRIT("Failed read buffer: %s", rp_HwGetError(result));
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

scpi_result_t RP_I2C_SMBUS_Write(scpi_t* context) {
    int32_t cmd[1] = {0};
    uint8_t reg = 0;
    if (!SCPI_CommandNumbers(context, cmd, 1, -1)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get parameters.")
        return SCPI_RES_ERR;
    }
    if (cmd[0] == -1) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get register for write.")
        return SCPI_RES_ERR;
    }
    reg = cmd[0];
    uint32_t value;
    if (!SCPI_ParamUInt32(context, &value, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }
    auto result = rp_I2C_SMBUS_Write(reg, value);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed write byte to i2c: %s", rp_HwGetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_I2C_SMBUS_WriteWord(scpi_t* context) {
    int32_t cmd[1] = {0};
    uint8_t reg = 0;
    if (!SCPI_CommandNumbers(context, cmd, 1, -1)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get parameters.")
        return SCPI_RES_ERR;
    }
    if (cmd[0] == -1) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get register for write.")
        return SCPI_RES_ERR;
    }
    reg = cmd[0];
    uint32_t value = 0;
    if (!SCPI_ParamUInt32(context, &value, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }
    auto result = rp_I2C_SMBUS_WriteWord(reg, value);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed write word to i2c: %s", rp_HwGetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_I2C_SMBUS_WriteBuffer(scpi_t* context) {
    size_t reg = 0;
    size_t size = 0;
    uint8_t* buffer = NULL;
    int32_t cmd[2] = {0, 0};
    if (!SCPI_CommandNumbers(context, cmd, 2, -1)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get parameters.")
        return SCPI_RES_ERR;
    }
    if (cmd[0] == -1) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get register for read.")
        return SCPI_RES_ERR;
    }
    if (cmd[1] == -1) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get size of buffer.")
        return SCPI_RES_ERR;
    }
    reg = cmd[0];
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
    auto result = rp_I2C_SMBUS_WriteBuffer(reg, buffer, buf_size);
    if (result != RP_HW_OK) {
        RP_LOG_CRIT("Failed write buffer to i2c: %s", rp_HwGetError(result));
        free(buffer);
        return SCPI_RES_ERR;
    }
    free(buffer);
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_I2C_IOCTL_WriteBuffer(scpi_t* context) {
    size_t size = 0;
    uint8_t* buffer = NULL;
    int32_t cmd[1] = {0};
    if (!SCPI_CommandNumbers(context, cmd, 1, -1)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get parameters.")
        return SCPI_RES_ERR;
    }
    if (cmd[0] == -1) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get size of buffer.")
        return SCPI_RES_ERR;
    }
    size = cmd[0];
    buffer = (uint8_t*)malloc(size * sizeof(uint8_t));
    if (!buffer) {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed allocate buffer with size: %d.", size)
        return SCPI_RES_ERR;
    }
    size_t buf_size = size;
    if (!SCPI_ParamBufferUInt8(context, buffer, &buf_size, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed get data for buffer")
        free(buffer);
        return SCPI_RES_ERR;
    }
    if (buf_size != size) {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Wrong data length.")
        free(buffer);
        return SCPI_RES_ERR;
    }
    auto result = rp_I2C_IOCTL_WriteBuffer(buffer, buf_size);
    if (result != RP_HW_OK) {
        RP_LOG_CRIT("Failed write buffer to i2c: %s", rp_HwGetError(result));
        free(buffer);
        return SCPI_RES_ERR;
    }
    free(buffer);
    RP_LOG_INFO("%s", rp_HwGetError(result))
    return SCPI_RES_OK;
}
