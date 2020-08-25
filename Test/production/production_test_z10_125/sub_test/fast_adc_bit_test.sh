#!/bin/bash
source ./sub_test/common_func.sh

# fast ADCs bit analysis
N_SAMPLES=8192
N_ADC_BITS=14
HALF_ADC_RANGE=8192
SIG_FREQ=1000
SIG_AMPL=2 # P-to-P
ADC_BUFF_SIZE=16384
ADC_FILENAME="adc.sig"
ADC_CH_A_FILENAME="adc_a.sig"
ADC_CH_B_FILENAME="adc_b.sig"

echo
echo -e "\e[94m########################################################################\e[0m"
echo -e "\e[94m#            Fast ADCs bit analysis                                    #\e[0m"
echo -e "\e[94m########################################################################\e[0m"
echo

STATUS=0

echo "    Acquisition with DAC signal ($SIG_AMPL Vpp / $SIG_FREQ Hz) - ADCs with HIGH gain"
echo


# Configure DIOx_P to inputs and DIOx_N to outputs to prevent Relay misbehaviour
$C_MONITOR 0x40000010 w 0x00 # -> Set P to inputs
sleep 0.2
$C_MONITOR 0x40000014 w 0xFF # -> Set N to outputs
sleep 0.2

# Configure relays  DIO5_N = 1, DIO6_N = 0, DIO7_N = 1 (lv jumper settings)
$C_MONITOR 0x4000001C w 0x00 # -> Set N outputs to zero - > reseet
sleep 0.2
$C_MONITOR 0x4000001C w 0xA0 # -> Set DIO5_N=1 -> Configure the ADC in high-gain mode -> DIO7_N = 1
sleep 1


# Turn the DAC signal generator on on both channels
$C_GENERATE 1 $SIG_AMPL $SIG_FREQ sine
$C_GENERATE 2 $SIG_AMPL $SIG_FREQ sine
sleep 1

# Acquire data from both channels - 1024 decimation factor
$C_ACQUIRE $ADC_BUFF_SIZE 1024 -c > /tmp/$ADC_FILENAME
cat /tmp/$ADC_FILENAME | awk '{print $1}' > /tmp/$ADC_CH_A_FILENAME
cat /tmp/$ADC_FILENAME | awk '{print $2}' > /tmp/$ADC_CH_B_FILENAME

STATE_REG=0

# Channel A #
echo "    Channel A bit analysis..."
echo
cnt=0

while read VAL
do
    # acquire a value and add half ADC range to make it positive
    NEW_VAL=$(($VAL+$HALF_ADC_RANGE))

    if [ $cnt -gt 0 ]
    then
        # Evaluate the bit differences and remember what bit changed
        VAL_XOR=$(($NEW_VAL ^ $OLD_VAL))
        STATE_REG=$(($STATE_REG | $VAL_XOR))
    fi

    # remember the value for next comparison
    OLD_VAL=$NEW_VAL

    # Consider only first 100 lines of file
    if [ $cnt -eq $N_SAMPLES ]
    then
        break
    fi

    # increment the loop counter
    cnt=$(($cnt+1))

done < /tmp/$ADC_CH_A_FILENAME

# Evaluate what bits have changed during the test
for i in $(seq $(($N_ADC_BITS-1)) -1 0)
do
    BIT_POWER=$(echo "2^$i" | bc)
    sleep 0.2

    if [ "$BIT_POWER" -gt "$STATE_REG" ]
    then
        echo "    Error: bit $i never changed during the test"
        STATUS=1
    else
        STATE_REG=$(($STATE_REG-$BIT_POWER))
    fi
done


STATE_REG=0

# Channel B #
echo "    Channel B bit analysis..."
echo
cnt=0

while read VAL
do
    # acquire a value and add half ADC range to make it positive
    NEW_VAL=$(($VAL+$HALF_ADC_RANGE))


    if [ $cnt -gt 0 ]
    then
        # Evaluate the bit differences and remember what bit changed
        VAL_XOR=$(($NEW_VAL ^ $OLD_VAL))
        STATE_REG=$(($STATE_REG | $VAL_XOR))
    fi

    # remember the value for next comparison
    OLD_VAL=$NEW_VAL

    # Consider only first 100 lines of file
    if [ $cnt -eq $N_SAMPLES ]
    then
        break
    fi

    # increment the loop counter
    cnt=$(($cnt+1))

done < /tmp/$ADC_CH_B_FILENAME

# Evaluate what bits have changed during the test
for i in $(seq $(($N_ADC_BITS-1)) -1 0)
do
    BIT_POWER=$(echo "2^$i" | bc)
    sleep 0.2

    if [ $BIT_POWER -gt $STATE_REG ]
    then
        echo "    Error: bit $i never changed during the test"
        STATUS=1
    else
        STATE_REG=$(($STATE_REG-$BIT_POWER))
    fi
done

sleep 1

echo "    Restoring DAC signals and ADC gain to idle conditions..."
disableGenerator
echo

disableAllDIOPin

if [[ $STATUS == 0 ]]
then
    print_test_ok
    SetBitState 0x100
else
    print_test_fail
fi
echo

sleep 1

exit $STATUS