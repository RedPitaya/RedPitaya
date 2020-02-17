#!/bin/bash

###############################################################################
#  Script for the manifacturing test of Red Pitaya units
#
#  Tests are implemented according with quality task #2831:
#  -2 preliminary tests for LEDs and Terminal functionality (red LED status)
#  -8 tests on network, USB OTG, HW and interfaces (LEDs 0-8 status)
#     LED 8 is not used, so in order to not confuse the operator, is turned on
#     if test 7 passes. Can be used if a new test is introduced.
#  rp_* utilities for signal generator and scope must be copied to /opt/ folder
#
#  Test is organized in two phases:
#  1. preliminary tests, and QR CODE reading & writing into EEPROM
#  2. manufacturing tests suite, print results on log file
###############################################################################

# Path variables
SD_CARD_PATH='/opt/redpitaya'
USB_DEVICE="/dev/sda1"
USB_MOUNT_FOLDER="/mnt/usb"
##USB_MOUNT_FOLDER="/media/usb"

# TEST Log variables
LOG_FILENAME='manuf_test.log'

#IP of Red Pitaya server will be best option or production PC
NFS_SERVER_IP='192.168.178.123'
NFS_SERVER_DIR="$NFS_SERVER_IP:/testni"
LOG_MNT_FOLDER='/mnt/log'

# Main commands shortcuts
MONITOR="$SD_CARD_PATH/bin/monitor"
#PRINTENV="$SD_CARD_PATH/redpitaya/bin/fw_printenv"
#SETENV="$SD_CARD_PATH/redpitaya/bin/fw_setenv"
PRINTENV="fw_printenv"
SETENV="fw_setenv"
GENERATE="$SD_CARD_PATH/bin/generate"
ACQUIRE="$SD_CARD_PATH/bin/acquire"
CALIB="$SD_CARD_PATH/bin/calib"

# Calibration parameters set during the process
FE_CH1_FS_G_HI=45870551
FE_CH2_FS_G_HI=45870551
FE_CH1_FS_G_LO=1016267064
FE_CH2_FS_G_LO=1016267064
FE_CH1_DC_offs=78
FE_CH2_DC_offs=25
BE_CH1_FS=42755331
BE_CH2_FS=42755331
BE_CH1_DC_offs=-150
BE_CH2_DC_offs=-150
SOME_eeprom_value=-1430532899
#SOME_eeprom_value is some value in eeprom which is not used for anything but after Crt added hv offset calib values this value also appeard.
FE_CH1_DC_offs_HI=100
FE_CH2_DC_offs_HI=100
#FE_CHx_DC_offs_HI  are dc offset parameters for HV jumper settings
#All calibration parameters in one string
FACTORY_CAL="$FE_CH1_FS_G_HI $FE_CH2_FS_G_HI $FE_CH1_FS_G_LO $FE_CH2_FS_G_LO $FE_CH1_DC_offs $FE_CH2_DC_offs $BE_CH1_FS $BE_CH2_FS $BE_CH1_DC_offs $BE_CH2_DC_offs $SOME_eeprom_value $FE_CH1_DC_offs_HI $FE_CH2_DC_offs_HI"

# I2C test configuration
TEST_LABEL='I2C_test'
SYM_LINK='/dev/eeprom_test'
I2C_TEST_CONFIG="$SYM_LINK  0x1800  0x0400  0x0400"

###############################################################################
# Test variables
###############################################################################

LOG_VAR=''
TEST_GLOBAL_STATUS=0
LOGFILE_STATUS=0
LED_ADDR=0x40000030
SLEEP_BETWEEN_TESTS=1

# network
EXP_LINK_SPEED='(1000/FULL)'
N_PING_PKG=5
PKT_SIZE=16000
PING_IP=$NFS_SERVER_IP
MIN_QR_LENGTH=17             # Set to 50 when using QR scanner
RP_MAC_BEGINNING='00:26:32'

# temperatures & voltages
MIN_TEMP=20
MAX_TEMP=85

TOLERANCE_PERC=5
REF_VCCAUX=1800
MIN_VCCAUX=$(($REF_VCCAUX-$REF_VCCAUX*$TOLERANCE_PERC/100))
MAX_VCCAUX=$(($REF_VCCAUX+$REF_VCCAUX*$TOLERANCE_PERC/100))

REF_VCCBRAM=1000
MIN_VCCBRAM=$(($REF_VCCBRAM-$REF_VCCBRAM*$TOLERANCE_PERC/100))
MAX_VCCBRAM=$(($REF_VCCBRAM+$REF_VCCBRAM*$TOLERANCE_PERC/100))

REF_VCCINT=1000
MIN_VCCINT=$(($REF_VCCINT-$REF_VCCINT*$TOLERANCE_PERC/100))
MAX_VCCINT=$(($REF_VCCINT+$REF_VCCINT*$TOLERANCE_PERC/100))

# USD flash drive test
USB_FILENAME='usb_device_testfile.txt'
USB_NEWFILENAME='usb_device_newname.txt'

# SATA BER test
N_SATA_CYC=10
SEC_PER_CYC=2

# TF rates expressed in W/s (word is 16 bits)
MAX_SATA_RATE=$((125000000/4))
EXP_SATA_RATE=$((125000000/32))
TOLERANCE_PERC=2
MIN_SATA_RATE=$(($EXP_SATA_RATE-$EXP_SATA_RATE*$TOLERANCE_PERC/100))
MAX_SATA_RATE=$(($EXP_SATA_RATE+$EXP_SATA_RATE*$TOLERANCE_PERC/100))

# slow ADCs and DACs
TOLERANCE_PERC=10
REF_RATIO=2
MIN_RATIO=$(($REF_RATIO-$REF_RATIO*$TOLERANCE_PERC/100))
MAX_RATIO=$(($REF_RATIO+$REF_RATIO*$TOLERANCE_PERC/100))

# fast ADCs and DACs data acquisitions
SIG_FREQ=1000
SIG_AMPL=2
ADC_BUFF_SIZE=16384

MAX_ABS_OFFS_HIGH_GAIN=500
MAX_ABS_OFFS_LOW_GAIN=250

MAX_NOISE_STD=8
MAX_NOISE_STD_NO_DEC=15
MAX_NOISE_P2P=60

MIN_SIG_STD_HIGH_GAIN=4500
MAX_SIG_STD_HIGH_GAIN=5500
MIN_SIG_STD_LOW_GAIN=170
MAX_SIG_STD_LOW_GAIN=230

MIN_SIG_P2P_HIGH_GAIN=12000
MAX_SIG_P2P_HIGH_GAIN=16000
MIN_SIG_P2P_LOW_GAIN=450
MAX_SIG_P2P_LOW_GAIN=650

# fast ADCs bit analysis
N_SAMPLES=100
N_ADC_BITS=14
HALF_ADC_RANGE=8192

ADC_FILENAME="adc.sig"
ADC_CH_A_FILENAME="adc_a.sig"
ADC_CH_B_FILENAME="adc_b.sig"

#Calibration parameters LV/HV jumper settings
MAX_VALUE_LV=8000
MAX_VALUE_HV=6000
MAX_OFF_VALUE_LV=300
MAX_OFF_VALUE_HV=300



if [ 1 -eq 0 ] ## Fi is on line # STEP 9: Fast ADCs and DACs CALIBRATION ... skip all until calibration
then



###############################################################################
# PHASE 1
###############################################################################

# check if unit-specific environment variables are already written in the EEPROM
READ_MAC=$( $PRINTENV | grep ethaddr= | awk 'BEGIN {FS="="}{print $2}') > /dev/null 2>&1
READ_NAV=$( $PRINTENV | grep nav_code= | awk 'BEGIN {FS="="}{print $2}') > /dev/null 2>&1
READ_HWREV=$( $PRINTENV | grep hw_rev= | awk 'BEGIN {FS="="}{print $2}') > /dev/null 2>&1
READ_SERIAL=$( $PRINTENV | grep serial= | awk 'BEGIN {FS="="}{print $2}') > /dev/null 2>&1
MAC_BEGIN=${READ_MAC:0:8}

if [[ "$MAC_BEGIN" != "$RP_MAC_BEGINNING" ]] || [[ "$READ_NAV" == "" ]] || [[ "$READ_HWREV" == "" ]] || [[ "$READ_SERIAL" == "" ]]
then 

    ###########################################################################
    # STEP PRELIMINARY 1: Blink all LEDs for 5 seconds
    ###########################################################################
    echo
    echo
    echo "########################################################################"
    echo "#                   PHASE-1    PRELIMINARY-TESTS                       #"
    echo "########################################################################"

    echo
    echo "--- PRELIMINARY TEST 1: LED's functionality test ---"
    echo

    N_REP=5

    #define control for led 8
    # echo 0 > /sys/class/gpio/export                # Zakomentrial za nov OS
    # echo out > /sys/class/gpio/gpio0/direction     # Zakomentrial za nov OS
    #define control for red-led 9
    # echo 7 > /sys/class/gpio/export                # Zakomentrial za nov OS
    # echo out > /sys/class/gpio/gpio7/direction     # Zakomentrial za nov OS

    echo "LEDs 0-7 will blink for $((1*$N_REP)) sec - USER: verify"
    echo
    for i in $(seq 1 1 $N_REP)
    do
        $MONITOR $LED_ADDR w 0xFF
        #echo 1 > /sys/class/gpio/gpio0/value       # Zakomentrial za nov OS
        #echo 1 > /sys/class/gpio/gpio7/value       # Zakomentrial za nov OS
        sleep 0.5

        $MONITOR $LED_ADDR w 0x00
        #echo 0 > /sys/class/gpio/gpio0/value       # Zakomentrial za nov OS
        #echo 0 > /sys/class/gpio/gpio7/value       # Zakomentrial za nov OS
        sleep 0.5
    done


    ###########################################################################
    # STEP PRELIMINARY 2: Check the teminal console functionality
    ###########################################################################
   
    echo
    echo "--- PRELIMINARY TEST 2: Terminal functionality test --- "
    echo
    echo "Checking terminal output - USER: check date update"
    echo

    for i in $(seq 1 1 $N_REP)
    do
        date
        sleep 1
    done

    echo
    echo "Checking terminal input - USER: press ENTER"
    read
    echo "Terminal test successful"

    ###########################################################################
    # STEP PRELIMINARY 3: Write RedPitaya environment variables to EEPROM
    ###########################################################################
    echo
    echo "--- PRELIMINARY TEST 3: Read QR code --- "
    echo

    # Ask the operator to enter the QR CODE
    echo "USER: enter the RedPitaya QR CODE to terminal:"
    read LOG_VAR

    # Check QR CODE length and contained MAC, convert it to uppercase
    QR_LENGTH=$(echo $LOG_VAR | wc -m)
    MAC_ADDR=$(echo $LOG_VAR  | awk 'BEGIN {FS="-"}{print $1}' | tr '[:lower:]' '[:upper:]' )

    MAC_BEGIN=${MAC_ADDR:0:8}

    # Ask the user to confirm the MAC address
    echo "Entered MAC address is $MAC_ADDR, press ENTER to confirm it, N/n to enter it again: "
    read USER_ENTER

    # Verify QR CODE length and contained MAC
    while [ $QR_LENGTH -lt $MIN_QR_LENGTH ] || [[ "$MAC_BEGIN" != "$RP_MAC_BEGINNING" ]] || [[ "$USER_ENTER" == "N" ]] || [[ "$USER_ENTER" == "n" ]]
    do
        echo "ERROR: QR CODE is NOT valid, enter it again:"
        read LOG_VAR

        QR_LENGTH=$(echo $LOG_VAR | wc -m)
        MAC_ADDR=$(echo $LOG_VAR | awk 'BEGIN {FS="-"}{print $1}' | tr '[:lower:]' '[:upper:]' )
        MAC_BEGIN=${MAC_ADDR:0:8}

        # Ask the user to confirm the MAC address
        echo
        echo "Entered MAC address is $MAC_ADDR, press ENTER to confirm it, N/n to read it again"
        read  USER_ENTER

        echo
    done

    # Parse also the other QR CODE parameters. HP: they are correct.
    NAV_CODE=$(echo $LOG_VAR | awk 'BEGIN {FS="-"}{print $2}')
    HW_REV=$(echo $LOG_VAR | awk 'BEGIN {FS="-"}{print $3}')
    SERIAL=$(echo $LOG_VAR | awk 'BEGIN {FS="-"}{print $4}')

    # Save the environment variables
    echo "Writing ethaddr $MAC_ADDR to EEPROM..."
    $SETENV ethaddr $MAC_ADDR > /dev/null 2>&1
    echo "Writing navision code $NAV_CODE to EEPROM..."
    $SETENV nav_code $NAV_CODE > /dev/null 2>&1
    echo "Writing hw_rev $HW_REV to EEPROM..."
    $SETENV hw_rev $HW_REV > /dev/null 2>&1
    echo "Writing serial $SERIAL to EEPROM..."
    $SETENV serial $SERIAL > /dev/null 2>&1


    # Verify wrote parameters
    echo
    echo "Verify wrote parameters..."
    READ_MAC=$( $PRINTENV | grep ethaddr= | awk 'BEGIN {FS="="}{print $2}') > /dev/null 2>&1
    READ_NAV=$( $PRINTENV | grep nav_code= | awk 'BEGIN {FS="="}{print $2}') > /dev/null 2>&1
    READ_HWREV=$( $PRINTENV | grep hw_rev= | awk 'BEGIN {FS="="}{print $2}') > /dev/null 2>&1
    READ_SERIAL=$( $PRINTENV | grep serial= | awk 'BEGIN {FS="="}{print $2}') > /dev/null 2>&1

    if [[ "$READ_MAC" != "$MAC_ADDR" ]] || [[ "$READ_NAV" != "$NAV_CODE" ]] || [[ "$READ_HWREV" != "$HW_REV" ]] || [[ "$READ_SERIAL" != "$SERIAL" ]]
    then
        echo "Parameters are NOT correctly written in the EEPROM"
        sleep 1
    else
        echo "Environment variables of this board are SET TO..."
        echo "-----------------------------------------------"
        echo "EEPROM MAC  $READ_MAC"
        echo "NAV         $READ_NAV"
        echo "HWREV       $READ_HWREV"
        echo "SERIAL      $READ_SERIAL"
        echo "-----------------------------------------------"
        echo
    fi


    # Set the CALIBRATION PARAMETERS to the private EEPROM memory partition (factory parameters)
    echo
    echo "Setting the factory calibration parameters into the EEPROM..."
    echo $FACTORY_CAL | $CALIB -wf
    if [ $? -ne 0 ]
    then
        echo
        echo "Factory calibration parameters are NOT correctly written in the EEPROM"
        sleep 1
    fi

    #Copy the factory CALIBRATION PARAMETERS to the user EEPROM memory partition
    
    # echo
    # echo "Copying the factory calibration parameters into the EEPROM user partition..."
    # $($CALIB -d)
    # if [ $? -ne 0 ]
    # then
        # echo
        # echo "User calibration parameters are NOT correctly written in the EEPROM"
        # sleep 1
    # fi

    #Copy the NEW CALIBRATION PARAMETERS to the user EEPROM memory partition
    echo "Setting the new calibration parameters into the user EEPROM space..."
    echo " "        
        echo $FACTORY_CAL | $CALIB -w  
        sleep 0.1
        if [ $? -ne 0 ]
        then
        echo
        echo "New calibration parameters are NOT correctly written in the user EEPROM space"
        sleep 1
        fi
    echo -ne '\n' | $CALIB -r

    #In any case, reboot the unit to apply the changes
    #reboot
    #sleep 2
 else 
    #NAV_CODE, HW_REW and SERIAL are allwys empty beacuse you type manually
    #only MAC address. Wehn you get QR scanner you will have this values 
    #inputed directly so you can uncomment reboot and when script is run agian
    #it will skip this step because 
    #condition givene above >>if [[ "$MAC_BEGIN" != "$RP_MAC_BEGINNING" ]] || [[ "$READ_NAV" == "" ]] || [[ "$READ_HWREV" == "" ]] || [[ "$READ_SERIAL" == "" ]]
    #will be false  

    echo
    echo
    echo "########################################################################"
    echo "#                   PHASE-1    PRELIMINARY-TESTS    SKIPPED            #"
    echo "########################################################################"
    echo
    echo "Environment variables are already written to the EEPROM and PHASE 1 is skipped!!!"
    echo "Environment variables of this board are next..."
    echo "-----------------------------------------------"
    echo "EEPROM MAC  $READ_MAC"
    echo "NAV         $READ_NAV"
    echo "HWREV       $READ_HWREV"
    echo "SERIAL      $READ_SERIAL"
    echo "-----------------------------------------------"
    echo
fi


###############################################################################
#PHASE 2
###############################################################################
echo
echo "########################################################################"
echo "#                   PHASE-2    MANUFACTURING TESTS                     #"
echo "########################################################################"
echo

N_REP=5
RED_LED_VALUE=1
TEST_STATUS=1


# kill nginx server - it is not needed for manufacturing tests
##echo "Killing nginx server..."
##killall nginx
##echo
## spremenil kilanje nginix

# re-define control for led 8
##echo 0 > /sys/class/gpio/export
##echo out > /sys/class/gpio/gpio0/direction

# re-define control for red-led 9
##echo 7 > /sys/class/gpio/export
##echo out > /sys/class/gpio/gpio7/direction

# Again, after reboot, make the LEDs blink 
echo "LEDs 1-9 will blink for $((1*$N_REP)) sec"
echo
for i in $(seq 1 1 $N_REP)
do
    $MONITOR $LED_ADDR w 0xFF
    ##echo 1 > /sys/class/gpio/gpio0/value
    ##echo 1 > /sys/class/gpio/gpio7/value
    sleep 0.5
    $MONITOR $LED_ADDR w 0x00
    ##echo 0 > /sys/class/gpio/gpio0/value
    ##echo 0 > /sys/class/gpio/gpio7/value
    sleep 0.5
done

# Read the DNA Zynq code (part1 and part2) and save it into the log informations
echo "Reading DNA Zynq code..."
echo
DNA_P1=$($MONITOR 0x40000004)
sleep 0.2
DNA_P2=$($MONITOR 0x40000008)
sleep 0.2
LOG_VAR="$DNA_P1 $DNA_P2"

#Added, check if teh variable is empty > unsucsefull read will return empty variable. in this case set variable to "x".
if [ -z "$DNA_P1" ]
then
    DNA_P1="x"
    echo "Unsuccessful readout of DNA_P1"
fi

if [ -z "$DNA_P2" ]
then
    DNA_P2="x"
    echo "Unsuccessful readout of DNA_P2"
fi

###############################################################################
# Calibration parameters read-back test
###############################################################################

# Verify the values of the factory calibration parameters
echo "Verifying the factory calibration parameters in the EEPROM..."
echo
wr_FE_CH1_FS_G_HI=$($CALIB -rfv | grep FE_CH1_FS_G_HI | awk '{print $3}')
wr_FE_CH2_FS_G_HI=$($CALIB -rfv | grep FE_CH2_FS_G_HI | awk '{print $3}')
wr_FE_CH1_FS_G_LO=$($CALIB -rfv | grep FE_CH1_FS_G_LO | awk '{print $3}')
wr_FE_CH2_FS_G_LO=$($CALIB -rfv | grep FE_CH2_FS_G_LO | awk '{print $3}')

#wr_FE_CH1_DC_offs=$($CALIB -rfv | grep FE_CH1_DC_offs | awk '{print $3}')
#wr_FE_CH2_DC_offs=$($CALIB -rfv | grep FE_CH2_DC_offs | awk '{print $3}')
wr_FE_CH1_DC_offs=$($CALIB -rfv | grep FE_CH1_DC_offs | awk 'FNR == 1 {print $3}')
wr_FE_CH2_DC_offs=$($CALIB -rfv | grep FE_CH2_DC_offs | awk 'FNR == 1 {print $3}')

wr_BE_CH1_FS=$($CALIB -rfv | grep BE_CH1_FS | awk '{print $3}')
wr_BE_CH2_FS=$($CALIB -rfv | grep BE_CH2_FS | awk '{print $3}')
wr_BE_CH1_DC_offs=$($CALIB -rfv | grep BE_CH1_DC_offs | awk '{print $3}')
wr_BE_CH2_DC_offs=$($CALIB -rfv | grep BE_CH2_DC_offs | awk '{print $3}')
##Added 
wr_SOME_eeprom_value=$($CALIB -rfv | grep Magic | awk '{print $3}')
##SOME_eeprom_value is some value in eeprom which is not used for anything but after Črt added hv offset calib values this value also appeard.
wr_FE_CH1_DC_offs_HI=$($CALIB -rfv | grep FE_CH1_DC_offs_HI | awk '{print $3}')
wr_FE_CH2_DC_offs_HI=$($CALIB -rfv | grep FE_CH2_DC_offs_HI | awk '{print $3}')


if [ "$wr_FE_CH1_FS_G_HI" -ne "$FE_CH1_FS_G_HI" ] || [ "$wr_FE_CH2_FS_G_HI" -ne "$FE_CH2_FS_G_HI" ] || [ "$wr_FE_CH1_FS_G_LO" -ne "$FE_CH1_FS_G_LO" ] || [ "$wr_FE_CH2_FS_G_LO" -ne "$FE_CH2_FS_G_LO" ] || [ "$wr_FE_CH1_DC_offs" -ne "$FE_CH1_DC_offs" ] || [ "$wr_FE_CH2_DC_offs" -ne "$FE_CH2_DC_offs" ] || [ "$wr_BE_CH1_FS" -ne "$BE_CH1_FS" ] || [ "$wr_BE_CH2_FS" -ne "$BE_CH2_FS" ] || [ "$wr_BE_CH1_DC_offs" -ne "$BE_CH1_DC_offs" ] || [ "$wr_BE_CH2_DC_offs" -ne "$BE_CH2_DC_offs" ] || [ "$wr_FE_CH1_DC_offs_HI" -ne "$FE_CH1_DC_offs_HI" ] || [ "$wr_FE_CH2_DC_offs_HI" -ne "$FE_CH2_DC_offs_HI" ]
then 
    echo "Factory calibration parameters are NOT correctly set into the EEPROM..."
        TEST_STATUS=0
    echo
else
    # Verify the values of the written calibration parameters
    echo "Verifying the user calibration parameters in the EEPROM..."
    echo
    $CALIB -rf > /tmp/read_factory_cal.txt
    $CALIB -r > /tmp/read_user_cal.txt

    PARAM_CMP=$(cmp /tmp/read_factory_cal.txt /tmp/read_user_cal.txt)

    if [ $? != 0 ]
    then
        echo "User calibration parameters are NOT correctly set into the EEPROM..."
        TEST_STATUS=0
        echo
    fi
fi


###############################################################################
# I2C bus functionality test
###############################################################################

#Verify the I2C bus functionality through the external EEPROM memory
echo "Verifying the I2C bus functionality..."
echo

#Change the access rights of the SD card to read-wrire
mount -o remount,rw $SD_CARD_PATH

#Change the name of the actual environment configuration file, and create a temporary one for the test

# mv /opt/etc/fw_env.config /opt/etc/fw_env.config_tmp
# echo $I2C_TEST_CONFIG > /opt/etc/fw_env.config
mv /opt/redpitaya/etc/fw_env.config /opt/redpitaya/etc/fw_env.config_tmp
echo $I2C_TEST_CONFIG > /opt/redpitaya/etc/fw_env.config


###########ZACETEK KAR NE DELA ####################

#Create the static link to the resource
ln -s /sys/bus/i2c/devices/0-0051/eeprom $SYM_LINK

# Read the EEPROM variable foreseen for this test
echo "Reading the test string from external EEPROM..."
echo
READ_LABEL=$( $PRINTENV | grep test_label | awk 'BEGIN {FS="="}{print $2}') > /dev/null 2>&1
if [[ "$READ_LABEL" != "$TEST_LABEL" ]]
then
    echo "External EEPROM read-back doesn't work. I2C might not work correctly"
    echo "Test lable is: $TEST_LABEL" 
    echo "Read lable is: $READ_LABEL"
    
    TEST_STATUS=0
    sleep 1
fi

#Revert back the changes

#mv -f /opt/etc/fw_env.config_tmp /opt/etc/fw_env.config
mv -f /opt/redpitaya/etc/fw_env.config_tmp /opt/redpitaya/etc/fw_env.config
mount -o remount,ro $SD_CARD_PATH
sleep $SLEEP_BETWEEN_TESTS

############KONC KAR NE DELA ######################


### TEST 0 - PHASE 1 checking #
# If test was OK, turn LED 0 ON
if [ $TEST_STATUS -eq 1 ]
then
    #Turn LED 0 on
    LOGFILE_STATUS=$(($LOGFILE_STATUS+$RED_LED_VALUE))
    $MONITOR $LED_ADDR w 0x01
fi

###############################################################################
# STEP 1: Wired network test
###############################################################################

echo
echo "--- TEST 1: Wired network test ---"
echo

TEST_STATUS=1
TEST_VALUE=2    #LED position related to the test


# Verify that eth configuration MAC matches the EEPROM MAC
echo "Verify MAC address consistence with EEPROM..."
EEPROM_MAC=$($PRINTENV | grep ethaddr= | awk 'BEGIN {FS="="}{print $2}') > /dev/null 2>&1
LINUX_MAC=$(ifconfig | grep HWaddr | awk '{print $5}')
# Dodal male crke v velike 
LINUX_MAC=$(echo $LINUX_MAC | awk 'BEGIN {FS="-"}{print $1}' | tr '[:lower:]' '[:upper:]' )

#Added, check if teh variable is empty > unsucsefull read will return empty variable. in this case set variable to "x".
if [ -z "$EEPROM_MAC" ]
then
    EEPROM_MAC="x"
    echo "Unsuccessful readout of EEPROM_MAC"
fi


                        echo " "
                        echo "------------------------------------------------Printing  Log variables  step 1 ------------------------------------------------------------------"
                        echo "$LOG_VAR"
                        echo "--------------------------------------------------------------------------------------------------------------------------------------------------"
                        echo " "


LOG_VAR="$LOG_VAR $EEPROM_MAC"


                        echo " "
                        echo "------------------------------------------------Printing  Log variables  step 2 ------------------------------------------------------------------"
                        echo "$LOG_VAR"
                        echo "--------------------------------------------------------------------------------------------------------------------------------------------------"
                        echo " "


if [[ "$EEPROM_MAC" != "$LINUX_MAC" ]]
then
    echo "    MAC address is not applied correctly to the network configuration"
    TEST_STATUS=0
fi

#Added for new OS
echo "EEPROM_MAC $EEPROM_MAC"
echo "LINUX_MAC $LINUX_MAC"
echo " "

#cd - > /dev/null

# Check the link speed
echo "    Verify link speed and ping to host $PING_IP..."

##LINK_SPEED=$(dmesg | grep link | awk '{print $5}')
LINK_SPEED=$(dmesg | grep link | awk 'END {print $7}')


if [[ "$LINK_SPEED" != "$EXP_LINK_SPEED" ]] 
then 
    echo "    Network link speed $LINK_SPEED is not the expected one $EXP_LINK_SPEED"
    echo "    Expected link speed is $LINK_SPEED"
    echo " "
    TEST_STATUS=0
fi

# Ping the defined IP 
RES=$(ping "$PING_IP" -c "$N_PING_PKG" -s "$PKT_SIZE" | grep 'transmitted' | awk '{print $4}' ) > /dev/null

if [[ "$RES" != "$N_PING_PKG" ]]
then
    echo "    Ping to unit $PING_IP failed"
    TEST_STATUS=0
else
    echo "    Ping to unit $PING_IP OKAY"
fi
 
sleep $SLEEP_BETWEEN_TESTS

# If test was OK, turn LED 1 on
if [ $TEST_STATUS -eq 1 ]
then
    TEST_GLOBAL_STATUS=$(($TEST_GLOBAL_STATUS+$TEST_VALUE))
    LOGFILE_STATUS=$(($LOGFILE_STATUS+$TEST_VALUE))
    #$MONITOR $LED_ADDR w "$(printf '0x%02x' $TEST_GLOBAL_STATUS)"
    $MONITOR $LED_ADDR w 0x02
fi



###############################################################################
# STEP 2: Temperature and Power supply voltages test
###############################################################################

echo
echo "--- TEST 2: Temperature and Power supply voltages test ---"
echo

TEST_STATUS=1
TEST_VALUE=4    # LED position related to the test

# Acquire temperature and voltages
$MONITOR -ams > /tmp/AMS 2>&1

TEMP=$(cat /tmp/AMS | grep Temp | awk '{ printf "%0.f\n", $4 }')
VCCAUX=$(cat /tmp/AMS | grep VCCAUX | awk '{ printf "%0.f\n", $4*1000 }')
VCCBRAM=$(cat /tmp/AMS | grep VCCBRAM | awk '{ printf "%0.f\n", $4*1000 }')
VCCINT=$(cat /tmp/AMS | grep VCCINT | awk '{ printf "%0.f\n", $4*1000 }')


#Added, check if teh variable is empty > unsucsefull read will return empty variable. in this case set variable to "x".
if [ -z "$TEMP" ]
then
    TEMP="x"
    echo "Unsuccessful readout of TEMP"
else
# Check if the values are within expectations
if [ $TEMP -lt $MIN_TEMP ] || [ $TEMP -gt $MAX_TEMP ]
then 
    echo "    Measured temperature ($TEMP) is outside expected range ($MIN_TEMP-$MAX_TEMP)"
    TEST_STATUS=0
else
    echo "    Measured temperature ($TEMP) is within expectations"
fi
fi


#Added, check if teh variable is empty > unsucsefull read will return empty variable. in this case set variable to "x".
if [ -z "$VCCAUX" ]
then
    VCCAUX="x"
    echo "Unsuccessful readout of VCCAUX"
else
# Check if the values are within expectations 
if [[ $VCCAUX -lt $MIN_VCCAUX ]] || [[ $VCCAUX -gt $MAX_VCCAUX ]]
then 
    echo "    Measured VCCAUX ($VCCAUX) is outside expected range ($MIN_VCCAUX-$MAX_VCCAUX)"
    TEST_STATUS=0
else
    echo "    Measured VCCAUX ($VCCAUX) is within expectations"
fi
fi


#Added, check if teh variable is empty > unsucsefull read will return empty variable. in this case set variable to "x".
if [ -z "$VCCBRAM" ]
then
    VCCBRAM="x"
    echo "Unsuccessful readout of VCCBRAM"
else
# Check if the values are within expectations 
if [[ $VCCBRAM -lt $MIN_VCCBRAM ]] || [[ $VCCBRAM -gt $MAX_VCCBRAM ]]
then 
    echo "    Measured VCCBRAM ($VCCBRAM) is outside expected range ($MIN_VCCBRAM-$MAX_VCCBRAM)"
    TEST_STATUS=0
else
    echo "    Measured VCCBRAM ($VCCBRAM) is within expectations"
fi
fi

#Added, check if teh variable is empty > unsucsefull read will return empty variable. in this case set variable to "x".
if [ -z "$VCCINT" ]
then
    VCCINT="x"
    echo "Unsuccessful readout of VCCINT"
else
# Check if the values are within expectations
if [[ $VCCINT -lt $MIN_VCCINT ]] || [[ $VCCINT -gt $MAX_VCCINT ]]
then 
    echo "    Measured VCCINT ($VCCINT) is outside expected range ($MIN_VCCINT-$MAX_VCCINT)"
    TEST_STATUS=0
else
    echo "    Measured VCCINT ($VCCINT) is within expectations"
fi
fi

# Log temperature and voltages
LOG_VAR="$LOG_VAR $TEMP $VCCAUX $VCCBRAM $VCCINT"

                        echo " "
                        echo "------------------------------------------------Printing  Log variables  step 3 ------------------------------------------------------------------"
                        echo "$LOG_VAR"
                        echo "--------------------------------------------------------------------------------------------------------------------------------------------------"
                        echo " "

sleep $SLEEP_BETWEEN_TESTS

# If test was OK, turn LED 2 on
if [ $TEST_STATUS -eq 1 ]
then
    TEST_GLOBAL_STATUS=$(($TEST_GLOBAL_STATUS+$TEST_VALUE))
    LOGFILE_STATUS=$(($LOGFILE_STATUS+$TEST_VALUE))
    #$MONITOR $LED_ADDR w "$(printf '0x%02x' $TEST_GLOBAL_STATUS)"
    $MONITOR $LED_ADDR w 0x04
fi



###############################################################################
# STEP 3: USB drive test
###############################################################################

echo
echo "--- TEST 3: USB drive test ---"
echo

DRIVE_TEST_STATUS=1   # Special flag, will be checked at the and for logging purposes
TEST_VALUE=8          # LED position related to the test

# Check that the USB drive is plugged on the board
USB_IN=$(fdisk -l | grep $USB_DEVICE)

if [ $? -ne 0 ]
then 
    echo "    USB device not found, check if USB device is connected"
    DRIVE_TEST_STATUS=0

else
    # Create the mounting point folder and mount the device
    mkdir $USB_MOUNT_FOLDER > /dev/null 2>&1
    mount $USB_DEVICE $USB_MOUNT_FOLDER > /dev/null 2>&1

    if [ $? -ne 0 ]
    then 
        echo "    Not possible to mount USB device"
        rm -rf $USB_MOUNT_FOLDER
        DRIVE_TEST_STATUS=0

    else
        # Move the file to /home and back with different name
        cp $USB_MOUNT_FOLDER/$USB_FILENAME /home
        mv /home/$USB_FILENAME $USB_MOUNT_FOLDER/$USB_NEWFILENAME

        # Compare the files and check they are the same
        cmp $USB_MOUNT_FOLDER/$USB_FILENAME $USB_MOUNT_FOLDER/$USB_NEWFILENAME

        if [ $? -eq 1 ]
        then 
            echo "    Read & Write test on USB DRIVE failed"
            DRIVE_TEST_STATUS=0
        else
            echo "    Read & Write test on USB DRIVE is OK"
        fi

        # remove the new file, umount the device and delete the folder
        rm $USB_MOUNT_FOLDER/$USB_NEWFILENAME
        umount $USB_MOUNT_FOLDER
        rm -rf $USB_MOUNT_FOLDER
    fi
fi

sleep $SLEEP_BETWEEN_TESTS

# If test was OK, turn LED 3 on
if [ $DRIVE_TEST_STATUS -eq 1 ]
then
    TEST_GLOBAL_STATUS=$(($TEST_GLOBAL_STATUS+$TEST_VALUE))
    LOGFILE_STATUS=$(($LOGFILE_STATUS+$TEST_VALUE))
    #$MONITOR $LED_ADDR w "$(printf '0x%02x' $TEST_GLOBAL_STATUS)"
    $MONITOR $LED_ADDR w 0x08
fi



###############################################################################
# STEP 4: Serial Communication (SATA) test
###############################################################################

echo
echo "--- TEST 4: Serial Communication (SATA) test---"
echo "--- SATA is skipped - FPGA - IZTOK needs to cehck it---"
echo

TEST_STATUS=1
TEST_VALUE=16    # LED position related to the test

# Enable TX, train the unit and check the expected value
echo "    Training..."
$MONITOR 0x40500000 w 0x1
sleep 0.2
$MONITOR 0x40500004 w 0x3
sleep 0.2
$MONITOR 0x40500000 w 0x3
sleep 0.2
$MONITOR 0x40500008 w 0x1
sleep 0.2
TRAIN_VALUE=$($MONITOR 0x40500008)
sleep 0.2

# Disable train
$MONITOR 0x40500008 w 0x0
sleep 0.2

echo "    Training value is $TRAIN_VALUE"
echo

# Send test pattern, reset counters and enable them again
echo "    Sending test pattern, resetting and enabling counters..."
echo
$MONITOR 0x40500004 w 0x5
sleep 0.2
$MONITOR 0x40500010 w 0x1
sleep 0.2
$MONITOR 0x40500010 w 0x0
sleep 0.2


# Check periodically the counters
for ind in $(seq 1 1 $N_SATA_CYC)
do 
    ERR_CNT=$($MONITOR 0x40500014)
    sleep 0.2
    TX_CNT=$($MONITOR 0x40500018)
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
            TEST_STATUS=0
        fi

        # IMP: transferred words are stored in a 8bit register. Sometimes negative rate is shown due to register reset
        if [ $TX_RATE -gt $MAX_SATA_RATE ] || [ $TX_RATE -lt $MIN_SATA_RATE ]; then
            echo "$ind:   Tx rate $TX_RATE, between $TX_DEC and $OLD_TX_DEC, out of specification"
            TEST_STATUS=0
        fi

    else
        echo "    $ind: Transfer count is $TX_DEC, error count is $ERR_DEC"
    fi

    OLD_TX_DEC=$TX_DEC

    # every cycle takes in total SEC_PER_CYC seconds, should wait SEC_PER_CYC-0.4
    sleep 1.6
done


sleep $SLEEP_BETWEEN_TESTS

# If test was OK, turn LED 4 on
if [ $TEST_STATUS -eq 1 ]
then
    TEST_GLOBAL_STATUS=$(($TEST_GLOBAL_STATUS+$TEST_VALUE))
    LOGFILE_STATUS=$(($LOGFILE_STATUS+$TEST_VALUE))
    ##$MONITOR $LED_ADDR w "$(printf '0x%02x' $TEST_GLOBAL_STATUS)"
    $MONITOR $LED_ADDR w 0x10
fi

###############################################################################
# STEP 5: GPIO connection test
###############################################################################

echo
echo "--- TEST 5: GPIO connection test ---"
echo

TEST_STATUS=1
TEST_VALUE=32    # LED position related to the test

# Configure both ports as input ports (avoid N and P in output configuration)
$MONITOR 0x40000010 w 0x00
sleep 0.2
$MONITOR 0x40000014 w 0x00
sleep 0.2

# P->N configuration
# Configure P as output port
$MONITOR 0x40000010 w 0xFF

# Set P-port output value
VALUE=0x000000ab
$MONITOR 0x40000018 w $VALUE

# Read N-port input value
READ_VALUE=$($MONITOR 0x40000024)

# Check if they are the same
if [[ $READ_VALUE != $VALUE ]]
then 
    echo "    GPIO (P->N check): read byte ($READ_VALUE) differs from written byte ($VALUE). Is IDS testboard connected?"

    # Provide information about what bits are wrong
    BIT_DEC_XOR=$(($READ_VALUE ^ $VALUE))
    BIT_BIN_XOR=$(echo "obase=2; $BIT_DEC_XOR" | bc )
    PADDED_XOR=$(echo | awk -v x=$BIT_BIN_XOR '{for(i=length(x);i<8;i++)x="0" x;}END{print x}')

    echo "    XOR performed on the two bytes returns: $PADDED_XOR, LSB on the right"

    TEST_STATUS=0
else
    echo "    GPIO (P->N check): read byte ($READ_VALUE) matches written byte ($VALUE)"
fi

# N->P configuration
# Configure P back as input port, and N as output port
$MONITOR 0x40000010 w 0x00
sleep 0.2
$MONITOR 0x40000014 w 0xFF

# Set N-port output value
VALUE=0x000000f8
$MONITOR 0x4000001C w $VALUE

# Read P-port input value
READ_VALUE=$($MONITOR 0x40000020)

# Check if they are the same
if [[ $READ_VALUE != $VALUE ]]
then 
    echo "    GPIO (N->P check): read byte ($READ_VALUE) differs from written byte ($VALUE). Is IDS testboard connected?"

    # Provide information about what bits are wrong
    BIT_DEC_XOR=$(($READ_VALUE ^ $VALUE))
    BIT_BIN_XOR=$(echo "obase=2; $BIT_DEC_XOR" | bc )
    PADDED_XOR=$(echo | awk -v x=$BIT_BIN_XOR '{for(i=length(x);i<8;i++)x="0" x;}END{print x}')

    echo "    XOR performed on the two bytes returns: $PADDED_XOR, LSB on the right"
    TEST_STATUS=0
else
    echo "    GPIO (N->P check): read byte ($READ_VALUE) matches written byte ($VALUE)"
fi

# Set back to default
$MONITOR 0x40000010 w 0x00
sleep 0.2
$MONITOR 0x40000014 w 0x00

sleep $SLEEP_BETWEEN_TESTS

# If test was OK, turn LED 5 on
if [ $TEST_STATUS -eq 1 ]
then
    TEST_GLOBAL_STATUS=$(($TEST_GLOBAL_STATUS+$TEST_VALUE))
    LOGFILE_STATUS=$(($LOGFILE_STATUS+$TEST_VALUE))
    $MONITOR $LED_ADDR w "$(printf '0x%02x' $TEST_GLOBAL_STATUS)"
fi


###############################################################################
# STEP 6: Slow ADCs and DACs test
###############################################################################

echo
echo "--- TEST 6: Slow ADCs and DACs test ---"
echo

TEST_STATUS=1
TEST_VALUE=64    # LED position related to the test

# Set the half-scale output level of all four DACs (5M)
DAC_VALUE=0x4C4B40
$MONITOR 0x40400020 w $DAC_VALUE
sleep 0.2
$MONITOR 0x40400024 w $DAC_VALUE
sleep 0.2
$MONITOR 0x40400028 w $DAC_VALUE
sleep 0.2
$MONITOR 0x4040002C w $DAC_VALUE
sleep 0.2

# Get the input level of all four ADCs
ADC1_A=$($MONITOR 0x40400000)
sleep 0.2
ADC2_A=$($MONITOR 0x40400004)
sleep 0.2
ADC3_A=$($MONITOR 0x40400008)
sleep 0.2
ADC4_A=$($MONITOR 0x4040000C)
sleep 0.2

echo "    ADC values - first acquisition - are $ADC1_A, $ADC2_A, $ADC3_A, $ADC4_A"

# Set almost full-scale output level of all four DACs (2x5M=10M)
DAC_VALUE=0x989680
$MONITOR 0x40400020 w $DAC_VALUE
sleep 0.2
$MONITOR 0x40400024 w $DAC_VALUE
sleep 0.2
$MONITOR 0x40400028 w $DAC_VALUE
sleep 0.2
$MONITOR 0x4040002C w $DAC_VALUE
sleep 0.2

# Get again the input level of all four DACs
ADC1_B=$($MONITOR 0x40400000)
sleep 0.2
ADC2_B=$($MONITOR 0x40400004)
sleep 0.2
ADC3_B=$($MONITOR 0x40400008)
sleep 0.2
ADC4_B=$($MONITOR 0x4040000C)
sleep 0.2

echo "    ADC values - second acquisition - are $ADC1_B, $ADC2_B, $ADC3_B, $ADC4_B"

# Evaluate the ratios
ADC1_R=$(echo | awk -v sb=$ADC1_B -v sa=$ADC1_A '{ printf "%0.f\n", sb/sa }')
ADC2_R=$(echo | awk -v sb=$ADC2_B -v sa=$ADC2_A '{ printf "%0.f\n", sb/sa }')
ADC3_R=$(echo | awk -v sb=$ADC3_B -v sa=$ADC3_A '{ printf "%0.f\n", sb/sa }')
ADC4_R=$(echo | awk -v sb=$ADC4_B -v sa=$ADC4_A '{ printf "%0.f\n", sb/sa }')


if [ $ADC1_R -lt $MIN_RATIO ] || [ $ADC1_R -gt $MAX_RATIO ] || [ $ADC2_R -lt $MIN_RATIO ] || [ $ADC2_R -gt $MAX_RATIO ]
then 
    echo "    Measured ratios after two slow ADC acquisitions ($ADC1_R, $ADC2_R, $ADC3_R, $ADC4_R) don't match expected ratio (2)"
    TEST_STATUS=0
else
    if [ $ADC3_R -ge $MIN_RATIO ] && [ $ADC3_R -le $MAX_RATIO ] && [ $ADC4_R -ge $MIN_RATIO ] && [ $ADC4_R -le $MAX_RATIO ] 
    then
        echo "    Measured ratios after two slow ADC acquisitions match expected ratio (2)"
    else
        echo "    Measured ratios after two slow ADC acquisitions ($ADC1_R, $ADC2_R, $ADC3_R, $ADC4_R) don't match expected ratio (2)"
        TEST_STATUS=0
    fi
fi

sleep $SLEEP_BETWEEN_TESTS

# If test was OK, turn LED 6 on
if [ $TEST_STATUS -eq 1 ]
then
    TEST_GLOBAL_STATUS=$(($TEST_GLOBAL_STATUS+$TEST_VALUE))
    LOGFILE_STATUS=$(($LOGFILE_STATUS+$TEST_VALUE))
    $MONITOR $LED_ADDR w "$(printf '0x%02x' $TEST_GLOBAL_STATUS)"
fi


###############################################################################
# STEP 7: Fast ADCs and DACs test
###############################################################################

echo
echo "--- TEST 7: Fast ADCs and DACs test ---"
echo

echo "    Acquisition without DAC signal - ADCs with HIGH gain"
echo

TEST_STATUS=1
TEST_VALUE=128    # LED position related to the test

# Assure tht DAC signals (ch 1 & 2) are OFF
$GENERATE 1 0 $SIG_FREQ
$GENERATE 2 0 $SIG_FREQ

# Configure the ADC in high-gain mode
$MONITOR 0x40000014 w 0x80
sleep 0.4
$MONITOR 0x4000001C w 0x80
sleep 0.4

# Acquire data with 1024 decimation factor
$ACQUIRE $ADC_BUFF_SIZE 1024 > /tmp/adc.txt   # WORKAROUND: First acquisition is thrown away
sleep 0.4
$ACQUIRE $ADC_BUFF_SIZE 1024 > /tmp/adc.txt
cat /tmp/adc.txt | awk '{print $1}' > /tmp/adc_a.txt
cat /tmp/adc.txt | awk '{print $2}' > /tmp/adc_b.txt

# NEW Acquire data also with no decimation
$ACQUIRE $ADC_BUFF_SIZE 1 > /tmp/adc_no_dec.txt
cat /tmp/adc_no_dec.txt | awk '{print $1}' > /tmp/adc_no_dec_a.txt
cat /tmp/adc_no_dec.txt | awk '{print $2}' > /tmp/adc_no_dec_b.txt

# Calculate mean value, std deviation and p2p diff value
ADC_A_MEAN=$(awk '{sum+=$1} END { print int(sum/NR)}' /tmp/adc_a.txt)
ADC_B_MEAN=$(awk '{sum+=$1} END { print int(sum/NR)}' /tmp/adc_b.txt)
##ADC_A_STD=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)**2))}' /tmp/adc_a.txt)
##ADC_B_STD=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)**2))}' /tmp/adc_b.txt)
ADC_A_STD=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)*(sum/NR)))}' /tmp/adc_a.txt)
ADC_B_STD=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)*(sum/NR)))}' /tmp/adc_b.txt)
ADC_A_PP=$(awk 'BEGIN{max=-1000;min=1000} { if (max < $1){ max = $1 }; if(min > $1){ min = $1 } } END{ print max-min}' /tmp/adc_a.txt)
ADC_B_PP=$(awk 'BEGIN{max=-1000;min=1000} { if (max < $1){ max = $1 }; if(min > $1){ min = $1 } } END{ print max-min}' /tmp/adc_b.txt)

# NEW Calculate std deviation in case of no decimation
##ADC_A_STD_NO_DEC=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)**2))}' /tmp/adc_no_dec_a.txt)
##ADC_B_STD_NO_DEC=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)**2))}' /tmp/adc_no_dec_b.txt)
ADC_A_STD_NO_DEC=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)*(sum/NR)))}' /tmp/adc_no_dec_a.txt)
ADC_B_STD_NO_DEC=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)*(sum/NR)))}' /tmp/adc_no_dec_b.txt)


# Print out the measurements
echo "    Measured ADC ch.A mean value is $ADC_A_MEAN"
echo "    Measured ADC ch.A std.deviation value is $ADC_A_STD"
echo "    Measured ADC ch.A std.deviation (no decimation) is $ADC_A_STD_NO_DEC"
echo "    Measured ADC ch.A p2p value is $ADC_A_PP"
echo "    Measured ADC ch.B mean value is $ADC_B_MEAN"
echo "    Measured ADC ch.B std.deviation value is $ADC_B_STD"
echo "    Measured ADC ch.B std.deviation (no decimation) is $ADC_B_STD_NO_DEC"
echo "    Measured ADC ch.B p2p value is $ADC_B_PP"

# Log informations
LOG_VAR="$LOG_VAR $ADC_A_MEAN $ADC_A_STD $ADC_A_STD_NO_DEC $ADC_A_PP $ADC_B_MEAN $ADC_B_STD $ADC_B_STD_NO_DEC $ADC_B_PP"

                        echo " "
                        echo "------------------------------------------------Printing  Log variables  step 4 ------------------------------------------------------------------"
                        echo "$LOG_VAR"
                        echo "--------------------------------------------------------------------------------------------------------------------------------------------------"
                        echo " "

# Check if the values are within expectations
if [[ $ADC_A_MEAN -gt $MAX_ABS_OFFS_HIGH_GAIN ]] || [[ $ADC_A_MEAN -lt $((-$MAX_ABS_OFFS_HIGH_GAIN)) ]] 
then
    echo "    Measured ch.A mean value ($ADC_A_MEAN) is outside expected range (+/- $MAX_ABS_OFFS_HIGH_GAIN)"
    TEST_STATUS=0
fi

if [[ $ADC_B_MEAN -gt $MAX_ABS_OFFS_HIGH_GAIN ]] || [[ $ADC_B_MEAN -lt $((-$MAX_ABS_OFFS_HIGH_GAIN)) ]] 
then
    echo "    Measured ch.B mean value ($ADC_B_MEAN) is outside expected range (+/- $MAX_ABS_OFFS_HIGH_GAIN)"
    TEST_STATUS=0
fi

if [[ $ADC_A_STD -gt $MAX_NOISE_STD ]]
then
    echo "    Measured ch.A std deviation value ($ADC_A_STD) is outside expected range (0-$MAX_NOISE_STD)"
    TEST_STATUS=0
fi

if [[ $ADC_B_STD -gt $MAX_NOISE_STD ]]
then
    echo "    Measured ch.B std deviation value ($ADC_B_STD) is outside expected range (0-$MAX_NOISE_STD)"
    TEST_STATUS=0
fi

if [[ $ADC_A_STD_NO_DEC -gt $MAX_NOISE_STD_NO_DEC ]]
then
    echo "    Measured ch.A std deviation value with no decimation ($ADC_A_STD_NO_DEC) is outside expected range (0-$MAX_NOISE_STD_NO_DEC)"
    TEST_STATUS=0
fi

if [[ $ADC_B_STD_NO_DEC -gt $MAX_NOISE_STD_NO_DEC ]]
then
    echo "    Measured ch.B std deviation value with no decimation ($ADC_B_STD_NO_DEC) is outside expected range (0-$MAX_NOISE_STD_NO_DEC)"
    TEST_STATUS=0
fi

if [[ $ADC_A_PP -gt $MAX_NOISE_P2P ]]
then
    echo "    Measured ch.A p2p value ($ADC_A_PP) is outside expected range (0-$MAX_NOISE_P2P)"
    TEST_STATUS=0
fi

if [[ $ADC_B_PP -gt $MAX_NOISE_P2P ]]
then
    echo "    Measured ch.B p2p value ($ADC_B_PP) is outside expected range (0-$MAX_NOISE_P2P)"
    TEST_STATUS=0
fi


###############################################################################

# Set DAC value to proper counts / frequency for both channels
echo
echo "    Acquisition with DAC signal ($SIG_AMPL Vpp / $SIG_FREQ Hz) - ADCs with HIGH gain"
echo

# Turn the DAC signal generator on on both channels
$GENERATE 1 $SIG_AMPL $SIG_FREQ
$GENERATE 2 $SIG_AMPL $SIG_FREQ

# Configure the ADC in high-gain mode
$MONITOR 0x40000014 w 0x80
sleep 0.4
$MONITOR 0x4000001C w 0x80
sleep 0.4

# Acquire data with 1024 decimation factor
$ACQUIRE $ADC_BUFF_SIZE 1024 > /tmp/adc.txt   # WORKAROUND: First acquisition is thrown away
sleep 0.4
$ACQUIRE $ADC_BUFF_SIZE 1024 > /tmp/adc.txt
cat /tmp/adc.txt | awk '{print $1}' > /tmp/adc_a.txt
cat /tmp/adc.txt | awk '{print $2}' > /tmp/adc_b.txt

# NEW Acquire data also with no decimation
$ACQUIRE $ADC_BUFF_SIZE 1 > /tmp/adc_no_dec.txt
cat /tmp/adc_no_dec.txt | awk '{print $1}' > /tmp/adc_no_dec_a.txt
cat /tmp/adc_no_dec.txt | awk '{print $2}' > /tmp/adc_no_dec_b.txt

# Calculate mean value, std deviation and p2p diff value
ADC_A_MEAN=$(awk '{sum+=$1} END { print int(sum/NR)}' /tmp/adc_a.txt)
ADC_B_MEAN=$(awk '{sum+=$1} END { print int(sum/NR)}' /tmp/adc_b.txt)
##ADC_A_STD=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)**2))}' /tmp/adc_a.txt)
##ADC_B_STD=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)**2))}' /tmp/adc_b.txt)
ADC_A_STD=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)*(sum/NR)))}' /tmp/adc_a.txt)
ADC_B_STD=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)*(sum/NR)))}' /tmp/adc_b.txt)
ADC_A_PP=$(awk 'BEGIN{max=-1000;min=1000} { if (max < $1){ max = $1 }; if(min > $1){ min = $1 } } END{ print max-min}' /tmp/adc_a.txt)
ADC_B_PP=$(awk 'BEGIN{max=-1000;min=1000} { if (max < $1){ max = $1 }; if(min > $1){ min = $1 } } END{ print max-min}' /tmp/adc_b.txt)

# NEW Calculate std deviation in case of no decimation
##ADC_A_STD_NO_DEC=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)**2))}' /tmp/adc_no_dec_a.txt)
##ADC_B_STD_NO_DEC=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)**2))}' /tmp/adc_no_dec_b.txt)
ADC_A_STD_NO_DEC=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)*(sum/NR)))}' /tmp/adc_no_dec_a.txt)
ADC_B_STD_NO_DEC=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)*(sum/NR)))}' /tmp/adc_no_dec_b.txt)


# Print out the measurements
echo "    Measured ADC ch.A mean value is $ADC_A_MEAN"
echo "    Measured ADC ch.A std.deviation value is $ADC_A_STD"
echo "    Measured ADC ch.A std.deviation (no decimation) is $ADC_A_STD_NO_DEC"
echo "    Measured ADC ch.A p2p value is $ADC_A_PP"
echo "    Measured ADC ch.B mean value is $ADC_B_MEAN"
echo "    Measured ADC ch.B std.deviation value is $ADC_B_STD"
echo "    Measured ADC ch.B std.deviation (no decimation) is $ADC_B_STD_NO_DEC"
echo "    Measured ADC ch.B p2p value is $ADC_B_PP"

# Log informations
LOG_VAR="$LOG_VAR $ADC_A_MEAN $ADC_A_STD $ADC_A_STD_NO_DEC $ADC_A_PP $ADC_B_MEAN $ADC_B_STD $ADC_B_STD_NO_DEC $ADC_B_PP"


                        echo " "
                        echo "------------------------------------------------Printing  Log variables  step 4 ------------------------------------------------------------------"
                        echo "$LOG_VAR"
                        echo "--------------------------------------------------------------------------------------------------------------------------------------------------"
                        echo " "

# Check if the values are within expectations
if [[ $ADC_A_MEAN -gt $MAX_ABS_OFFS_HIGH_GAIN ]] || [[ $ADC_A_MEAN -lt $((-$MAX_ABS_OFFS_HIGH_GAIN)) ]] 
then
    echo "    Measured ch.A mean value ($ADC_A_MEAN) is outside expected range (+/- $MAX_ABS_OFFS_HIGH_GAIN)"
    TEST_STATUS=0
fi

if [[ $ADC_B_MEAN -gt $MAX_ABS_OFFS_HIGH_GAIN ]] || [[ $ADC_B_MEAN -lt $((-$MAX_ABS_OFFS_HIGH_GAIN)) ]] 
then
    echo "    Measured ch.B mean value ($ADC_B_MEAN) is outside expected range (+/- $MAX_ABS_OFFS_HIGH_GAIN)"
    TEST_STATUS=0
fi

if [[ $ADC_A_STD -lt $MIN_SIG_STD_HIGH_GAIN ]] || [[ $ADC_A_STD -gt $MAX_SIG_STD_HIGH_GAIN ]]
then
    echo "    Measured ch.A std deviation value ($ADC_A_STD) is outside expected range ($MIN_SIG_STD_HIGH_GAIN-$MAX_SIG_STD_HIGH_GAIN)"
    TEST_STATUS=0
fi

if [[ $ADC_B_STD -lt $MIN_SIG_STD_HIGH_GAIN ]] || [[ $ADC_B_STD -gt $MAX_SIG_STD_HIGH_GAIN ]]
then
    echo "    Measured ch.B std deviation value ($ADC_B_STD) is outside expected range ($MIN_SIG_STD_HIGH_GAIN-$MAX_SIG_STD_HIGH_GAIN)"
    TEST_STATUS=0
fi

if [[ $ADC_A_PP -lt $MIN_SIG_P2P_HIGH_GAIN ]] || [[ $ADC_A_PP -gt $MAX_SIG_P2P_HIGH_GAIN ]]
then
    echo "    Measured ch.A p2p value ($ADC_A_PP) is outside expected range ($MIN_SIG_P2P_HIGH_GAIN-$MAX_SIG_P2P_HIGH_GAIN)"
    TEST_STATUS=0
fi

if [[ $ADC_B_PP -lt $MIN_SIG_P2P_HIGH_GAIN ]] || [[ $ADC_B_PP -gt $MAX_SIG_P2P_HIGH_GAIN ]]
then
    echo "    Measured ch.B p2p value ($ADC_B_PP) is outside expected range ($MIN_SIG_P2P_HIGH_GAIN-$MAX_SIG_P2P_HIGH_GAIN)"
    TEST_STATUS=0
fi


###############################################################################

echo
echo "    Acquisition without DAC signal - ADCs with LOW gain"
echo

# Turn the DAC signal generator OFF on both channels (ch 1 & 2)
$GENERATE 1 0 $SIG_FREQ
$GENERATE 2 0 $SIG_FREQ
sleep 0.2

# Change jumper position, configure the ADC in low-gain mode
$MONITOR 0x40000014 w 0x80
sleep 0.4
$MONITOR 0x4000001C w 0x00
sleep 0.4

# Acquire data with 1024 decimation factor
$ACQUIRE $ADC_BUFF_SIZE 1024 > /tmp/adc.txt   # WORKAROUND: First acquisition is thrown away
sleep 0.4
$ACQUIRE $ADC_BUFF_SIZE 1024 > /tmp/adc.txt
cat /tmp/adc.txt | awk '{print $1}' > /tmp/adc_a.txt
cat /tmp/adc.txt | awk '{print $2}' > /tmp/adc_b.txt

# NEW Acquire data also with no decimation
$ACQUIRE $ADC_BUFF_SIZE 1 > /tmp/adc_no_dec.txt
cat /tmp/adc_no_dec.txt | awk '{print $1}' > /tmp/adc_no_dec_a.txt
cat /tmp/adc_no_dec.txt | awk '{print $2}' > /tmp/adc_no_dec_b.txt

# Calculate mean value, std deviation and p2p diff value
ADC_A_MEAN=$(awk '{sum+=$1} END { print int(sum/NR)}' /tmp/adc_a.txt)
ADC_B_MEAN=$(awk '{sum+=$1} END { print int(sum/NR)}' /tmp/adc_b.txt)
##ADC_A_STD=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)**2))}' /tmp/adc_a.txt)
##ADC_B_STD=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)**2))}' /tmp/adc_b.txt)
ADC_A_STD=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)*(sum/NR)))}' /tmp/adc_a.txt)
ADC_B_STD=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)*(sum/NR)))}' /tmp/adc_b.txt)
ADC_A_PP=$(awk 'BEGIN{max=-1000;min=1000} { if (max < $1){ max = $1 }; if(min > $1){ min = $1 } } END{ print max-min}' /tmp/adc_a.txt)
ADC_B_PP=$(awk 'BEGIN{max=-1000;min=1000} { if (max < $1){ max = $1 }; if(min > $1){ min = $1 } } END{ print max-min}' /tmp/adc_b.txt)

# NEW Calculate std deviation in case of no decimation
##ADC_A_STD_NO_DEC=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)**2))}' /tmp/adc_no_dec_a.txt)
##ADC_B_STD_NO_DEC=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)**2))}' /tmp/adc_no_dec_b.txt)
ADC_A_STD_NO_DEC=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)*(sum/NR)))}' /tmp/adc_no_dec_a.txt)
ADC_B_STD_NO_DEC=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)*(sum/NR)))}' /tmp/adc_no_dec_b.txt)



# Print out the measurements
echo "    Measured ADC ch.A mean value is $ADC_A_MEAN"
echo "    Measured ADC ch.A std.deviation value is $ADC_A_STD"
echo "    Measured ADC ch.A std.deviation (no decimation) is $ADC_A_STD_NO_DEC"
echo "    Measured ADC ch.A p2p value is $ADC_A_PP"
echo "    Measured ADC ch.B mean value is $ADC_B_MEAN"
echo "    Measured ADC ch.B std.deviation value is $ADC_B_STD"
echo "    Measured ADC ch.B std.deviation (no decimation) is $ADC_B_STD_NO_DEC"
echo "    Measured ADC ch.B p2p value is $ADC_B_PP"

# Log informations
LOG_VAR="$LOG_VAR $ADC_A_MEAN $ADC_A_STD $ADC_A_STD_NO_DEC $ADC_A_PP $ADC_B_MEAN $ADC_B_STD $ADC_B_STD_NO_DEC $ADC_B_PP"

                        echo " "
                        echo "------------------------------------------------Printing  Log variables  step 5 ------------------------------------------------------------------"
                        echo "$LOG_VAR"
                        echo "--------------------------------------------------------------------------------------------------------------------------------------------------"
                        echo " "


# Check if the values are within expectations
if [[ $ADC_A_MEAN -gt $MAX_ABS_OFFS_LOW_GAIN ]] || [[ $ADC_A_MEAN -lt $((-$MAX_ABS_OFFS_LOW_GAIN)) ]] 
then
    echo "    Measured ch.A mean value ($ADC_A_MEAN) is outside expected range (+/- $MAX_ABS_OFFS_LOW_GAIN)"
    TEST_STATUS=0
fi

if [[ $ADC_B_MEAN -gt $MAX_ABS_OFFS_LOW_GAIN ]] || [[ $ADC_B_MEAN -lt $((-$MAX_ABS_OFFS_LOW_GAIN)) ]] 
then
    echo "    Measured ch.B mean value ($ADC_B_MEAN) is outside expected range (+/- $MAX_ABS_OFFS_LOW_GAIN)"
    TEST_STATUS=0
fi

if [[ $ADC_A_STD -gt $MAX_NOISE_STD ]]
then
    echo "    Measured ch.A std deviation value ($ADC_A_STD) is outside expected range (0-$MAX_NOISE_STD)"
    TEST_STATUS=0
fi

if [[ $ADC_B_STD -gt $MAX_NOISE_STD ]]
then
    echo "    Measured ch.B std deviation value ($ADC_B_STD) is outside expected range (0-$MAX_NOISE_STD)"
    TEST_STATUS=0
fi

if [[ $ADC_A_STD_NO_DEC -gt $MAX_NOISE_STD_NO_DEC ]]
then
    echo "    Measured ch.A std deviation value with no decimation ($ADC_A_STD_NO_DEC) is outside expected range (0-$MAX_NOISE_STD_NO_DEC)"
    TEST_STATUS=0
fi

if [[ $ADC_B_STD_NO_DEC -gt $MAX_NOISE_STD_NO_DEC ]]
then
    echo "    Measured ch.B std deviation value with no decimation ($ADC_B_STD_NO_DEC) is outside expected range (0-$MAX_NOISE_STD_NO_DEC)"
    TEST_STATUS=0
fi

if [[ $ADC_A_PP -gt $MAX_NOISE_P2P ]]
then
    echo "    Measured ch.A p2p value ($ADC_A_PP) is outside expected range (0-$MAX_NOISE_P2P)"
    TEST_STATUS=0
fi

if [[ $ADC_B_PP -gt $MAX_NOISE_P2P ]]
then
    echo "    Measured ch.B p2p value ($ADC_B_PP) is outside expected range (0-$MAX_NOISE_P2P)"
    TEST_STATUS=0
fi


###############################################################################

echo
echo "    Acquisition with DAC signal ($SIG_AMPL Vpp / $SIG_FREQ Hz) - ADCs with LOW gain"
echo

# Turn the DAC signal generator on on both channels
$GENERATE 1 $SIG_AMPL $SIG_FREQ
$GENERATE 2 $SIG_AMPL $SIG_FREQ
sleep 0.5

# Acquire data with 1024 decimation factor
$ACQUIRE $ADC_BUFF_SIZE 1024 > /tmp/adc.txt   # WORKAROUND: First acquisition is thrown away
sleep 0.4
$ACQUIRE $ADC_BUFF_SIZE 1024 > /tmp/adc.txt
cat /tmp/adc.txt | awk '{print $1}' > /tmp/adc_a.txt
cat /tmp/adc.txt | awk '{print $2}' > /tmp/adc_b.txt

# NEW Acquire data also with no decimation
$ACQUIRE $ADC_BUFF_SIZE 1 > /tmp/adc_no_dec.txt
cat /tmp/adc_no_dec.txt | awk '{print $1}' > /tmp/adc_no_dec_a.txt
cat /tmp/adc_no_dec.txt | awk '{print $2}' > /tmp/adc_no_dec_b.txt

# Calculate mean value, std deviation and p2p diff value
ADC_A_MEAN=$(awk '{sum+=$1} END { print int(sum/NR)}' /tmp/adc_a.txt)
ADC_B_MEAN=$(awk '{sum+=$1} END { print int(sum/NR)}' /tmp/adc_b.txt)
##ADC_A_STD=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)**2))}' /tmp/adc_a.txt)
##ADC_B_STD=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)**2))}' /tmp/adc_b.txt)
ADC_A_STD=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)*(sum/NR)))}' /tmp/adc_a.txt)
ADC_B_STD=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)*(sum/NR)))}' /tmp/adc_b.txt)
ADC_A_PP=$(awk 'BEGIN{max=-1000;min=1000} { if (max < $1){ max = $1 }; if(min > $1){ min = $1 } } END{ print max-min}' /tmp/adc_a.txt)
ADC_B_PP=$(awk 'BEGIN{max=-1000;min=1000} { if (max < $1){ max = $1 }; if(min > $1){ min = $1 } } END{ print max-min}' /tmp/adc_b.txt)

# NEW Calculate std deviation in case of no decimation
##ADC_A_STD_NO_DEC=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)**2))}' /tmp/adc_no_dec_a.txt)
##ADC_B_STD_NO_DEC=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)**2))}' /tmp/adc_no_dec_b.txt)
ADC_A_STD_NO_DEC=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)*(sum/NR)))}' /tmp/adc_no_dec_a.txt)
ADC_B_STD_NO_DEC=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)*(sum/NR)))}' /tmp/adc_no_dec_b.txt)


# Print out the measurements
echo "    Measured ADC ch.A mean value is $ADC_A_MEAN"
echo "    Measured ADC ch.A std.deviation value is $ADC_A_STD"
echo "    Measured ADC ch.A std.deviation (no decimation) is $ADC_A_STD_NO_DEC"
echo "    Measured ADC ch.A p2p value is $ADC_A_PP"
echo "    Measured ADC ch.B mean value is $ADC_B_MEAN"
echo "    Measured ADC ch.B std.deviation value is $ADC_B_STD"
echo "    Measured ADC ch.B std.deviation (no decimation) is $ADC_B_STD_NO_DEC"
echo "    Measured ADC ch.B p2p value is $ADC_B_PP"

# Log informations
LOG_VAR="$LOG_VAR $ADC_A_MEAN $ADC_A_STD $ADC_A_STD_NO_DEC $ADC_A_PP $ADC_B_MEAN $ADC_B_STD $ADC_B_STD_NO_DEC $ADC_B_PP"


                        echo " "
                        echo "------------------------------------------------Printing  Log variables  step 6 ------------------------------------------------------------------"
                        echo "$LOG_VAR"
                        echo "--------------------------------------------------------------------------------------------------------------------------------------------------"
                        echo " "
# Check if the values are within expectations
if [[ $ADC_A_MEAN -gt $MAX_ABS_OFFS_LOW_GAIN ]] || [[ $ADC_A_MEAN -lt $((-$MAX_ABS_OFFS_LOW_GAIN)) ]] 
then
    echo "    Measured ch.A mean value ($ADC_A_MEAN) is outside expected range (+/- $MAX_ABS_OFFS_LOW_GAIN)"
    TEST_STATUS=0
fi

if [[ $ADC_B_MEAN -gt $MAX_ABS_OFFS_LOW_GAIN ]] || [[ $ADC_B_MEAN -lt $((-$MAX_ABS_OFFS_LOW_GAIN)) ]] 
then
    echo "    Measured ch.B mean value ($ADC_B_MEAN) is outside expected range (+/- $MAX_ABS_OFFS_LOW_GAIN)"
    TEST_STATUS=0
fi

if [[ $ADC_A_STD -lt $MIN_SIG_STD_LOW_GAIN ]] || [[ $ADC_A_STD -gt $MAX_SIG_STD_LOW_GAIN ]]
then
    echo "    Measured ch.A std deviation value ($ADC_A_STD) is outside expected range ($MIN_SIG_STD_LOW_GAIN-$MAX_SIG_STD_LOW_GAIN)"
    TEST_STATUS=0
fi

if [[ $ADC_B_STD -lt $MIN_SIG_STD_LOW_GAIN ]] || [[ $ADC_B_STD -gt $MAX_SIG_STD_LOW_GAIN ]]
then
    echo "    Measured ch.B std deviation value ($ADC_B_STD) is outside expected range ($MIN_SIG_STD_LOW_GAIN-$MAX_SIG_STD_LOW_GAIN)"
    TEST_STATUS=0
fi

if [[ $ADC_A_PP -lt $MIN_SIG_P2P_LOW_GAIN ]] || [[ $ADC_A_PP -gt $MAX_SIG_P2P_LOW_GAIN ]]
then
    echo "    Measured ch.A p2p value ($ADC_A_PP) is outside expected range ($MIN_SIG_P2P_LOW_GAIN-$MAX_SIG_P2P_LOW_GAIN)"
    TEST_STATUS=0
fi

if [[ $ADC_B_PP -lt $MIN_SIG_P2P_LOW_GAIN ]] || [[ $ADC_B_PP -gt $MAX_SIG_P2P_LOW_GAIN ]]
then
    echo "    Measured ch.B p2p value ($ADC_B_PP) is outside expected range ($MIN_SIG_P2P_LOW_GAIN-$MAX_SIG_P2P_LOW_GAIN)"
    TEST_STATUS=0
fi


# Set DAC value to 0 for both channels (1 & 2)
echo
echo "    Restoring DAC signals and ADC gain to idle conditions"
$GENERATE 1 0 $SIG_FREQ
$GENERATE 2 0 $SIG_FREQ
$MONITOR 0x40000014 w 0x0
echo

sleep $SLEEP_BETWEEN_TESTS

# If test was OK, turn LED 7 on
if [ $TEST_STATUS -eq 1 ]
then
    TEST_GLOBAL_STATUS=$(($TEST_GLOBAL_STATUS+$TEST_VALUE))
    LOGFILE_STATUS=$(($LOGFILE_STATUS+$TEST_VALUE))
    $MONITOR $LED_ADDR w "$(printf '0x%02x' $TEST_GLOBAL_STATUS)"
fi



###############################################################################
# STEP 8: Fast ADCs bit analysis
###############################################################################

echo
echo "--- TEST 8: Fast ADCs bit analysis test ---"
echo

echo "    Acquisition with DAC signal ($SIG_AMPL Vpp / $SIG_FREQ Hz) - ADCs with HIGH gain"
echo

TEST_STATUS=1
TEST_VALUE=256      # used for logfile status, led 8 is controlled separately

# Turn the DAC signal generator on on both channels
$GENERATE 1 $SIG_AMPL $SIG_FREQ
$GENERATE 2 $SIG_AMPL $SIG_FREQ

# Configure the ADCs in high-gain mode
$MONITOR 0x40000014 w 0x80
sleep 0.2
$MONITOR 0x4000001C w 0x80
sleep 0.2

# Acquire data from both channels - 1024 decimation factor
$ACQUIRE $ADC_BUFF_SIZE 1024 > /tmp/$ADC_FILENAME
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
    #BIT_POWER=$(echo "2^$i" | /opt/bin/bc)
    BIT_POWER=$(echo "2^$i" | bc)
    sleep 0.2

    if [ "$BIT_POWER" -gt "$STATE_REG" ]
    then
        echo "    Error: bit $i never changed during the test"
        TEST_STATUS=0
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
    #BIT_POWER=$(echo "2^$i" | /opt/bin/bc)
    BIT_POWER=$(echo "2^$i" | bc)
    sleep 0.2

    if [ $BIT_POWER -gt $STATE_REG ]
    then
        echo "    Error: bit $i never changed during the test"
        TEST_STATUS=0
    else
        STATE_REG=$(($STATE_REG-$BIT_POWER))
    fi
done

sleep $SLEEP_BETWEEN_TESTS

# If test was OK, turn LED 8 on
if [ $TEST_STATUS -eq 1 ]
then
    LOGFILE_STATUS=$(($LOGFILE_STATUS+$TEST_VALUE))
 ##   echo 1 > /sys/class/gpio/gpio0/value
    echo "    Bit analysis test was successfull"
fi


# Set DAC value to 0 for both channels (1 & 2) 
echo
echo "    Restoring DAC signals and ADC gain to idle conditions..."
$GENERATE 1 0 $SIG_FREQ
$GENERATE 2 0 $SIG_FREQ
$MONITOR 0x40000014 w 0x0
echo

sleep $SLEEP_BETWEEN_TESTS

                        echo 
                        echo "------------------------------------------------Printing  Log variables  step 7 ------------------------------------------------------------------"
                        echo "$LOG_VAR"
                        echo "--------------------------------------------------------------------------------------------------------------------------------------------------"
                        echo 
fi
###################################################################################################################################################################################
###################################################################################################################################################################################
###################################################################################################################################################################################
#
# STEP 9: Fast ADCs and DACs CALIBRATION
#
###############################################################################
echo
echo "--- TEST 9: Fast ADCs and DACs CALIBRATION ---"
echo

echo "Set LEDs OFF, Set outputs OFF, Set DIO for relays to output... "
echo 

# SET ALL LEDs OFF
$MONITOR 0x40000030 w 0x00

# Assure tht DAC signals (ch 1 & 2) are OFF
$GENERATE 1 0 $SIG_FREQ
$GENERATE 2 0 $SIG_FREQ

# Set Directions of DIO to output
$MONITOR 0x40000014 w 0xC0   # - N line DIO7_N and DI06_N
sleep 0.4
$MONITOR 0x40000010 w 0x80   # - P line DIO7_P
sleep 0.4

echo "LV jumper settings DC offset calibration is started..."
echo "Setting relay states...-> Set LV jumper settings... ->Connect IN1&IN2 to the GND..."
echo "To continue press ENTER" ### This line will be removed when new test board is used.
echo
read

# Set LV jumper settings Connect IN1&IN2 to the GND -> Set relay states  DIO6_N, DIO7_N and DIO7_P
# DIO6_N = 0
# DIO7_N = 0
# DIO7_P = 0
# Set DIO6_N,DIO7_N,DIO7_P  - also all other DIOs are set to 0
$MONITOR 0x4000001C w 0x00   # N line DIOx_N 
sleep 0.4
$MONITOR 0x40000018 w 0x00   # P line DIOx_P
sleep 0.4

#Acquire data with 1024 decimation factor
$ACQUIRE -e -s $ADC_BUFF_SIZE 1024 > /tmp/adc.txt   # WORKAROUND: First acquisition is thrown away
sleep 0.4
$ACQUIRE -e -s $ADC_BUFF_SIZE 1024 > /tmp/adc.txt
cat /tmp/adc.txt | awk '{print $1}' > /tmp/adc_a.txt
cat /tmp/adc.txt | awk '{print $2}' > /tmp/adc_b.txt

# Calculate mean value
ADC_A_MEAN=$(awk '{sum+=$1} END { print int(sum/NR)}' /tmp/adc_a.txt)
ADC_B_MEAN=$(awk '{sum+=$1} END { print int(sum/NR)}' /tmp/adc_b.txt)

# Print out the measurements
echo "IN1 LV DC offset value is $ADC_A_MEAN"
echo "IN2 LV DC offset value is $ADC_B_MEAN"
echo 

# Check if the values are within expectations 
if [[ $ADC_A_MEAN -gt $MAX_OFF_VALUE_LV ]] || [[ $ADC_A_MEAN -lt $((-$MAX_OFF_VALUE_LV)) ]] 
then
    echo "      Measured IN1 LV DC offset value ($ADC_A_MEAN) is outside expected range (+/- $MAX_OFF_VALUE_LV)"
    TEST_STATUS=0
fi

if [[ $ADC_B_MEAN -gt $MAX_OFF_VALUE_LV ]] || [[ $ADC_B_MEAN -lt $((-$MAX_OFF_VALUE_LV)) ]] 
then
    echo "      Measured IN2 LV DC offset value ($ADC_B_MEAN) is outside expected range (+/- $MAX_OFF_VALUE_LV)"
    TEST_STATUS=0
fi

#For old scope DC_offset cal param must be multiplied by -1
#FE_CH1_DC_offs=$(( -1*ADC_A_MEAN ))
#FE_CH2_DC_offs=$(( -1*ADC_B_MEAN ))

FE_CH1_DC_offs=$(( ADC_A_MEAN ))
FE_CH2_DC_offs=$(( ADC_B_MEAN ))
N1_LV=$ADC_A_MEAN
N2_LV=$ADC_B_MEAN

# Print out the new cal parameters
echo "      NEW IN1 LV DC offset cal param >>FE_CH1_DC_offs<< is $FE_CH1_DC_offs"
echo "      NEW IN2 LV DC offset cal param >>FE_CH2_DC_offs<< is $FE_CH2_DC_offs"
echo 

LOG_VAR="$LOG_VAR $FE_CH1_DC_offs $FE_CH2_DC_offs"

                        echo " "
                        echo "------------------------------------------------Printing  Log variables  step 8 ------------------------------------------------------------------"
                        echo "$LOG_VAR"
                        echo "--------------------------------------------------------------------------------------------------------------------------------------------------"
                        echo " "

echo "Inputs  LV GAIN calibration is started..."
echo "Setting relay states...-> Set LV jumper settings... ->Connect IN1&IN2 to the REF_VALUE_LV..."

# Set LV jumper settings Connect IN1&IN2 to the REF VALUE -> Set relay states  DIO6_N, DIO7_N and DIO7_P
# DIO6_N = 1
# DIO7_N = 0
# DIO7_P = 1

#First reset ouputs 
$MONITOR 0x4000001C w 0x00   # N line DIOx_N 
sleep 0.4
$MONITOR 0x40000018 w 0x00   # P line DIOx_P
sleep 0.4

# Set DIO6_N,DIO7_N,DIO7_P  
$MONITOR 0x4000001C w 0x40   # N line DIOx_N 
sleep 0.4
$MONITOR 0x40000018 w 0x80   # P line DIOx_P
sleep 0.4

echo "Connecting reference voltage 0.9V..."
echo "To continue press ENTER" ### This line will be removed when new test board is used.
echo
read

sleep 0.4 
#Acquire data with 1024 decimation factor
$ACQUIRE -e -s $ADC_BUFF_SIZE 1024 > /tmp/adc.txt   # WORKAROUND: First acquisition is thrown away
sleep 0.4
$ACQUIRE -e -s $ADC_BUFF_SIZE 1024 > /tmp/adc.txt
cat /tmp/adc.txt | awk '{print $1}' > /tmp/adc_a.txt
cat /tmp/adc.txt | awk '{print $2}' > /tmp/adc_b.txt

# Calculate mean value
ADC_A_MEAN=$(awk '{sum+=$1} END { print int(sum/NR)}' /tmp/adc_a.txt)
ADC_B_MEAN=$(awk '{sum+=$1} END { print int(sum/NR)}' /tmp/adc_b.txt)

# Print out the measurements
echo "IN1 mean value is $ADC_A_MEAN"
echo "IN2 mean value is $ADC_B_MEAN"
echo 


# Check if the values are within expectations
if [[ $ADC_A_MEAN -gt $MAX_VALUE_LV ]] || [[ $ADC_A_MEAN -lt $((-$MAX_VALUE_LV)) ]] 
then
    echo "      Measured IN1 value ($ADC_A_MEAN) is outside expected range (+/- $MAX_VALUE_LV)"
    TEST_STATUS=0
fi

if [[ $ADC_B_MEAN -gt $MAX_VALUE_LV ]] || [[ $ADC_B_MEAN -lt $((-$MAX_VALUE_LV)) ]] 
then
    echo "      Measured IN2 value ($ADC_B_MEAN) is outside expected range (+/- $MAX_VALUE_LV)"
    TEST_STATUS=0
fi

#Gain calibration y=xk+n
REF_VALUE_LV=7373       #0.9 VOLTS reference voltage in ADC counts

GAIN1_LV=$(awk -v N1_LV=$N1_LV -v REF_VALUE_LV=$REF_VALUE_LV -v ADC_A_MEAN=$ADC_A_MEAN 'BEGIN { print ( ( REF_VALUE_LV) / ( ADC_A_MEAN-N1_LV ) ) }')
GAIN2_LV=$(awk -v N2_LV=$N2_LV -v REF_VALUE_LV=$REF_VALUE_LV -v ADC_B_MEAN=$ADC_B_MEAN 'BEGIN { print ( ( REF_VALUE_LV) / ( ADC_B_MEAN-N2_LV ) ) }')

# Print out the measurements
echo "IN1_LV_Gain is $GAIN1_LV"
echo "IN2_LV_Gain is $GAIN2_LV"
echo 

FE_CH1_FS_G_LO=$(awk -v GAIN1_LV=$GAIN1_LV 'BEGIN { print int((858993459*GAIN1_LV)) }')
FE_CH2_FS_G_LO=$(awk -v GAIN2_LV=$GAIN2_LV 'BEGIN { print int((858993459*GAIN2_LV)) }')

# Print out the measurements
echo "      NEW IN1 LV gain cal param >>FE_CH1_FS_G_LO<< is $FE_CH1_FS_G_LO"
echo "      NEW IN2 LV gain cal param >>FE_CH2_FS_G_LO<< is $FE_CH2_FS_G_LO"
echo 

LOG_VAR="$LOG_VAR $FE_CH1_FS_G_LO $FE_CH2_FS_G_LO"

                        echo 
                        echo "------------------------------------------------Printing  Log variables  step 9 ------------------------------------------------------------------"
                        echo "$LOG_VAR"
                        echo "--------------------------------------------------------------------------------------------------------------------------------------------------"
                        echo 

#################################################################################
# COPY NEW CALIBRATION PARAMETERS IN TO USER  EPROM SPACE/PARTITION  #
#################################################################################

NEW_CAL_PARAMS="$FE_CH1_FS_G_HI $FE_CH2_FS_G_HI $FE_CH1_FS_G_LO $FE_CH2_FS_G_LO $FE_CH1_DC_offs $FE_CH2_DC_offs $BE_CH1_FS $BE_CH2_FS $BE_CH1_DC_offs $BE_CH2_DC_offs $SOME_eeprom_value $FE_CH1_DC_offs_HI $FE_CH2_DC_offs_HI"

# Copy the NEW CALIBRATION PARAMETERS to the user EEPROM memory partition
echo "Setting the new calibration parameters into the user EEPROM space..."
echo         
        echo $NEW_CAL_PARAMS | $CALIB -w  
        sleep 0.1
        if [ $? -ne 0 ]
        then
        echo
        echo "New calibration parameters are NOT correctly written in the user EEPROM space"
        sleep 1
        fi
echo -ne '\n' | $CALIB -r 
sleep 0.4

# CHECK CALIBRATION
#Acquire data with 1024 decimation factor
$ACQUIRE -e -s $ADC_BUFF_SIZE 1024 > /tmp/adc.txt   # WORKAROUND: First acquisition is thrown away
sleep 0.4
$ACQUIRE -e -s $ADC_BUFF_SIZE 1024 > /tmp/adc.txt
cat /tmp/adc.txt | awk '{print $1}' > /tmp/adc_a.txt
cat /tmp/adc.txt | awk '{print $2}' > /tmp/adc_b.txt

# Calculate mean value
ADC_A_MEAN=$(awk -v N1_LV=$N1_LV -v GAIN1_LV=$GAIN1_LV '{sum+=$1} END { print int( ((sum/NR)-N1_LV)*GAIN1_LV)}' /tmp/adc_a.txt)
ADC_B_MEAN=$(awk -v N2_LV=$N2_LV -v GAIN2_LV=$GAIN2_LV '{sum+=$1} END { print int( ((sum/NR)-N2_LV)*GAIN2_LV)}' /tmp/adc_b.txt)

IN1_ERROR_LV=$(awk -v ADC_A_MEAN=$ADC_A_MEAN -v REF_VALUE_LV=$REF_VALUE_LV 'BEGIN { print (((ADC_A_MEAN-REF_VALUE_LV)/REF_VALUE_LV)*100) }')
IN2_ERROR_LV=$(awk -v ADC_B_MEAN=$ADC_B_MEAN -v REF_VALUE_LV=$REF_VALUE_LV 'BEGIN { print (((ADC_B_MEAN-REF_VALUE_LV)/REF_VALUE_LV)*100) }')

# Print out the measurements
echo "      IN1 LV Error after the calibration is $IN1_ERROR_LV %"
echo "      IN2 LV Error after the calibration is $IN2_ERROR_LV %"
echo 

####################################################################################################
######################### HV jumper settings #######################################################
####################################################################################################

echo "HV jumper settings DC offset calibration is started..."
echo "Setting relay states...-> Set HV jumper settings... ->Connect IN1&IN2 to the GND..."
echo "To continue press ENTER" ### This line will be removed when new test board is used.
echo
read
## Here to add monitor to set relays



 

#Acquire data with 1024 decimation factor
$ACQUIRE -e -s -1 hv -2 hv $ADC_BUFF_SIZE 1024 > /tmp/adc.txt   # WORKAROUND: First acquisition is thrown away
sleep 0.4
$ACQUIRE -e -s -1 hv -2 hv $ADC_BUFF_SIZE 1024 > /tmp/adc.txt
cat /tmp/adc.txt | awk '{print $1}' > /tmp/adc_a.txt
cat /tmp/adc.txt | awk '{print $2}' > /tmp/adc_b.txt

# Calculate mean value
ADC_A_MEAN=$(awk '{sum+=$1} END { print int(sum/NR)}' /tmp/adc_a.txt)
ADC_B_MEAN=$(awk '{sum+=$1} END { print int(sum/NR)}' /tmp/adc_b.txt)

# Print out the measurements
echo "IN1 HV DC offset value is $ADC_A_MEAN"
echo "IN2 HV DC offset value is $ADC_B_MEAN"
echo 

# Check if the values are within expectations 
if [[ $ADC_A_MEAN -gt $MAX_OFF_VALUE_HV ]] || [[ $ADC_A_MEAN -lt $((-$MAX_OFF_VALUE_HV)) ]] 
then
    echo "    Measured IN1 DC offset value ($ADC_A_MEAN) is outside expected range (+/- $MAX_OFF_VALUE_HV)"
    TEST_STATUS=0
fi

if [[ $ADC_B_MEAN -gt $MAX_OFF_VALUE_HV ]] || [[ $ADC_B_MEAN -lt $((-$MAX_OFF_VALUE_HV)) ]] 
then
    echo "    Measured IN2 DC offset value ($ADC_B_MEAN) is outside expected range (+/- $MAX_OFF_VALUE_HV)"
    TEST_STATUS=0
fi

FE_CH1_DC_offs_HI=$(( ADC_A_MEAN ))
FE_CH2_DC_offs_HI=$(( ADC_B_MEAN ))
N1_HV=$ADC_A_MEAN
N2_HV=$ADC_B_MEAN

# Print out the new cal parameters
echo "      NEW IN1 HV DC offset cal param >>FE_CH1_DC_offs_HI<< is $FE_CH1_DC_offs_HI"
echo "      NEW IN2 HV DC offset cal param >>FE_CH2_DC_offs_HI<< is $FE_CH2_DC_offs_HI"
echo 

LOG_VAR="$LOG_VAR $FE_CH1_DC_offs_HI $FE_CH2_DC_offs_HI"

                        echo 
                        echo "------------------------------------------------Printing  Log variables  step 9 ------------------------------------------------------------------"
                        echo "$LOG_VAR"
                        echo "--------------------------------------------------------------------------------------------------------------------------------------------------"
                        echo 

echo "Inputs  HV GAIN calibration is started..."
echo "Setting relay states...-> Set HV jumper settings... ->Connect IN1&IN2 to the REF_VALUE_HV..."
echo "Connecting reference voltage 10.9V..."
echo "To continue press ENTER" ### This line will be removed when new test board is used.
echo
read
sleep 0.4

# Set LV jumper settings Connect IN1&IN2 to the GND -> Set relay states  DIO6_N, DIO7_N and DIO7_P
# DIO6_N = 1
# DIO7_N = 0
# DIO7_P = 1

#First reset ouputs 
$MONITOR 0x4000001C w 0x00   # N line DIOx_N 
sleep 0.4
$MONITOR 0x40000018 w 0x00   # P line DIOx_P
sleep 0.4

# Set DIO6_N,DIO7_N,DIO7_P  
$MONITOR 0x4000001C w 0x40   # N line DIOx_N 
sleep 0.4
$MONITOR 0x40000018 w 0x80   # P line DIOx_P
sleep 0.4

#Acquire data with 1024 decimation factor
$ACQUIRE -e -s -1 hv -2 hv $ADC_BUFF_SIZE 1024 > /tmp/adc.txt   # WORKAROUND: First acquisition is thrown away
sleep 0.4
$ACQUIRE -e -s -1 hv -2 hv $ADC_BUFF_SIZE 1024 > /tmp/adc.txt
cat /tmp/adc.txt | awk '{print $1}' > /tmp/adc_a.txt
cat /tmp/adc.txt | awk '{print $2}' > /tmp/adc_b.txt

# Calculate mean value
ADC_A_MEAN=$(awk '{sum+=$1} END { print int(sum/NR)}' /tmp/adc_a.txt)
ADC_B_MEAN=$(awk '{sum+=$1} END { print int(sum/NR)}' /tmp/adc_b.txt)

# Print out the measurements
echo "      IN1 mean value is $ADC_A_MEAN"
echo "      IN2 mean value is $ADC_B_MEAN"
echo 

# Check if the values are within expectations 
if [[ $ADC_A_MEAN -gt $MAX_VALUE_HV ]] || [[ $ADC_A_MEAN -lt $((-$MAX_VALUE_HV)) ]] 
then
    echo "      Measured IN1 value ($ADC_A_MEAN) is outside expected range (+/- $MAX_VALUE_HV)"
    TEST_STATUS=0
fi

if [[ $ADC_B_MEAN -gt $MAX_VALUE_HV ]] || [[ $ADC_B_MEAN -lt $((-$MAX_VALUE_HV)) ]] 
then
    echo "      Measured IN2 value ($ADC_B_MEAN) is outside expected range (+/- $MAX_VALUE_HV)"
    TEST_STATUS=0
fi

#Gain calibration y=xk+n
REF_VALUE_HV=4465 # 10.9 VOLTS reference voltage IN COUNTS
GAIN1_HV=$(awk -v N1_HV=$N1_HV -v REF_VALUE_HV=$REF_VALUE_HV -v ADC_A_MEAN=$ADC_A_MEAN 'BEGIN { print ( ( REF_VALUE_HV) / ( ADC_A_MEAN-N1_HV ) ) }')
GAIN2_HV=$(awk -v N2_HV=$N2_HV -v REF_VALUE_HV=$REF_VALUE_HV -v ADC_B_MEAN=$ADC_B_MEAN 'BEGIN { print ( ( REF_VALUE_HV) / ( ADC_B_MEAN-N2_HV ) ) }')

# Print out the measurements
echo "IN1_HV_Gain is $GAIN1_HV"
echo "IN2_HV_Gain is $GAIN2_HV"
echo 

FE_CH1_FS_G_HI=$(awk -v GAIN1_HV=$GAIN1_HV 'BEGIN { print int((42949673*GAIN1_HV)) }')
FE_CH2_FS_G_HI=$(awk -v GAIN2_HV=$GAIN2_HV 'BEGIN { print int((42949673*GAIN2_HV)) }')

# Print out the measurements
echo "      NEW IN1 HV gain cal param >>FE_CH1_FS_G_HI<< is $FE_CH1_FS_G_HI"
echo "      NEW IN2 HV gain cal param >>FE_CH2_FS_G_HI<< is $FE_CH2_FS_G_HI"
echo 

LOG_VAR="$LOG_VAR $FE_CH1_FS_G_HI $FE_CH2_FS_G_HI"

                        echo 
                        echo "------------------------------------------------Printing  Log variables  step 9 ------------------------------------------------------------------"
                        echo "$LOG_VAR"
                        echo "--------------------------------------------------------------------------------------------------------------------------------------------------"
                        echo 

#################################################################################
# COPY NEW CALIBRATION PARAMETERS IN TO USER EPROM SPACE/PARTITION  #
#################################################################################

NEW_CAL_PARAMS="$FE_CH1_FS_G_HI $FE_CH2_FS_G_HI $FE_CH1_FS_G_LO $FE_CH2_FS_G_LO $FE_CH1_DC_offs $FE_CH2_DC_offs $BE_CH1_FS $BE_CH2_FS $BE_CH1_DC_offs $BE_CH2_DC_offs $SOME_eeprom_value $FE_CH1_DC_offs_HI $FE_CH2_DC_offs_HI"

# Copy the NEW CALIBRATION PARAMETERS to the user EEPROM memory partition
echo "Setting the new calibration parameters into the user EEPROM space..."
echo         
        echo $NEW_CAL_PARAMS | $CALIB -w  
        sleep 0.1
        if [ $? -ne 0 ]
        then
        echo
        echo "New calibration parameters are NOT correctly written in the user EEPROM space"
        sleep 1
        fi
echo -ne '\n' | $CALIB -r 
sleep 0.4

# CHECK CALIBRATION
#Acquire data with 1024 decimation factor
$ACQUIRE -e -s -1 hv -2 hv $ADC_BUFF_SIZE 1024 > /tmp/adc.txt   # WORKAROUND: First acquisition is thrown away
sleep 0.4
$ACQUIRE -e -s -1 hv -2 hv $ADC_BUFF_SIZE 1024 > /tmp/adc.txt
cat /tmp/adc.txt | awk '{print $1}' > /tmp/adc_a.txt
cat /tmp/adc.txt | awk '{print $2}' > /tmp/adc_b.txt

# Calculate mean value
ADC_A_MEAN=$(awk -v N1_HV=$N1_HV -v GAIN1_HV=$GAIN1_HV '{sum+=$1} END { print int( ((sum/NR)-N1_HV)*GAIN1_HV)}' /tmp/adc_a.txt)
ADC_B_MEAN=$(awk -v N2_HV=$N2_HV -v GAIN2_HV=$GAIN2_HV '{sum+=$1} END { print int( ((sum/NR)-N2_HV)*GAIN2_HV)}' /tmp/adc_b.txt)

IN1_ERROR_HV=$(awk -v ADC_A_MEAN=$ADC_A_MEAN -v REF_VALUE_HV=$REF_VALUE_HV 'BEGIN { print (((ADC_A_MEAN-REF_VALUE_HV)/REF_VALUE_HV)*100) }')
IN2_ERROR_HV=$(awk -v ADC_B_MEAN=$ADC_B_MEAN -v REF_VALUE_HV=$REF_VALUE_HV 'BEGIN { print (((ADC_B_MEAN-REF_VALUE_HV)/REF_VALUE_HV)*100) }')

# Print out the measurements
echo
echo "      IN1 HV Error after the calibration is $IN1_ERROR_HV %"
echo "      IN2 HV Error after the calibration is $IN2_ERROR_HV %"
echo " "

###############################################
########## OUTPUTS CALIBRATION  ###############
###############################################
echo
echo "Outputs DC offset calibration is started..."
echo "Setting relay states...-> Set LV jumper settings... ->Connect OUT1&OUT2 to the IN1&IN2"
echo "To continue press ENTER" ### This line will be removed when new test board is used.
echo
read

#Gain calibration y=xk+n
OUT_AMP_cnt=7373 # 0.9V = 1.8Vpp  VOLTS VPP reference voltage
OUT_AMP=1.8      # VPP

#JUST for checking
#N1_LV=-172
#N2_LV=-70
#BE_CH1_DC_offs=-150
#BE_CH2_DC_offs=-150
#GAIN1_LV=1.0442
#GAIN2_LV=1.0491

# Set LV jumper settings Connect IN1&IN2 to the GND -> Set relay states  DIO6_N, DIO7_N and DIO7_P
# DIO6_N = 1
# DIO7_N = 0
# DIO7_P = 1
#First reset ouputs 
$MONITOR 0x4000001C w 0x00   # N line DIOx_N 
sleep 0.4
$MONITOR 0x40000018 w 0x00   # P line DIOx_P
sleep 0.4

# Set DIO6_N,DIO7_N,DIO7_P  
$MONITOR 0x4000001C w 0x40   # N line DIOx_N 
sleep 0.4
$MONITOR 0x40000018 w 0x80   # P line DIOx_P
sleep 0.4

#Generate DC output signal with amplitude 
$GENERATE 1 0 0 sqr
$GENERATE 2 0 0 sqr
sleep 0.4
#Acquire data with 1024 decimation factor
$ACQUIRE -e -s $ADC_BUFF_SIZE 1024 > /tmp/adc.txt   # WORKAROUND: First acquisition is thrown away
sleep 0.4
$ACQUIRE -e -s $ADC_BUFF_SIZE 1024 > /tmp/adc.txt
cat /tmp/adc.txt | awk '{print $1}' > /tmp/adc_a.txt
cat /tmp/adc.txt | awk '{print $2}' > /tmp/adc_b.txt
# Calculate mean value

ADC_A_MEAN=$(awk -v N1_LV=$N1_LV '{sum+=$1} END { print int((sum/NR)-N1_LV)}' /tmp/adc_a.txt)
ADC_B_MEAN=$(awk -v N2_LV=$N2_LV '{sum+=$1} END { print int((sum/NR)-N2_LV)}' /tmp/adc_b.txt)

OUT1_DC_offs=$(awk -v ADC_A_MEAN=$ADC_A_MEAN -v BE_CH1_DC_offs=$BE_CH1_DC_offs 'BEGIN { print int(BE_CH1_DC_offs-ADC_A_MEAN)}')
OUT2_DC_offs=$(awk -v ADC_B_MEAN=$ADC_B_MEAN -v BE_CH2_DC_offs=$BE_CH2_DC_offs 'BEGIN { print int(BE_CH2_DC_offs-ADC_B_MEAN)}')

# Print out the measurements
echo "OUT1 DC offset value is $OUT1_DC_offs"
echo "OUT2 DC offset value is $OUT2_DC_offs"
echo 

# Check if the values are within expectations 
if [[ $ADC_A_MEAN -gt $MAX_OFF_VALUE_LV ]] || [[ $ADC_A_MEAN -lt $((-$MAX_OFF_VALUE_LV)) ]] 
then
    echo "      Measured OUT1 DC offset value ($ADC_A_MEAN) is outside expected range (+/- $MAX_OFF_VALUE_LV)"
    TEST_STATUS=0
fi

if [[ $ADC_B_MEAN -gt $MAX_OFF_VALUE_LV ]] || [[ $ADC_B_MEAN -lt $((-$MAX_OFF_VALUE_LV)) ]] 
then
    echo "      Measured OUT2 DC offset value ($ADC_B_MEAN) is outside expected range (+/- $MAX_OFF_VALUE_LV)"
    TEST_STATUS=0
fi

BE_CH1_DC_offs=$OUT1_DC_offs 
BE_CH2_DC_offs=$OUT2_DC_offs

# Print out the measurements
echo "      NEW OUT1 DC offset cal param >>BE_CH1_DC_offs<<  is $BE_CH1_DC_offs"
echo "      NEW OUT2 DC offset cal param >>BE_CH2_DC_offs<<  is $BE_CH2_DC_offs"
echo
sleep 0.4 

LOG_VAR="$LOG_VAR $BE_CH1_DC_offs $BE_CH2_DC_offs"

                        echo 
                        echo "------------------------------------------------Printing  Log variables  step 9 ------------------------------------------------------------------"
                        echo "$LOG_VAR"
                        echo "--------------------------------------------------------------------------------------------------------------------------------------------------"
                        echo 

#################################################################################
# COPY NEW CALIBRATION PARAMETERS IN TO USER EPROM SPACE/PARTITION  #
#################################################################################
NEW_CAL_PARAMS="$FE_CH1_FS_G_HI $FE_CH2_FS_G_HI $FE_CH1_FS_G_LO $FE_CH2_FS_G_LO $FE_CH1_DC_offs $FE_CH2_DC_offs $BE_CH1_FS $BE_CH2_FS $BE_CH1_DC_offs $BE_CH2_DC_offs $SOME_eeprom_value $FE_CH1_DC_offs_HI $FE_CH2_DC_offs_HI"

# Copy the NEW CALIBRATION PARAMETERS to the user EEPROM memory partition
echo "Setting the new calibration parameters into the user EEPROM space..."
echo         
        echo $NEW_CAL_PARAMS | $CALIB -w  
       sleep 0.1
       if [ $? -ne 0 ]
        then
        echo
        echo "New calibration parameters are NOT correctly written in the user EEPROM space"
        sleep 1
        fi
echo -ne '\n' | $CALIB -r 

##################################################################################
# OUTPUTS GAIN CALIBRATION 
##################################################################################
echo " "
echo "Output gain calibration is started "
#Generate DC output signal 
$GENERATE 1 $OUT_AMP 0 sqr
$GENERATE 2 $OUT_AMP 0 sqr

#Acquire data with 1024 decimation factor
$ACQUIRE -e -s $ADC_BUFF_SIZE 1024 > /tmp/adc.txt   # WORKAROUND: First acquisition is thrown away
sleep 0.4
$ACQUIRE -e -s $ADC_BUFF_SIZE 1024 > /tmp/adc.txt
cat /tmp/adc.txt | awk '{print $1}' > /tmp/adc_a.txt
cat /tmp/adc.txt | awk '{print $2}' > /tmp/adc_b.txt

# Calculate mean value and correct it using CALIBRATED input gains
ADC_A_MEAN=$(awk -v N1_LV=$N1_LV -v GAIN1_LV=$GAIN1_LV '{sum+=$1} END { print int(((sum/NR)-N1_LV)*GAIN1_LV)}' /tmp/adc_a.txt)
ADC_B_MEAN=$(awk -v N2_LV=$N2_LV -v GAIN2_LV=$GAIN1_LV '{sum+=$1} END { print int(((sum/NR)-N2_LV)*GAIN2_LV)}' /tmp/adc_b.txt)

GAIN1_OUT=$(awk -v BE_CH1_DC_offs=$BE_CH1_DC_offs -v OUT_AMP_cnt=$OUT_AMP_cnt -v ADC_A_MEAN=$ADC_A_MEAN 'BEGIN {print ((ADC_A_MEAN-BE_CH1_DC_offs)/OUT_AMP_cnt) }')
GAIN2_OUT=$(awk -v BE_CH2_DC_offs=$BE_CH2_DC_offs -v OUT_AMP_cnt=$OUT_AMP_cnt -v ADC_B_MEAN=$ADC_B_MEAN 'BEGIN {print ((ADC_B_MEAN-BE_CH2_DC_offs)/OUT_AMP_cnt) }')

# Print out the measurements
echo "GAIN1_OUT is $GAIN1_OUT"
echo "GAIN2_OUT is $GAIN2_OUT"
echo 

BE_CH1_FS=$(awk -v GAIN1_OUT=$GAIN1_OUT 'BEGIN { print int((42949673*GAIN1_OUT)) }')
BE_CH2_FS=$(awk -v GAIN2_OUT=$GAIN2_OUT 'BEGIN { print int((42949673*GAIN2_OUT)) }')

# Print out the measurements
echo "      NEW OUT1 gain cal param >>BE_CH1_FS<< is $BE_CH1_FS"
echo "      NEW OUT2 gain cal param >>BE_CH2_FS<< is $BE_CH2_FS"
echo 



LOG_VAR="$LOG_VAR $BE_CH1_FS $BE_CH2_FS"

                        echo 
                        echo "------------------------------------------------Printing  Log variables  step 9 ------------------------------------------------------------------"
                        echo "$LOG_VAR"
                        echo "--------------------------------------------------------------------------------------------------------------------------------------------------"
                        echo 

#################################################################################
# COPY NEW CALIBRATION PARAMETERS IN TO USER EPROM SPACE/PARTITION  #
#################################################################################

NEW_CAL_PARAMS="$FE_CH1_FS_G_HI $FE_CH2_FS_G_HI $FE_CH1_FS_G_LO $FE_CH2_FS_G_LO $FE_CH1_DC_offs $FE_CH2_DC_offs $BE_CH1_FS $BE_CH2_FS $BE_CH1_DC_offs $BE_CH2_DC_offs $SOME_eeprom_value $FE_CH1_DC_offs_HI $FE_CH2_DC_offs_HI"

# Copy the NEW CALIBRATION PARAMETERS to the user EEPROM memory partition
echo "Setting the new calibration parameters into the user EEPROM space..."
echo         
        echo $NEW_CAL_PARAMS | $CALIB -w  
        sleep 0.1
        if [ $? -ne 0 ]
        then
        echo
        echo "New calibration parameters are NOT correctly written in the user EEPROM space"
        sleep 1
        fi
echo -ne '\n' | $CALIB -r 


###############################################################################
#END - LOGGING TEST RESULTS to Red Pitaya d.d SERVER
###############################################################################
echo "Testing and calibration are FINISHED"
echo "LOG variables are next..."
echo
LOG_VAR="$(printf '0x%03x' $LOGFILE_STATUS) $LOG_VAR"


                        echo " "
                        echo "------------------------------------------------SAVING     Log variables  ------------------------------------------------------------------------"
                        echo "$LOG_VAR"
                        echo "--------------------------------------------------------------------------------------------------------------------------------------------------"
                        echo " "

exit

echo 
echo "Logging test result to the Red Pitaya d.d main server..."
# ADD code




###############################################################################
## END - LOGGING TEST RESULTS
###############################################################################
echo "Logging test result to local PC and SD card"

# SAVE RESULTS INTO THE SD-CARD
# Remount the SD card with R&W rigths
mount -o remount,rw $SD_CARD_PATH

# Log information to the log file
echo "    Logging test statistics to -> $SD_CARD_PATH/$LOG_FILENAME..."
echo $LOG_VAR >> "$SD_CARD_PATH/$LOG_FILENAME"

# Remount the SD card with Read only rigths
mount -o remount,ro $SD_CARD_PATH


# SAVE RESULTS INTO THE FLASH-DRIVE
# Check that the USB drive is plugged on the board
if [ $DRIVE_TEST_STATUS -eq 1 ]
then
    # Create the mounting point folder and mount the device
    mkdir $USB_MOUNT_FOLDER > /dev/null 2>&1
    sleep 0.2
    mount $USB_DEVICE $USB_MOUNT_FOLDER > /dev/null 2>&1
    sleep 0.2

    # Append the test result to the log file
    echo "    Logging test statistics to -> $USB_MOUNT_FOLDER/$LOG_FILENAME..."
    echo $LOG_VAR >> "$USB_MOUNT_FOLDER/$LOG_FILENAME"
    sleep 0.2

    # Umount the device and delete the folder
    umount $USB_MOUNT_FOLDER
    sleep 0.2
    rm -rf $USB_MOUNT_FOLDER
    sleep 0.2
else
    echo "    Not possible to log test statistics to USB drive..."
fi


# SAVE RESULTS INTO THE REMOTE PC
# Create the mounting point folder and mount the device
RES=$(ping "$NFS_SERVER_IP" -c "$N_PING_PKG" | grep 'transmitted' | awk '{print $4}' ) > /dev/null

if [[ "$RES" == "$N_PING_PKG" ]]
then
    echo "    Logging test statistics to -> $NFS_SERVER_DIR/$LOG_FILENAME..."
    mkdir $LOG_MNT_FOLDER > /dev/null 2>&1
    mount -o nolock $NFS_SERVER_DIR $LOG_MNT_FOLDER > /dev/null 2>&1
    sleep 0.5

    echo $LOG_VAR >> "$LOG_MNT_FOLDER/$LOG_FILENAME"
else
    echo "    Not possible to log test statistics to remote PC..."
fi


echo
echo "--- UNIT TEST FINISHED"
echo