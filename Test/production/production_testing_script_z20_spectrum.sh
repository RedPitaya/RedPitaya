#!/bin/bash

GENERATE_CH0_FREQ='25678901'
GENERATE_CH1_FREQ='29012345'
GENERATE_CH0_VALUE='-4'
GENERATE_CH1_VALUE='-4'
SFDR_LEVEL='60'
TEST_STATUS=0

CH_FREQ_TOLERANCE='2500000'
CH_VALUE_TOLERANCE='2'

CH0_PEAK_FREQ_MIN=$(bc -l <<< "${GENERATE_CH0_FREQ} - ${CH_FREQ_TOLERANCE}")
CH0_PEAK_FREQ_MAX=$(bc -l <<< "${GENERATE_CH0_FREQ} + ${CH_FREQ_TOLERANCE}")
CH0_PEAK_VALUE_MIN=$(bc -l <<< "${GENERATE_CH0_VALUE} + ${CH_VALUE_TOLERANCE}")
CH0_PEAK_VALUE_MAX=$(bc -l <<< "${GENERATE_CH0_VALUE} - ${CH_VALUE_TOLERANCE}")

CH1_PEAK_FREQ_MIN=$(bc -l <<<"${GENERATE_CH1_FREQ} - ${CH_FREQ_TOLERANCE}")
CH1_PEAK_FREQ_MAX=$(bc -l <<<  "${GENERATE_CH1_FREQ} + ${CH_FREQ_TOLERANCE}")
CH1_PEAK_VALUE_MIN=$(bc -l <<< "${GENERATE_CH1_VALUE} + ${CH_VALUE_TOLERANCE}")
CH1_PEAK_VALUE_MAX=$(bc -l <<< "${GENERATE_CH1_VALUE} - ${CH_VALUE_TOLERANCE}")

export LD_LIBRARY_PATH='/opt/redpitaya/lib'

# Peak measurement
test_peak_measurement() {
    # global in:
    # CH0_PEAK_FREQ_MIN
    # CH0_PEAK_FREQ_MAX
    # CH0_PEAK_VALUE_MIN
    # CH0_PEAK_VALUE_MAX
    # CH1_PEAK_FREQ_MIN
    # CH1_PEAK_FREQ_MAX
    # CH1_PEAK_VALUE_MIN
    # CH1_PEAK_VALUE_MAX
    #
    # global out:
    # CH0_PEAK_FREQ
    # CH0_PEAK_VALUE
    # CH1_PEAK_FREQ
    # CH1_PEAK_VALUE

    local CHANNEL=$1

    local TEST_RESULT=0

    local SPECTRUM_RESULT="$(spectrum -t)"
    if [[ "$CHANNEL" = "CH0" ]] 
    then
    CH0_PEAK_FREQ=$(gawk 'match($0, /^ch0 peak\:\s(.+)\sHz\,\s(.+)\sdB$/, a) {print a[1]}' <<< "${SPECTRUM_RESULT}")
    CH0_PEAK_VALUE=$(gawk 'match($0, /^ch0 peak\:\s(.+)\sHz\,\s(.+)\sdB$/, a) {print a[2]}' <<< "${SPECTRUM_RESULT}")
  
    local BC_RESULT=$(bc -l <<< "(${CH0_PEAK_FREQ} >= ${CH0_PEAK_FREQ_MIN}) && (${CH0_PEAK_FREQ} <= ${CH0_PEAK_FREQ_MAX})")
    if [[ "$BC_RESULT" = '1' ]]
    then
        echo "CH0_PEAK_FREQ, meas: ${CH0_PEAK_FREQ}, min: ${CH0_PEAK_FREQ_MIN}, max: ${CH0_PEAK_FREQ_MAX}"
    else
        TEST_RESULT=1
        echo "Error CH0_PEAK_FREQ, meas: ${CH0_PEAK_FREQ}, min: ${CH0_PEAK_FREQ_MIN}, max: ${CH0_PEAK_FREQ_MAX}"
    fi

    BC_RESULT=$(bc -l <<< "(${CH0_PEAK_VALUE} <= ${CH0_PEAK_VALUE_MIN}) && (${CH0_PEAK_VALUE} >= ${CH0_PEAK_VALUE_MAX})")
    if [[ "$BC_RESULT" = '1' ]]
    then
        echo "CH0_PEAK_VALUE, meas: ${CH0_PEAK_VALUE}, min: ${CH0_PEAK_VALUE_MIN}, max: ${CH0_PEAK_VALUE_MAX}"
    else
        TEST_RESULT=1
        echo "Error CH0_PEAK_VALUE, meas: ${CH0_PEAK_VALUE}, min: ${CH0_PEAK_VALUE_MIN}, max: ${CH0_PEAK_VALUE_MAX}"
    fi
    fi

    if [[ "$CHANNEL" = "CH1" ]] 
    then
    CH1_PEAK_FREQ=$(gawk 'match($0, /^ch1 peak\:\s(.+)\sHz\,\s(.+)\sdB$/, a) {print a[1]}' <<< "${SPECTRUM_RESULT}")
    CH1_PEAK_VALUE=$(gawk 'match($0, /^ch1 peak\:\s(.+)\sHz\,\s(.+)\sdB$/, a) {print a[2]}' <<< "${SPECTRUM_RESULT}")


    BC_RESULT=$(bc -l <<< "(${CH1_PEAK_FREQ} >= ${CH1_PEAK_FREQ_MIN}) && (${CH1_PEAK_FREQ} <= ${CH1_PEAK_FREQ_MAX})")
    if [[ "$BC_RESULT" = '1' ]]
    then
        echo "CH1_PEAK_FREQ, meas: ${CH1_PEAK_FREQ}, min: ${CH1_PEAK_FREQ_MIN}, max: ${CH1_PEAK_FREQ_MAX}"
    else
        TEST_RESULT=1
        echo "Error CH1_PEAK_FREQ, meas: ${CH1_PEAK_FREQ}, min: ${CH1_PEAK_FREQ_MIN}, max: ${CH1_PEAK_FREQ_MAX}"
    fi

    BC_RESULT=$(bc -l <<< "(${CH1_PEAK_VALUE} <= ${CH1_PEAK_VALUE_MIN}) && (${CH1_PEAK_VALUE} >= ${CH1_PEAK_VALUE_MAX})")
    if [[ "$BC_RESULT" = '1' ]]
    then
        echo "CH1_PEAK_VALUE, meas: ${CH1_PEAK_VALUE}, min: ${CH1_PEAK_VALUE_MIN}, max: ${CH1_PEAK_VALUE_MAX}"
    else
        TEST_RESULT=1
        echo "Error CH1_PEAK_VALUE, meas: ${CH1_PEAK_VALUE}, min: ${CH1_PEAK_VALUE_MIN}, max: ${CH1_PEAK_VALUE_MAX}"
    fi
    fi
    return "${TEST_RESULT}"
}

test_sfdr() {
    # global in:
    # SFDR_LEVEL
    # CH0_PEAK_VALUE
    # CH1_PEAK_VALUE
    # CH0_PEAK_FREQ_MIN
    # CH0_PEAK_FREQ_MAX
    # CH1_PEAK_FREQ_MIN
    # CH1_PEAK_FREQ_MAX

    local CHANNEL=$1

    local SPECTRUM_RESULT="$(spectrum -t -m 1 -M 62500000 -C)"
    local CH0_LEVEL=$(bc -l <<< "${CH0_PEAK_VALUE} - ${SFDR_LEVEL}")
    local CH1_LEVEL=$(bc -l <<< "${CH1_PEAK_VALUE} - ${SFDR_LEVEL}")

    VAR=$(spectrum_sfdr_test.py \
        --ch0-freq-min "${CH0_PEAK_FREQ_MIN}" \
        --ch0-freq-max "${CH0_PEAK_FREQ_MAX}" \
        --ch0-level "${CH0_LEVEL}" \
        --ch1-freq-min "${CH1_PEAK_FREQ_MIN}" \
        --ch1-freq-max "${CH1_PEAK_FREQ_MAX}" \
        --ch1-level "${CH1_LEVEL}" \
        --ch-mode "${CHANNEL}" \
        <<< "${SPECTRUM_RESULT}")
    local TEST_STATUS=$?
    echo "$VAR"
    CH0_VAL=$(gawk 'match($0, /^CH0: carrier value:(.+), floor noise:(.+), avg noise:(.+), min noise: (.+), max noise: (.+)$/, a) {print a[1]}' <<< "${VAR}")
    CH0_AVG=$(gawk 'match($0, /^CH0: carrier value:(.+), floor noise:(.+), avg noise:(.+), min noise: (.+), max noise: (.+)$/, a) {print a[3]}' <<< "${VAR}")
    CH0_MIN=$(gawk 'match($0, /^CH0: carrier value:(.+), floor noise:(.+), avg noise:(.+), min noise: (.+), max noise: (.+)$/, a) {print a[4]}' <<< "${VAR}")
    CH0_MAX=$(gawk 'match($0, /^CH0: carrier value:(.+), floor noise:(.+), avg noise:(.+), min noise: (.+), max noise: (.+)$/, a) {print a[5]}' <<< "${VAR}")
    CH1_VAL=$(gawk 'match($0, /^CH1: carrier value:(.+), floor noise:(.+), avg noise:(.+), min noise: (.+), max noise: (.+)$/, a) {print a[1]}' <<< "${VAR}")
    CH1_AVG=$(gawk 'match($0, /^CH1: carrier value:(.+), floor noise:(.+), avg noise:(.+), min noise: (.+), max noise: (.+)$/, a) {print a[3]}' <<< "${VAR}")
    CH1_MIN=$(gawk 'match($0, /^CH1: carrier value:(.+), floor noise:(.+), avg noise:(.+), min noise: (.+), max noise: (.+)$/, a) {print a[4]}' <<< "${VAR}")
    CH1_MAX=$(gawk 'match($0, /^CH1: carrier value:(.+), floor noise:(.+), avg noise:(.+), min noise: (.+), max noise: (.+)$/, a) {print a[5]}' <<< "${VAR}")


#    echo "$CH0_VAL"
#    echo "$CH0_AVG"
#    echo "$CH0_MIN"
#    echo "$CH0_MAX"

#    echo "$CH1_VAL"
#    echo "$CH1_AVG"
#    echo "$CH1_MIN"
#    echo "$CH1_MAX"

    return "$TEST_STATUS"
}

disable_generator() { 
  generate 1 0 0
  generate 2 0 0
  sleep 1
}

# FPGA firmware
cat '/opt/redpitaya/fpga/fpga_0.94.bit' > '/dev/xdevcfg'
sleep 2

# DIO*_P to inputs
monitor 0x40000010 w 0x00
sleep 0.2

# DIO5_N, DIO6_N to outputs
# monitor 0x40000014 w 0x60

# DIO5_N, DIO6_N, DIO7_N to outputs
monitor 0x40000014 w 0xE0
sleep 0.2

# monitor 0x4000001C w 0x60 # DIO5_N = 1, DIO6_N = 1 (IN = external signal)
monitor 0x4000001C w 0xE0 # DIO5_N = 1, DIO6_N = 1, DIO7_N = 1 (IN = external signal, LV)
sleep 1



echo "SFDR EXTERNAL TEST"

# Test 1-1: measurement (external signal)
test_peak_measurement "CH0"
if [ $? -eq "0" ]
then
    echo 'Test 1 (external): SUCCESS'	
else
    echo 'Test 1 (external): FAIL'
    TEST_STATUS=1
fi

# Test 1-2: measurement (external signal)
test_peak_measurement "CH1"
if [ $? -eq "0" ]
then
    echo 'Test 1 (external): SUCCESS'	
else
    echo 'Test 1 (external): FAIL'
    TEST_STATUS=1
fi

# Test 2-1: SFDR (external signal)
test_sfdr "CH0+1"
if [ $? -eq "0" ]
then
    echo 'Test 2 (external): SUCCESS'   
else
    echo 'Test 2 (external): FAIL'
    TEST_STATUS=1
fi




# monitor 0x4000001C w 0x20 # DIO5_N = 1, DIO6_N = 0 (IN = OUT)
monitor 0x4000001C w 0xA0 # DIO5_N = 1, DIO6_N = 0, DIO7_N = 1 (IN = OUT, LV)
sleep 1

# Enable generator
disable_generator
generate 1 0.5 "${GENERATE_CH0_FREQ}" sine
sleep 1

# Test 3: measurement (output)
test_peak_measurement "CH0"
if [ $? -eq "0" ]
then
    echo 'Test 1 (internal): SUCCESS'
else
    echo 'Test 1 (internal): FAIL'
    TEST_STATUS=1
fi

# Enable generator
disable_generator
generate 2 0.5 "${GENERATE_CH1_FREQ}" sine
sleep 1

# Test 4: measurement (output)
test_peak_measurement "CH1"
if [ $? -eq "0" ]
then
    echo 'Test 1 (internal): SUCCESS'
else
    echo 'Test 1 (internal): FAIL'
    TEST_STATUS=1
fi


# Enable generator
disable_generator
generate 1 0.5 "${GENERATE_CH0_FREQ}" sine
sleep 1


# Test 5: SFDR (output)
test_sfdr "CH0+1"
if [ $? -eq "0" ]
then
    echo 'Test 2 OUT1 (internal): SUCCESS'
else
    echo 'Test 2 OUT1 (internal): FAIL'
    TEST_STATUS=1
fi

RES_SFDR1="$CH0_VAL $CH0_MIN $CH0_MAX $CH0_AVG $CH1_VAL $CH1_MIN $CH1_MAX $CH1_AVG"

# Enable generator
disable_generator
generate 2 0.5 "${GENERATE_CH1_FREQ}" sine
sleep 1


# Test 6: SFDR (output)
test_sfdr "CH0+1"
if [ $? -eq "0" ]
then
    echo 'Test 3 OUT2 (internal): SUCCESS'
else
    echo 'Test 3 OUT2 (internal): FAIL'
    TEST_STATUS=1
fi
RES_SFDR2="$CH0_VAL $CH0_MIN $CH0_MAX $CH0_AVG $CH1_VAL $CH1_MIN $CH1_MAX $CH1_AVG"
RES_SFDR="$RES_SFDR1 $RES_SFDR2"
echo "RES_SFDR=$RES_SFDR;"
# monitor 0x4000001C w 0x00 # DIO5_N = 0, DIO6_N = 0 (IN = GND)
monitor 0x4000001C w 0x80 # DIO5_N = 0, DIO6_N = 0, DIO7_N = 1 (IN = GND, LV)
sleep 1
echo "SFDR_TEST_STATUS=$TEST_STATUS"
disable_generator

exit "$TEST_STATUS"
