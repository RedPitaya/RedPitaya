#!/bin/bash

if [[ ! -f /opt/redpitaya/bin/production_testing_script.sh ]]
then
    /opt/redpitaya/sbin/mkoverlay.sh v0.94

    /opt/redpitaya/sbin/getsysinfo.sh

    # Turns on the power to the ADC and DAC

    MODEL=$(/opt/redpitaya/bin/monitor -f)

    if [ "$MODEL" = "z20_250_1_0" ]; then
    /opt/redpitaya/bin/rp_power_on -P
    /opt/redpitaya/bin/rp_power_on -C
    fi

    if [ "$MODEL" = "z20_250" ]; then
    /opt/redpitaya/bin/rp_power_on -P
    /opt/redpitaya/bin/rp_power_on -C
    fi

fi

# Here you can specify commands for autorun at system startup
