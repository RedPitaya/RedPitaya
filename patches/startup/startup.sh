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

    PROD_MODE=$(cat /root/production_start_mode.conf 2> /dev/null)

    if [ "$PROD_MODE" = "gen1" ]; then
        /opt/redpitaya/bin/production_env_gen1_install.sh
    fi

    if [ "$PROD_MODE" = "gen2" ]; then
        /opt/redpitaya/bin/production_env_gen2_install.sh
    fi
fi

# Here you can specify commands for autorun at system startup
