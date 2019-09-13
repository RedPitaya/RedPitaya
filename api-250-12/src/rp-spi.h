#pragma once

#ifdef  __cplusplus
extern "C" {
#endif

/* Must be called in order to display debugging information. */
void rp_spi_enable_verbous();
void rp_spi_disable_verbous();

 int rp_spi_load(const char *configuration_file);
 int rp_spi_print(const char *configuration_file);

 int rp_spi_compare(const char *configuration_file);

 int rp_write_to_spi(const char* spi_dev_path,int reg_addr, unsigned char spi_val_to_write);
 int rp_read_from_spi(const char* spi_dev_path,int reg_addr, char &value);

#ifdef  __cplusplus
}
#endif

