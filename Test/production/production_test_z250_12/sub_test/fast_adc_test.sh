#!/bin/bash
source ./sub_test/common_func.sh

# fast ADCs and DACs data acquisitions
export SIG_FREQ=1000
export SIG_AMPL=2 # P-to-P
export ADC_BUFF_SIZE=16384

MAX_ABS_OFFS_HIGH_GAIN=500
MAX_ABS_OFFS_LOW_GAIN=300 # was 250

MAX_NOISE_STD=15 # Old value 8 -> Change to 25 because od +15V switching PS on the test board
MAX_NOISE_STD_NO_DEC=25 # Old value 15 -> Change to 25 because od +15V switching PS on the test board
MAX_NOISE_P2P=80  # # Old value 60 -> Change to 25 because od +15V switching PS on the test board

MIN_SIG_STD_HIGH_GAIN=4200
MAX_SIG_STD_HIGH_GAIN=5500
MIN_SIG_STD_LOW_GAIN=170
MAX_SIG_STD_LOW_GAIN=350

MIN_SIG_P2P_HIGH_GAIN=12000
MAX_SIG_P2P_HIGH_GAIN=16000
MIN_SIG_P2P_LOW_GAIN=450
MAX_SIG_P2P_LOW_GAIN=850

echo
echo -e "\e[94m########################################################################\e[0m"
echo -e "\e[94m#            Fast ADCs and DACs test                                   #\e[0m"
echo -e "\e[94m########################################################################\e[0m"
echo
echo "TEST IN AC MODE"
echo "    Acqusition without DAC signal - ADCs with HIGH gain"
echo

STATUS=0


enableK1Pin
sleep 1

# Assure tht DAC signals (ch 1 & 2) are OFF
disableGenerator
sleep 1

export MAX_DEVIATION=$MAX_NOISE_STD
export MIN_DEVIATION=0
export MAX_OFFSET=$MAX_ABS_OFFS_HIGH_GAIN
export ENABLE_NOISE_NO_DEC=1
export MAX_NOISE_NO_DEC=$MAX_NOISE_STD_NO_DEC
export MAX_P2P=$MAX_NOISE_P2P
export MIN_P2P=0

./sub_test/fast_adc_sub_test.sh
if [[ $? == 1 ]]
then
STATUS=$?
fi

# Set DAC value to proper counts / frequency for both channels
echo
echo "    Acqusition with DAC signal ($SIG_AMPL Vpp / $SIG_FREQ Hz) - ADCs with HIGH gain"
echo

# Turn the DAC signal generator on on both channels
$C_GENERATE 1 $SIG_AMPL $SIG_FREQ x1 sine
$C_GENERATE 2 $SIG_AMPL $SIG_FREQ x1 sine
sleep 1

export MAX_DEVIATION=$MAX_SIG_STD_HIGH_GAIN
export MIN_DEVIATION=$MIN_SIG_STD_HIGH_GAIN
export MAX_OFFSET=$MAX_ABS_OFFS_HIGH_GAIN
export ENABLE_NOISE_NO_DEC=0
export MAX_NOISE_NO_DEC=$MAX_NOISE_STD_NO_DEC
export MAX_P2P=$MAX_SIG_P2P_HIGH_GAIN
export MIN_P2P=$MIN_SIG_P2P_HIGH_GAIN

./sub_test/fast_adc_sub_test.sh
if [[ $? == 1 ]]
then
STATUS=$?
fi


echo
echo "    Acqusition without DAC signal - ADCs with attenuator 1:20"
echo

# Turn the DAC signal generator OFF on both channels (ch 1 & 2)
disableGenerator
sleep 0.2

export MAX_DEVIATION=$MAX_NOISE_STD
export MIN_DEVIATION=0
export MAX_OFFSET=$MAX_ABS_OFFS_LOW_GAIN
export ENABLE_NOISE_NO_DEC=1
export MAX_NOISE_NO_DEC=$MAX_NOISE_STD_NO_DEC
export MAX_P2P=$MAX_NOISE_P2P
export MIN_P2P=0
export ACQ_PARAM="-1 20 -2 20"

./sub_test/fast_adc_sub_test.sh
if [[ $? == 1 ]]
then
STATUS=$?
fi

echo
echo "    Acqusition with DAC signal ($SIG_AMPL Vpp / $SIG_FREQ Hz) - ADCs with attenuator 1:20"
echo

# Turn the DAC signal generator on on both channels
$C_GENERATE 1 $SIG_AMPL $SIG_FREQ
$C_GENERATE 2 $SIG_AMPL $SIG_FREQ
sleep 0.5

export MAX_DEVIATION=$MAX_SIG_STD_LOW_GAIN
export MIN_DEVIATION=$MIN_SIG_STD_LOW_GAIN
export MAX_OFFSET=$MAX_ABS_OFFS_LOW_GAIN
export ENABLE_NOISE_NO_DEC=0
export MAX_NOISE_NO_DEC=$MAX_NOISE_STD_NO_DEC
export MAX_P2P=$MAX_SIG_P2P_LOW_GAIN
export MIN_P2P=$MIN_SIG_P2P_LOW_GAIN
export ACQ_PARAM="-1 20 -2 20"

./sub_test/fast_adc_sub_test.sh
if [[ $? == 1 ]]
then
STATUS=$?
fi

echo
echo "TEST IN DC MODE"
echo "    Acqusition without DAC signal - ADCs with HIGH gain"
echo

# Assure tht DAC signals (ch 1 & 2) are OFF
disableGenerator
sleep 1

export MAX_DEVIATION=$MAX_NOISE_STD
export MIN_DEVIATION=0
export MAX_OFFSET=$MAX_ABS_OFFS_HIGH_GAIN
export ENABLE_NOISE_NO_DEC=1
export MAX_NOISE_NO_DEC=$MAX_NOISE_STD_NO_DEC
export MAX_P2P=$MAX_NOISE_P2P
export MIN_P2P=0
export ACQ_PARAM="-d B"

./sub_test/fast_adc_sub_test.sh
if [[ $? == 1 ]]
then
STATUS=$?
fi

# Set DAC value to proper counts / frequency for both channels
echo
echo "    Acqusition with DAC signal ($SIG_AMPL Vpp / $SIG_FREQ Hz) - ADCs with HIGH gain"
echo

# Turn the DAC signal generator on on both channels
$C_GENERATE 1 $SIG_AMPL $SIG_FREQ x1 sine
$C_GENERATE 2 $SIG_AMPL $SIG_FREQ x1 sine
sleep 1

export MAX_DEVIATION=$MAX_SIG_STD_HIGH_GAIN
export MIN_DEVIATION=$MIN_SIG_STD_HIGH_GAIN
export MAX_OFFSET=$MAX_ABS_OFFS_HIGH_GAIN
export ENABLE_NOISE_NO_DEC=0
export MAX_NOISE_NO_DEC=$MAX_NOISE_STD_NO_DEC
export MAX_P2P=$MAX_SIG_P2P_HIGH_GAIN
export MIN_P2P=$MIN_SIG_P2P_HIGH_GAIN
export ACQ_PARAM="-d B"

./sub_test/fast_adc_sub_test.sh
if [[ $? == 1 ]]
then
STATUS=$?
fi


echo
echo "    Acqusition without DAC signal - ADCs with attenuator 1:20"
echo

# Turn the DAC signal generator OFF on both channels (ch 1 & 2)
disableGenerator
sleep 0.2

export MAX_DEVIATION=$MAX_NOISE_STD
export MIN_DEVIATION=0
export MAX_OFFSET=$MAX_ABS_OFFS_LOW_GAIN
export ENABLE_NOISE_NO_DEC=1
export MAX_NOISE_NO_DEC=$MAX_NOISE_STD_NO_DEC
export MAX_P2P=$MAX_NOISE_P2P
export MIN_P2P=0
export ACQ_PARAM="-1 20 -2 20 -d B"

./sub_test/fast_adc_sub_test.sh
if [[ $? == 1 ]]
then
STATUS=$?
fi

echo
echo "    Acqusition with DAC signal ($SIG_AMPL Vpp / $SIG_FREQ Hz) - ADCs with attenuator 1:20"
echo

# Turn the DAC signal generator on on both channels
$C_GENERATE 1 $SIG_AMPL $SIG_FREQ
$C_GENERATE 2 $SIG_AMPL $SIG_FREQ
sleep 0.5

export MAX_DEVIATION=$MAX_SIG_STD_LOW_GAIN
export MIN_DEVIATION=$MIN_SIG_STD_LOW_GAIN
export MAX_OFFSET=$MAX_ABS_OFFS_LOW_GAIN
export ENABLE_NOISE_NO_DEC=0
export MAX_NOISE_NO_DEC=$MAX_NOISE_STD_NO_DEC
export MAX_P2P=$MAX_SIG_P2P_LOW_GAIN
export MIN_P2P=$MIN_SIG_P2P_LOW_GAIN
export ACQ_PARAM="-1 20 -2 20 -d B"

./sub_test/fast_adc_sub_test.sh
if [[ $? == 1 ]]
then
STATUS=$?
fi

# Set DAC value to 0 for both channels (1 & 2)
echo
echo "    Restoring DAC signals and ADC gain to idle conditions"
disableGenerator
echo

disableAllDIOPin

if [[ $STATUS == 0 ]]
then
    print_test_ok
else
    print_test_fail
fi

sleep 1

exit $STATUS