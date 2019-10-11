#pragma once

#ifdef  __cplusplus
extern "C" {
#endif

namespace rp_i2c {

    /* Must be called in order to display debugging information. */
    void rp_i2c_enable_verbous();
    void rp_i2c_disable_verbous();
    
    /* Load configuration in i2c */
    int rp_i2c_load(const char *configuration_file);

    /* Print on screen data from i2c */
    int rp_i2c_print(const char *configuration_file);

    /* Check data in i2c with file configuration */
    int rp_i2c_compare(const char *configuration_file);

    /* The function of recording data from the i2c bus.
    If successful, reading returns status 0 
    i2c_dev_node_path must be like "/dev/i2c-0"
    */

    int rp_write_to_i2c(const char* i2c_dev_path,int i2c_dev_address,int i2c_dev_reg_addr, unsigned short i2c_val_to_write);

    /* The function of reading data from the i2c bus.
    If successful, reading returns status 0 
    i2c_dev_node_path must be like "/dev/i2c-0"
    */

    int rp_read_from_i2c(const char* i2c_dev_path,int i2c_dev_address,int i2c_dev_reg_addr, char &value);
}

#ifdef  __cplusplus
}
#endif

