#!/bin/bash
source ./sub_test/common_func.sh

###############################################################################
# Memory test
###############################################################################

echo
echo -e "\e[94m########################################################################\e[0m"
echo -e "\e[94m#            Test of PLL                                               #\e[0m"
echo -e "\e[94m########################################################################\e[0m"

STATUS=0
LIGHT_STATUS=$($C_MONITOR 0x40000030)

echo
echo "Start PLL test..."
echo

enableK1Pin
sleep 0.5
echo
echo -n "  * start generate ref signal 1kHz/5V "
$C_GENERATE 1 2 1000 x5 sine
print_ok
echo
sleep 0.5

echo -n "  * enable PLL Control "
$C_MONITOR 0x40000040 w 0x1
print_ok
echo
sleep 0.5

echo -n "  * check PLL Control "
STATE=$($C_MONITOR 0x40000040) 
echo $STATE
REF_FOUND="$(( $STATE & 0x10 ))"
echo -n "  * Reference detected $REF_FOUND " 
if [[ $REF_FOUND != 0 ]]
then
    print_ok
else
    STATUS=1
    print_fail
fi
sleep 0.5

LOCK_FOUND="$(( $STATE & 0x100 ))"
echo -n "  * Locked $LOCK_FOUND " 
if [[ $LOCK_FOUND != 0 ]]
then
    print_ok
else
    STATUS=1
    print_fail
fi
sleep 0.5

echo
echo -n "PLL testing completed "
$C_MONITOR 0x40000030 w $LIGHT_STATUS
if [[ $STATUS != 0 ]]
then
    print_test_fail
else
    print_test_ok
    RPLight5
    SetBitState 0x800
fi


exit $STATUS