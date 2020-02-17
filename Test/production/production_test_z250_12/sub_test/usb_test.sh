#!/bin/bash
source ./sub_test/common_func.sh

TEST_STRING="Future Technology Devices International, Ltd Bridge(I2C/SPI/UART/FIFO)"

function setUSB1() {
    
    $C_MONITOR 0x40000010 w 0x80
    sleep 0.2
    $C_MONITOR 0x40000014 w 0x40
    sleep 0.2

    # SET P pins in 0 values
    $C_MONITOR 0x40000018 w 0x80
    sleep 0.2
    # SET N pins in 0 values
    $C_MONITOR 0x4000001C w 0x40
    sleep 0.2
} 

function setUSB2() {
    $C_MONITOR 0x40000014 w 0x40
    sleep 0.2

    # SET N pins in 0 values
    $C_MONITOR 0x4000001C w 0x40
    sleep 0.2
} 

echo
echo -e "\e[94m########################################################################\e[0m"
echo -e "\e[94m#            Test of USB data                                          #\e[0m"
echo -e "\e[94m########################################################################\e[0m"
echo

STATUS=0

echo
echo -n "Set ACTIVE USB1 "
disableAllDIOPin
setUSB1
print_ok
sleep 1
RESULT=$(lsusb | grep "$TEST_STRING")
echo "Check USB1"
if [ -z "$RESULT" ]
then
    print_test_fail
    STATUS=1
else
    print_test_ok
    $C_UART_TOOL 'LED:GRN 1 6' -s
fi

echo
echo -n "Set ACTIVE USB2 "
disableAllDIOPin
setUSB2
print_ok
sleep 1
RESULT=$(lsusb | grep "$TEST_STRING")
echo "Check USB2"
if [ -z "$RESULT" ]
then
    print_test_fail
    STATUS=1
else
    print_test_ok
    $C_UART_TOOL 'LED:GRN 1 7' -s
fi

echo
echo -n "Set ACTIVE USB3 "
# Disabled pin set active usb3
disableAllDIOPin 
print_ok
sleep 1
RESULT=$(lsusb | grep "$TEST_STRING")
echo "Check USB2"
if [ -z "$RESULT" ]
then
    print_test_fail
    STATUS=1
else
    print_test_ok
    $C_UART_TOOL 'LED:GRN 0 0' -s
fi

echo
echo "All Test:"
if [[ $STATUS == 0 ]]
then
    print_test_ok
    SetBitState 0x20
else
    print_test_fail
fi

sleep 1

exit $STATUS