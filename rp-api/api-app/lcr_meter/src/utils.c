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
#include <math.h>

#include "utils.h"
#define I2C_SLAVE_FORCE 		   		0x0706
#define EXPANDER_ADDR            	   	0x20

int checkExtensionModuleConnection() {
    int status;
    int fd;
    char buf[2];

    // Open the device.
    fd = open("/dev/i2c-0", O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "Cannot open the I2C device: %d\n", fd);
        return -1;
    }

    // set slave address
    status = ioctl(fd, I2C_SLAVE_FORCE, EXPANDER_ADDR);
    if (status < 0) {
        close(fd);
        fprintf(stderr, "Unable to set the I2C address: %d\n", status);
        return -2;
    }

    int retVal = -90;
    retVal = read(fd, buf, 1);

    if(retVal < 0) {
        close(fd);
        fprintf(stderr, "RP checkExtensionModuleConnection(): I2C address resolving failed: %d\n", retVal);
        return -3;
    }

    close(fd);

    return 0;
}

// switching shunt resistors
int set_IIC_Shunt(int k) {

    int  dat;
    int  fd;
    int  status;
    char str [1+2*11];

    // parse input arguments
    dat = 1<<k;

    // Open the device.
    fd = open("/dev/i2c-0", O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "Cannot open the I2C device: %d\n", fd);
        return 1;
    }

    // set slave address
    status = ioctl(fd, I2C_SLAVE_FORCE, EXPANDER_ADDR);
    if (status < 0) {
        close(fd);
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

float trapezoidalApprox(double *data, float T, int size){
    double result = 0;

    for(int i = 0; i < size - 1; i++){
        result += data[i] + data[i+1];
    }
    result = ((T / 2.0) * result);
    return result;
}

void FirRectangleWindow(float *data, int data_size, int window_size){
    double K = 1.0 / window_size;
    for(int i = 0; i < data_size; ++i){
        double tmp = 0;
        for(int j = 0; (j < window_size) && ((i+j) < data_size); ++j){
            tmp += K * data[i+j];
        }
        data[i] = tmp;
    }
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
    float **data_out = malloc(2 * sizeof(float*));
    /* For each element in the first dimension,
         * allocate a size equal to the second dimension*/
    for(int i = 0; i < 2; i++){
        data_out[i] = malloc(second_dimenson * sizeof(float));
    }

    return data_out;
}

int delMultiDimensionVector(float** data)
{
    // Free memory
    for(int i = 0; i < 2; i++) {
        free(data[i]);
    }
    free(data);

    return RP_OK;
}

bool isSineTester(float **data, uint32_t size, double T)
{
    double ch0_rms[size];
    double ch0_avr[size];
    double ch1_rms[size];
    double ch1_avr[size];
    for(uint32_t i = 0; i < size; i++) {
        ch0_rms[i] = data[0][i] * data[0][i];
        ch0_avr[i] = ABS(data[0][i]);
        ch1_rms[i] = data[1][i] * data[1][i];
        ch1_avr[i] = ABS(data[1][i]);
    }
    double K0 = sqrtf(T * size * trapezoidalApprox(ch0_rms, T, size)) / trapezoidalApprox(ch0_avr, T, size);
    double K1 = sqrtf(T * size * trapezoidalApprox(ch1_rms, T, size)) / trapezoidalApprox(ch1_avr, T, size);

    bool isK0sine = ((K0 > 1.2) && (K0 < 1.25));
    bool isK1sine = ((K1 > 1.2) && (K1 < 1.25));

    return isK0sine && isK1sine;
}


void lcr_getDecimationValue(float frequency,
                            rp_acq_decimation_t *api_dec,
                            int *dec_val,
                            uint32_t adc_rate){

    if (adc_rate <= 125e6){
        switch((int)frequency) {
            case 1000000:
                *api_dec = RP_DEC_1;
                *dec_val = 1;
                break;
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
            case 10:
                *api_dec = RP_DEC_4096;
                *dec_val = 4096;
                break;
            default:
                *api_dec = RP_DEC_1;
                *dec_val = 1;
                break;
        }
    }

    if (adc_rate == 250e6){
        switch((int)frequency) {
            case 1000000:
                *api_dec = RP_DEC_1;
                *dec_val = 1;
                break;
            case 100000:
                *api_dec = RP_DEC_2;
                *dec_val = 2;
                break;
            case 10000:
                *api_dec = RP_DEC_16;
                *dec_val = 16;
                break;
            case 1000:
                *api_dec = RP_DEC_128;
                *dec_val = 128;
                break;
            case 100:
                *api_dec = RP_DEC_2048;
                *dec_val = 2048;
                break;
            case 10:
                *api_dec = RP_DEC_8192;
                *dec_val = 8192;
                break;
            default:
                *api_dec = RP_DEC_1;
                *dec_val = 1;
                break;
            }
    }

    *api_dec = RP_DEC_1;
    *dec_val = 1;
    fprintf(stderr,"[Fatal error:lcr_getDecimationValue] undefined adc rate %d\n",adc_rate);
}



void Fir(double *data, int data_size){
    double core[5] = {1,2,4,2,1};
    if (data_size < 10) return;
    double core_w = 10;
    double core_w_i_1 = 9;
    double core_w_i_0 = 7;
    double new_buf[data_size];
    for(int i = 0; i < data_size; ++i){
        double cur_core = core_w;
        new_buf[i] = 0;
        if (i == 1 || i == data_size - 2) cur_core = core_w_i_1;
        if (i == 0 || i == data_size - 1) cur_core = core_w_i_0;
        for( int j = -2; j <= 2; j++){
            int index = i + j;
            if (index < 0 || index >= data_size) continue;
            new_buf[i] += data[index] * core[j + 2];
        }
        new_buf[i] /= cur_core;
    }
    memcpy(new_buf,data,data_size * sizeof(double));
}

void Normalize(double *data, int data_size){
    double mean = 0;
    double min,max;
    min = data[0];
    max = data[0];
    for(int i = 1; i < data_size; ++i){
        if (data[i] > max) max = data[i];
        if (data[i] < min) min = data[i];
    }
    double norm = (max - min);
    mean = (max + min)/2.0;
    for(int i = 0; i < data_size; ++i){
        data[i] = (data[i] - mean) * (2.0 / norm);
    }
}
