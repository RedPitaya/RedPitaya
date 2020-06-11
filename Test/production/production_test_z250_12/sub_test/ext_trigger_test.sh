#!/bin/bash
source ./sub_test/common_func.sh

###############################################################################
# Memory test
###############################################################################

echo
echo -e "\e[94m########################################################################\e[0m"
echo -e "\e[94m#            Test of External trigger                                  #\e[0m"
echo -e "\e[94m########################################################################\e[0m"

STATUS=0
LIGHT_STATUS=$($C_MONITOR 0x40000030)
echo
echo "Start ext trigger test..."
echo

enableK1Pin

echo -n "   * Start generate square 1KHz signal for trigger and input "
$C_GENERATE 1 2 1000 x5 sine
$C_GENERATE 2 2 1000 x5 sine
print_ok

sleep 1

echo -n "   * Signal acquire with external trigger. Set level 1.5V "

timeout -k 5 5 $C_ACQUIRE -r 1000 1024 -d B -t EP -l 1.5 > /dev/null

if [[ $? != 0 ]]
then
    STATUS=1
    print_fail
else
    print_ok
fi

sleep 1

echo -n "   * Signal acquire with external trigger. Set level 2.5V "

timeout -k 5 5 $C_ACQUIRE -r 1000 1024 -d B -t EP -l 2.5 > /dev/null

if [[ $? != 0 ]]
then
    print_ok
else
    STATUS=1
    print_fail
fi

sleep 1

#recover light on RP
$C_MONITOR 0x40000030 w $LIGHT_STATUS
if [[ $STATUS != 0 ]]
then
    print_test_fail
else
    RPLight6
    print_test_ok
    SetBitState 0x1000
fi



exit $STATUS
