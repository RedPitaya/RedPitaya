/**
 * $Id: $
 *
 * @brief Red Pitaya I2C Module.
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#ifndef __I2C_H
#define __I2C_H


#include "rp_hw.h"

int  i2c_InitDevice(const char *_device,uint8_t addr);
int  i2c_setForceMode(bool force);
bool i2c_getForceMode();
int  i2c_getDevAddress();

int  i2c_SMBUS_Read(uint8_t reg,uint8_t *value);
int  i2c_SMBUS_ReadWord(uint8_t reg,uint16_t *value);
int  i2c_SMBUS_ReadCommand(uint8_t *value);
int  i2c_SMBUS_ReadBuffer(uint8_t reg, uint8_t *buffer, int *len);

int  i2c_SMBUS_Write(uint8_t reg,uint8_t value);
int  i2c_SMBUS_WriteWord(uint8_t reg,uint16_t value);
int  i2c_SMBUS_WriteCommand(uint8_t value);
int  i2c_SMBUS_WriteBuffer(uint8_t reg, uint8_t *buffer, int len);

int  i2c_IOCTL_ReadBuffer(uint8_t *buffer, int len);
int  i2c_IOCTL_WriteBuffer(uint8_t *buffer, int len);

#endif
