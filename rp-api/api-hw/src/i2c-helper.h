/**
 * $Id: $
 *
 * @brief Red Pitaya I2C Helper Module.
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef __I2C_HELPER_H
#define __I2C_HELPER_H

#include "rp_hw.h"

#ifdef  __cplusplus
extern "C" {
#endif

int read_from_i2c(const char* i2c_dev_node_path,uint8_t i2c_dev_address,uint8_t i2c_dev_reg_addr, uint8_t *value, bool force);

int read_from_i2c_word(const char* i2c_dev_node_path,uint8_t i2c_dev_address,uint8_t i2c_dev_reg_addr, uint16_t *value, bool force);

int read_from_i2c_command(const char* i2c_dev_node_path,uint8_t i2c_dev_address, uint8_t *value, bool force);

int read_to_i2c_buffer(const char* i2c_dev_node_path,uint8_t i2c_dev_address,uint8_t i2c_dev_reg_addr,uint8_t *buffer, int *len, bool force);

int write_to_i2c(const char* i2c_dev_node_path,uint8_t i2c_dev_address,uint8_t i2c_dev_reg_addr, uint8_t i2c_val_to_write, bool force);

int write_to_i2c_word(const char* i2c_dev_node_path,uint8_t i2c_dev_address,uint8_t i2c_dev_reg_addr, uint16_t i2c_val_to_write, bool force);

int write_to_i2c_command(const char* i2c_dev_node_path,uint8_t i2c_dev_address,uint8_t i2c_dev_command, bool force);
 
int write_to_i2c_buffer(const char* i2c_dev_node_path,uint8_t i2c_dev_address,uint8_t i2c_dev_reg_addr,const uint8_t *buffer, int len, bool force);

#ifdef  __cplusplus
}
#endif

#endif


