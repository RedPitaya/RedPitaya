#!/bin/bash

## include common functions
source ./sub_test/common_func.sh

echo "START MAIN TEST"
InitBitState
load_fpga_0_94
sleep 1

echo "Load ADC/DAC configuration"
$C_POWER_ON_TOOL -C

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
if [[ $G_CONSOLE_TEST == 1 ]]
then
    ./sub_test/test_console_and_mac.sh
    if [[ "$?" = '1' ]]
    then
        SetBackLog "Console and EEPROM write" $(print_ok)
    else 
        SetBackLog "Console and EEPROM write" $(print_fail)
    fi
else
    SetBackLog "Console and EEPROM write" $(print_skip)    
fi

echo
echo
# Get identificator if zynq
readZynqCode
echo "Zynq code received $ZYNQ_CODE"
PrintToFile "zynq_code" "$ZYNQ_CODE"

###############################################################################
# memory test
###############################################################################
if [[ $G_MEM_TEST == 1 ]]
then
    ./sub_test/memtest.sh
    if [[ "$?" = '0' ]]
    then
        SetBackLog "Memory check" $(print_ok)
        PrintToFile "mem_test" "1"
    else 
        SetBackLog "Memory check" $(print_fail)
        PrintToFile "mem_test" "0"
    fi
else
    SetBackLog "Memory check" $(print_skip)   
    PrintToFile "mem_test" "2" 
fi

###############################################################################
# I2C and SPI bus functionality test
###############################################################################
if [[ $G_SPI_TEST == 1 ]]
then
    ./sub_test/i2c_spi_test.sh
    if [[ "$?" = '0' ]]
    then
        SetBackLog "I2C and SPI bus functionality" $(print_ok)
    else 
        SetBackLog "I2C and SPI bus functionality" $(print_fail)
    fi
else
    SetBackLog "I2C and SPI bus functionality" $(print_skip)    
fi

###############################################################################
# Ethernet network test
###############################################################################
if [[ $G_ETHERNET_TEST == 1 ]]
then
    ./sub_test/ethernet_test.sh
    if [[ "$?" = '0' ]]
    then
        SetBackLog "Ethernet" $(print_ok)
    else 
        SetBackLog "Ethernet" $(print_fail)
    fi
else
    SetBackLog "Ethernet" $(print_skip)    
fi

###############################################################################
# Temperature and Power supply voltages test
###############################################################################
if [[ $G_POWER_TEST == 1 ]]
then
    ./sub_test/temperatures_and_power_test.sh
    if [[ "$?" = '0' ]]
    then
        SetBackLog "Temperature and Power" $(print_ok)
    else 
        SetBackLog "Temperature and Power" $(print_fail)
    fi
else
    SetBackLog "Temperature and Power" $(print_skip)  
fi

###############################################################################
# Test of USB data 
###############################################################################
if [[ $G_USB_TEST == 1 ]]
then
    ./sub_test/usb_test.sh
    if [[ "$?" = '0' ]]
    then
        SetBackLog "USB" $(print_ok)
    else 
        SetBackLog "USB" $(print_fail)
    fi
else
    SetBackLog "USB" $(print_skip)  
fi

###############################################################################
# Test of SATA loopback connection 
###############################################################################
if [[ $G_SATA_TEST == 1 ]]
then
    ./sub_test/sata_test.sh
    if [[ "$?" = '0' ]]
    then
        SetBackLog "SATA loopback" $(print_ok)
    else 
        SetBackLog "SATA loopback" $(print_fail)
    fi
else
    SetBackLog "SATA loopback" $(print_skip)  
fi

###############################################################################
# Test of GPIO connection 
###############################################################################
if [[ $G_GPIO_TEST == 1 ]]
then
    ./sub_test/gpio_test.sh
    if [[ "$?" = '0' ]]
    then
        SetBackLog "GPIO" $(print_ok)
    else 
        SetBackLog "GPIO" $(print_fail)
    fi
else
    SetBackLog "GPIO" $(print_skip)  
fi

###############################################################################
# Slow ADCs and DACs test 
###############################################################################
if [[ $G_SLOW_ADC_DAC_TEST == 1 ]]
then
    ./sub_test/slow_adc_dac_test.sh
    if [[ "$?" = '0' ]]
    then
        SetBackLog "Slow ADC and DAC" $(print_ok)
    else 
        SetBackLog "Slow ADC and DAC" $(print_fail)
    fi
else
    SetBackLog "Slow ADC and DAC" $(print_skip)  
fi

###############################################################################
# Fast ADCs and DACs test 
###############################################################################
if [[ $G_FAST_ADC_DAC_TEST == 1 ]]
then
    ./sub_test/fast_adc_test.sh
    if [[ "$?" = '0' ]]
    then
        SetBackLog "Fast ADC and DAC" $(print_ok)
    else 
        SetBackLog "Fast ADC and DAC" $(print_fail)
    fi
else
    SetBackLog "Fast ADC and DAC" $(print_skip)  
fi

###############################################################################
# Fast ADCs bit analysis
###############################################################################
if [[ $G_FAST_ADC_BIT_TEST == 1 ]]
then
    ./sub_test/fast_adc_bit_test.sh
    if [[ "$?" = '0' ]]
    then
        SetBackLog "Fast ADC and DAC bit" $(print_ok)
    else 
        SetBackLog "Fast ADC and DAC bit" $(print_fail)
    fi
else
    SetBackLog "Fast ADC and DAC bit" $(print_skip)  
fi

###############################################################################
# ADCs and DACs CALIBRATION
###############################################################################
if [[ $G_CALIBRATION == 1 ]]
then
    ./sub_test/calibration.sh
    if [[ "$?" = '0' ]]
    then
        SetBackLog "ADC and DAC calibration" $(print_ok)
        PrintToFile "calib_test" "1"
    else 
        SetBackLog "ADC and DAC calibration" $(print_fail)
        PrintToFile "calib_test" "0"
    fi
else
    SetBackLog "ADC and DAC calibration" $(print_skip)  
    PrintToFile "calib_test" "2"
fi

###############################################################################
# DAC test
###############################################################################
if [[ $G_FAST_DAC_TEST == 1 ]]
then
    ./sub_test/fast_dac_test.sh
    if [[ "$?" = '0' ]]
    then
        SetBackLog "DAC" $(print_ok)
    else 
        SetBackLog "DAC" $(print_fail)
    fi
else
    SetBackLog "DAC" $(print_skip)  
fi

###############################################################################
# Pll test
###############################################################################
if [[ $G_PLL_TEST == 1 ]]
then
    ./sub_test/pll_test.sh
    if [[ "$?" = '0' ]]
    then
        SetBackLog "PLL" $(print_ok)
    else 
        SetBackLog "PLL" $(print_fail)
    fi
else
    SetBackLog "PLL" $(print_skip)  
fi

###############################################################################
# External trigger test
###############################################################################
if [[ $G_EXT_TRIGGER_TEST == 1 ]]
then
    ./sub_test/ext_trigger_test.sh
    if [[ "$?" = '0' ]]
    then
        SetBackLog "External trigger" $(print_ok)
    else 
        SetBackLog "External trigger" $(print_fail)
    fi
else
    SetBackLog "External trigger" $(print_skip)  
fi

echo
./sub_test/print_result.sh
./sub_test/save_result.sh
echo
echo "END MAIN TEST"
