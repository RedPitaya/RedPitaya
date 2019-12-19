#!/bin/bash

## include common functions
source ./sub_test/common_func.sh

echo "START MAIN TEST"
echo "LOAD FPGA IMAGE" 
cat /opt/redpitaya/fpga/fpga_0.94.bit > /dev/xdevcfg
sleep 2
echo "FPGA LOADED SUCCESSFULLY"

echo
echo
echo -e "\e[94m########################################################################\e[0m"
echo -e "\e[94m#           Test communication with rest setup (UART)                  #\e[0m"
echo -e "\e[94m########################################################################\e[0m"
$C_UART_TOOL 'INIT' -s
RET=$($C_UART_TOOL 'IDN?')

if [[ "$RET" = 'TD:STEM250:V1.0' ]]
then
    echo -en "UART communication \033[92m[OK]\e[0m"
    # INIT TEST BOARD
    ./init_productions_script.sh

    $C_UART_TOOL 'LED:GRN 0 1' -s
else
    echo -en "UART communication \033[91m[FAIL]\e[0m"
    $C_UART_TOOL 'LED:RED 0 1' -s
fi

echo
echo "LEDs 1-8 will blink for 5 sec - USER: verify"
echo
for i in $(seq 1 1 5)
do
    $C_MONITOR 0x40000030 w 0xFF
    sleep 0.5
    $C_MONITOR 0x40000030 w 0x00
    sleep 0.5
done

########################################################################
#            Check the teminal console functionality                   #
#            Test of EEPROM write                                      #
#        Setting the default calibration parameters into the EEPROM... # 
########################################################################
./sub_test/test_console_and_mac.sh

echo
echo
# Get identificator if zynq
readZynqCode
echo "Zynq code received $ZYNQ_CODE"

###############################################################################
# I2C and SPI bus functionality test
###############################################################################
./sub_test/i2c_spi_test.sh

###############################################################################
# Ethernet network test
###############################################################################
./sub_test/ethernet_test.sh

###############################################################################
# Temperature and Power supply voltages test
###############################################################################
./sub_test/temperatures_and_power_test.sh

###############################################################################
# Test of GPIO connection 
###############################################################################
./sub_test/gpio_test.sh

echo
echo
echo "END MAIN TEST"
