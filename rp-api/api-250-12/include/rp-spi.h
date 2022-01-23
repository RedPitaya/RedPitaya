#pragma once

#ifdef  __cplusplus
extern "C" {
#endif


namespace rp_spi_fpga{

/* Must be called in order to display debugging information. */
void rp_spi_enable_verbous();
void rp_spi_disable_verbous();

 int rp_spi_load(const char *configuration_file);
 int rp_spi_print(const char *configuration_file);

 int rp_spi_compare(const char *configuration_file);

 int rp_write_to_spi_fpga(const char* spi_dev_path,unsigned int fpga_address,unsigned short dev_address,int reg_addr, unsigned char spi_val_to_write);
 int rp_read_from_spi_fpga(const char* spi_dev_path,unsigned int fpga_address,unsigned short dev_address,int reg_addr, char &value);

 int rp_spi_load_via_fpga(const char *configuration_file);

}

#ifdef  __cplusplus
}
#endif

