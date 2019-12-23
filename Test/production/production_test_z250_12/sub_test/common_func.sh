#!/bin/bash

function readZynqCode() {
# Read the DNA Zynq code (part1 and part2) and save it into the log informations
echo "Reading DNA Zynq code..."
echo
DNA_P1=$($C_MONITOR 0x40000004)
sleep 0.2
DNA_P2=$($C_MONITOR 0x40000008)
sleep 0.2
LOG_VAR=

#Added, check if teh variable is empty > unsucsefull read will return empty variable. in this case set variable to "x".
if [ -z "$DNA_P1" ]
then
    DNA_P1="x"
    echo "Unsuccessful readout of DNA_P1"
fi

if [ -z "$DNA_P2" ]
then
    DNA_P2="x"
    echo "Unsuccessful readout of DNA_P2"
fi
ZYNQ_CODE="$DNA_P1 $DNA_P2"
}

function disableAllDIOPin() {
    # SET P pins in IN mode
    $C_MONITOR 0x40000010 w 0x00
    sleep 0.2
    # SET N pins in IN mode
    $C_MONITOR 0x40000014 w 0x00
    sleep 0.2

    # SET P pins in 0 values
    $C_MONITOR 0x40000018 0x00
    sleep 0.2
    # SET N pins in 0 values
    $C_MONITOR 0x4000001C 0x00
    sleep 0.2
}

function hexToDec() {
    local VALUE
    read VALUE
    printf "%d\n" "$VALUE"
}



function get_rtrn(){
    echo `echo $1|cut --delimiter=, -f $2`
}

function print_test_ok(){
    echo -e "TEST RESULT: \033[92m[OK]\e[0m"
}

function print_test_fail(){
    echo -e "TEST RESULT: \033[91m[FAIL]\e[0m"
}

function print_ok(){
    echo -e "\033[92m[OK]\e[0m"
}

function print_fail(){
    echo -e "\033[91m[FAIL]\e[0m"
}


function load_fpga_0_94(){
echo "LOAD FPGA 0.94 IMAGE"
cat /opt/redpitaya/fpga/fpga_0.94.bit > /dev/xdevcfg
sleep 2
echo "FPGA LOADED SUCCESSFULLY"
}

function load_fpga_mercury(){
echo "LOAD FPGA MERCURY IMAGE"
cat /opt/redpitaya/fpga/mercury/fpga.bit > /dev/xdevcfg
sleep 2
echo "FPGA LOADED SUCCESSFULLY"
}