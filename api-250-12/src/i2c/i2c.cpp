#include "i2c.h"

#include <linux/i2c-dev.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <iostream>
#include <fstream>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define MSG_A(args...) fprintf(stdout,args);

pthread_mutex_t i2c_mutex = PTHREAD_MUTEX_INITIALIZER;

int write_to_i2c(const char* i2c_dev_node_path,int i2c_dev_address,int i2c_dev_reg_addr, unsigned char i2c_val_to_write, bool force){
	pthread_mutex_lock(&i2c_mutex);
    int i2c_dev_node = 0;
	int ret_val = 0;
    __s32 read_value = 0;

	/* Open the device node for the I2C adapter of bus 4 */
	i2c_dev_node = open(i2c_dev_node_path, O_RDWR);
	if (i2c_dev_node < 0) {
		MSG_A("[rp_i2c] Unable to open device node ERROR: %d: %s\n", errno, strerror(errno));
		pthread_mutex_unlock(&i2c_mutex);
		return -1;
	}

	unsigned long flag = I2C_SLAVE;
	if (force) flag = I2C_SLAVE_FORCE;

	/* Set I2C_SLAVE for adapter 4 */
	ret_val = ioctl(i2c_dev_node, flag ,i2c_dev_address);
	if (ret_val < 0) {
        MSG_A("[rp_i2c] Could not set I2C_SLAVE.\n");
        close(i2c_dev_node);
		pthread_mutex_unlock(&i2c_mutex);
		return -1;
	}

	/* Read byte from the register of the I2C_SLAVE device */
	read_value = i2c_smbus_read_byte_data(i2c_dev_node, i2c_dev_reg_addr);
	if (read_value < 0) {
        MSG_A("[rp_i2c] I2C Read operation failed.\n");
        close(i2c_dev_node);
		pthread_mutex_unlock(&i2c_mutex);
        return -1;
	}
	ret_val = i2c_smbus_write_byte_data(i2c_dev_node,
					i2c_dev_reg_addr,
					i2c_val_to_write);

	if (ret_val < 0) {
        MSG_A("[rp_i2c] I2C Write Operation failed.\n");
        close(i2c_dev_node);
		pthread_mutex_unlock(&i2c_mutex);
        return -1;
	}
    close(i2c_dev_node);                 
	pthread_mutex_unlock(&i2c_mutex); 
	return 0;
}

int write_to_i2c_word(const char* i2c_dev_node_path,int i2c_dev_address,int i2c_dev_reg_addr, unsigned short i2c_val_to_write, bool force){
	pthread_mutex_lock(&i2c_mutex);
    int i2c_dev_node = 0;
	int ret_val = 0;
    __s32 read_value = 0;

	/* Open the device node for the I2C adapter of bus 4 */
	i2c_dev_node = open(i2c_dev_node_path, O_RDWR);
	if (i2c_dev_node < 0) {
		MSG_A("[rp_i2c] Unable to open device node ERROR: %d: %s\n", errno, strerror(errno));
		pthread_mutex_unlock(&i2c_mutex);
		return -1;
	}

	unsigned long flag = I2C_SLAVE;
	if (force) flag = I2C_SLAVE_FORCE;

	/* Set I2C_SLAVE for adapter 4 */
	ret_val = ioctl(i2c_dev_node, flag ,i2c_dev_address);
	if (ret_val < 0) {
        MSG_A("[rp_i2c] Could not set I2C_SLAVE.\n");
        close(i2c_dev_node);
		pthread_mutex_unlock(&i2c_mutex);
		return -1;
	}

	/* Read byte from the register of the I2C_SLAVE device */
	read_value = i2c_smbus_read_byte_data(i2c_dev_node, i2c_dev_reg_addr);
	if (read_value < 0) {
        MSG_A("[rp_i2c] I2C Read operation failed.\n");
        close(i2c_dev_node);
		pthread_mutex_unlock(&i2c_mutex);
        return -1;
	}

	ret_val = i2c_smbus_write_word_data(i2c_dev_node,
					i2c_dev_reg_addr,
					i2c_val_to_write);
	if (ret_val < 0) {
        MSG_A("[rp_i2c] I2C Write Operation failed.\n");
        close(i2c_dev_node);
		pthread_mutex_unlock(&i2c_mutex);
        return -1;
	}
    close(i2c_dev_node);      
	pthread_mutex_unlock(&i2c_mutex);            
	return 0;
}

int read_from_i2c(const char* i2c_dev_node_path,int i2c_dev_address,int i2c_dev_reg_addr, char &value, bool force){
	pthread_mutex_lock(&i2c_mutex);
    int i2c_dev_node = 0;
	int ret_val = 0;
    __s32 read_value = 0;

	/* Open the device node for the I2C adapter of bus 4 */
	i2c_dev_node = open(i2c_dev_node_path, O_RDWR);
	if (i2c_dev_node < 0) {
		MSG_A("[rp_i2c] Unable to open device node ERROR: %d: %s\n", errno, strerror(errno));
		pthread_mutex_unlock(&i2c_mutex);
		return -1;
	}

	unsigned long flag = I2C_SLAVE;
	if (force) flag = I2C_SLAVE_FORCE;

	/* Set I2C_SLAVE for adapter 4 */
	ret_val = ioctl(i2c_dev_node,flag,i2c_dev_address);
	if (ret_val < 0) {
        MSG_A("[rp_i2c] Could not set I2C_SLAVE.\n");
        close(i2c_dev_node);
		pthread_mutex_unlock(&i2c_mutex);
		return -1;
	}

	/* Read byte from the register of the I2C_SLAVE device */
	read_value = i2c_smbus_read_byte_data(i2c_dev_node, i2c_dev_reg_addr);
	if (read_value < 0) {
        MSG_A("[rp_i2c] I2C Read operation failed.\n");
        close(i2c_dev_node);
		pthread_mutex_unlock(&i2c_mutex);
        return -1;
	}
	close(i2c_dev_node);
    value = (char)read_value;      
	pthread_mutex_unlock(&i2c_mutex);              
	return 0;
}

int read_from_i2c_command(const char* i2c_dev_node_path,int i2c_dev_address, char &value, bool force){
	pthread_mutex_lock(&i2c_mutex);
    int i2c_dev_node = 0;
	int ret_val = 0;
    __s32 read_value = 0;

	/* Open the device node for the I2C adapter of bus 4 */
	i2c_dev_node = open(i2c_dev_node_path, O_RDWR);
	if (i2c_dev_node < 0) {
		MSG_A("[rp_i2c] Unable to open device node ERROR: %d: %s\n", errno, strerror(errno));
		pthread_mutex_unlock(&i2c_mutex);
		return -1;
	}

	unsigned long flag = I2C_SLAVE;
	if (force) flag = I2C_SLAVE_FORCE;

	/* Set I2C_SLAVE for adapter 4 */
	ret_val = ioctl(i2c_dev_node,flag,i2c_dev_address);
	if (ret_val < 0) {
        MSG_A("[rp_i2c] Could not set I2C_SLAVE.\n");
        close(i2c_dev_node);
		pthread_mutex_unlock(&i2c_mutex);
		return -1;
	}

	/* Read byte from the register of the I2C_SLAVE device */
	read_value = i2c_smbus_read_byte(i2c_dev_node);
	if (read_value < 0) {
        MSG_A("[rp_i2c] I2C Read operation failed.\n");
        close(i2c_dev_node);
		pthread_mutex_unlock(&i2c_mutex);
        return -1;
	}
	close(i2c_dev_node);
    value = (char)read_value;  
	pthread_mutex_unlock(&i2c_mutex);                  
	return 0;
}


int write_to_i2c_command(const char* i2c_dev_node_path,int i2c_dev_address,int i2c_dev_command, bool force){
	pthread_mutex_lock(&i2c_mutex);
    int i2c_dev_node = 0;
	int ret_val = 0;
    __s32 read_value = 0;

	/* Open the device node for the I2C adapter of bus 4 */
	i2c_dev_node = open(i2c_dev_node_path, O_RDWR);
	if (i2c_dev_node < 0) {
		MSG_A("[rp_i2c] Unable to open device node ERROR: %d: %s\n", errno, strerror(errno));
		pthread_mutex_unlock(&i2c_mutex);
		return -1;
	}

	unsigned long flag = I2C_SLAVE;
	if (force) flag = I2C_SLAVE_FORCE;

	/* Set I2C_SLAVE for adapter 4 */
	ret_val = ioctl(i2c_dev_node,flag,i2c_dev_address);
	if (ret_val < 0) {
        MSG_A("[rp_i2c] Could not set I2C_SLAVE.\n");
        close(i2c_dev_node);
		pthread_mutex_unlock(&i2c_mutex);
		return -1;
	}

	/* Read byte from the register of the I2C_SLAVE device */
	read_value = i2c_smbus_read_byte_data(i2c_dev_node, i2c_dev_command);
	if (read_value < 0) {
        MSG_A("[rp_i2c] I2C Read operation failed.\n");
        close(i2c_dev_node);
		pthread_mutex_unlock(&i2c_mutex);
        return -1;
	}
	ret_val = i2c_smbus_write_byte(i2c_dev_node,i2c_dev_command);

	if (ret_val < 0) {
        MSG_A("[rp_i2c] I2C Write Operation failed.\n");
        close(i2c_dev_node);
		pthread_mutex_unlock(&i2c_mutex);
        return -1;
	}
    close(i2c_dev_node);     
	pthread_mutex_unlock(&i2c_mutex);             
	return 0;
}
 
int write_to_i2c_buffer(const char* i2c_dev_node_path,int i2c_dev_address,int i2c_dev_reg_addr,const unsigned char *buffer,int len, bool force){
	pthread_mutex_lock(&i2c_mutex);
    int i2c_dev_node = 0;
	int ret_val = 0;
    __s32 read_value = 0;

	/* Open the device node for the I2C adapter of bus 4 */
	i2c_dev_node = open(i2c_dev_node_path, O_RDWR);
	if (i2c_dev_node < 0) {
		MSG_A("[rp_i2c] Unable to open device node ERROR: %d: %s\n", errno, strerror(errno));
		pthread_mutex_unlock(&i2c_mutex);
		return -1;
	}

	unsigned long flag = I2C_SLAVE;
	if (force) flag = I2C_SLAVE_FORCE;

	/* Set I2C_SLAVE for adapter 4 */
	ret_val = ioctl(i2c_dev_node,flag,i2c_dev_address);
	if (ret_val < 0) {
        MSG_A("[rp_i2c] Could not set I2C_SLAVE.\n");
        close(i2c_dev_node);
		pthread_mutex_unlock(&i2c_mutex);
		return -1;
	}

	/* Read byte from the register of the I2C_SLAVE device */
	read_value = i2c_smbus_read_byte_data(i2c_dev_node, i2c_dev_reg_addr);
	if (read_value < 0) {
        MSG_A("[rp_i2c] I2C Read operation failed.\n");
        close(i2c_dev_node);
		pthread_mutex_unlock(&i2c_mutex);
        return -1;
	}
	ret_val = i2c_smbus_write_block_data(i2c_dev_node,i2c_dev_reg_addr,len,buffer);

	if (ret_val < 0) {
        MSG_A("[rp_i2c] I2C Write Operation failed.\n");
        close(i2c_dev_node);
		pthread_mutex_unlock(&i2c_mutex);
        return -1;
	}
    close(i2c_dev_node);
	pthread_mutex_unlock(&i2c_mutex);                  
	return 0;
}

int read_to_i2c_buffer(const char* i2c_dev_node_path,int i2c_dev_address,int i2c_dev_reg_addr,unsigned char *buffer, bool force){
	pthread_mutex_lock(&i2c_mutex);
    int i2c_dev_node = 0;
	int ret_val = 0;
    __s32 read_value = 0;

	/* Open the device node for the I2C adapter of bus 4 */
	i2c_dev_node = open(i2c_dev_node_path, O_RDWR);
	if (i2c_dev_node < 0) {
		MSG_A("[rp_i2c] Unable to open device node ERROR: %d: %s\n", errno, strerror(errno));
		pthread_mutex_unlock(&i2c_mutex);
		return -1;
	}

	unsigned long flag = I2C_SLAVE;
	if (force) flag = I2C_SLAVE_FORCE;

	/* Set I2C_SLAVE for adapter 4 */
	ret_val = ioctl(i2c_dev_node,flag,i2c_dev_address);
	if (ret_val < 0) {
        MSG_A("[rp_i2c] Could not set I2C_SLAVE.\n");
        close(i2c_dev_node);
		pthread_mutex_unlock(&i2c_mutex);
		return -1;
	}

	/* Read byte from the register of the I2C_SLAVE device */
	read_value = i2c_smbus_read_i2c_block_data(i2c_dev_node, i2c_dev_reg_addr,6,buffer);
	if (read_value < 0) {
        MSG_A("[rp_i2c] I2C Read operation failed.\n");
        close(i2c_dev_node);
		pthread_mutex_unlock(&i2c_mutex);
        return -1;
	}
    close(i2c_dev_node);                 
	pthread_mutex_unlock(&i2c_mutex); 
	return read_value;
}