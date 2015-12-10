/**
 * $Id: $
 *
 * @brief Red Pitaya Applications library common module interface
 *
 * @Author Luka Golinar, RedPitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syslog.h>

#include "utils.h"
#define I2C_SLAVE_FORCE 		   		0x0706
#define EXPANDER_ADDR            	   	0x20

// switching shunt resistors
int set_IIC_Shunt(int k) {

    int  dat;
    int  fd; 
    int  status;
    char str [1+2*11];

    // parse input arguments
    dat = ~(1<<k);

    // Open the device.
    fd = open("/dev/i2c-0", O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "Cannot open the I2C device\n");
        return 1;
    }

    // set slave address
    status = ioctl(fd, I2C_SLAVE_FORCE, EXPANDER_ADDR);
    if (status < 0) {
        fprintf(stderr, "Unable to set the I2C address\n");
        return -1;
    }

    // Write to expander
    str [0] = 0; // set address to 0
    str [1+0x00] = 0x00; // IODIRA - set all to output
    str [1+0x01] = 0x00; // IODIRB - set all to output
    str [1+0x02] = 0x00; // IPOLA
    str [1+0x03] = 0x00; // IPOLB
    str [1+0x04] = 0x00; // GPINTENA
    str [1+0x05] = 0x00; // GPINTENB
    str [1+0x06] = 0x00; // DEFVALA
    str [1+0x07] = 0x00; // DEFVALB
    str [1+0x08] = 0x00; // INTCONA
    str [1+0x09] = 0x00; // INTCONB
    str [1+0x0A] = 0x00; // IOCON
    str [1+0x0B] = 0x00; // IOCON
    str [1+0x0C] = 0x00; // GPPUA
    str [1+0x0D] = 0x00; // GPPUB
    str [1+0x0E] = 0x00; // INTFA
    str [1+0x0F] = 0x00; // INTFB
    str [1+0x10] = 0x00; // INTCAPA
    str [1+0x11] = 0x00; // INTCAPB
    str [1+0x12] = (dat >> 0) & 0xff; // GPIOA
    str [1+0x13] = (dat >> 8) & 0xff; // GPIOB
    str [1+0x14] = (dat >> 0) & 0xff; // OLATA
    str [1+0x15] = (dat >> 8) & 0xff; // OLATB
    status = write(fd, str, 1+2*11);

    if (!status) fprintf(stderr, "Error I2C write\n");
    
    close(fd);
    return 0;
}

float vectorMax(float *data, int size){
	
	float max = -1e6;
	
	for(int i = 0; i < size; i++){
		(data[i] > max) ? (max = data[i]) : (max = max);
	}

	return max;
}

float trapezoidalApprox(float *data, float T, int size){
	float result = 0;

	for(int i = 0; i < size - 1; i++){
		result += data[i] + data[i+1];
	}
	result = ((T / 2.0) * result);
	return result;
}

float vectorMean(float *data, int size){
	float result = 0;
	for(int i = 0; i < size; i++){
		result += data[i];
	}
	result = result / size;
	return result;
}

float **multiDimensionVector(int second_dimenson){

	/* Allocate first dimension */
	float **data_out = malloc(2 * sizeof(float));
	/* For each element in the first dimension, 
	 * allocate a size equal to the second dimension*/
	for(int i = 0; i < 2; i++){
		data_out[i] = malloc(second_dimenson * sizeof(float));
	}

	return data_out;
}

int lcr_checkRShunt(float z_ampl, double r_shunt, int *new_shunt){

	if((z_ampl >= (3.0 * r_shunt)) || (z_ampl <= ((1.0 / 3.0) * r_shunt))){
		
		if(z_ampl <= 50) 								*new_shunt = 0;
		else if(z_ampl <= 100 && z_ampl > 50) 			*new_shunt = 1;
		else if(z_ampl <= 500 && z_ampl > 100) 			*new_shunt = 2;
		else if(z_ampl <= 1000 && z_ampl > 500) 		*new_shunt = 3;
		else if(z_ampl <= 5000 && z_ampl > 1000) 		*new_shunt = 4;
		else if(z_ampl <= 10000 && z_ampl > 5000) 		*new_shunt = 5;
		else if(z_ampl <= 50000 && z_ampl > 10000) 		*new_shunt = 6;
		else if(z_ampl <= 100000 && z_ampl > 50000) 	*new_shunt = 7;
		else if(z_ampl <= 500000 && z_ampl > 100000) 	*new_shunt = 8;
		else if(z_ampl > 500000) 						*new_shunt = 9;
	}

	return RP_OK;
}


void lcr_getDecimationValue(float frequency,
						rp_acq_decimation_t *api_dec,
						int *dec_val){

		switch((int)frequency) {
			case 100000:
				*api_dec = RP_DEC_1;
				*dec_val = 1;
				break;
			case 10000:
				*api_dec = RP_DEC_8;
				*dec_val = 8;
				break;
			case 1000:
				*api_dec = RP_DEC_64;
				*dec_val = 64;
				break;
			case 100:
				*api_dec = RP_DEC_1024;
				*dec_val = 1024;
				break;
		}
}