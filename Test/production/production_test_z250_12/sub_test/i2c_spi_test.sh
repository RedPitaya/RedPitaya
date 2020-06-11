#!/bin/bash
source ./sub_test/common_func.sh

###############################################################################
# I2C bus functionality test
###############################################################################

echo
echo -e "\e[94m########################################################################\e[0m"
echo -e "\e[94m#            Test of EEPROM, SPI, I2C                                  #\e[0m"
echo -e "\e[94m########################################################################\e[0m"

STATUS=0
# I2C test configuration
TEST_LABEL='I2C_test'
I2C_TEST_CONFIG='/sys/bus/i2c/devices/0-0051/eeprom 0x1800 0x0400'

# Verify the I2C bus functionality through the external EEPROM memory
echo
echo "Verifying the I2C bus functionality..."
echo

# Read the EEPROM variable foreseen for this test
echo "Reading the test string from external EEPROM..."
echo
sleep 0.2

# Create config file for EEPROM on the test board
I2C_TEST_CONFIG_FILE="$(mktemp)"
echo "$I2C_TEST_CONFIG" > "$I2C_TEST_CONFIG_FILE"

READ_LABEL=$($C_PRINTENV -c "$I2C_TEST_CONFIG_FILE" test_label 2> /dev/null | grep -Po '(?<=test_label\=).+(?=)')
echo "Test lable is: $TEST_LABEL"
echo "Read lable is: $READ_LABEL"

if [[ "$READ_LABEL" != "$TEST_LABEL" ]]
then
    echo "External EEPROM read-back doesn't work. I2C might not work correctly"
    STATUS=1
    sleep 1
    print_test_fail
    $C_UART_TOOL 'LED:RED 0 3' -s
else
    print_test_ok
    $C_UART_TOOL 'LED:GRN 0 3' -s
    SetBitState 0x02
fi



rm "$I2C_TEST_CONFIG_FILE"

# TEST i2c and spi bus
# 
echo
echo
echo "Test of internal i2c and spi"

sleep 1
i2c_spi_test_tool
if [[ $? != 0 ]]
then
    print_test_fail
    STATUS=1
    sleep 1
    $C_UART_TOOL 'LED:RED 0 3' -s
else 
    print_test_ok
    SetBitState 0x04
fi

# TEST SPI this test board via external connector 
# 
echo
echo
echo "Test of external spi"
spi_ext_test_tool
if [[ $? != 0 ]]
then
    print_test_fail
    STATUS=1
    sleep 1
else 
    print_test_ok
    $C_UART_TOOL 'LED:GRN 0 2' -s
fi

exit $STATUS