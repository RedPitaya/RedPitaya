/* @brief This is a simple application for testing I2C communication on a RedPitaya
* @Author Luka Golinar <luka.golinar@redpitaya.com>
*
* (c) Red Pitaya  http://www.redpitaya.com
*
* This part of code is written in C programming language.
* Please visit http://en.wikipedia.org/wiki/C_(programming_language)
* for more details on the language used herein.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "rp_hw.h"


int main(int argc, char *argv[]){

 
    int res = rp_I2C_InitDevice("/dev/i2c-0",0x50); // Init i2c api.
    printf("Init result: %d\n",res);
    
    res = rp_I2C_setForceMode(true); // Set force mode.
    printf("Set force mode: %d\n",res);
    
    uint8_t wb[2] = {0,0};
    res = rp_I2C_IOCTL_WriteBuffer(wb,2); // Write position for reading.
    printf("Write 2 bytes: %d\n",res);

    usleep(100000);

    int32_t rb[12];
    res = rp_I2C_IOCTL_ReadBuffer((uint8_t*)rb,32); // Read 32 bytes from I2C
    printf("Read 32 bytes: %d\n",res);
    
    res = rp_I2C_IOCTL_ReadBuffer((uint8_t*)(rb + 8),16);  // Read 16 bytes from I2C
    printf("Read 16 bytes: %d\n",res); 

    printf("ADC Ch1 High %d\n",rb[2]);
    printf("ADC Ch2 High %d\n",rb[3]);
    printf("ADC Ch1 Low %d\n",rb[4]);
    printf("ADC Ch2 Low %d\n",rb[5]);
    printf("ADC Ch1 Low offset %d\n",rb[6]);
    printf("ADC Ch2 Low offset %d\n",rb[7]);
    printf("DAC Ch1 %d\n",rb[8]);
    printf("DAC Ch2 %d\n",rb[9]);
    printf("DAC Ch1 offset %d\n",rb[10]);
    printf("DAC Ch2 offset %d\n",rb[11]);

    return 0;
}