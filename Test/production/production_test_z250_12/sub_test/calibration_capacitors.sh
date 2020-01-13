#!/bin/bash
source ./sub_test/common_func.sh
source ./sub_test/default_calibration_values.sh

echo
echo -e "\e[94m########################################################################\e[0m"
echo -e "\e[94m#              Calibration capacitor in AC mode                        #\e[0m"
echo -e "\e[94m########################################################################\e[0m"
echo


echo "Calibrate 1 channel in 1:1 mode"
sleep 0.5
echo -n "  * Connect IN to OUT "
# connect in to out 
enableK1Pin
print_ok
sleep 0.5

echo "  * Begin calibrate"
echo "    Please turn the knob until you get the minimum value."
echo "    After calibrate, PRESS ANY KEY TO CONTINUE"

$C_CAPACITOR_CALIB_TOOL -C1

echo -n "Calibrate "
print_ok

echo
echo 

echo "Calibrate 1 channel in 1:20 mode"
sleep 0.5
echo -n "  * Connect IN to OUT "
# connect in to out 
enableK1Pin
print_ok
sleep 0.5

echo "  * Begin calibrate"
echo "    Please turn the knob until you get the minimum value."
echo "    After calibrate, PRESS ANY KEY TO CONTINUE"

$C_CAPACITOR_CALIB_TOOL -C1 -A20

echo -n "Calibrate "
print_ok

echo
echo 


echo "Calibrate 2 channel in 1:1 mode"
sleep 0.5
echo -n "  * Connect IN to OUT "
# connect in to out 
enableK1Pin
print_ok
sleep 0.5

echo "  * Begin calibrate"
echo "    Please turn the knob until you get the minimum value."
echo "    After calibrate, PRESS ANY KEY TO CONTINUE"

$C_CAPACITOR_CALIB_TOOL -C2 

echo -n "Calibrate "
print_ok

echo
echo 

echo "Calibrate 2 channel in 1:20 mode"
sleep 0.5
echo -n "  * Connect IN to OUT "
# connect in to out 
enableK1Pin
print_ok
sleep 0.5

echo "  * Begin calibrate"
echo "    Please turn the knob until you get the minimum value."
echo "    After calibrate, PRESS ANY KEY TO CONTINUE"

$C_CAPACITOR_CALIB_TOOL -C2 -A20

echo -n "Calibrate "
print_ok

echo
echo 