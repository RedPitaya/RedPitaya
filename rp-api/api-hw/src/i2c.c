/**
 * $Id: $
 *
 * @brief Red Pitaya I2C Module.
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
#include "i2c.h"
#include "i2c-helper.h"

#define DEV_PATH_LEN 256

char    g_devicePath[DEV_PATH_LEN] = "/dev/i2c-0";
bool    g_forceMode = false;
int     g_addr = -1;

int i2c_InitDevice(const char *_device, uint8_t addr){

    if (addr < 0x03 || addr > 0x77) {
        fprintf(stderr,"[rp_i2c] Device address out of range [0x03-0x77].\n");
        return RP_HW_EIIIC;
    }
    g_addr = addr;

    int len = strlen(_device);
    if (len >= DEV_PATH_LEN){
        return RP_HW_EIIIC;
    }

    strcpy(g_devicePath,_device);
    return RP_HW_OK;
}

int i2c_setForceMode(bool force){
    g_forceMode = force;
    return RP_HW_OK;
}

bool i2c_getForceMode(){
    return g_forceMode;
}

int i2c_getDevAddress(){
    return g_addr;
}


int i2c_SMBUS_Read(uint8_t reg,uint8_t *value){
    if (g_addr < 0) {
        fprintf(stderr,"[rp_i2c] Device address not set.\n");
        return RP_HW_EIIIC;
    }
    return i2c_SBMUS_read_byte(g_devicePath,g_addr,reg,value,g_forceMode);
}

int i2c_SMBUS_ReadWord(uint8_t reg,uint16_t *value){
    if (g_addr < 0) {
        fprintf(stderr,"[rp_i2c] Device address not set.\n");
        return RP_HW_EIIIC;
    }
    return i2c_SBMUS_read_word(g_devicePath,g_addr,reg,value,g_forceMode);
}

int i2c_SMBUS_ReadCommand(uint8_t *value){
    if (g_addr < 0) {
        fprintf(stderr,"[rp_i2c] Device address not set.\n");
        return RP_HW_EIIIC;
    }
    return i2c_SBMUS_read_command(g_devicePath,g_addr,value,g_forceMode);
}

int i2c_SMBUS_ReadBuffer(uint8_t reg, uint8_t *buffer, int *len){
    if (g_addr < 0) {
        fprintf(stderr,"[rp_i2c] Device address not set.\n");
        return RP_HW_EIIIC;
    }
    return i2c_SBMUS_read_buffer(g_devicePath,g_addr,reg,buffer,len,g_forceMode);
}

int i2c_SMBUS_Write(uint8_t reg,uint8_t value){
    if (g_addr < 0) {
        fprintf(stderr,"[rp_i2c] Device address not set.\n");
        return RP_HW_EIIIC;
    }
    return i2c_SBMUS_write_byte(g_devicePath,g_addr,reg,value,g_forceMode);
}

int i2c_SMBUS_WriteWord(uint8_t reg,uint16_t value){
    if (g_addr < 0) {
        fprintf(stderr,"[rp_i2c] Device address not set.\n");
        return RP_HW_EIIIC;
    }
    return i2c_SBMUS_write_word(g_devicePath,g_addr,reg,value,g_forceMode);
}

int i2c_SMBUS_WriteCommand(uint8_t value){
    if (g_addr < 0) {
        fprintf(stderr,"[rp_i2c] Device address not set.\n");
        return RP_HW_EIIIC;
    }
    return i2c_SBMUS_write_command(g_devicePath,g_addr,value,g_forceMode);
}

int i2c_SMBUS_WriteBuffer(uint8_t reg, uint8_t *buffer, int len){
    if (g_addr < 0) {
        fprintf(stderr,"[rp_i2c] Device address not set.\n");
        return RP_HW_EIIIC;
    }
    return i2c_SBMUS_write_buffer(g_devicePath,g_addr,reg,buffer,len,g_forceMode);
}

int  i2c_IOCTL_ReadBuffer(uint8_t *buffer, int len){
    if (g_addr < 0) {
        fprintf(stderr,"[rp_i2c] Device address not set.\n");
        return RP_HW_EIIIC;
    }
    return i2c_IOCTL_read_buffer(g_devicePath,g_addr,buffer,len,g_forceMode);
}

int  i2c_IOCTL_WriteBuffer(uint8_t *buffer, int len){
    if (g_addr < 0) {
        fprintf(stderr,"[rp_i2c] Device address not set.\n");
        return RP_HW_EIIIC;
    }
    return i2c_IOCTL_write_buffer(g_devicePath,g_addr,buffer,len,g_forceMode);
}
