#pragma once
#include <stdint.h>

int write_to_fpga_spi(const char* _path,unsigned int fpga_address,unsigned short dev_address,char a_addr, uint8_t spi_val_to_write);

int read_from_fpga_spi(const char* _path,unsigned int fpga_address,unsigned short dev_address,char a_addr, uint8_t *value);






