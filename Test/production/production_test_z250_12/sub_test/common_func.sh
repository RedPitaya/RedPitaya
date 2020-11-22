#!/bin/bash

function readZynqCode() {
# Read the DNA Zynq code (part1 and part2) and save it into the log informations
echo "Reading DNA Zynq code..."
echo
DNA_P1=$($C_MONITOR 0x40000004)
sleep 0.2
DNA_P2=$($C_MONITOR 0x40000008)
sleep 0.2

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
    $C_MONITOR 0x40000010 w 0x0FFF
    sleep 0.2
    # SET N pins in IN mode
    $C_MONITOR 0x40000014 w 0x0FFF
    sleep 0.2

    # SET P pins in 0 values
    $C_MONITOR 0x40000018 0x0000
    sleep 0.2
    # SET N pins in 0 values
    $C_MONITOR 0x4000001C 0x0000
    sleep 0.2
}

function enableK1Pin() {
    disableAllDIOPin

    $C_MONITOR 0x40000014 w 0x0080 # -> Set N to outputs
    sleep 0.2
    $C_MONITOR 0x4000001C w 0x0080 # ->  Set DIO7_N = 1
    sleep 0.2
}

function enableK2Pin() {
    disableAllDIOPin

    # Configure DIOx_P to inputs and DIOx_N to outputs to prevent Relay misbehaviour
    $C_MONITOR 0x40000010 w 0x0080 # -> Set P to inputs
    sleep 0.2
    $C_MONITOR 0x40000018 w 0x0080 # ->  Set DIO7_N = 1
    sleep 0.2
}

function enableK3Pin() {
    disableAllDIOPin

    $C_MONITOR 0x40000014 w 0x0100 # -> Set N to outputs
    sleep 0.2
    $C_MONITOR 0x4000001C w 0x0100 # ->  Set DIO7_N = 1
    sleep 0.2
}

function enableK4Pin() {
    disableAllDIOPin

    # Configure DIOx_P to inputs and DIOx_N to outputs to prevent Relay misbehaviour
    $C_MONITOR 0x40000010 w 0x0200 # -> Set P to inputs
    sleep 0.2
    $C_MONITOR 0x40000018 w 0x0200 # ->  Set DIO7_N = 1
    sleep 0.2
}

function disableGenerator(){
    $C_GENERATE 1 0 0 x1 sine
    $C_GENERATE 2 0 0 x1 sine
}

function disableGeneratorX5(){
    $C_GENERATE 1 0 0 x5 sine
    $C_GENERATE 2 0 0 x5 sine
}

function disableGeneratorPower() {
    $C_MONITOR 0x40000060 0x03 
    sleep 0.2
    $C_MONITOR 0x40000064 0xFF 
    sleep 0.2
}

function enableGeneratorPower() {
    $C_MONITOR 0x40000060 0x03 
    sleep 0.2
    $C_MONITOR 0x40000064 0x00 
    sleep 0.2
}


function hexToDec() {
    local VALUE
    read VALUE
    printf "%d\n" "$VALUE"
}

function getLowRefValue(){
    REF_V=$($C_UART_TOOL 'GET:VREF:LOW')
#    REF_V=0.444
    REF_V=$(printf %f $(bc -l <<< "scale=0; 8192 * $REF_V"))
}

function getHighRefValue(){
    REF_V=$($C_UART_TOOL 'GET:VREF:HI')
#    REF_V=9.03
    REF_V=$(printf %f $(bc -l <<< "scale=0; 8192 * $REF_V / 20"))
}

function getLowRefVoltage(){
    REF_V=$($C_UART_TOOL 'GET:VREF:LOW')
#    REF_V=0.444
}

function getHighRefVoltage(){
    REF_V=$($C_UART_TOOL 'GET:VREF:HI')
#    REF_V=9.03
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

function print_skip(){
    echo -e "\033[94m[SKIPPED]\e[0m"
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

function print_calib(){
    calib -rv
}

function disableRPLight(){
    $C_MONITOR 0x40000030 w 0x00
}

function RPLight1(){
L_CUR_VALUE=$($C_MONITOR 0x40000030)
L_CUR_VALUE=$(( 0x10 | $L_CUR_VALUE ))
    $C_MONITOR 0x40000030 w $L_CUR_VALUE
}

function RPLight2(){
L_CUR_VALUE=$($C_MONITOR 0x40000030)
L_CUR_VALUE=$(( 0x20 | $L_CUR_VALUE ))
    $C_MONITOR 0x40000030 w $L_CUR_VALUE
}

function RPLight3(){
L_CUR_VALUE=$($C_MONITOR 0x40000030)
L_CUR_VALUE=$(( 0x40 | $L_CUR_VALUE ))
    $C_MONITOR 0x40000030 w $L_CUR_VALUE
}

function RPLight4(){
L_CUR_VALUE=$($C_MONITOR 0x40000030)
L_CUR_VALUE=$(( 0x80 | $L_CUR_VALUE ))
    $C_MONITOR 0x40000030 w $L_CUR_VALUE
}

function RPLight5(){
L_CUR_VALUE=$($C_MONITOR 0x40000030)
L_CUR_VALUE=$(( 0x01 | $L_CUR_VALUE ))
    $C_MONITOR 0x40000030 w $L_CUR_VALUE
}

function RPLight6(){
L_CUR_VALUE=$($C_MONITOR 0x40000030)
L_CUR_VALUE=$(( 0x02 | $L_CUR_VALUE ))
    $C_MONITOR 0x40000030 w $L_CUR_VALUE
}

function RPLight7(){
L_CUR_VALUE=$($C_MONITOR 0x40000030)
L_CUR_VALUE=$(( 0x04 | $L_CUR_VALUE ))
    $C_MONITOR 0x40000030 w $L_CUR_VALUE
}

function RPLight8(){
L_CUR_VALUE=$($C_MONITOR 0x40000030)
L_CUR_VALUE=$(( 0x08 | $L_CUR_VALUE ))
    $C_MONITOR 0x40000030 w $L_CUR_VALUE
}

function InitBitState(){
    echo "0x0000" > $TEST_TMP_DIR/bit_value
}

function SetBitState(){
    local VALUE=$(cat $TEST_TMP_DIR/bit_value)
    VALUE=$(( $VALUE | $1 ))
    echo  $(printf "0x%04X\n" $VALUE) > $TEST_TMP_DIR/bit_value
}

function SetBackLog(){
    NAME=$1
    local line='----------------------------------------'
    echo " *" $(printf "%s %s %s\n" "$NAME" "${line:${#NAME}}" "$2") >> $TEST_TMP_DIR/back_log
}

function PrintBackLog(){
    echo
    echo "Test result (number bits) $(cat $TEST_TMP_DIR/bit_value)"
    echo
    cat $TEST_TMP_DIR/back_log
}

function PrintToFile(){
    echo -n "$2" >> $TEST_TMP_DIR/$1
}

function CombineLogVar(){
    LOG_VAR=$(date +'%d %m %y %T %Z %Y')
    LOG_VAR="$LOG_VAR $(cat $TEST_TMP_DIR/bit_value 2> /dev/null)"
    LOG_VAR="$LOG_VAR $(cat $TEST_TMP_DIR/zynq_code 2> /dev/null)"
    LOG_VAR="$LOG_VAR $(cat $TEST_TMP_DIR/mac_addr 2> /dev/null)"
    LOG_VAR="$LOG_VAR $(cat $TEST_TMP_DIR/temp_and_power 2> /dev/null)"
    LOG_VAR="$LOG_VAR$(cat $TEST_TMP_DIR/fast_adc 2> /dev/null)" # don't add space char
    LOG_VAR="$LOG_VAR $(calib -r | xargs echo -n)"
    LOG_VAR="$LOG_VAR $(cat $TEST_TMP_DIR/capacitors 2> /dev/null)"
    LOG_VAR="$LOG_VAR $(cat $TEST_TMP_DIR/mem_test 2> /dev/null)"
    LOG_VAR="$LOG_VAR $(cat $TEST_TMP_DIR/calib_test 2> /dev/null)"
    LOG_VAR="$LOG_VAR $(cat $TEST_TMP_DIR/hw_rev 2> /dev/null)"
}

function CombineLogVarLocal(){
    LOG_VAR=$(date)
    LOG_VAR="$LOG_VAR $(cat $TEST_TMP_DIR/bit_value 2> /dev/null)"
    LOG_VAR="$LOG_VAR $(cat $TEST_TMP_DIR/zynq_code 2> /dev/null)"
    LOG_VAR="$LOG_VAR $(cat $TEST_TMP_DIR/mac_addr 2> /dev/null)"
    LOG_VAR="$LOG_VAR $(cat $TEST_TMP_DIR/temp_and_power 2> /dev/null)"
    LOG_VAR="$LOG_VAR$(cat $TEST_TMP_DIR/fast_adc 2> /dev/null)" # don't add space char
    LOG_VAR="$LOG_VAR $(calib -r | xargs echo -n)"
    LOG_VAR="$LOG_VAR $(cat $TEST_TMP_DIR/capacitors 2> /dev/null)"
    LOG_VAR="$LOG_VAR $(cat $TEST_TMP_DIR/mem_test 2> /dev/null)"
    LOG_VAR="$LOG_VAR $(cat $TEST_TMP_DIR/calib_test 2> /dev/null)"
    LOG_VAR="$LOG_VAR $(cat $TEST_TMP_DIR/hw_rev 2> /dev/null)"
}

function CheckTestPass(){
    TEST_RES=$(cat $TEST_TMP_DIR/bit_value)
    if [[ "$TEST_RES" = "0x7FFF" ]]
    then
        $C_UART_TOOL 'LED:GRN 0 7' -s
        STATUS=1
    else
        STATUS=0
    fi
}

function GetCalibValue()
{
    STR="{print $"
    STR+=$1
    STR+="}"
    calib -r > $TEST_TMP_DIR/calib
    CALIB_RET_VALUE=$(awk "$STR" $TEST_TMP_DIR/calib)
}