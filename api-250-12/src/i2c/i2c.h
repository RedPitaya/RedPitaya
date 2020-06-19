#pragma once

#ifdef  __cplusplus
extern "C" {
#endif

int write_to_i2c(const char* i2c_dev_node_path,int i2c_dev_address,int i2c_dev_reg_addr, unsigned char i2c_val_to_write, bool force);

int write_to_i2c_word(const char* i2c_dev_node_path,int i2c_dev_address,int i2c_dev_reg_addr, unsigned short i2c_val_to_write, bool force);

int read_from_i2c(const char* i2c_dev_node_path,int i2c_dev_address,int i2c_dev_reg_addr, char &value, bool force);

int read_from_i2c_command(const char* i2c_dev_node_path,int i2c_dev_address, char &value, bool force);

int write_to_i2c_command(const char* i2c_dev_node_path,int i2c_dev_address,int i2c_dev_command, bool force);
 
int write_to_i2c_buffer(const char* i2c_dev_node_path,int i2c_dev_address,int i2c_dev_reg_addr,const unsigned char *buffer, int len, bool force);

int read_to_i2c_buffer(const char* i2c_dev_node_path,int i2c_dev_address,int i2c_dev_reg_addr,unsigned char *buffer, bool force);

#ifdef  __cplusplus
}
#endif



