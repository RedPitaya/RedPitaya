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
export G_LOCAL_SERVER_IP='10.0.1.160'
export G_LOCAL_SERVER_PASS='redpitaya'
export G_LOCAL_SERVER_DIR='/home/redpitaya/Desktop/Test_LOGS'
export G_LOCAL_USER='redpitaya'

# For loging on the Red Pitaya itself (practicali the same as loging in to SD card it sohuld )
export G_LOG_MNT_FOLDER='/mnt/log'

# Main commands shortcuts
export C_A_SIGNAL="$G_SD_CARD_PATH/bin/analyze_signal"
export C_MONITOR="$G_SD_CARD_PATH/bin/monitor"
export C_PRINTENV="fw_printenv"
export C_SETENV="fw_setenv"
export C_GENERATE="$G_SD_CARD_PATH/bin/generate"
export C_ACQUIRE="$G_SD_CARD_PATH/bin/acquire"
export C_CALIB="$G_SD_CARD_PATH/bin/calib"
export C_UART_TOOL="$G_SD_CARD_PATH/bin/uart_prod_tool"

#enable tests
# export G_CONSOLE_TEST=1
# export G_SPI_TEST=1
# export G_ETHERNET_TEST=1
# export G_POWER_TEST=1
# export G_USB_TEST=1
# export G_SATA_TEST=1
# export G_GPIO_TEST=1
# export G_SLOW_ADC_DAC_TEST=1
# export G_FAST_ADC_DAC_TEST=1
# export G_FAST_ADC_BIT_TEST=1
 export G_CALIBRATION=1

# RUN MAIN TEST
./main_test.sh
