#!/bin/bash
source ./sub_test/common_func.sh
source ./sub_test/default_calibration_values.sh

echo
echo -e "\e[94m########################################################################\e[0m"
echo -e "\e[94m#              Calibration capacitor in AC mode                        #\e[0m"
echo -e "\e[94m########################################################################\e[0m"
echo

LIGHT_STATUS=$($C_MONITOR 0x40000030)

echo "Calibrate 1 channel in 1:1 mode"
sleep 0.5
echo -n "  * Connect IN to OUT "
# connect in to out 
enableK1Pin
print_ok
sleep 0.5

echo
echo "  * Begin calibrate"
echo "    Please turn the knob until you get the minimum value."
echo "    After calibrate, PRESS ANY KEY TO CONTINUE"
echo 

$C_CAPACITOR_CALIB_TOOL -C1
CAL_VALUE=$(cat /tmp/calib_test_result.txt)
echo
echo -n "Calibrate value $CAL_VALUE "
print_ok
PrintToFile "capacitors" "$CAL_VALUE "
echo
echo 

echo "Calibrate 1 channel in 1:20 mode"
sleep 0.5
echo -n "  * Connect IN to OUT "
# connect in to out 
enableK1Pin
print_ok
sleep 0.5

echo
echo "  * Begin calibrate"
echo "    Please turn the knob until you get the minimum value."
echo "    After calibrate, PRESS ANY KEY TO CONTINUE"
echo 

$C_CAPACITOR_CALIB_TOOL -C1 -A20
CAL_VALUE=$(cat /tmp/calib_test_result.txt)
echo
echo -n "Calibrate value $CAL_VALUE "
print_ok
PrintToFile "capacitors" "$CAL_VALUE "
echo
echo 


echo "Calibrate 2 channel in 1:1 mode"
sleep 0.5
echo -n "  * Connect IN to OUT "
# connect in to out 
enableK1Pin
print_ok
sleep 0.5

echo 
echo "  * Begin calibrate"
echo "    Please turn the knob until you get the minimum value."
echo "    After calibrate, PRESS ANY KEY TO CONTINUE"
echo 

$C_CAPACITOR_CALIB_TOOL -C2 
CAL_VALUE=$(cat /tmp/calib_test_result.txt)
echo
echo -n "Calibrate value $CAL_VALUE "
print_ok
PrintToFile "capacitors" "$CAL_VALUE "
echo
echo 

echo "Calibrate 2 channel in 1:20 mode"
sleep 0.5
echo -n "  * Connect IN to OUT "
# connect in to out 
enableK1Pin
print_ok
sleep 0.5

echo
echo "  * Begin calibrate"
echo "    Please turn the knob until you get the minimum value."
echo "    After calibrate, PRESS ANY KEY TO CONTINUE"
echo

$C_CAPACITOR_CALIB_TOOL -C2 -A20
CAL_VALUE=$(cat /tmp/calib_test_result.txt)
echo
echo -n "Calibrate value $CAL_VALUE "
print_ok
PrintToFile "capacitors" "$CAL_VALUE"

#recover light on RP
$C_MONITOR 0x40000030 w $LIGHT_STATUS
RPLight4
echo
echo 