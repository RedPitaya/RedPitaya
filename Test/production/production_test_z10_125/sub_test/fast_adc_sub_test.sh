#!/bin/bash
source ./sub_test/common_func.sh

STATUS=0

# Acquire data with 1024 decimation factor
$C_ACQUIRE $ACQ_PARAM $ADC_BUFF_SIZE 1024 > /tmp/adc.txt   # WORKAROUND: First acquisition is thrown away
sleep 0.4
$C_ACQUIRE $ACQ_PARAM $ADC_BUFF_SIZE 1024 > /tmp/adc.txt
cat /tmp/adc.txt | awk '{print $1}' > /tmp/adc_a.txt
cat /tmp/adc.txt | awk '{print $2}' > /tmp/adc_b.txt

# NEW Acquire data also with no decimation
$C_ACQUIRE $ACQ_PARAM $ADC_BUFF_SIZE 1 > /tmp/adc_no_dec.txt
cat /tmp/adc_no_dec.txt | awk '{print $1}' > /tmp/adc_no_dec_a.txt
cat /tmp/adc_no_dec.txt | awk '{print $2}' > /tmp/adc_no_dec_b.txt

# Calculate mean value, std deviation and p2p diff value
ADC_A_MEAN=$(awk '{sum+=$1} END { print int(sum/NR)}' /tmp/adc_a.txt)
ADC_B_MEAN=$(awk '{sum+=$1} END { print int(sum/NR)}' /tmp/adc_b.txt)
ADC_A_STD=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)*(sum/NR)))}' /tmp/adc_a.txt)
ADC_B_STD=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)*(sum/NR)))}' /tmp/adc_b.txt)
ADC_A_PP=$(awk 'BEGIN{max=-1000;min=1000} { if (max < $1){ max = $1 }; if(min > $1){ min = $1 } } END{ print max-min}' /tmp/adc_a.txt)
ADC_B_PP=$(awk 'BEGIN{max=-1000;min=1000} { if (max < $1){ max = $1 }; if(min > $1){ min = $1 } } END{ print max-min}' /tmp/adc_b.txt)
ADC_A_STD_NO_DEC=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)*(sum/NR)))}' /tmp/adc_no_dec_a.txt)
ADC_B_STD_NO_DEC=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)*(sum/NR)))}' /tmp/adc_no_dec_b.txt)


# Print out the measurements
echo "    Measured ADC ch.A mean value is $ADC_A_MEAN"
echo "    Measured ADC ch.A std.deviation value is $ADC_A_STD"
echo "    Measured ADC ch.A std.deviation (no decimation) is $ADC_A_STD_NO_DEC"
echo "    Measured ADC ch.A p2p value is $ADC_A_PP"
echo "    Measured ADC ch.B mean value is $ADC_B_MEAN"
echo "    Measured ADC ch.B std.deviation value is $ADC_B_STD"
echo "    Measured ADC ch.B std.deviation (no decimation) is $ADC_B_STD_NO_DEC"
echo "    Measured ADC ch.B p2p value is $ADC_B_PP"
PrintToFile "fast_adc" " $ADC_A_MEAN $ADC_A_STD $ADC_A_PP $ADC_A_STD_NO_DEC $ADC_B_MEAN $ADC_B_STD $ADC_B_PP $ADC_B_STD_NO_DEC"

# Check if the values are within expectations
if [[ $ADC_A_MEAN -gt $MAX_OFFSET ]] || [[ $ADC_A_MEAN -lt $((-$MAX_OFFSET)) ]]
then
    echo -n "    Measured ch.A mean value ($ADC_A_MEAN) is outside expected range (+/- $MAX_OFFSET) "
    print_fail
    STATUS=1
fi

if [[ $ADC_B_MEAN -gt $MAX_OFFSET ]] || [[ $ADC_B_MEAN -lt $((-$MAX_OFFSET)) ]]
then
    echo -n "    Measured ch.B mean value ($ADC_B_MEAN) is outside expected range (+/- $MAX_OFFSET) "
    print_fail
    STATUS=1
fi

if [[ $ADC_A_STD -lt $MIN_DEVIATION ]] || [[ $ADC_A_STD -gt $MAX_DEVIATION ]]
then
    echo -n "    Measured ch.A std deviation value ($ADC_A_STD) is outside expected range ($MIN_DEVIATION-$MAX_DEVIATION) "
    print_fail
    STATUS=1
fi

if [[ $ADC_B_STD -lt $MIN_DEVIATION ]] || [[ $ADC_B_STD -gt $MAX_DEVIATION ]]
then
    echo -n "    Measured ch.B std deviation value ($ADC_B_STD) is outside expected range ($MIN_DEVIATION-$MAX_DEVIATION) "
    print_fail
    STATUS=1
fi

if [[ $ENABLE_NOISE_NO_DEC == 1 ]]
then
if [[ $ADC_A_STD_NO_DEC -gt $MAX_NOISE_NO_DEC ]]
then
    echo -n "    Measured ch.A std deviation value with no decimation ($ADC_A_STD_NO_DEC) is outside expected range (0-$MAX_NOISE_NO_DEC) "
    print_fail
    STATUS=1
fi

if [[ $ADC_B_STD_NO_DEC -gt $MAX_NOISE_NO_DEC ]]
then
    echo -n "    Measured ch.B std deviation value with no decimation ($ADC_B_STD_NO_DEC) is outside expected range (0-$MAX_NOISE_NO_DEC) "
    print_fail
    STATUS=1
fi
fi

if [[ $ADC_A_PP -lt $MIN_P2P ]] || [[ $ADC_A_PP -gt $MAX_P2P ]]
then
    echo -n "    Measured ch.A p2p value ($ADC_A_PP) is outside expected range ($MIN_P2P-$MAX_P2P) "
    print_fail
    STATUS=1
fi

if [[ $ADC_B_PP -lt $MIN_P2P ]] || [[ $ADC_B_PP -gt $MAX_P2P ]]
then
    echo -n "    Measured ch.B p2p value ($ADC_B_PP) is outside expected range ($MIN_P2P-$MAX_P2P) "
    print_fail
    STATUS=1
fi


exit $STATUS