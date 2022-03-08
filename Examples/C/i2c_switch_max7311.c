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


int main(int argc, char *argv[]){


    int res = rp_I2C_InitDevice("/dev/i2c-0",0x21); // Init i2c api.
    printf("Init result: %d\n",res);

    res = rp_I2C_setForceMode(true); // Set force mode.
    printf("Set force mode: %d\n",res);
    
    printf("Turn on AC/DC ch1 & ch2\n");

    uint16_t value = 0x0055;

    res = rp_I2C_SMBUS_WriteWord(0x02,value);
    printf("Write 2 bytes: %d\n",res);

    usleep(1000000);

    value = value & ~ 0x000F;

    res = rp_I2C_SMBUS_WriteWord(0x02,value);
    printf("Write 2 bytes: %d\n",res);

    usleep(3000000);

    printf("Turn off AC/DC ch1 & ch2\n");

    value = 0x00AA;

    res = rp_I2C_SMBUS_WriteWord(0x02,value);
    printf("Write 2 bytes: %d\n",res);

    usleep(1000000);

    value = value & ~ 0x000F;

    res = rp_I2C_SMBUS_WriteWord(0x02,value);
    printf("Write 2 bytes: %d\n",res);

    uint16_t read_value = 0;

    res = rp_I2C_SMBUS_ReadWord(0x02,&read_value);
    printf("Read 2 bytes: 0x%x (res: %d)\n",read_value, res);

    return 0;
}