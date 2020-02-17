#!/bin/bash

source ./sub_test/common_func.sh

STATUS=0

if [[ -z $FACTORY_CAL ]]
then
    echo
    echo -n "Setting the default calibration parameters into the EEPROM... "
    print_fail
    echo "NO PARAMETERS FOR CALIBRATION!"
    STATUS=1
else
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
fi

sleep 1

exit $STATUS