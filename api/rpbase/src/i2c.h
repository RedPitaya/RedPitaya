/**
* $Id: $
*
* @brief Red Pitaya library I2C module interface
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

#define I2C_DEVICE_NAME "/dev/i2c-0"

int i2c_Init();
int i2c_Release();

int i2c_read(int addr, char *data, int length);
int i2c_write(int addr, char *data, int length);

#endif //__I2C_H