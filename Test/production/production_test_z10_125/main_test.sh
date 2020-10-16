#!/bin/bash

## include common functions
source ./sub_test/common_func.sh

echo "START MAIN TEST"
InitBitState
load_fpga_0_94
sleep 1


echo
echo "LEDs 1-8 will blink for 5 sec - USER: verify"
echo
for i in $(seq 1 1 5)
do
    $C_MONITOR 0x40000030 w 0xFE
    sleep 0.5
    $C_MONITOR 0x40000030 w 0x00
    sleep 0.5
done

########################################################################
#            Check the teminal console functionality                   #
#            Test of EEPROM write                                      #
#        Setting the default calibration parameters into the EEPROM... # 
########################################################################
CONSOLE_TEST_RES=0
if [[ $G_CONSOLE_TEST == 1 ]]
then
    ./sub_test/test_console_and_mac.sh
    if [[ "$?" = '1' ]]
    then
        SetBackLog "Console and EEPROM write" $(print_ok)
        CONSOLE_TEST_RES=1
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
SPI_TEST_RES=0
if [[ $G_SPI_TEST == 1 ]]
then
    ./sub_test/i2c_spi_test.sh
    if [[ "$?" = '0' ]]
    then
        SetBackLog "I2C and SPI bus functionality" $(print_ok)
        SPI_TEST_RES=1
    else
        SetBackLog "I2C and SPI bus functionality" $(print_fail)
    fi
else
    SetBackLog "I2C and SPI bus functionality" $(print_skip)
fi

if [[ $CONSOLE_TEST_RES == 1 ]]
then
    if [[ $SPI_TEST_RES == 1 ]]
    then
        RPLight1
        SetBitState 0x01
    fi
fi

###############################################################################
# Ethernet network test
###############################################################################
ETHERNET_TEST_RES=0
if [[ $G_ETHERNET_TEST == 1 ]]
then
    ./sub_test/ethernet_test.sh
    if [[ "$?" = '0' ]]
    then
        SetBackLog "Ethernet" $(print_ok)
        ETHERNET_TEST_RES=1
    else
        SetBackLog "Ethernet" $(print_fail)
    fi
else
    SetBackLog "Ethernet" $(print_skip)
fi

###############################################################################
# Temperature and Power supply voltages test
###############################################################################
POWER_TEST_RES=0
if [[ $G_POWER_TEST == 1 ]]
then
    ./sub_test/temperatures_and_power_test.sh
    if [[ "$?" = '0' ]]
    then
        SetBackLog "Temperature and Power" $(print_ok)
        POWER_TEST_RES=1
    else 
        SetBackLog "Temperature and Power" $(print_fail)
    fi
else
    SetBackLog "Temperature and Power" $(print_skip)  
fi

### Combine result of network and power test
if [[ $ETHERNET_TEST_RES == 1 ]]
then
    if [[ $G_POWER_TEST == 1 ]]
    then
        RPLight2
    fi
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
SATA_TEST_RES=0
if [[ $G_SATA_TEST == 1 ]]
then
    ./sub_test/sata_test.sh
    if [[ "$?" = '0' ]]
    then
        SetBackLog "SATA loopback" $(print_ok)
        SATA_TEST_RES=1
    else
        SetBackLog "SATA loopback" $(print_fail)
    fi
else
    SetBackLog "SATA loopback" $(print_skip)  
fi

###############################################################################
# Test of GPIO connection 
###############################################################################
GPIO_TEST_RES=0
if [[ $G_GPIO_TEST == 1 ]]
then
    ./sub_test/gpio_test.sh
    if [[ "$?" = '0' ]]
    then
        SetBackLog "GPIO" $(print_ok)
        GPIO_TEST_RES=1
    else
        SetBackLog "GPIO" $(print_fail)
    fi
else
    SetBackLog "GPIO" $(print_skip)  
fi

if [[ $SATA_TEST_RES == 1 ]]
then
    if [[ $GPIO_TEST_RES == 1 ]]
    then
        RPLight4
    fi
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
FAST_ADC_TEST_RES=0
if [[ $G_FAST_ADC_DAC_TEST == 1 ]]
then
    ./sub_test/fast_adc_test.sh
    if [[ "$?" = '0' ]]
    then
        SetBackLog "Fast ADC and DAC" $(print_ok)
        FAST_ADC_TEST_RES=1
    else 
        SetBackLog "Fast ADC and DAC" $(print_fail)
    fi
else
    SetBackLog "Fast ADC and DAC" $(print_skip)  
fi

###############################################################################
# Fast ADCs bit analysis
###############################################################################
FAST_ADC_BIT_TEST_RES=0
if [[ $G_FAST_ADC_BIT_TEST == 1 ]]
then
    ./sub_test/fast_adc_bit_test.sh
    if [[ "$?" = '0' ]]
    then
        SetBackLog "Fast ADC and DAC bit" $(print_ok)
        FAST_ADC_BIT_TEST_RES=1
    else 
        SetBackLog "Fast ADC and DAC bit" $(print_fail)
    fi
else
    SetBackLog "Fast ADC and DAC bit" $(print_skip)  
fi

if [[ $FAST_ADC_TEST_RES == 1 ]]
then
    if [[ $FAST_ADC_BIT_TEST_RES == 1 ]]
    then
        RPLight6
    fi
fi


###############################################################################
# ADCs and DACs CALIBRATION
###############################################################################
CheckTestPassForCalib
if [[ $G_CALIBRATION == 1 ]] && [[ $STATUS == 0 ]]
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


echo
./sub_test/print_result.sh
./sub_test/save_result.sh
echo
echo "END MAIN TEST"
