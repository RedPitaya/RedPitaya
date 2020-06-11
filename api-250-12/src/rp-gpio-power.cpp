#include "rp-gpio-power.h"
#include "gpio/gpio.h"
#include <iostream>

int rp_gpio_power::rp_set_power_mode(int module,int state){

    if (gpio_export(module)==-1)
        return -1;
    
    if (gpio_pin_direction(module,RP_GPIO_OUT)==-1){
        gpio_unexport(module);
        return -1;
    }

    if (gpio_write(module,state)==-1) {
        gpio_unexport(module);
        return -1;
    }

    if (gpio_unexport(module)==-1)
        return -1;

    return 0;
}

int rp_gpio_power::rp_get_power_mode(int module){

    if (gpio_export(module)==-1)
        return -1;
    
    if (gpio_pin_direction(module,RP_GPIO_OUT)==-1){
        gpio_unexport(module);
        return -1;
    }

    int ret = gpio_read(module);
    
    if (gpio_unexport(module)==-1)
        return -1;

    return ret;
}