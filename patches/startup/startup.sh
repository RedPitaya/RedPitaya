#!/bin/bash

if [[ -f /opt/redpitaya/sbin/auto_resize.sh ]]
then
    /opt/redpitaya/sbin/auto_resize.sh
    echo 'REBOOT AFTER RESIZE SD CARD' > /dev/kmsg
fi

if [[ ! -f /opt/redpitaya/bin/production_testing_script.sh ]]
then
    /opt/redpitaya/sbin/mkoverlay.sh v0.94

    /opt/redpitaya/sbin/getsysinfo.sh

    # Turns on the power to the ADC and DAC

    MODEL=$(/opt/redpitaya/bin/profiles -i)

    # STEM_250_12_v1_0
    if [ "$MODEL" = "11" ]; then
    /opt/redpitaya/bin/rp_power_on -P
    /opt/redpitaya/bin/rp_power_on -C
    fi

    # STEM_250_12_v1_1
    if [ "$MODEL" = "12" ]; then
    /opt/redpitaya/bin/rp_power_on -P
    /opt/redpitaya/bin/rp_power_on -C
    fi

    # STEM_250_12_v1_2
    if [ "$MODEL" = "13" ]; then
    /opt/redpitaya/bin/rp_power_on -P
    /opt/redpitaya/bin/rp_power_on -C
    fi

    # STEM_250_12_120
    if [ "$MODEL" = "14" ]; then
    /opt/redpitaya/bin/rp_power_on -P
    /opt/redpitaya/bin/rp_power_on -C
    fi

    # STEM_250_12_v1_2a
    if [ "$MODEL" = "15" ]; then
    /opt/redpitaya/bin/rp_power_on -P
    /opt/redpitaya/bin/rp_power_on -C
    fi

    # STEM_250_12_v1_2b
    if [ "$MODEL" = "16" ]; then
    /opt/redpitaya/bin/rp_power_on -P
    /opt/redpitaya/bin/rp_power_on -C
    fi

    # STEM_125_14_Z7020_LL_v1_1
    if [ "$MODEL" = "25" ]; then
    /opt/redpitaya/bin/rp_power_on -C1
    fi

    # STEM_65_16_Z7020_LL_v1_1
    if [ "$MODEL" = "26" ]; then
    /opt/redpitaya/bin/rp_power_on -C2
    fi

    # STEM_125_14_Z7020_LL_v1_2
    if [ "$MODEL" = "27" ]; then
    /opt/redpitaya/bin/rp_power_on -C1
    fi

    # STEM_125_14_Z7020_TI_v1_3
    if [ "$MODEL" = "28" ]; then
    /opt/redpitaya/bin/rp_power_on -C1
    fi

    # STEM_65_16_Z7020_TI_v1_3
    if [ "$MODEL" = "29" ]; then
    /opt/redpitaya/bin/rp_power_on -C2
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
