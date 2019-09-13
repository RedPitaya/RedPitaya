#pragma once

#ifdef  __cplusplus
extern "C" {
#endif

int write_to_spi(const char* spi_dev_path,char *buffer_header,int header_length, unsigned char spi_val_to_write);

int read_from_spi(const char* spi_dev_path,char *buffer_header,int header_length, char &value);

#ifdef  __cplusplus
}
#endif



