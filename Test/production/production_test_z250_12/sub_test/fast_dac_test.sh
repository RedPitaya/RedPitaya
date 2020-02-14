#!/bin/bash
source ./sub_test/common_func.sh

function acquireData(){
    #Acquire data with $DECIMATION decimation factor
    $C_ACQUIRE $ADC_PARAM $ADC_BUFF_SIZE $DECIMATION > /tmp/adc.txt   # WORKAROUND: First acquisition is thrown away
    sleep 0.4
    $C_ACQUIRE $ADC_PARAM $ADC_BUFF_SIZE $DECIMATION > /tmp/adc.txt
    cat /tmp/adc.txt | awk '{print $1}' > /tmp/adc_a.txt
    cat /tmp/adc.txt | awk '{print $2}' > /tmp/adc_b.txt

    # Calculate mean value
    ADC_A=$(awk '{sum+=$1} END { print int(sum/NR)}' /tmp/adc_a.txt)
    ADC_B=$(awk '{sum+=$1} END { print int(sum/NR)}' /tmp/adc_b.txt)
}

function checkError(){
if [[ `echo "$1>$2"|bc` -eq 1 ]] || [[ `echo "$1<-$2"|bc` -eq 1 ]]
then
    echo -n "error value ($1 %) is outside expected range (-$2 - $2) "
    print_fail
    STATUS=1
else
    echo -n "error value $1 % "
    print_ok
fi
}

###############################################################################
# DAC functionality test
###############################################################################

echo
echo -e "\e[94m########################################################################\e[0m"
echo -e "\e[94m#            Test of DAC                                               #\e[0m"
echo -e "\e[94m########################################################################\e[0m"

STATUS=0
DECIMATION=1024
ADC_BUFF_SIZE=16384
ERROR_VAL=1
ERROR_VAL_GAIN=2

echo
echo "TEST DAC with 0.45V on Gain x1"


sleep 0.5
echo -n "  * Connect REF (0.45V) to IN "
# connect in to out 
enableK3Pin
print_ok
sleep 0.5
# get data from adc
echo -n "  * Get data from ADC "
ADC_PARAM="-d B"
acquireData
print_ok

ADC_A_REF_045=$ADC_A
ADC_B_REF_045=$ADC_B

# Print out the measurements
echo "      IN1 mean value is $ADC_A_REF_045"
echo "      IN2 mean value is $ADC_B_REF_045"

sleep 0.5
echo -n "  * Start generator in DC (0.45V) " 
generate_with_api -d1
print_ok

sleep 0.5
echo -n "  * Connect IN to OUT "
# connect in to out 
enableK1Pin
print_ok

# get data from adc
echo -n "  * Get data from ADC "
ADC_PARAM="-d B"
acquireData
print_ok

ADC_A_DAC_045=$(bc -l <<< "2 * $ADC_A")
ADC_B_DAC_045=$(bc -l <<< "2 * $ADC_B")

echo "      IN1 mean value is $ADC_A_DAC_045"
echo "      IN2 mean value is $ADC_B_DAC_045"

ERROR_A=$(printf %f $(bc -l <<< "(($ADC_A_REF_045 /  $ADC_A_DAC_045) - 1.0) * 100.0"))
ERROR_B=$(printf %f $(bc -l <<< "(($ADC_B_REF_045 /  $ADC_B_DAC_045) - 1.0) * 100.0"))
echo
echo -n "      IN1 "
checkError $ERROR_A $ERROR_VAL
echo -n "      IN2 "
checkError $ERROR_B $ERROR_VAL
echo


echo
echo "TEST DAC with 9V on Gain x5"


sleep 0.5
echo -n "  * Connect REF (9V) to IN "
# connect in to out 
enableK4Pin
print_ok
sleep 0.5
# get data from adc
echo -n "  * Get data from ADC "
ADC_PARAM="-d B -1 20 -2 20"
acquireData
print_ok

ADC_A_REF_045=$ADC_A
ADC_B_REF_045=$ADC_B

# Print out the measurements
echo "      IN1 mean value is $ADC_A_REF_045"
echo "      IN2 mean value is $ADC_B_REF_045"


sleep 0.5
echo -n "  * Start generator in DC (0.9V with gain x5) " 
generate_with_api -g
print_ok

sleep 0.5
echo -n "  * Connect IN to OUT "
# connect in to out 
enableK1Pin
print_ok

# get data from adc
echo -n "  * Get data from ADC "
ADC_PARAM="-d B  -1 20 -2 20"
acquireData
print_ok

ADC_A_DAC_045=$(bc -l <<< "4 * $ADC_A")
ADC_B_DAC_045=$(bc -l <<< "4 * $ADC_B")

echo "      IN1 mean value is $ADC_A_DAC_045"
echo "      IN2 mean value is $ADC_B_DAC_045"

ERROR_A=$(printf %f $(bc -l <<< "(($ADC_A_REF_045 /  $ADC_A_DAC_045) - 1.0) * 100.0"))
ERROR_B=$(printf %f $(bc -l <<< "(($ADC_B_REF_045 /  $ADC_B_DAC_045) - 1.0) * 100.0"))
echo
echo -n "      IN1 "
checkError $ERROR_A $ERROR_VAL_GAIN
echo -n "      IN2 "
checkError $ERROR_B $ERROR_VAL_GAIN
echo

if [[ $STATUS == 0 ]]
then
    SetBitState 0x4000
    print_test_ok
else
    print_test_fail
fi

exit $STATUS