#!/bin/bash
source ./sub_test/common_func.sh


echo
echo -e "\e[94m########################################################################\e[0m"
echo -e "\e[94m#            Test of SATA loopback connection                          #\e[0m"
echo -e "\e[94m########################################################################\e[0m"
echo

STATUS=0

# SATA BER test
N_SATA_CYC=5 # Old value was 10
SEC_PER_CYC=2

# TF rates expressed in W/s (word is 16 bits)
EXP_SATA_RATE=$((125000000/32))
TOLERANCE_PERC=2
MIN_SATA_RATE=$(($EXP_SATA_RATE-$EXP_SATA_RATE*$TOLERANCE_PERC/100))
MAX_SATA_RATE=$(($EXP_SATA_RATE+$EXP_SATA_RATE*$TOLERANCE_PERC/100))



# Enable TX, train the unit and check the expected value
echo "    Training..."
$C_MONITOR 0x40500000 0x1
sleep 0.2
$C_MONITOR 0x40500004 0x3
sleep 0.2
$C_MONITOR 0x40500000 0x3
sleep 0.2
$C_MONITOR 0x40500008 0x1
sleep 0.2
TRAIN_VALUE=$($C_MONITOR 0x40500008)
sleep 0.2

# Disable train
$C_MONITOR 0x40500008 w 0x0
sleep 0.2

echo "    Training value is $TRAIN_VALUE"
echo

# Send test pattern, reset counters and enable them again
echo "    Sending test pattern, resetting and enabling counters..."
echo
$C_MONITOR 0x40500004 w 0x5
sleep 0.2
$C_MONITOR 0x40500010 w 0x1
sleep 0.2
$C_MONITOR 0x40500010 w 0x0
sleep 0.2


# Check periodically the counters
for ind in $(seq 1 1 $N_SATA_CYC)
do
    ERR_CNT=$($C_MONITOR 0x40500014)
    sleep 0.2
    TX_CNT=$($C_MONITOR 0x40500018)
    sleep 0.2

    TX_DEC=$(printf "%d" $TX_CNT)
    ERR_DEC=$(printf "%d" $ERR_CNT)

    # Calculate the transfer rate
    if [ $ind -gt 1 ]
    then
        TX_RATE=$((($TX_DEC-$OLD_TX_DEC)/$SEC_PER_CYC))
        echo "    $ind: Transfer count is $TX_DEC, error count is $ERR_DEC, transfer rate is $TX_RATE expected is $EXP_SATA_RATE "

        # Check if performances are within expectations
        if [ $ERR_DEC -gt 0 ]; then
            echo "$ind:   Error count $ERR_DEC out of specifications"
            STATUS=1
        fi

        # IMP: transferred words are stored in a 8bit register. Sometimes negative rate is shown due to register reset
        if [ $TX_RATE -gt $MAX_SATA_RATE ] || [ $TX_RATE -lt $MIN_SATA_RATE ]; then
            echo "$ind:   Tx rate $TX_RATE, between $TX_DEC and $OLD_TX_DEC, out of specification"
            STATUS=1
        fi

    else
        echo "    $ind: Transfer count is $TX_DEC, error count is $ERR_DEC"
    fi

    OLD_TX_DEC=$TX_DEC

    # every cycle takes in total SEC_PER_CYC seconds, should wait SEC_PER_CYC-0.4
    sleep 1.6
done


echo
if [[ $STATUS == 0 ]]
then
    print_test_ok
    SetBitState 0x10
else
    print_test_fail
fi
echo

sleep 1

exit $STATUS
