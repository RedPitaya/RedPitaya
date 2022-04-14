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
#include <string.h>
#include <stdlib.h>

#include "common.h"
#include "i2c.h"
#include "scpi/parser.h"
#include "rp_hw.h"

scpi_result_t RP_I2C_Dev(scpi_t * context){
    RP_F_NAME(const char func[] = "I2C:DEV#";)
    int32_t cmd[1] = {0};
    uint8_t dev_address = 0;
    if (!SCPI_CommandNumbers(context,cmd,1,-1)){
        RP_LOG(LOG_ERR, "*%s Failed to get parameters.\n",func);
        return SCPI_RES_ERR;
    }
    if (cmd[0] == -1){
        RP_LOG(LOG_ERR, "*%s Failed to get address of device.\n",func);
        return SCPI_RES_ERR;
    }
    dev_address = cmd[0];

    char  dev[255];
    const char * param = NULL;
    size_t param_len = 0;
    memset(dev,0,255);

    if (!SCPI_ParamCharacters(context, &param, &param_len, true)) {
        RP_LOG(LOG_ERR, "*%s is missing first parameter.\n",func);
        return SCPI_RES_ERR;
    }

    if (param_len == 0) {
        RP_LOG(LOG_ERR, "*%s is missing first parameter.\n",func);
        return SCPI_RES_ERR;
    }

    strncpy(dev,param,param_len);
    int result = rp_I2C_InitDevice(dev,dev_address);
    if (RP_HW_OK != result) {
        RP_LOG(LOG_ERR, "*%s Failed to init Red Pitaya spi: %d\n" ,func, result);
        return SCPI_RES_ERR;
    }

    RP_LOG(LOG_INFO, "*%s Successfully init spi.\n",func);
    return SCPI_RES_OK;
}

scpi_result_t RP_I2C_DevQ(scpi_t * context){
    RP_F_NAME(const char func[] = "I2C:DEV?";)
    int32_t address;
    int result = rp_I2C_getDevAddress(&address);

    if (RP_OK != result) {
        RP_LOG(LOG_ERR, "*%s Failed to get i2c device address: %d",func, result);
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultUInt32Base(context, address, 10);

    RP_LOG(LOG_INFO, "*%s Successfully returned i2c device address.\n",func);
    return SCPI_RES_OK;
}

scpi_result_t RP_I2C_ForceMode(scpi_t * context){
    RP_F_NAME(const char func[] = "I2C:FMODE";)   
    scpi_bool_t value;

    if (!SCPI_ParamBool(context, &value, true)) {
        RP_LOG(LOG_ERR, "*%s is missing first parameter.\n",func);
        return SCPI_RES_ERR;
    }

    int result = rp_I2C_setForceMode(value);
    if (RP_HW_OK != result) {
        RP_LOG(LOG_ERR, "*%s Failed to set i2c force mode: %d\n",func, result);
        return SCPI_RES_ERR;
    }

    RP_LOG(LOG_INFO, "*%s Successfully set i2c force mode.\n",func);
    return SCPI_RES_OK;
}

scpi_result_t RP_I2C_ForceModeQ(scpi_t * context){
    RP_F_NAME(const char func[] = "I2C:FMODE?";)
    bool value;
    int result = rp_I2C_getForceMode(&value);

    if (RP_OK != result) {
        RP_LOG(LOG_ERR, "*%s Failed to get i2c force mode: %d",func, result);
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultBool(context, value);

    RP_LOG(LOG_INFO, "*%s Successfully returned i2c force mode.\n",func);
    return SCPI_RES_OK;
}

scpi_result_t RP_I2C_SMBUS_Read(scpi_t * context){
    RP_F_NAME(const char func[] = "I2C:Smbus:Read#";)
    int32_t cmd[1] = {0};
    uint8_t reg = 0;
    
    if (!SCPI_CommandNumbers(context,cmd,1,-1)){
        RP_LOG(LOG_ERR, "*%s Failed to get parameters.\n",func);
        return SCPI_RES_ERR;
    }
    if (cmd[0] == -1){
        RP_LOG(LOG_ERR, "*%s Failed to get register for read\n",func);
        return SCPI_RES_ERR;
    }
    reg = cmd[0];

    uint8_t value;
    int result = rp_I2C_SMBUS_Read(reg, &value);

    if (RP_OK != result) {
        RP_LOG(LOG_ERR, "*%s Failed to read byte from i2c: %d",func, result);
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultUInt32Base(context, value, 10);

    RP_LOG(LOG_INFO, "*%s Successfully returned value from i2c.\n",func);
    return SCPI_RES_OK;
}

scpi_result_t RP_I2C_SMBUS_ReadWord(scpi_t * context){
    RP_F_NAME(const char func[] = "I2C:Smbus:Read#:Word";)
    int32_t cmd[1] = {0};
    uint8_t reg = 0;
    
    if (!SCPI_CommandNumbers(context,cmd,1,-1)){
        RP_LOG(LOG_ERR, "*%s Failed to get parameters.\n",func);
        return SCPI_RES_ERR;
    }
    if (cmd[0] == -1){
        RP_LOG(LOG_ERR, "*%s Failed to get register for read\n",func);
        return SCPI_RES_ERR;
    }
    reg = cmd[0];

    uint16_t value;
    int result = rp_I2C_SMBUS_ReadWord(reg, &value);

    if (RP_OK != result) {
        RP_LOG(LOG_ERR, "*%s Failed to read word from i2c: %d",func, result);
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultUInt32Base(context, value, 10);

    RP_LOG(LOG_INFO, "*%s Successfully returned value from i2c.\n",func);
    return SCPI_RES_OK;
}

scpi_result_t RP_I2C_SMBUS_ReadBuffer(scpi_t * context){
    RP_F_NAME(const char func[] = "I2C:Smbus:Read#:Buffer#";)
    size_t reg = 0;
    size_t size = 0;
    uint8_t *buffer = NULL;
    int32_t cmd[2] = {0,0};

    
    if (!SCPI_CommandNumbers(context,cmd,2,-1)){
        RP_LOG(LOG_ERR, "*%s Failed to get parameters.\n",func);
        return SCPI_RES_ERR;
    }
    
    if (cmd[0] == -1){
        RP_LOG(LOG_ERR, "*%s Failed to get register for read.\n",func);
        return SCPI_RES_ERR;
    }

    if (cmd[1] == -1){
        RP_LOG(LOG_ERR, "*%s Failed to get size of buffer.\n",func);
        return SCPI_RES_ERR;
    }

    reg = cmd[0];
    size = cmd[1];

    buffer = malloc(size * sizeof(uint8_t));
    if (!buffer){
        RP_LOG(LOG_ERR, "*%s Failed allocate buffer with size: %d.\n",func,size);
        return SCPI_RES_ERR;
    }

    int read_size = size;
    int result = rp_I2C_SMBUS_ReadBuffer(reg,buffer, &read_size);
    if(result != RP_HW_OK){
        RP_LOG(LOG_ERR, "*%s Failed read buffer: %d\n", func, result);
        free(buffer);        
        return SCPI_RES_ERR;
    }

    SCPI_ResultBufferUInt8(context, buffer, read_size);
    free(buffer);
    RP_LOG(LOG_INFO, "*%s Successfully returned i2c buffer.\n",func);
    return SCPI_RES_OK;
}

scpi_result_t RP_I2C_IOCTL_ReadBuffer(scpi_t * context){
    RP_F_NAME(const char func[] = "I2C:IOctl:Read:Buffer#";)
    size_t size = 0;
    uint8_t *buffer = NULL;
    int32_t cmd[1] = {0};
    
    if (!SCPI_CommandNumbers(context,cmd,1,-1)){
        RP_LOG(LOG_ERR, "*%s Failed to get parameters.\n",func);
        return SCPI_RES_ERR;
    }
    
    if (cmd[0] == -1){
        RP_LOG(LOG_ERR, "*%s Failed to get size of buffer.\n",func);
        return SCPI_RES_ERR;
    }

    size = cmd[0];

    buffer = malloc(size * sizeof(uint8_t));
    if (!buffer){
        RP_LOG(LOG_ERR, "*%s Failed allocate buffer with size: %d.\n",func,size);
        return SCPI_RES_ERR;
    }

    int read_size = size;
    int result = rp_I2C_IOCTL_ReadBuffer(buffer, read_size);
    if(result != RP_HW_OK){
        RP_LOG(LOG_ERR, "*%s Failed read buffer: %d\n", func, result);
        free(buffer);        
        return SCPI_RES_ERR;
    }
    
    SCPI_ResultBufferUInt8(context, buffer, read_size);
    free(buffer);
    RP_LOG(LOG_INFO, "*%s Successfully returned i2c buffer.\n",func);
    return SCPI_RES_OK;
}


scpi_result_t RP_I2C_SMBUS_Write(scpi_t * context){
    RP_F_NAME(const char func[] = "I2C:Smbus:Write#";)
    int32_t cmd[1] = {0};
    uint8_t reg = 0;
    
    if (!SCPI_CommandNumbers(context,cmd,1,-1)){
        RP_LOG(LOG_ERR, "*%s Failed to get parameters.\n",func);
        return SCPI_RES_ERR;
    }
    if (cmd[0] == -1){
        RP_LOG(LOG_ERR, "*%s Failed to get register for write\n",func);
        return SCPI_RES_ERR;
    }
    reg = cmd[0];

    uint32_t value;

    if (!SCPI_ParamUInt32(context, &value, true)) {
        RP_LOG(LOG_ERR, "*%s is missing first parameter.\n",func);
        return SCPI_RES_ERR;
    }

    int result = rp_I2C_SMBUS_Write(reg, value);

    if (RP_OK != result) {
        RP_LOG(LOG_ERR, "*%s Failed write byte to i2c: %d",func, result);
        return SCPI_RES_ERR;
    }

    RP_LOG(LOG_INFO, "*%s Successfully write to i2c.\n",func);
    return SCPI_RES_OK;
}

scpi_result_t RP_I2C_SMBUS_WriteWord(scpi_t * context){
    RP_F_NAME(const char func[] = "I2C:Smbus:Write#:Word";)
    int32_t cmd[1] = {0};
    uint8_t reg = 0;
    
    if (!SCPI_CommandNumbers(context,cmd,1,-1)){
        RP_LOG(LOG_ERR, "*%s Failed to get parameters.\n",func);
        return SCPI_RES_ERR;
    }
    if (cmd[0] == -1){
        RP_LOG(LOG_ERR, "*%s Failed to get register for write\n",func);
        return SCPI_RES_ERR;
    }
    reg = cmd[0];

    uint32_t value;

    if (!SCPI_ParamUInt32(context, &value, true)) {
        RP_LOG(LOG_ERR, "*%s is missing first parameter.\n",func);
        return SCPI_RES_ERR;
    }

    int result = rp_I2C_SMBUS_WriteWord(reg, value);

    if (RP_OK != result) {
        RP_LOG(LOG_ERR, "*%s Failed write byte to i2c: %d",func, result);
        return SCPI_RES_ERR;
    }

    RP_LOG(LOG_INFO, "*%s Successfully write to i2c.\n",func);
    return SCPI_RES_OK;
}

scpi_result_t RP_I2C_SMBUS_WriteBuffer(scpi_t * context){
    RP_F_NAME(const char func[] = "I2C:Smbus:Write#:Buffer#";)
    size_t reg = 0;
    size_t size = 0;
    uint8_t *buffer = NULL;
    int32_t cmd[2] = {0,0};

    
    if (!SCPI_CommandNumbers(context,cmd,2,-1)){
        RP_LOG(LOG_ERR, "*%s Failed to get parameters.\n",func);
        return SCPI_RES_ERR;
    }
    
    if (cmd[0] == -1){
        RP_LOG(LOG_ERR, "*%s Failed to get register for read.\n",func);
        return SCPI_RES_ERR;
    }

    if (cmd[1] == -1){
        RP_LOG(LOG_ERR, "*%s Failed to get size of buffer.\n",func);
        return SCPI_RES_ERR;
    }

    reg = cmd[0];
    size = cmd[1];

    buffer = malloc(size * sizeof(uint8_t));
    if (!buffer){
        RP_LOG(LOG_ERR, "*%s Failed allocate buffer with size: %d.\n",func,size);
        return SCPI_RES_ERR;
    }

    size_t buf_size = size;
    if(!SCPI_ParamBufferUInt8(context, buffer, &buf_size, true)){
        RP_LOG(LOG_ERR, "*%s Failed get data for buffer.\n",func);
        free(buffer);
        return SCPI_RES_ERR;
    }

    if (buf_size != size){
        RP_LOG(LOG_ERR, "*%s Wrong data length.\n",func);
        free(buffer);
        return SCPI_RES_ERR;
    }
    
    int result = rp_I2C_SMBUS_WriteBuffer(reg,buffer, buf_size);
    if(result != RP_HW_OK){
        RP_LOG(LOG_ERR, "*%s Failed write buffer: %d\n", func, result);
        free(buffer);        
        return SCPI_RES_ERR;
    }

    free(buffer);
    RP_LOG(LOG_INFO, "*%s Successfully write data to i2c.\n",func);
    return SCPI_RES_OK;
}

scpi_result_t RP_I2C_IOCTL_WriteBuffer(scpi_t * context){
    RP_F_NAME(const char func[] = "I2C:IOctl:Write:Buffer#";)
    size_t size = 0;
    uint8_t *buffer = NULL;
    int32_t cmd[1] = {0};

    
    if (!SCPI_CommandNumbers(context,cmd,1,-1)){
        RP_LOG(LOG_ERR, "*%s Failed to get parameters.\n",func);
        return SCPI_RES_ERR;
    }
    
    if (cmd[0] == -1){
        RP_LOG(LOG_ERR, "*%s Failed to get size of buffer.\n",func);
        return SCPI_RES_ERR;
    }

    size = cmd[0];

    buffer = malloc(size * sizeof(uint8_t));
    if (!buffer){
        RP_LOG(LOG_ERR, "*%s Failed allocate buffer with size: %d.\n",func,size);
        return SCPI_RES_ERR;
    }

    size_t buf_size = size;
    if(!SCPI_ParamBufferUInt8(context, buffer, &buf_size, true)){
        RP_LOG(LOG_ERR, "*%s Failed get data for buffer.\n",func);
        free(buffer);
        return SCPI_RES_ERR;
    }

    if (buf_size != size){
        RP_LOG(LOG_ERR, "*%s Wrong data length.\n",func);
        free(buffer);
        return SCPI_RES_ERR;
    }
    
    int result = rp_I2C_IOCTL_WriteBuffer(buffer, buf_size);
    if(result != RP_HW_OK){
        RP_LOG(LOG_ERR, "*%s Failed write buffer: %d\n", func, result);
        free(buffer);        
        return SCPI_RES_ERR;
    }

    free(buffer);
    RP_LOG(LOG_INFO, "*%s Successfully write data to i2c.\n",func);
    return SCPI_RES_OK;

}


