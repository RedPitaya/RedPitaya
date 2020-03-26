#!/bin/bash

source ./sub_test/common_func.sh

STATUS=0

if [[ -z $FACTORY_CAL ]]
then
    echo
    echo -n "Setting the calibration parameters into the EEPROM... "
    print_fail
    echo "NO PARAMETERS FOR CALIBRATION!"
else
    # Copy the NEW CALIBRATION PARAMETERS to the user EEPROM memory partition
    echo -n "Setting the  default calibration parameters into the user EEPROM space... "
    echo $FACTORY_CAL | $C_CALIB -w
    if [ $? -ne 0 ]
    then
        echo
        echo -n "New calibration parameters are NOT correctly written in the user EEPROM space"
        print_fail
        sleep 1
        STATUS=1
    else 
        print_ok
    fi
fi

sleep 1

exit $STATUS