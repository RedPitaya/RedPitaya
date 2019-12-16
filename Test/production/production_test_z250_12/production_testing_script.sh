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
export C_MONITOR="$SD_CARD_PATH/bin/monitor"
export C_PRINTENV="fw_printenv"
export C_SETENV="fw_setenv"
export C_GENERATE="$SD_CARD_PATH/bin/generate"
export C_ACQUIRE="$SD_CARD_PATH/bin/acquire"
export C_CALIB="$SD_CARD_PATH/bin/calib"

###############################################################################
# define GLOBAL FUNCTIONS
###############################################################################

hexToDec() {
    local VALUE
    read VALUE
    printf "%d\n" "$VALUE"
}
export -f hexToDec

get_rtrn(){
    echo `echo $1|cut --delimiter=, -f $2`
}
export -f get_rtrn

# RUN MAIN TEST
./main_test.sh
