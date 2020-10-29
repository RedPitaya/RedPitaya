#!/bin/bash
source ./sub_test/common_func.sh

# temperatures
MIN_TEMP=20
MAX_TEMP=85
TOLERANCE_PERC=5
REF_VCCAUX=1800
MIN_VCCAUX=$(($REF_VCCAUX-$REF_VCCAUX*$TOLERANCE_PERC/100))
MAX_VCCAUX=$(($REF_VCCAUX+$REF_VCCAUX*$TOLERANCE_PERC/100))

REF_VCCBRAM=1000
MIN_VCCBRAM=$(($REF_VCCBRAM-$REF_VCCBRAM*$TOLERANCE_PERC/100))
MAX_VCCBRAM=$(($REF_VCCBRAM+$REF_VCCBRAM*$TOLERANCE_PERC/100))

REF_VCCINT=1000
MIN_VCCINT=$(($REF_VCCINT-$REF_VCCINT*$TOLERANCE_PERC/100))
MAX_VCCINT=$(($REF_VCCINT+$REF_VCCINT*$TOLERANCE_PERC/100))

echo
echo -e "\e[94m########################################################################\e[0m"
echo -e "\e[94m#            Temperature and Power supply voltages test                #\e[0m"
echo -e "\e[94m########################################################################\e[0m"
echo
STATUS=0


# Acquire temperature and voltages
$C_MONITOR -ams > /tmp/AMS 2>&1

TEMP=$(cat /tmp/AMS | grep Temp | awk '{ printf "%0.f\n", $4 }')
VCCAUX=$(cat /tmp/AMS | grep VCCAUX | awk '{ printf "%0.f\n", $4*1000 }')
VCCBRAM=$(cat /tmp/AMS | grep VCCBRAM | awk '{ printf "%0.f\n", $4*1000 }')
VCCINT=$(cat /tmp/AMS | grep VCCINT | awk '{ printf "%0.f\n", $4*1000 }')


#Added, check if teh variable is empty > unsucsefull read will return empty variable. in this case set variable to "x".
if [ -z "$TEMP" ]
then
    TEMP="x"
    echo -n "Unsuccessful readout of TEMP "
    print_fail
    STATUS=1
else
# Check if the values are within expectations
if [ $TEMP -lt $MIN_TEMP ] || [ $TEMP -gt $MAX_TEMP ]
then
    echo -n "Measured temperature ($TEMP [deg]) is outside expected range ($MIN_TEMP-$MAX_TEMP) "
    print_fail
    STATUS=1
else
    echo -n "Measured temperature ($TEMP [deg]) is within expectations "
    print_ok
fi
fi

#Added, check if teh variable is empty > unsucsefull read will return empty variable. in this case set variable to "x".
if [ -z "$VCCAUX" ]
then
    VCCAUX="x"
    echo -n "Unsuccessful readout of VCCAUX "
    print_fail
    STATUS=1
else
# Check if the values are within expectations
if [[ $VCCAUX -lt $MIN_VCCAUX ]] || [[ $VCCAUX -gt $MAX_VCCAUX ]]
then
    echo -n "Measured VCCAUX ($VCCAUX [mV]) is outside expected range ($MIN_VCCAUX-$MAX_VCCAUX) "
    print_fail
    STATUS=1
else
    echo -n "Measured VCCAUX ($VCCAUX [mV]) is within expectations "
    print_ok
fi
fi


#Added, check if teh variable is empty > unsucsefull read will return empty variable. in this case set variable to "x".
if [ -z "$VCCBRAM" ]
then
    VCCBRAM="x"
    echo -n "Unsuccessful readout of VCCBRAM "
    print_fail
    STATUS=1
else
# Check if the values are within expectations
if [[ $VCCBRAM -lt $MIN_VCCBRAM ]] || [[ $VCCBRAM -gt $MAX_VCCBRAM ]]
then
    echo -n "Measured VCCBRAM ($VCCBRAM [mV]) is outside expected range ($MIN_VCCBRAM-$MAX_VCCBRAM) "
    print_fail
    STATUS=1
else
    echo -n "Measured VCCBRAM ($VCCBRAM [mV]) is within expectations "
    print_ok
fi
fi

#Added, check if teh variable is empty > unsucsefull read will return empty variable. in this case set variable to "x".
if [ -z "$VCCINT" ]
then
    VCCINT="x"
    echo -n "Unsuccessful readout of VCCINT "
    print_fail
    STATUS=1
else
# Check if the values are within expectations
if [[ $VCCINT -lt $MIN_VCCINT ]] || [[ $VCCINT -gt $MAX_VCCINT ]]
then
    echo -n "Measured VCCINT ($VCCINT [mV]) is outside expected range ($MIN_VCCINT-$MAX_VCCINT) "
    print_fail
    STATUS=1
else
    echo -n "Measured VCCINT ($VCCINT [mV]) is within expectations "
    print_ok
fi
fi


if [[ "$STATUS" != "0" ]]
then
    print_test_fail
else
    print_test_ok
    SetBitState 0x04
fi

PrintToFile "temp_and_power" "$TEMP $VCCAUX $VCCBRAM $VCCINT"

sleep 1

exit $STATUS