#!/bin/bash

###############################################################################
#  Script for the manifacturing test of Red Pitaya units
#
###############################################################################

###############################################################################
# define GLOBAL VAIABLES
###############################################################################

# Path variables
export G_SD_CARD_PATH='/opt/redpitaya'
export G_USB_DEVICE="/dev/sda1"
export G_USB_MOUNT_FOLDER="/mnt/usb"

# Production PC/SERVER variables
export G_LOCAL_SERVER_IP='192.168.1.2'
export G_LOCAL_SERVER_PASS='redpitaya'
export G_LOCAL_SERVER_DIR='/home/redpitaya/Desktop/Test_LOGS'
export G_LOCAL_USER='redpitaya'
export G_PASS="xBqRnS8r"


# For loging on the Red Pitaya itself (practicali the same as loging in to SD card it sohuld )
export G_LOG_MNT_FOLDER='/mnt/log'
export G_LOG_FILENAME='manuf_test.log'

# Main commands shortcuts
export C_A_SIGNAL="$G_SD_CARD_PATH/bin/analyze_signal"
export C_MONITOR="$G_SD_CARD_PATH/bin/monitor"
export C_PRINTENV="fw_printenv"
export C_SETENV="fw_setenv"
export C_GENERATE="$G_SD_CARD_PATH/bin/generate"
export C_ACQUIRE="$G_SD_CARD_PATH/bin/acquire"
export C_CALIB="$G_SD_CARD_PATH/bin/calib"
export C_UART_TOOL="$G_SD_CARD_PATH/bin/uart_prod_tool"
export C_POWER_ON_TOOL="$G_SD_CARD_PATH/bin/rp_power_on"
export C_CAPACITOR_CALIB_TOOL="$G_SD_CARD_PATH/bin/capacitor_calib_tool"
export C_MEM_TEST_TOOL="memtester"

#enable tests
export G_CONSOLE_TEST=1
#export G_MEM_TEST=1
export G_SPI_TEST=1
export G_ETHERNET_TEST=1
export G_POWER_TEST=1
export G_USB_TEST=1
export G_SATA_TEST=1
export G_GPIO_TEST=1
export G_SLOW_ADC_DAC_TEST=1
export G_FAST_ADC_DAC_TEST=1
export G_FAST_ADC_BIT_TEST=1
export G_CALIBRATION=1
export G_FAST_DAC_TEST=1
export G_PLL_TEST=1
export G_EXT_TRIGGER_TEST=1

#enable save result
export G_SAVE_TO_PC=1

# make temprary directory
export TEST_TMP_DIR=$(mktemp -d)

# RUN MAIN TEST
./main_test.sh

rm -rf $TEST_TMP_DIR
