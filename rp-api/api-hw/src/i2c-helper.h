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


int i2c_SBMUS_read_byte(const char* i2c_dev_node_path,uint8_t i2c_dev_address,uint8_t i2c_dev_reg_addr, uint8_t *value, bool force);

int i2c_SBMUS_read_word(const char* i2c_dev_node_path,uint8_t i2c_dev_address,uint8_t i2c_dev_reg_addr, uint16_t *value, bool force);

int i2c_SBMUS_read_command(const char* i2c_dev_node_path,uint8_t i2c_dev_address, uint8_t *value, bool force);

int i2c_SBMUS_read_buffer(const char* i2c_dev_node_path,uint8_t i2c_dev_address,uint8_t i2c_dev_reg_addr,uint8_t *buffer, int *len, bool force);

int i2c_SBMUS_write_byte(const char* i2c_dev_node_path,uint8_t i2c_dev_address,uint8_t i2c_dev_reg_addr, uint8_t i2c_val_to_write, bool force);

int i2c_SBMUS_write_word(const char* i2c_dev_node_path,uint8_t i2c_dev_address,uint8_t i2c_dev_reg_addr, uint16_t i2c_val_to_write, bool force);

int i2c_SBMUS_write_command(const char* i2c_dev_node_path,uint8_t i2c_dev_address,uint8_t i2c_dev_command, bool force);
 
int i2c_SBMUS_write_buffer(const char* i2c_dev_node_path,uint8_t i2c_dev_address,uint8_t i2c_dev_reg_addr,const uint8_t *buffer, int len, bool force);

int i2c_IOCTL_read_buffer(const char* i2c_dev_node_path,uint8_t i2c_dev_address,uint8_t *buffer, int len, bool force);

int i2c_IOCTL_write_buffer(const char* i2c_dev_node_path,uint8_t i2c_dev_address,uint8_t *buffer, int len, bool force);



#endif


