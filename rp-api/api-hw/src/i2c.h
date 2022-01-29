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

#ifndef __I2C_H
#define __I2C_H


#include "rp_hw.h"

int i2c_InitDevice(char *_device,uint8_t addr);
int i2c_setForceMode(bool force);
bool i2c_getForceMode();
int i2c_getDevAddress();

int i2c_Read(uint8_t reg,uint8_t *value);
int i2c_ReadWord(uint8_t reg,uint16_t *value);
int i2c_ReadCommand(uint8_t *value);
int i2c_ReadBuffer(uint8_t reg, uint8_t *buffer, int *len);

int i2c_Write(uint8_t reg,uint8_t value);
int i2c_WriteWord(uint8_t reg,uint16_t value);
int i2c_WriteCommand(uint8_t value);
int i2c_WriteBuffer(uint8_t reg, uint8_t *buffer, int len);

#endif
