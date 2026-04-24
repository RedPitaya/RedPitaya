/**
 * $Id: $
 *
 * @brief Red Pitaya I2C Module.
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#include <stdio.h>
#include <string.h>
#include "i2c.h"
#include "i2c-helper.h"
#include "rp_log.h"

#define DEV_PATH_LEN 256

char    g_devicePath[DEV_PATH_LEN] = "/dev/i2c-0";
bool    g_forceMode = false;
int     g_addr = -1;

int i2c_InitDevice(const char *_device, uint8_t addr){
    if (_device == NULL) {
        ERROR_LOG("Device path is NULL");
        return RP_HW_EIIIC;
    }

    if (addr < 0x03 || addr > 0x77) {
        ERROR_LOG("Device address 0x%02X out of range [0x03-0x77].", addr);
        return RP_HW_EIIIC;
    }

    int len = strlen(_device);
    if (len >= DEV_PATH_LEN){
        ERROR_LOG("Device path too long: %d >= %d, path: %s", len, DEV_PATH_LEN, _device);
        return RP_HW_EIIIC;
    }

    if (len == 0) {
        ERROR_LOG("Device path is empty string");
        return RP_HW_EIIIC;
    }

    g_addr = addr;
    strcpy(g_devicePath, _device);
    TRACE_SHORT("I2C device initialized: %s, addr=0x%02X", _device, addr);
    return RP_HW_OK;
}

int i2c_setForceMode(bool force){
    g_forceMode = force;
    TRACE_SHORT("I2C force mode set to: %s", force ? "true" : "false");
    return RP_HW_OK;
}

int i2c_getForceMode(bool* _out_value){
    if (_out_value == NULL) {
        ERROR_LOG("Output parameter _out_value is NULL");
        return RP_HW_EBIIC;
    }
    *_out_value = g_forceMode;
    return RP_HW_OK;
}

int i2c_getDevAddress(int* _out_value){
    if (_out_value == NULL) {
        ERROR_LOG("Output parameter _out_value is NULL");
        return RP_HW_EBIIC;
    }

    if (g_addr < 0) {
        ERROR_LOG("Device not initialized. Call i2c_InitDevice first");
        return RP_HW_EIIIC;
    }

    *_out_value = g_addr;
    return RP_HW_OK;
}

static int check_i2c_initialized(void) {
    if (g_addr < 0) {
        ERROR_LOG("I2C device not initialized. Call i2c_InitDevice() first.");
        return RP_HW_EIIIC;
    }
    return RP_HW_OK;
}

int i2c_SMBUS_Read(uint8_t reg, uint8_t *value){
    if (value == NULL) {
        ERROR_LOG("Output parameter value is NULL");
        return RP_HW_EBIIC;
    }

    int ret = check_i2c_initialized();
    if (ret != RP_HW_OK) return ret;

    ret = i2c_SMBUS_read_byte(g_devicePath, g_addr, reg, value, g_forceMode);
    if (ret != RP_HW_OK) {
        TRACE("Failed to read byte from register 0x%02X on device 0x%02X", reg, g_addr);
    }
    return ret;
}

int i2c_SMBUS_ReadWord(uint8_t reg, uint16_t *value){
    if (value == NULL) {
        ERROR_LOG("Output parameter value is NULL");
        return RP_HW_EBIIC;
    }

    int ret = check_i2c_initialized();
    if (ret != RP_HW_OK) return ret;

    ret = i2c_SMBUS_read_word(g_devicePath, g_addr, reg, value, g_forceMode);
    if (ret != RP_HW_OK) {
        TRACE("Failed to read word from register 0x%02X on device 0x%02X", reg, g_addr);
    }
    return ret;
}

int i2c_SMBUS_ReadCommand(uint8_t *value){
    if (value == NULL) {
        ERROR_LOG("Output parameter value is NULL");
        return RP_HW_EBIIC;
    }

    int ret = check_i2c_initialized();
    if (ret != RP_HW_OK) return ret;

    ret = i2c_SMBUS_read_command(g_devicePath, g_addr, value, g_forceMode);
    if (ret != RP_HW_OK) {
        TRACE("Failed to read command from device 0x%02X", g_addr);
    }
    return ret;
}

int i2c_SMBUS_ReadBuffer(uint8_t reg, uint8_t *buffer, int *len){
    if (buffer == NULL) {
        ERROR_LOG("Output buffer is NULL");
        return RP_HW_EBIIC;
    }

    if (len == NULL) {
        ERROR_LOG("Length parameter is NULL");
        return RP_HW_EBIIC;
    }

    if (*len <= 0) {
        ERROR_LOG("Invalid buffer length: %d. Must be > 0", *len);
        return RP_HW_EBIIC;
    }

    int ret = check_i2c_initialized();
    if (ret != RP_HW_OK) return ret;

    ret = i2c_SMBUS_read_buffer(g_devicePath, g_addr, reg, buffer, len, g_forceMode);
    if (ret != RP_HW_OK) {
        TRACE("Failed to read buffer from register 0x%02X on device 0x%02X, requested len: %d",
                  reg, g_addr, *len);
    }
    return ret;
}

int i2c_SMBUS_Write(uint8_t reg, uint8_t value){
    int ret = check_i2c_initialized();
    if (ret != RP_HW_OK) return ret;

    ret = i2c_SMBUS_write_byte(g_devicePath, g_addr, reg, value, g_forceMode);
    if (ret != RP_HW_OK) {
        TRACE("Failed to write byte 0x%02X to register 0x%02X on device 0x%02X",
                  value, reg, g_addr);
    }
    return ret;
}

int i2c_SMBUS_WriteWord(uint8_t reg, uint16_t value){
    int ret = check_i2c_initialized();
    if (ret != RP_HW_OK) return ret;

    ret = i2c_SMBUS_write_word(g_devicePath, g_addr, reg, value, g_forceMode);
    if (ret != RP_HW_OK) {
        TRACE("Failed to write word 0x%04X to register 0x%02X on device 0x%02X",
                  value, reg, g_addr);
    }
    return ret;
}

int i2c_SMBUS_WriteCommand(uint8_t value){
    int ret = check_i2c_initialized();
    if (ret != RP_HW_OK) return ret;

    ret = i2c_SMBUS_write_command(g_devicePath, g_addr, value, g_forceMode);
    if (ret != RP_HW_OK) {
        TRACE("Failed to write command 0x%02X to device 0x%02X", value, g_addr);
    }
    return ret;
}

int i2c_SMBUS_WriteBuffer(uint8_t reg, uint8_t *buffer, int len){
    if (buffer == NULL) {
        ERROR_LOG("Input buffer is NULL");
        return RP_HW_EBIIC;
    }

    if (len <= 0) {
        ERROR_LOG("Invalid buffer length: %d. Must be > 0", len);
        return RP_HW_EBIIC;
    }

    if (len > I2C_SMBUS_BLOCK_MAX) {
        ERROR_LOG("Buffer length %d exceeds maximum %d", len, I2C_SMBUS_BLOCK_MAX);
        return RP_HW_EBIIC;
    }

    int ret = check_i2c_initialized();
    if (ret != RP_HW_OK) return ret;

    ret = i2c_SMBUS_write_buffer(g_devicePath, g_addr, reg, buffer, len, g_forceMode);
    if (ret != RP_HW_OK) {
        TRACE("Failed to write buffer (len: %d) to register 0x%02X on device 0x%02X",
                  len, reg, g_addr);
    }
    return ret;
}

int i2c_IOCTL_ReadBuffer(uint8_t *buffer, int len){
    if (buffer == NULL) {
        ERROR_LOG("Output buffer is NULL");
        return RP_HW_EBIIC;
    }

    if (len <= 0) {
        ERROR_LOG("Invalid buffer length: %d. Must be > 0", len);
        return RP_HW_EBIIC;
    }

    int ret = check_i2c_initialized();
    if (ret != RP_HW_OK) return ret;

    ret = i2c_IOCTL_read_buffer(g_devicePath, g_addr, buffer, len, g_forceMode);
    if (ret != RP_HW_OK) {
        TRACE("Failed to read buffer (len: %d) from device 0x%02X using IOCTL", len, g_addr);
    }
    return ret;
}

int i2c_IOCTL_WriteBuffer(uint8_t *buffer, int len){
    if (buffer == NULL) {
        ERROR_LOG("Input buffer is NULL");
        return RP_HW_EBIIC;
    }

    if (len <= 0) {
        ERROR_LOG("Invalid buffer length: %d. Must be > 0", len);
        return RP_HW_EBIIC;
    }

    int ret = check_i2c_initialized();
    if (ret != RP_HW_OK) return ret;

    ret = i2c_IOCTL_write_buffer(g_devicePath, g_addr, buffer, len, g_forceMode);
    if (ret != RP_HW_OK) {
        TRACE("Failed to write buffer (len: %d) to device 0x%02X using IOCTL", len, g_addr);
    }
    return ret;
}