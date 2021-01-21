#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include "rp-i2c.h"
#include "rp-i2c-mcp47x6.h"
#include "rp-i2c-max7311.h"
#include "rp-spi.h"
#include "rp-gpio-power.h"

#define MSG(X,Y) printf("%s status %d\n",X,Y);

int main()
{
    int status = 0;
    char value = 0;
    
    status |= rp_spi_fpga::rp_read_from_spi_fpga("/dev/mem",0x40000000,0x50,0x01,value); // Get CHIP ID
    printf("Check SPI bus with AD9613 chip %s\n",(value == 0x83)?"[OK]":"[ERROR]");
    status |= (value != 0x83);

    RP_MCP47X6::mcp47x6 chip(RP_MCP47X6::mcp47x6_model::MCP4716,"/dev/i2c-0");
    chip.readConfig();
    chip.setOutputLevel(10);
    chip.writeConfig();
    chip.readConfig();
    printf("Check I2C bus with MCP4716 chip %s\n",(chip.getOutputLevel() == 10)?"[OK]":"[ERROR]");
    status |= (chip.getOutputLevel() == 10)?0:-1;

    rp_max7311::rp_initController();
    status |= max7311::setPIN(PIN_0,true);
    int state = max7311::getPIN(PIN_0); 
    if (state != true) {
        printf("Check I2C bus with MAX7311 chip [ERROR] (TEST 1)\n");
        status |= -1;
    }
    else{
        printf("Check I2C bus with MAX7311 chip [OK] (TEST 1)\n");
    }

    status |= max7311::setPIN(PIN_0,false);
    state = max7311::getPIN(PIN_0); 
    if (state != false) {
        printf("Check I2C bus with MAX7311 chip [ERROR] (TEST 2)\n");
        status |= -1;
    }
    else{
        printf("Check I2C bus with MAX7311 chip [OK] (TEST 2)\n");
    }

    status |= max7311::setPIN(PIN_1,true);
    state = max7311::getPIN(PIN_1); 
    if (state != true) {
        printf("Check I2C bus with MAX7311 chip [ERROR] (TEST 3)\n");
        status |= -1;
    }
    else{
        printf("Check I2C bus with MAX7311 chip [OK] (TEST 3)\n");
    }

    status |= max7311::setPIN(PIN_1,false);
    state = max7311::getPIN(PIN_1); 
    if (state != false) {
        printf("Check I2C bus with MAX7311 chip [ERROR] (TEST 4)\n");
        status |= -1;
    }
    else{
        printf("Check I2C bus with MAX7311 chip [OK] (TEST 4)\n");
    }

    return status;
}
