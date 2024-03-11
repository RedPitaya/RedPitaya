#include <stdint.h>
#include <stdio.h>

#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <i2c/smbus.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "i2c-helper.h"


pthread_mutex_t i2c_mutex = PTHREAD_MUTEX_INITIALIZER;


int openDevice(const char* i2c_dev_node_path,uint8_t i2c_dev_address, bool force,int *i2c_dev_node){
	int ret_val = 0;
	/* Open the device node for the I2C adapter of bus 4 */
	*i2c_dev_node = open(i2c_dev_node_path, O_RDWR);
	if (*i2c_dev_node < 0) {
		fprintf(stderr,"[rp_i2c] Unable to open device node ERROR: %d: %s\n", errno, strerror(errno));
		return RP_HW_EIIIC;
	}

	unsigned long flag = I2C_SLAVE;
	if (force) flag = I2C_SLAVE_FORCE;

	/* Set I2C_SLAVE for adapter 4 */
	ret_val = ioctl(*i2c_dev_node, flag ,i2c_dev_address);
	if (ret_val < 0) {
        fprintf(stderr,"[rp_i2c] Could not set I2C_SLAVE.\n");
        close(*i2c_dev_node);
		return RP_HW_ESIIC;
	}

	return RP_HW_OK;
}


int i2c_SBMUS_write_byte(const char* i2c_dev_node_path,uint8_t i2c_dev_address,uint8_t i2c_dev_reg_addr, uint8_t i2c_val_to_write, bool force){
	pthread_mutex_lock(&i2c_mutex);
	int i2c_dev_node = 0;
	int ret_val = 0;
    // __s32 read_value = 0;

	ret_val = openDevice(i2c_dev_node_path,i2c_dev_address,force,&i2c_dev_node);

	if (ret_val != RP_HW_OK){
		pthread_mutex_unlock(&i2c_mutex);
		return ret_val;
	}

	ret_val = i2c_smbus_write_byte_data(i2c_dev_node,
					i2c_dev_reg_addr,
					i2c_val_to_write);

	if (ret_val < 0) {
        fprintf(stderr,"[rp_i2c] I2C Write Operation failed.\n");
        close(i2c_dev_node);
		pthread_mutex_unlock(&i2c_mutex);
        return RP_HW_EWIIC;
	}
	close(i2c_dev_node);
	pthread_mutex_unlock(&i2c_mutex);
	return RP_HW_OK;
}

int i2c_SBMUS_write_word(const char* i2c_dev_node_path,uint8_t i2c_dev_address,uint8_t i2c_dev_reg_addr, uint16_t i2c_val_to_write, bool force){
	pthread_mutex_lock(&i2c_mutex);
    int i2c_dev_node = 0;
	int ret_val = 0;
    // __s32 read_value = 0;

	ret_val = openDevice(i2c_dev_node_path,i2c_dev_address,force,&i2c_dev_node);

	if (ret_val != RP_HW_OK){
		pthread_mutex_unlock(&i2c_mutex);
		return ret_val;
	}

	ret_val = i2c_smbus_write_word_data(i2c_dev_node,
					i2c_dev_reg_addr,
					i2c_val_to_write);
	if (ret_val < 0) {
        fprintf(stderr,"[rp_i2c] I2C Write Operation failed.\n");
        close(i2c_dev_node);
		pthread_mutex_unlock(&i2c_mutex);
        return RP_HW_EWIIC;
	}
	close(i2c_dev_node);
	pthread_mutex_unlock(&i2c_mutex);
	return RP_HW_OK;
}


int i2c_SBMUS_write_command(const char* i2c_dev_node_path,uint8_t i2c_dev_address,uint8_t i2c_dev_command, bool force){
	pthread_mutex_lock(&i2c_mutex);
    int i2c_dev_node = 0;
	int ret_val = 0;
    //__s32 read_value = 0;

	ret_val = openDevice(i2c_dev_node_path,i2c_dev_address,force,&i2c_dev_node);

	if (ret_val != RP_HW_OK){
		pthread_mutex_unlock(&i2c_mutex);
		return ret_val;
	}

	ret_val = i2c_smbus_write_byte(i2c_dev_node,i2c_dev_command);

	if (ret_val < 0) {
        fprintf(stderr,"[rp_i2c] I2C Write Operation failed - %d.\n",errno);
        close(i2c_dev_node);
		pthread_mutex_unlock(&i2c_mutex);
        return RP_HW_EWIIC;
	}
    close(i2c_dev_node);
	pthread_mutex_unlock(&i2c_mutex);
	return RP_HW_OK;
}

int i2c_SBMUS_write_buffer(const char* i2c_dev_node_path,uint8_t i2c_dev_address,uint8_t i2c_dev_reg_addr,const uint8_t *buffer,int len, bool force){
 	if (!buffer) {
		return RP_HW_EBIIC;
	}
	pthread_mutex_lock(&i2c_mutex);
    int i2c_dev_node = 0;
	int ret_val = 0;

	ret_val = openDevice(i2c_dev_node_path,i2c_dev_address,force,&i2c_dev_node);

	if (ret_val != RP_HW_OK){
		pthread_mutex_unlock(&i2c_mutex);
		return ret_val;
	}

	ret_val = i2c_smbus_write_block_data(i2c_dev_node,i2c_dev_reg_addr,len,buffer);

	if (ret_val < 0) {
        fprintf(stderr,"[rp_i2c] I2C Write Operation failed - %d.\n",errno);
        close(i2c_dev_node);
		pthread_mutex_unlock(&i2c_mutex);
        return RP_HW_EWIIC;
	}
    close(i2c_dev_node);
	pthread_mutex_unlock(&i2c_mutex);
	return RP_HW_OK;
}


int i2c_SBMUS_read_byte(const char* i2c_dev_node_path,uint8_t i2c_dev_address,uint8_t i2c_dev_reg_addr, uint8_t *value, bool force){
	pthread_mutex_lock(&i2c_mutex);
    int i2c_dev_node = 0;
	int ret_val = 0;
    __s32 read_value = 0;

	ret_val = openDevice(i2c_dev_node_path,i2c_dev_address,force,&i2c_dev_node);

	if (ret_val != RP_HW_OK){
		pthread_mutex_unlock(&i2c_mutex);
		return ret_val;
	}

	/* Read byte from the register of the I2C_SLAVE device */
	read_value = i2c_smbus_read_byte_data(i2c_dev_node, i2c_dev_reg_addr);
	if (read_value < 0) {
        fprintf(stderr,"[rp_i2c] I2C Read operation failed.\n");
        close(i2c_dev_node);
		pthread_mutex_unlock(&i2c_mutex);
        return RP_HW_ERIIC;
	}
	close(i2c_dev_node);
    *value = (uint8_t)read_value;
	pthread_mutex_unlock(&i2c_mutex);
	return RP_HW_OK;
}

int i2c_SBMUS_read_word(const char* i2c_dev_node_path,uint8_t i2c_dev_address,uint8_t i2c_dev_reg_addr, uint16_t *value, bool force){
	pthread_mutex_lock(&i2c_mutex);
    int i2c_dev_node = 0;
	int ret_val = 0;
    __s32 read_value = 0;

	ret_val = openDevice(i2c_dev_node_path,i2c_dev_address,force,&i2c_dev_node);

	if (ret_val != RP_HW_OK){
		pthread_mutex_unlock(&i2c_mutex);
		return ret_val;
	}

	/* Read byte from the register of the I2C_SLAVE device */
	read_value = i2c_smbus_read_word_data(i2c_dev_node, i2c_dev_reg_addr);
	if (read_value < 0) {
        fprintf(stderr,"[rp_i2c] I2C Read operation failed.\n");
        close(i2c_dev_node);
		pthread_mutex_unlock(&i2c_mutex);
        return RP_HW_ERIIC;
	}
	close(i2c_dev_node);
    *value = (uint16_t)read_value;
	pthread_mutex_unlock(&i2c_mutex);
	return RP_HW_OK;
}


int i2c_SBMUS_read_command(const char* i2c_dev_node_path,uint8_t i2c_dev_address, uint8_t *value, bool force){
	pthread_mutex_lock(&i2c_mutex);
    int i2c_dev_node = 0;
	int ret_val = 0;
    __s32 read_value = 0;

	ret_val = openDevice(i2c_dev_node_path,i2c_dev_address,force,&i2c_dev_node);

	if (ret_val != RP_HW_OK){
		pthread_mutex_unlock(&i2c_mutex);
		return ret_val;
	}

	/* Read byte from the register of the I2C_SLAVE device */
	read_value = i2c_smbus_read_byte(i2c_dev_node);
	if (read_value < 0) {
        fprintf(stderr,"[rp_i2c] I2C Read operation failed.\n");
        close(i2c_dev_node);
		pthread_mutex_unlock(&i2c_mutex);
        return RP_HW_ERIIC;
	}
	close(i2c_dev_node);
    *value = (char)read_value;
	pthread_mutex_unlock(&i2c_mutex);
	return RP_HW_OK;
}

int i2c_SBMUS_read_buffer(const char* i2c_dev_node_path,uint8_t i2c_dev_address,uint8_t i2c_dev_reg_addr,uint8_t *buffer, int *len, bool force){
	if (!buffer) {
		return RP_HW_EBIIC;
	}

	pthread_mutex_lock(&i2c_mutex);
    int i2c_dev_node = 0;
	int ret_val = 0;
    __s32 read_value = 0;

	ret_val = openDevice(i2c_dev_node_path,i2c_dev_address,force,&i2c_dev_node);

	if (ret_val != RP_HW_OK){
		pthread_mutex_unlock(&i2c_mutex);
		return ret_val;
	}


	/* Read byte from the register of the I2C_SLAVE device */
	read_value = i2c_smbus_read_i2c_block_data(i2c_dev_node, i2c_dev_reg_addr,*len,buffer);
	if (read_value < 0) {
        fprintf(stderr,"[rp_i2c] I2C Read operation failed - %d.\n",errno);
        close(i2c_dev_node);
		pthread_mutex_unlock(&i2c_mutex);
        return RP_HW_ERIIC;
	}

	close(i2c_dev_node);
	pthread_mutex_unlock(&i2c_mutex);
	*len = read_value;
	return RP_HW_OK;
}

int i2c_IOCTL_read_buffer(const char* i2c_dev_node_path,uint8_t i2c_dev_address,uint8_t *buffer, int len, bool force){
	if (!buffer) {
		return RP_HW_EBIIC;
	}

	pthread_mutex_lock(&i2c_mutex);

	int i2c_dev_node = 0;
	int ret_val = 0;

	ret_val = openDevice(i2c_dev_node_path,i2c_dev_address,force,&i2c_dev_node);

	if (ret_val != RP_HW_OK){
		pthread_mutex_unlock(&i2c_mutex);
		return ret_val;
	}

	struct i2c_rdwr_ioctl_data data;
	struct i2c_msg message;
	/*
		* .addr - address on bus
		* .flags - (0 - w, 1 - r)
		* .len - lenght of read/write
		* .buf - ptr to buffer
		*/
	message.addr = i2c_dev_address;
	message.flags = 1;
	message.len = len;
	message.buf = (unsigned char*)buffer;

	data.msgs = &message;
	data.nmsgs = 1;

	if (ioctl(i2c_dev_node, I2C_RDWR, &data) < 0){
		fprintf(stderr,"[rp_i2c] I2C Read operation failed - %d.\n",errno);
		pthread_mutex_unlock(&i2c_mutex);
		return RP_HW_ERIIC;
	}
	pthread_mutex_unlock(&i2c_mutex);
	return RP_HW_OK;
}

int i2c_IOCTL_write_buffer(const char* i2c_dev_node_path,uint8_t i2c_dev_address,uint8_t *buffer, int len, bool force){
	if (!buffer) {
		return RP_HW_EBIIC;
	}

	pthread_mutex_lock(&i2c_mutex);

	int i2c_dev_node = 0;
	int ret_val = 0;

	ret_val = openDevice(i2c_dev_node_path,i2c_dev_address,force,&i2c_dev_node);

	if (ret_val != RP_HW_OK){
		pthread_mutex_unlock(&i2c_mutex);
		return ret_val;
	}

	struct i2c_rdwr_ioctl_data data;
	struct i2c_msg message;
	/*
		* .addr - address on bus
		* .flags - (0 - w, 1 - r)
		* .len - lenght of read/write
		* .buf - ptr to buffer
		*/
	message.addr = i2c_dev_address;
	message.flags = 0;
	message.len = len;
	message.buf = (unsigned char*)buffer;

	data.msgs = &message;
	data.nmsgs = 1;

	if (ioctl(i2c_dev_node, I2C_RDWR, &data) < 0){
		fprintf(stderr,"[rp_i2c] I2C Write operation failed - %d.\n",errno);
		pthread_mutex_unlock(&i2c_mutex);
		return RP_HW_EWIIC;
	}
	pthread_mutex_unlock(&i2c_mutex);
	return RP_HW_OK;
}
