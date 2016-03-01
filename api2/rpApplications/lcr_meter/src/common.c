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

#include "common.h"

#define I2C_SLAVE_FORCE		0x0706
#define EXPANDER_ADDR		0x20

int set_IIC_Shunt(uint32_t shunt){

	char str[1+2*11];
	int iic_shunt;

	int fd = open("/dev/i2c-0", O_RDWR);

	if(fd < 0){
		printf("Error writing to IIC.\n");
		return RP_EOOR;
	}

	int status = ioctl(fd, I2C_SLAVE_FORCE, EXPANDER_ADDR);
	if(status < 0){
		printf("Unable to set IIC address.\n");
		return RP_EOOR;
	}

	switch(shunt){
		case 30:
			iic_shunt = 0xFFFE;
			break;
		case 75:
			iic_shunt = 0xFFFD;
			break;
		case 300:
			iic_shunt = 0xFFFB;
			break;
		case 750:
			iic_shunt = 0xFFF7;
			break;
		case 3000:
			iic_shunt = 0xFFEF;
			break;
		case 7500:
			iic_shunt = 0xFFDF;
			break;
		case 30000:
			iic_shunt = 0xFFBF;
			break;
		case 80000:
			iic_shunt = 0xFF7F;
			break;
		case 430000:
			iic_shunt = 0xFEFF;
			break;
		case 3000000:
			iic_shunt = 0xFDFF;
			break;
		case -1:
			iic_shunt = 0xFFFF;
		default:
			iic_shunt = 0x0;
			break;
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
	str [1+0x12] = (iic_shunt >> 0) & 0xff; // GPIOA
    str [1+0x13] = (iic_shunt >> 8) & 0xff; // GPIOB
    str [1+0x14] = (iic_shunt >> 0) & 0xff; // OLATA
    str [1+0x15] = (iic_shunt >> 8) & 0xff; // OLATB

    status = write(fd, str, (1 + 2*11));
    if(status < 0){
    	printf("Error writing resistor data.\n");
    	return RP_EOOR;
    }

	return RP_OK;
}

float vectorMax(float *data, int size){
	
	float max = -1e6;
	
	for(int i = 0; i < size; i++){
		(data[i] > max) ? (max = data[i]) : (max = max);
	}

	return max;
}

#if 0
float vectorApprox(float *data, int size, float approx_val, bool min){

	/* Get vector max value */
	float closet_val = vectorMax(data, size);

	for(int i = 0; i < size; ++i){
		if(min){
			if(abs(data[i] - approx_val < closet_val) && (data[i] < approx_val)){
				closet_val = data[i];
			} 
		}else if(abs(data[i] - approx_val < closet_val) && (data[i] > approx_val)){
			closet_val = data[i];
		}	
	}

	return closet_val;
}
#endif

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

/* TODO: SWD, function placement open discussion. Directly in 
   FS/MS or in common as a helper tool */
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

#if 0
/* This functions recursivly creates a directory path 
 * Example: /tmp/imp_data/calibration 
 * One we come find the second / symbol, we check if the directory
 * exists. If it does, the function continues. If not, it creates the 
 * directory and calls itself with the same char *array as param. */
int createPath(char *path){

	/* Get array size */
	int idx = 0;
	int size = strlen(&path[0]);
	char *strip_path;
	const char seek_char = '/';

	while(idx < size){

		/* Get seek index */
		idx = strcspn(&path[idx], &seek_char);

		strip_path = malloc(idx++ * sizeof(char));
		memcpy(strip_path, path, idx++);

		/* Create single directory chunk */
		createSingle(&strip_path[0]);
		free(strip_path);
		
		idx++;
	}

	return RP_OK;
}

int createSingle(char *path){

	/* Stat structure */
	struct stat st = {0};

	/* Directory doesn't exist yet */
	if(stat(&path[0], &st) == -1){
		mkdir(&path[0], S_IWUSR);	
	}
	
	return RP_OK;
}

#endif