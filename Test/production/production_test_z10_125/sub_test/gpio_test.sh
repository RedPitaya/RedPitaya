#!/bin/bash
source ./sub_test/common_func.sh


echo
echo -e "\e[94m########################################################################\e[0m"
echo -e "\e[94m#            Test of GPIO connection                                   #\e[0m"
echo -e "\e[94m########################################################################\e[0m"
echo

STATUS=0


# Configure both ports as input ports (avoid N and P in output configuration)
# Here we change DIOx_N and DIOx_P so relays can switch uncontrolabre if the EARTH is not connected to the TEST BOARD.
# But even if this happens (relay switching) it will not affect on the TEST 5.
# After TEST 5 we will set DIOx_P to inputs and DIOx_N to outputs to prevent relay uncontrolabre switching
$C_MONITOR 0x40000010 0x00
sleep 0.2
$C_MONITOR 0x40000014 0x00
sleep 0.2

# P->N configuration -Configure P as output port
$C_MONITOR 0x40000010 0xFF
# Set P-port output value
VALUE=0x000000ab
$C_MONITOR 0x40000018 $VALUE

# Read N-port input value
READ_VALUE=$($C_MONITOR 0x40000024)

# Check if they are the same
if [[ $READ_VALUE != $VALUE ]]
then
    echo "    GPIO (P->N check): read byte ($READ_VALUE) differs from written byte ($VALUE). Is IDS testboard connected?"

    # Provide information about what bits are wrong
    BIT_DEC_XOR=$(($READ_VALUE ^ $VALUE))
    BIT_BIN_XOR=$(echo "obase=2; $BIT_DEC_XOR" | bc )
    PADDED_XOR=$(echo | awk -v x=$BIT_BIN_XOR '{for(i=length(x);i<8;i++)x="0" x;}END{print x}')

    echo -n "    XOR performed on the two bytes returns: $PADDED_XOR, LSB on the right "
    print_fail
    STATUS=1
else
    echo -n  "    GPIO (P->N check): read byte ($READ_VALUE) matches written byte ($VALUE) "
    print_ok
fi

# N->P configuration - Configure P back as input port, and N as output port
$C_MONITOR 0x40000010 0x00
sleep 0.2
$C_MONITOR 0x40000014 0xFF

# Set N-port output value
VALUE=0x000000f8
$C_MONITOR 0x4000001C $VALUE

# Read P-port input value
READ_VALUE=$($C_MONITOR 0x40000020)

# Check if they are the same
if [[ $READ_VALUE != $VALUE ]]
then
    echo "    GPIO (N->P check): read byte ($READ_VALUE) differs from written byte ($VALUE). Is IDS testboard connected?"

    # Provide information about what bits are wrong
    BIT_DEC_XOR=$(($READ_VALUE ^ $VALUE))
    BIT_BIN_XOR=$(echo "obase=2; $BIT_DEC_XOR" | bc )
    PADDED_XOR=$(echo | awk -v x=$BIT_BIN_XOR '{for(i=length(x);i<8;i++)x="0" x;}END{print x}')

    echo -n  "    XOR performed on the two bytes returns: $PADDED_XOR, LSB on the right "
    print_fail
    STATUS=1
else
    echo -n  "    GPIO (N->P check): read byte ($READ_VALUE) matches written byte ($VALUE) "
    print_ok
fi

sleep 1

# DIO test is finished- Configure DIOx_P to inputs and DIOx_N to outputs to prevent Relay misbehaviour
$C_MONITOR 0x40000010 0x00 # -> Set P to inputs
sleep 0.2
$C_MONITOR 0x40000014 0x00 # -> Set N to outputs
sleep 0.2

echo
if [[ $STATUS == 0 ]]
then
    print_test_ok
    SetBitState 0x20
else
    print_test_fail
fi
echo 
sleep 1

exit $STATUS