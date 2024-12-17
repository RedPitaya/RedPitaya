#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <algorithm>
#include "rp-spi.h"
#include "rp-gpio-power.h"
#include "rp-i2c-max7311.h"

char* getCmdOption(char ** begin, char ** end, const std::string & option,int index = 0)
{
    //    Example
    //    char * filename = getCmdOption(argv, argv + argc, "-f");

    char ** itr = std::find(begin, end, option);
    while(itr != end && ++itr != end){
        if (index <= 0)
            return *itr;
        index--;
    };
    return 0;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option)
{
    //    Example
    //    if(cmdOptionExists(argv, argv+argc, "-h"))
    //    {  // Do stuff    }
    return std::find(begin, end, option) != end;
}

bool CheckMissing(const char* val,const char* Message)
{
    if (val == NULL) {
        std::cout << "Missing parameters: " << Message << std::endl;
        return true;
    }
    return false;
}

void UsingArgs(){
    printf("Usage with file: [-P]|[-C]|[-C1]|[-C2]\n");
    printf("\t-P Power on ADC and DAC. Initialization MAX7311.\n");
    printf("\t-С Load configuration of ADC and DAC. \n");
    printf("\t-С1 Load configuration of ADC and DAC for 125-14 LL v1.1. \n");
    printf("\t-С2 Load configuration of ADC and DAC for 65-16 LL v1.1. \n");
    printf("\tDo not use flags together");

    exit(-1);
}


int main(int argc, char* argv[])
{
    bool mode1 = cmdOptionExists(argv, argv + argc, "-P");
    bool mode2 = cmdOptionExists(argv, argv + argc, "-C");
    bool mode3 = cmdOptionExists(argv, argv + argc, "-C1");
    bool mode4 = cmdOptionExists(argv, argv + argc, "-C2");

    uint8_t count = 0;
    if (mode1) count++;
    if (mode2) count++;
    if (mode3) count++;
    if (mode4) count++;
    if (count > 1) {
        UsingArgs();
    }

    if (mode1) {
        rp_gpio_power::rp_set_power_mode(ADC_POWER,POWER_ON);
        rp_gpio_power::rp_set_power_mode(DAC_POWER,POWER_ON);
        rp_max7311::rp_initController();
    }

    if (mode2) {
        rp_spi_fpga::rp_spi_load_via_fpga("/opt/redpitaya/lib/configs/AD9613BCPZ-250.xml");
        rp_spi_fpga::rp_spi_load_via_fpga("/opt/redpitaya/lib/configs/AD9746BCPZ-250.xml");
    }
    if (mode3) {
        rp_spi_fpga::rp_spi_load_via_fpga("/opt/redpitaya/lib/configs/ADC3664-125.xml");
        rp_spi_fpga::rp_spi_load_via_fpga("/opt/redpitaya/lib/configs/DAC2904-125.xml");
    }
    if (mode4) {
        rp_spi_fpga::rp_spi_load_via_fpga("/opt/redpitaya/lib/configs/ADC3663-65.xml");
        rp_spi_fpga::rp_spi_load_via_fpga("/opt/redpitaya/lib/configs/DAC2904-125.xml");
    }

    return 0;
}
