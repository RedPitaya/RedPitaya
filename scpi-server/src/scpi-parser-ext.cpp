/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server SCPI 
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <arpa/inet.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "error.h"
#include "scpi-parser-ext.h"

UARTProtocol g_arduino_protocol;

float hton_f(float value) {
    union {
        float f;
        unsigned int i;
    } val;

    val.f = value;

    val.i = htonl(val.i);
    return val.f;
};

/**
 * Interface general commands
 */
size_t SCPI_Write(scpi_t* context, const char* data, size_t len) {

    size_t total = 0;
    if (context->user_context != NULL) {
        user_context_t* uc = (user_context_t*)context->user_context;
        int fd = uc->fd;
        while (len > 0) {
            ssize_t written = write(fd, data, len);
            if (written < 0) {
                RP_LOG_INFO("Failed to write. Should send %zu bytes. Could send only %zu bytes", len, written);
                return total;
            }

            len -= written;
            data += written;
            total += written;
        }
    }
    return total;
}

size_t SCPI_WriteUartProtocol(scpi_t* context, const char* data, size_t len) {

    size_t total = 0;
    if (context->user_context != NULL) {
        user_context_t* uc = (user_context_t*)context->user_context;
        int fd = uc->fd;
        while (len > 0) {
            size_t written = g_arduino_protocol.writeTo(fd, (uint8_t*)data, len);
            if (written != len) {
                RP_LOG_INFO("Failed to write into the uart protocol. Should send %zu bytes. Could send only %zu bytes", len, written);
                return total;
            }

            len -= written;
            data += written;
            total += written;
        }
    }
    return total;
}

size_t writeDataEx(scpi_t* context, const char* data, size_t len, bool* error) {
    *error = true;
    if ((len > 0) && (data != NULL)) {
        auto res = context->interface->write(context, data, len);
        *error = res != len;
        return res;
    }
    return 0;
}

size_t writeDelimiterEx(scpi_t* context, bool* error) {
    *error = true;
    if (context->output_count > 0) {
        return writeDataEx(context, ",", 1, error);
    }
    return 0;
}

scpi_result_t SCPI_Flush(scpi_t*) {
    return SCPI_RES_OK;
}

int SCPI_Error(scpi_t* context, int_fast16_t err) {
    RP_LOG_INFO("**ERROR: %d, \"%s\"", (int32_t)err, SCPI_ErrorTranslate(err));
    return 0;
}

scpi_result_t SCPI_Control(scpi_t* context, scpi_ctrl_name_t ctrl, scpi_reg_val_t) {
    if (SCPI_CTRL_SRQ == ctrl) {
        RP_LOG_INFO("**SRQ not implemented");
    } else {
        RP_LOG_INFO("**CTRL not implemented");
    }

    return SCPI_RES_ERR;
}

scpi_result_t SCPI_Reset(scpi_t* context) {
    /* Terminating all scpi operations */
    RP_LOG_INFO("Sucsessfuly reset scpi server.")
    return SCPI_RES_OK;
}

scpi_result_t SCPI_SystemCommTcpipControlQ(scpi_t* context) {
    RP_LOG_INFO("**SCPI_SystemCommTcpipControlQ not implemented");
    return SCPI_RES_ERR;
}

scpi_result_t SCPI_SystemErrorNextQEx(scpi_t* context) {
    if (rp_errorCount(context)) {
        auto err = rp_popError(context);
        SCPI_ResultInt32(context, err.baseCode + err.errorCode);
        SCPI_ResultText(context, err.msg.c_str());
        return SCPI_RES_OK;
    }

    return SCPI_SystemErrorNextQ(context);
}

scpi_result_t SCPI_SystemErrorCountQEx(scpi_t* context) {
    SCPI_ResultInt32(context, SCPI_ErrorCount(context) + rp_errorCount(context));
    return SCPI_RES_OK;
}

scpi_result_t SCPI_CoreClsEx(scpi_t* context) {
    rp_resetErrorList(context);
    return SCPI_CoreCls(context);
}

UARTProtocol* getUARTProtocol() {
    return &g_arduino_protocol;
}

/**
 * Writes header for binary data
 * @param context
 * @param numElems - number of items in the array
 * @param sizeOfElem - size of each item [sizeof(float), sizeof(int), ...]
 * @return number of characters written
 */
size_t writeBinHeader(scpi_t* context, uint32_t numElems, size_t sizeOfElem, bool* error) {

    size_t result = 0;
    char numBytes[9 + 1];
    char numOfNumBytes[2];

    // Calculate number of bytes needed for all elements
    size_t numDataBytes = numElems * sizeOfElem;

    // Do not allow more than 9 character long size
    if (numDataBytes > 999999999) {
        return result;
    }

    // Convert to string and calculate string length
    size_t len = SCPI_UInt32ToStrBase(numDataBytes, numBytes, sizeof(numBytes), 10);

    // Convert len to sting
    SCPI_UInt32ToStrBase(len, numOfNumBytes, sizeof(numOfNumBytes), 10);

    result += writeDataEx(context, "#", 1, error);
    CHECK_ERROR_PTR

    result += writeDataEx(context, numOfNumBytes, 1, error);
    CHECK_ERROR_PTR

    result += writeDataEx(context, numBytes, len, error);
    CHECK_ERROR_PTR

    return result;
}

static size_t resultBufferInt16Bin(scpi_t* context, const int16_t* data, size_t size, bool* error) {
    size_t result = 0;

    result += writeBinHeader(context, size, sizeof(int16_t), error);
    CHECK_ERROR_PTR

    static const size_t pack_size = 0x20000;
    size_t i;
    uint16_t new_buff[pack_size];
    for (size_t j = 0; j < size; j += pack_size) {
        size_t send_size = (size - j) < pack_size ? (size - j) : pack_size;
        for (i = 0; i < send_size; i++) {
            new_buff[i] = htons((uint16_t)data[i + j]);
        }
        result += writeDataEx(context, (char*)new_buff, sizeof(int16_t) * send_size, error);
        CHECK_ERROR_PTR
    }
    return result;
}

static size_t resultBufferUInt8Bin(scpi_t* context, const uint8_t* data, size_t size, bool* error) {
    size_t result = 0;

    result += writeBinHeader(context, size, sizeof(uint8_t), error);
    CHECK_ERROR_PTR
    result += writeDataEx(context, (char*)data, sizeof(uint8_t) * size, error);
    return result;
}

static size_t resultBufferInt16Ascii(scpi_t* context, const int16_t* data, size_t size, bool* error) {
    size_t result = 0;

    result += writeDataEx(context, "{", 1, error);
    CHECK_ERROR_PTR

    char buffer[20];
    for (size_t i = 0; i < size; i++) {
        snprintf(buffer, sizeof(buffer), "%" PRIi16 "%s", data[i], (i < size - 1 ? "," : ""));
        auto len = strlen(buffer);
        result += writeDataEx(context, buffer, len, error);
        CHECK_ERROR_PTR
    }

    result += writeDataEx(context, "}", 1, error);
    CHECK_ERROR_PTR

    context->output_count++;
    return result;
}

static size_t resultBufferUInt8Ascii(scpi_t* context, const uint8_t* data, size_t size, bool* error) {
    size_t result = 0;

    result += writeDataEx(context, "{", 1, error);
    CHECK_ERROR_PTR

    char buffer[20];
    for (size_t i = 0; i < size; i++) {
        snprintf(buffer, sizeof(buffer), "%" PRIi8 "%s", data[i], (i < size - 1 ? "," : ""));
        auto len = strlen(buffer);
        result += writeDataEx(context, buffer, len, error);
        CHECK_ERROR_PTR
    }

    result += writeDataEx(context, "}", 1, error);
    CHECK_ERROR_PTR

    context->output_count++;
    return result;
}

size_t SCPI_ResultBufferInt16(scpi_t* context, const int16_t* data, size_t size, bool* error) {
    user_context_t* uc = (user_context_t*)context->user_context;
    if (uc->binary_format == true) {
        return resultBufferInt16Bin(context, data, size, error);
    } else {
        return resultBufferInt16Ascii(context, data, size, error);
    }
}

size_t SCPI_ResultBufferUInt8(scpi_t* context, const uint8_t* data, size_t size, bool* error) {
    user_context_t* uc = (user_context_t*)context->user_context;

    if (uc->binary_format == true) {
        return resultBufferUInt8Bin(context, data, size, error);
    } else {
        return resultBufferUInt8Ascii(context, data, size, error);
    }
}

static size_t resultBufferFloatBin(scpi_t* context, const float* data, size_t size, bool* error) {
    size_t result = 0;

    result += writeBinHeader(context, size, sizeof(float), error);
    CHECK_ERROR_PTR

    static const size_t pack_size = 0x20000;
    size_t i;
    float new_buff[pack_size];
    for (size_t j = 0; j < size; j += pack_size) {
        size_t send_size = (size - j) < pack_size ? (size - j) : pack_size;
        for (i = 0; i < send_size; i++) {
            new_buff[i] = hton_f((uint8_t)data[i + j]);
        }
        result += writeDataEx(context, (char*)new_buff, sizeof(float) * send_size, error);
        CHECK_ERROR_PTR
    }
    return result;
}

static size_t resultBufferFloatAscii(scpi_t* context, const float* data, size_t size, bool* error) {
    size_t result = 0;
    result += writeDataEx(context, "{", 1, error);
    CHECK_ERROR_PTR

    size_t i;
    char buffer[128];
    for (i = 0; i < size; i++) {
        snprintf(buffer, 128, "%f%s", data[i], (i < size - 1 ? "," : ""));
        result += writeDataEx(context, buffer, strlen(buffer), error);
        CHECK_ERROR_PTR
    }

    result += writeDataEx(context, "}", 1, error);
    CHECK_ERROR_PTR

    context->output_count++;
    return result;
}

size_t SCPI_ResultBufferFloat(scpi_t* context, const float* data, uint32_t size, bool* error) {
    user_context_t* uc = (user_context_t*)context->user_context;
    if (uc->binary_format == true) {
        return resultBufferFloatBin(context, data, size, error);
    } else {
        return resultBufferFloatAscii(context, data, size, error);
    }
}

scpi_bool_t SCPI_ParamUInt8(scpi_t* context, uint8_t* value, scpi_bool_t mandatory) {
    uint32_t value32 = 0;
    scpi_bool_t ret = SCPI_ParamUInt32(context, &value32, mandatory);
    *value = (uint8_t)value32;
    return ret;
}

scpi_bool_t SCPI_ParamBufferUInt8(scpi_t* context, uint8_t* data, uint32_t* size, scpi_bool_t mandatory) {
    uint32_t max_size = *size;
    *size = 0;
    uint8_t value;
    while (*size < max_size) {
        if (!SCPI_ParamUInt8(context, &value, mandatory)) {
            break;
        }
        data[*size] = (uint8_t)value;
        *size = *size + 1;
        mandatory = false;  // only first is mandatory
    }
    return true;
}

/**
 * Red Pitaya added function
 * TODO, replace with upstream equivalent
 */
scpi_bool_t SCPI_ParamBufferFloat(scpi_t* context, float* data, uint32_t* size, scpi_bool_t mandatory) {
    *size = 0;
    double value;
    while (true) {
        if (!SCPI_ParamDouble(context, &value, mandatory)) {
            break;
        }
        data[*size] = (float)value;
        *size = *size + 1;
        mandatory = false;  // only first is mandatory
    }
    return true;
}