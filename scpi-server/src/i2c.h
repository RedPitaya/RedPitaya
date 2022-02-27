/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server I2C SCPI commands interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */


#ifndef SCPI_I2C_H_
#define SCPI_I2C_H_

#include "scpi/types.h"

scpi_result_t RP_I2C_Dev(scpi_t * context);
scpi_result_t RP_I2C_DevQ(scpi_t * context);
scpi_result_t RP_I2C_ForceMode(scpi_t * context);
scpi_result_t RP_I2C_ForceModeQ(scpi_t * context);

scpi_result_t RP_I2C_SMBUS_Read(scpi_t * context);
scpi_result_t RP_I2C_SMBUS_ReadWord(scpi_t * context);
scpi_result_t RP_I2C_SMBUS_ReadBuffer(scpi_t * context);


scpi_result_t RP_I2C_SMBUS_Write(scpi_t * context);
scpi_result_t RP_I2C_SMBUS_WriteWord(scpi_t * context);
scpi_result_t RP_I2C_SMBUS_WriteBuffer(scpi_t * context);

scpi_result_t RP_I2C_IOCTL_ReadBuffer(scpi_t * context);
scpi_result_t RP_I2C_IOCTL_WriteBuffer(scpi_t * context);


#endif /* SCPI_I2C_H_ */
