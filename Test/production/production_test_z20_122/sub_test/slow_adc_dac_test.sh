#!/bin/bash
source ./sub_test/common_func.sh

# slow ADCs and DACs
TOLERANCE_PERC=10
REF_RATIO=2
MIN_RATIO=$(bc -l <<< "$REF_RATIO - $REF_RATIO * $TOLERANCE_PERC / 100")
MAX_RATIO=$(bc -l <<< "$REF_RATIO + $REF_RATIO * $TOLERANCE_PERC / 100")

echo
echo -e "\e[94m########################################################################\e[0m"
echo -e "\e[94m#            Test of Slow ADCs and DACs                                #\e[0m"
echo -e "\e[94m########################################################################\e[0m"
echo

STATUS=0

sleep 2
$C_MONITOR 0x40000010 w 0x00 # -> Set P to inputs
sleep 0.2
$C_MONITOR 0x40000014 w 0x00 # -> Set N to inputs
sleep 0.2

# Set the half-scale output level of all four DACs (5M)
DAC_VALUE=0x4C4B40
$C_MONITOR 0x40400020 w $DAC_VALUE
sleep 0.2
$C_MONITOR 0x40400024 w $DAC_VALUE
sleep 0.2
$C_MONITOR 0x40400028 w $DAC_VALUE
sleep 0.2
$C_MONITOR 0x4040002C w $DAC_VALUE
sleep 0.2

getAiValue() {
    local AI_PATH='/sys/bus/iio/devices/iio:device1'
    local AI_RAW="$(cat ${AI_PATH}/${1}_raw)"
    local AI_SCALE="$(cat ${AI_PATH}/${1}_scale)"
    bc -l <<< "${AI_RAW} * ${AI_SCALE}"
}

# Get the input level of all four ADCs
ADC1_A="$(getAiValue in_voltage11_vaux8)"
ADC2_A="$(getAiValue in_voltage9_vaux0)"
ADC3_A="$(getAiValue in_voltage10_vaux1)"
ADC4_A="$(getAiValue in_voltage12_vaux9)"

echo "    ADC values - first acquisition - are $ADC1_A, $ADC2_A, $ADC3_A, $ADC4_A"

# Set almost full-scale output level of all four DACs (2x5M=10M)
DAC_VALUE=0x989680
$C_MONITOR 0x40400020 w $DAC_VALUE
sleep 0.2
$C_MONITOR 0x40400024 w $DAC_VALUE
sleep 0.2
$C_MONITOR 0x40400028 w $DAC_VALUE
sleep 0.2
$C_MONITOR 0x4040002C w $DAC_VALUE
sleep 0.2

# Get again the input level of all four DACs
ADC1_B="$(getAiValue in_voltage11_vaux8)"
ADC2_B="$(getAiValue in_voltage9_vaux0)"
ADC3_B="$(getAiValue in_voltage10_vaux1)"
ADC4_B="$(getAiValue in_voltage12_vaux9)"

echo "    ADC values - second acquisition - are $ADC1_B, $ADC2_B, $ADC3_B, $ADC4_B"

# Evaluate the ratios
ADC1_R=$(bc -l <<< "$ADC1_B / $ADC1_A")
ADC2_R=$(bc -l <<< "$ADC2_B / $ADC2_A")
ADC3_R=$(bc -l <<< "$ADC3_B / $ADC3_A")
ADC4_R=$(bc -l <<< "$ADC4_B / $ADC4_A")
BC_RESULT=$(bc -l <<< "\
($ADC1_R >= $MIN_RATIO) && \
($ADC1_R <= $MAX_RATIO) && \
($ADC2_R >= $MIN_RATIO) && \
($ADC2_R <= $MAX_RATIO) && \
($ADC3_R >= $MIN_RATIO) && \
($ADC3_R <= $MAX_RATIO) && \
($ADC4_R >= $MIN_RATIO) && \
($ADC4_R <= $MAX_RATIO) \
")

if [[ "$BC_RESULT" = '1' ]]
then
    echo "    Measured ratios after two slow ADC acquisitions match expected ratio (2)"
else
    echo "    Measured ratios after two slow ADC acquisitions ($ADC1_R, $ADC2_R, $ADC3_R, $ADC4_R) don't match expected ratio (2)"
    STATUS=1
fi

sleep 1

disableAllDIOPin


echo
if [[ $STATUS == 0 ]]
then
    RPLight5
    SetBitState 0x40
    print_test_ok
else 
    print_test_fail
fi
echo

sleep 1

exit $STATUS