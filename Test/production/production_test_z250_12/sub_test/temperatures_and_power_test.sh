#!/bin/bash
source ./sub_test/common_func.sh

# temperatures
MIN_TEMP=20
MAX_TEMP=85

echo
echo -e "\e[94m########################################################################\e[0m"
echo -e "\e[94m#            Temperature and Power supply voltages test                #\e[0m"
echo -e "\e[94m########################################################################\e[0m"
echo
STATUS=0


IN_TEMP0_RAW="$(cat '/sys/bus/iio/devices/iio:device0/in_temp0_raw')"
IN_TEMP0_OFFSET="$(cat '/sys/bus/iio/devices/iio:device0/in_temp0_offset')"
IN_TEMP0_SCALE="$(cat '/sys/bus/iio/devices/iio:device0/in_temp0_scale')"
TEMP=$(bc -l <<< "($IN_TEMP0_RAW + $IN_TEMP0_OFFSET) * $IN_TEMP0_SCALE / 1000" | awk '{ printf "%d\n", $1 }')

#Added, check if teh variable is empty > unsucsefull read will return empty variable. in this case set variable to "x".

if [ -z "$TEMP" ]
then
    TEMP="x"
    echo "Unsuccessful readout of TEMP"
    print_test_fail    
    STATUS=1
else
# Check if the values are within expectations
if [ $TEMP -lt $MIN_TEMP ] || [ $TEMP -gt $MAX_TEMP ]
then
    echo "Measured temperature ($TEMP [deg]) is outside expected range ($MIN_TEMP-$MAX_TEMP)"
    STATUS=1
    print_test_fail
else
    echo "Measured temperature ($TEMP [deg]) is within expectations"
    print_test_ok
fi
fi

sleep 1
V_RESULT=$($C_UART_TOOL 'STAT:VPWR')
echo "POWER TEST = $V_RESULT"

if [[ "$V_RESULT" != "1111111111111111111" ]]
then
    print_test_fail
    STATUS=1
else
    print_test_ok
    RPLight8
    SetBitState 0x10
fi

PrintToFile "temp_and_power" "$TEMP $V_RESULT $($C_UART_TOOL 'GET:VREF:HI') $($C_UART_TOOL 'GET:VREF:LOW')"

sleep 1

exit $STATUS