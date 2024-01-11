/* @brief This is a simple application for testing I2C communication on a RedPitaya
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
#include "rp_hw-calib.h"


int main(int argc, char *argv[]){

    // This example shows how to work with EEPROM via I2C

    int res = rp_I2C_InitDevice("/dev/i2c-0",0x50); // Init i2c api.
    printf("Init result: %d\n",res);

    res = rp_I2C_setForceMode(true); // Set force mode.
    printf("Set force mode: %d\n",res);

    uint8_t wb[2] = {0,0};
    res = rp_I2C_IOCTL_WriteBuffer(wb,2); // Write position for reading.
    printf("Write 2 bytes: %d\n",res);

    usleep(100000);

    uint8_t rb[1];

    res = rp_I2C_IOCTL_ReadBuffer(rb,1); // Read 1 byte from I2C
    printf("Read 1 byte: %d\n",res);
    uint8_t df = rb[0];
    printf("Data format %d\n",df);
    res = rp_I2C_IOCTL_WriteBuffer(wb,2); // Write position for reading.
    printf("Write 2 bytes: %d\n",res);
    usleep(100000);

    rp_calib_params_t calib;

    if (df == 5){
        rp_calib_params_universal_t data;
        uint16_t size = sizeof(data);
        res = rp_I2C_IOCTL_ReadBuffer((uint8_t*)&data,size);
        printf("Read %d byte: %d\n",size,res);
        res = rp_CalibConvertEEPROM((uint8_t*)&data,size,&calib);
        printf("Convert calib: %d\n",res);
    }else{
        rp_eepromWpData_t data;
        uint16_t size = sizeof(data);
        res = rp_I2C_IOCTL_ReadBuffer((uint8_t*)&data,size);
        printf("Read %d byte: %d\n",size,res);
        res = rp_CalibConvertEEPROM((uint8_t*)&data,size,&calib);
        printf("Convert calib: %d\n",res);
    }
    rp_CalibPrint(&calib);
    return 0;
}