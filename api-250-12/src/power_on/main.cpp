#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include "rp-spi.h"
#include "rp-gpio-power.h"
#include "rp-i2c-max7311.h"


int main()
{
    rp_gpio_power::rp_set_power_mode(ADC_POWER,POWER_ON);
    rp_gpio_power::rp_set_power_mode(DAC_POWER,POWER_ON);
    rp_spi_fpga::rp_spi_load_via_fpga("/opt/redpitaya/lib/configs/AD9613BCPZ-250.xml");
    rp_spi_fpga::rp_spi_load_via_fpga("/opt/redpitaya/lib/configs/AD9746BCPZ-250.xml");
    rp_max7311::rp_initController();
    return 0;
}
