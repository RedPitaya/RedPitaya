#!/bin/bash

source ./sub_test/common_func.sh

STATUS=0

# # new unsed parameters
# GEN_CH1_G_1=0 
# GEN_CH2_G_1=0
# GEN_CH1_OFF_1=0
# GEN_CH2_OFF_1=0
# GEN_CH1_G_5=0
# GEN_CH2_G_5=0
# GEN_CH1_OFF_5=0
# GEN_CH2_OFF_5=0
# OSC_CH1_G_1_AC=0
# OSC_CH2_G_1_AC=0
# OSC_CH1_OFF_1_AC=0
# OSC_CH2_OFF_1_AC=0
# OSC_CH1_G_1_DC=0
# OSC_CH2_G_1_DC=0
# OSC_CH1_OFF_1_DC=0
# OSC_CH2_OFF_1_DC=0
# OSC_CH1_G_20_AC=0
# OSC_CH2_G_20_AC=0
# OSC_CH1_OFF_20_AC=0
# OSC_CH2_OFF_20_AC=0
# OSC_CH1_G_20_DC=0
# OSC_CH2_G_20_DC=0
# OSC_CH1_OFF_20_DC=0
# OSC_CH2_OFF_20_DC=0
# FACTORY_CAL="$GEN_CH1_G_1 $GEN_CH2_G_1 $GEN_CH1_OFF_1 $GEN_CH2_OFF_1 $GEN_CH1_G_5 $GEN_CH2_G_5 $GEN_CH1_OFF_5 $GEN_CH2_OFF_5 $OSC_CH1_G_1_AC $OSC_CH2_G_1_AC $OSC_CH1_OFF_1_AC $OSC_CH2_OFF_1_AC $OSC_CH1_G_1_DC $OSC_CH2_G_1_DC $OSC_CH1_OFF_1_DC $OSC_CH2_OFF_1_DC $OSC_CH1_G_20_AC $OSC_CH2_G_20_AC $OSC_CH1_OFF_20_AC $OSC_CH2_OFF_20_AC $OSC_CH1_G_20_DC $OSC_CH2_G_20_DC $OSC_CH1_OFF_20_DC $OSC_CH2_OFF_20_DC"

FE_CH1_FS_G_HI=21474836
FE_CH2_FS_G_HI=21474836
FE_CH1_FS_G_LO=429496729
FE_CH2_FS_G_LO=429496729
FE_CH1_DC_offs=0
FE_CH2_DC_offs=0
BE_CH1_FS=42949672
BE_CH2_FS=42949672
BE_CH1_DC_offs=0
BE_CH2_DC_offs=0
Magic=-1430532899
FE_CH1_DC_offs_HI=0
FE_CH2_DC_offs_HI=0
#All calibration parameters in one string
FACTORY_CAL="$FE_CH1_FS_G_HI $FE_CH2_FS_G_HI $FE_CH1_FS_G_LO $FE_CH2_FS_G_LO $FE_CH1_DC_offs $FE_CH2_DC_offs $BE_CH1_FS $BE_CH2_FS $BE_CH1_DC_offs $BE_CH2_DC_offs $Magic $FE_CH1_DC_offs_HI $FE_CH2_DC_offs_HI"

# Set the CALIBRATION PARAMETERS to the FACTORY -wf EEPROM memory partition (factory parameters)
echo
echo -n "Setting the default calibration parameters into the EEPROM... "
echo $FACTORY_CAL | $C_CALIB -wf
if [ $? -ne 0 ]
then
    echo
    echo -n "Default calibration parameters are NOT correctly written in the factory EEPROM space "
    print_fail
    sleep 1
    STATUS=1
    else
    print_ok
fi

# Copy the NEW CALIBRATION PARAMETERS to the user EEPROM memory partition
echo -n "Setting the  default calibration parameters into the user EEPROM space... "
    echo $FACTORY_CAL | $C_CALIB -w
    if [ $? -ne 0 ]
    then
    echo
    echo - n "New calibration parameters are NOT correctly written in the user EEPROM space"
    print_fail
    sleep 1
    STATUS=1
    else 
    print_ok
    fi

$C_CALIB -rv


sleep 1

exit $STATUS