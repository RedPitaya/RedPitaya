#!/bin/bash

E3=$(profiles -v e3)
if [[ "$E3" == "1" ]]; then
    E3_DEVICE=$(e3_led_controller -d)
    if [[ "$E3_DEVICE" == "1" ]]; then
        e3_i2c_controller -wd PWR_DWN
    else
        poweroff
    fi
else
    poweroff
fi