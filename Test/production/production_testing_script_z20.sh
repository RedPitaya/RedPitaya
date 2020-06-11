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

FPGA_MODEL=Z20

hexToDec() {
    local VALUE
    read VALUE
    printf "%d\n" "$VALUE"
}

function get_rtrn(){
    echo `echo $1|cut --delimiter=, -f $2`
}

# Path variables
SD_CARD_PATH='/opt/redpitaya'
USB_DEVICE="/dev/sda1"
USB_MOUNT_FOLDER="/mnt/usb"

# TEST Log variables
LOG_FILENAME='manuf_test.log'

# Production PC/SERVER variables
#LOCAL_SERVER_IP='192.168.1.200'
LOCAL_SERVER_IP='192.168.1.2'
LOCAL_SERVER_PASS='redpitaya'
LOCAL_SERVER_DIR='/home/redpitaya/Desktop/Test_LOGS'
LOCAL_USER='redpitaya'
#LOCAL_SERVER_DIR="$LOCAL_SERVER_IP:/home/itech/Desktop/Test_LOGS"

# For loging on the Red Pitaya itself (practicali the same as loging in to SD card it sohuld )
LOG_MNT_FOLDER='/mnt/log'

# Main commands shortcuts
MONITOR="$SD_CARD_PATH/bin/monitor_old"
PRINTENV="fw_printenv"
SETENV="fw_setenv"
GENERATE="$SD_CARD_PATH/bin/generate"
ACQUIRE="$SD_CARD_PATH/bin/acquire"
CALIB="$SD_CARD_PATH/bin/calib"


# Default calibration parameters set during the process
FE_CH1_FS_G_HI=21474836
FE_CH2_FS_G_HI=21474836
FE_CH1_FS_G_LO=858993459
FE_CH2_FS_G_LO=858993459
FE_CH1_DC_offs=0
FE_CH2_DC_offs=0
BE_CH1_FS=42949672
BE_CH2_FS=42949672
BE_CH1_DC_offs=0
BE_CH2_DC_offs=0
SOME_eeprom_value=-143053289  #SOME_eeprom_value is some value in eeprom which is not used for anything but after Crt added hv offset calib values this value also appeard.
FE_CH1_DC_offs_HI=0
FE_CH2_DC_offs_HI=0

#All calibration parameters in one string
FACTORY_CAL="$FE_CH1_FS_G_HI $FE_CH2_FS_G_HI $FE_CH1_FS_G_LO $FE_CH2_FS_G_LO $FE_CH1_DC_offs $FE_CH2_DC_offs $BE_CH1_FS $BE_CH2_FS $BE_CH1_DC_offs $BE_CH2_DC_offs $SOME_eeprom_value $FE_CH1_DC_offs_HI $FE_CH2_DC_offs_HI"

# I2C test configuration
TEST_LABEL='I2C_test'
I2C_TEST_CONFIG='/sys/bus/i2c/devices/0-0051/eeprom 0x1800 0x0400'


# ETHERNET_TEST
ENABLE_ETHERNET_TEST=1

###############################################################################
# Test variables
###############################################################################

LOG_VAR=''
TEST_GLOBAL_STATUS=0
LOGFILE_STATUS=0
LED_ADDR=0x40000030
SLEEP_BETWEEN_TESTS=1

# network
EXP_LINK_SPEED='1000'
EXP_DUPLEX='full'
N_PING_PKG=5
PKT_SIZE=16000
PING_IP=$LOCAL_SERVER_IP
MIN_QR_LENGTH=50             # Set to 50 when using QR scanner
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
N_SATA_CYC=5 # Old value was 10
SEC_PER_CYC=2

# TF rates expressed in W/s (word is 16 bits)
EXP_SATA_RATE=$((125000000/32))
TOLERANCE_PERC=2
MIN_SATA_RATE=$(($EXP_SATA_RATE-$EXP_SATA_RATE*$TOLERANCE_PERC/100))
MAX_SATA_RATE=$(($EXP_SATA_RATE+$EXP_SATA_RATE*$TOLERANCE_PERC/100))

# slow ADCs and DACs
TOLERANCE_PERC=10
REF_RATIO=2
MIN_RATIO=$(bc -l <<< "$REF_RATIO - $REF_RATIO * $TOLERANCE_PERC / 100")
MAX_RATIO=$(bc -l <<< "$REF_RATIO + $REF_RATIO * $TOLERANCE_PERC / 100")

# fast ADCs and DACs data acquisitions
SIG_FREQ=1000000
SIG_AMPL=0.5
ADC_BUFF_SIZE=16384

MAX_ABS_OFFS_HIGH_GAIN=500
MAX_ABS_OFFS_LOW_GAIN=300 # was 250

MAX_NOISE_STD=15 # Old value 8 -> Change to 25 because od +15V switching PS on the test board
MAX_NOISE_STD_NO_DEC=25 # Old value 15 -> Change to 25 because od +15V switching PS on the test board
MAX_NOISE_P2P=80  # # Old value 60 -> Change to 25 because od +15V switching PS on the test board

MIN_SIG_STD_HIGH_GAIN=4200
MAX_SIG_STD_HIGH_GAIN=5500
MIN_SIG_STD_LOW_GAIN=170
MAX_SIG_STD_LOW_GAIN=350

MIN_SIG_P2P_HIGH_GAIN=12000
MAX_SIG_P2P_HIGH_GAIN=16000
MIN_SIG_P2P_LOW_GAIN=450
MAX_SIG_P2P_LOW_GAIN=850

# fast ADCs bit analysis
N_SAMPLES=100

if [ "$FPGA_MODEL" = "Z20" ]; then
    N_ADC_BITS=16
    HALF_ADC_RANGE=32768
else
    N_ADC_BITS=14
    HALF_ADC_RANGE=8192
fi

ADC_FILENAME="adc.sig"
ADC_CH_A_FILENAME="adc_a.sig"
ADC_CH_B_FILENAME="adc_b.sig"



# Calibration parameters LV/HV jumper settings
MAX_VALUE_LV=8000
MAX_VALUE_HV=6000
MAX_OFF_VALUE_LV=300
MAX_OFF_VALUE_HV=300

#Decimal to binary
D2B=({0..1}{0..1}{0..1}{0..1}{0..1}{0..1}{0..1}{0..1}{0..1})

cat /opt/redpitaya/fpga/fpga_0.94.bit > /dev/xdevcfg
sleep 2

# Configure DIOx_P to inputs and DIOx_N to outputs to prevent Relay misbehaviour
# During DIO test this will be changed and after DIO test set back to this condition
$MONITOR 0x40000010 w 0x00 # -> Set P to inputs
sleep 0.2
$MONITOR 0x40000014 w 0xFF # -> Set N to outputs
sleep 0.2

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

    echo "LEDs 1-7 will blink for $((1*$N_REP)) sec - USER: verify"
    echo
    for i in $(seq 1 1 $N_REP)
    do
        $MONITOR $LED_ADDR w 0xFE
        sleep 0.5
        $MONITOR $LED_ADDR w 0x00
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
    echo "--- PRELIMINARY TEST 3.1: DEFAULT environment parameters --- "
    echo


    # Setting enviroment parameters
    echo
    echo "Setting DEFAULT environment parameters on the external EEPROM...."
    cat /opt/redpitaya/environment_parameters.txt > /sys/devices/soc0/amba/e0004000.i2c/i2c-0/0-0050/eeprom
    echo
    sleep 1



    echo
    echo "--- PRELIMINARY TEST 3.2: Read QR code --- "
    echo

    # Ask the operator to enter the QR CODE
    echo "USER: enter the RedPitaya QR CODE to terminal:"
    read LOG_VAR
    sleep 1

    echo  "MAC address is scaned, press enter 2 times to continue"
    read
    read

    # Check QR CODE length and contained MAC, convert it to uppercase
    QR_LENGTH=$(echo $LOG_VAR | wc -m)
    MAC_ADDR=$(echo $LOG_VAR  | awk -F'[=&-]' '{print $3}' | tr '[:lower:]' '[:upper:]' )
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
        MAC_ADDR=$(echo $LOG_VAR  | awk -F'[=&-]' '{print $3}' | tr '[:lower:]' '[:upper:]' )
        MAC_BEGIN=${MAC_ADDR:0:8}

        # Ask the user to confirm the MAC address
        echo
        echo "Entered MAC address is $MAC_ADDR, press ENTER to confirm it, N/n to read it again"
        read  USER_ENTER

        echo
    done

    # Parse also the other QR CODE parameters. HP: they are correct.
    NAV_CODE=$(echo $LOG_VAR | awk -F'[=&-]' '{print $5}')
    HW_REV=$(echo $LOG_VAR   | awk -F'[=&]' '{print $6}')
    #SERIAL=$(echo $LOG_VAR   | awk -F'[=&-]' '{print $0}')
    SERIAL='162400000'

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

    # Set the CALIBRATION PARAMETERS to the FACTORY -wf EEPROM memory partition (factory parameters)
    echo
    echo "Setting the default calibration parameters into the EEPROM..."
    echo $FACTORY_CAL | $CALIB -wf
    sleep 0.2
    if [ $? -ne 0 ]
    then
        echo
        echo "Default calibration parameters are NOT correctly written in the factory EEPROM space"
        sleep 1
    fi

    # Copy the NEW CALIBRATION PARAMETERS to the user EEPROM memory partition
    echo "Setting the  default calibration parameters into the user EEPROM space..."
    echo " "
        echo $FACTORY_CAL | $CALIB -w
        sleep 0.2
        if [ $? -ne 0 ]
        then
        echo
        echo "New calibration parameters are NOT correctly written in the user EEPROM space"
        sleep 1
        fi
    echo -ne '\n' | $CALIB -r

    #In any case, reboot the unit to apply the changes
    echo "Rebooting now to apply Enviroment parameters (MAC address)...."
    echo
    echo "Press enter to continue...."
    read
    sleep 1
    reboot
    exit


 else
    # If enviromante variables are allready saved on RP skip step above step
    # Condition givene above >>if [[ "$MAC_BEGIN" != "$RP_MAC_BEGINNING" ]] || [[ "$READ_NAV" == "" ]] || [[ "$READ_HWREV" == "" ]] || [[ "$READ_SERIAL" == "" ]]

echo
echo
echo "########################################################################"
echo "#           PHASE-1    PRELIMINARY-TESTS    SKIPPED                    #"
echo "########################################################################"
echo
echo "Environment variables are already written to the EEPROM"
echo "Environment variables of this board are next..."
echo
echo "-----------------------------------------------"
echo "EEPROM MAC  $READ_MAC"
echo "NAV         $READ_NAV"
echo "HWREV       $READ_HWREV"
echo "SERIAL      $READ_SERIAL"
echo "-----------------------------------------------"
echo
                # Set default calibration parameters  if the step above is skiped. This is added here
                # beceause Calibration can/will change calibration parameters and if the board  is set once more troguh testing
                # then if cal parameter are not set back to deafult testing step bellow will return an error.
                # Also, when board is set to new testign calibration should be aslo repeated.

                # Set tte CALIBRATION PARAMETERS to the FACTORY -wf EEPROM memory partition (factory parameters)
                echo $FACTORY_CAL | $CALIB -wf
                    sleep 0.2
                    if [ $? -ne 0 ]
                    then
                        echo
                        echo "Default calibration parameters are NOT correctly written in the factory EEPROM space"
                        sleep 1
                    fi
                # Copy the NEW CALIBRATION PARAMETERS to the user EEPROM memory partition
                echo $FACTORY_CAL | $CALIB -w
                    sleep 0.2
                    if [ $? -ne 0 ]
                    then
                        echo
                        echo "Default calibration parameters are NOT correctly written in the user EEPROM space"
                        sleep 1
                    fi

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
TEST_STATUS=1
TEST_VALUE=1
TEST_VALUE_LED=2
#(LED1->TEST_VALUE_LED=2     is used for enviroment parameters test)
#(LED2->TEST_VALUE_LED=4     is used for ethernet, zyng temperature and voltages test)
#(LED3->TEST_VALUE_LED=8     is used for USB drive test)
#(LED4->TEST_VALUE_LED=16    is used for GPIO and SATA tests)
#(LED5->TEST_VALUE_LED=32    is used for slow ADC and DAC tests)
#(LED6->TEST_VALUE_LED=64    is used for fast ADC and DAC + bit analysis tests)
#(LED7->TEST_VALUE_LED=128   is used for calibration tests)

# Again, after reboot, make the LEDs blink
echo "LEDs 1-7 will blink for $((1*$N_REP)) sec"
echo
for i in $(seq 1 1 $N_REP)
do
    $MONITOR $LED_ADDR w 0xFE
    sleep 0.5
    $MONITOR $LED_ADDR w 0x00
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

                        echo " "
                        echo "--------------------------------------Printing  Log variables  step 1: DNA_P1  DNA_P2-------------------------------------------------------------"
                        echo "$LOG_VAR"
                        echo "--------------------------------------------------------------------------------------------------------------------------------------------------"
                        echo " "

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
wr_FE_CH1_DC_offs=$($CALIB -rfv | grep FE_CH1_DC_offs | awk 'FNR == 1 {print $3}')
wr_FE_CH2_DC_offs=$($CALIB -rfv | grep FE_CH2_DC_offs | awk 'FNR == 1 {print $3}')
wr_BE_CH1_FS=$($CALIB -rfv | grep BE_CH1_FS | awk '{print $3}')
wr_BE_CH2_FS=$($CALIB -rfv | grep BE_CH2_FS | awk '{print $3}')
wr_BE_CH1_DC_offs=$($CALIB -rfv | grep BE_CH1_DC_offs | awk '{print $3}')
wr_BE_CH2_DC_offs=$($CALIB -rfv | grep BE_CH2_DC_offs | awk '{print $3}')
wr_SOME_eeprom_value=$($CALIB -rfv | grep Magic | awk '{print $3}')
# SOME_eeprom_value is some value in eeprom which is not used for anything but after Črt added hv offset calib values this value also appeard.
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
    sleep 0.2
    $CALIB -r > /tmp/read_user_cal.txt
    sleep 0.2
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

# Verify the I2C bus functionality through the external EEPROM memory
echo "Verifying the I2C bus functionality..."
echo

# Read the EEPROM variable foreseen for this test
echo "Reading the test string from external EEPROM..."
echo
sleep 0.2

# Create config file for EEPROM on the test board
I2C_TEST_CONFIG_FILE="$(mktemp)"
echo "$I2C_TEST_CONFIG" > "$I2C_TEST_CONFIG_FILE"

READ_LABEL=$($PRINTENV -c "$I2C_TEST_CONFIG_FILE" test_label 2> /dev/null | grep -Po '(?<=test_label\=).+(?=)')
if [[ "$READ_LABEL" != "$TEST_LABEL" ]]
then
    echo "External EEPROM read-back doesn't work. I2C might not work correctly"
    TEST_STATUS=0
    sleep 1
fi

echo "Test lable is: $TEST_LABEL"
echo "Read lable is: $READ_LABEL"

rm "$I2C_TEST_CONFIG_FILE"
sleep $SLEEP_BETWEEN_TESTS

# TEST 0 - Enviroment parameters test - If was OK, turn LED1 ON, Writte  "1"  in logfile status byte
if [ $TEST_STATUS -eq 1 ]
then
    TEST_GLOBAL_STATUS=$(($TEST_GLOBAL_STATUS+$TEST_VALUE_LED))
    LOGFILE_STATUS=$(($LOGFILE_STATUS+$TEST_VALUE))
    $MONITOR $LED_ADDR w "$(printf '0x%02x' $TEST_GLOBAL_STATUS)"
fi
echo
echo
echo "**********************************************************************************"
echo " Current Tests status of: T8-T7-T6-T5-T4-T3-T2-T1-T0 -> ${D2B[$LOGFILE_STATUS]}   "
echo "**********************************************************************************"
echo
#(LED1->TEST_VALUE_LED=2     is used for enviroment parameters test)
#(LED2->TEST_VALUE_LED=4     is used for ethernet, zyng temperature and voltages test)
#(LED3->TEST_VALUE_LED=8     is used for USB drive test)
#(LED4->TEST_VALUE_LED=16    is used for GPIO and SATA tests)
#(LED5->TEST_VALUE_LED=32    is used for slow ADC and DAC tests)
#(LED6->TEST_VALUE_LED=64    is used for fast ADC and DAC + bit analysis tests)
#(LED7->TEST_VALUE_LED=128   is used for calibration tests)

###############################################################################
# STEP 1: Wired network test
###############################################################################

echo
echo "--- TEST 1: Ethernet network test ---"
echo

TEST_STATUS=1
TEST_VALUE=2
TEST_VALUE_LED=4
PREVIOUS_TEST=1
#(LED1->TEST_VALUE_LED=2     is used for enviroment parameters test)
#(LED2->TEST_VALUE_LED=4     is used for ethernet, zyng temperature and voltages test)
#(LED3->TEST_VALUE_LED=8     is used for USB drive test)
#(LED4->TEST_VALUE_LED=16    is used for GPIO and SATA tests)
#(LED5->TEST_VALUE_LED=32    is used for slow ADC and DAC tests)
#(LED6->TEST_VALUE_LED=64    is used for fast ADC and DAC + bit analysis tests)
#(LED7->TEST_VALUE_LED=128   is used for calibration tests)

if [ $ENABLE_ETHERNET_TEST -eq 1 ]
then

# Verify that eth configuration MAC matches the EEPROM MAC
echo "Verify MAC address consistence with EEPROM..."
EEPROM_MAC=$($PRINTENV | grep ethaddr= | awk 'BEGIN {FS="="}{print $2}') > /dev/null 2>&1
LINUX_MAC=$(cat '/sys/class/net/eth0/address' | tr '[:lower:]' '[:upper:]')

#Added, check if teh variable is empty > unsucsefull read will return empty variable. in this case set variable to "x".
if [ -z "$EEPROM_MAC" ]
then
    EEPROM_MAC="x"
    echo "Unsuccessful readout of EEPROM_MAC"
fi

LOG_VAR="$LOG_VAR $EEPROM_MAC"

#Added for new OS
echo "EEPROM_MAC $EEPROM_MAC"
echo "LINUX_MAC $LINUX_MAC"
echo " "

                        echo " "
                        echo "------------------------------------------------Printing  Log variables  step 2 -> MAC ADDRESS----------------------------------------------------"
                        echo "$LOG_VAR"
                        echo "--------------------------------------------------------------------------------------------------------------------------------------------------"
                        echo " "

if [[ "$EEPROM_MAC" != "$LINUX_MAC" ]]
then
    echo "    MAC address is not applied correctly to the network configuration"
    TEST_STATUS=0
    PREVIOUS_TEST=0
fi

# Check the link speed
echo "Verify link speed and ping to host $PING_IP..."

LINK_SPEED=$(cat /sys/class/net/eth0/speed 2> /dev/null)
DUPLEX=$(cat /sys/class/net/eth0/duplex 2> /dev/null)

if [[ "$LINK_SPEED" != "$EXP_LINK_SPEED" ]] || [[ "$DUPLEX" != "$EXP_DUPLEX" ]]
then
    echo "    Network link speed or duplex are unexpected."
    echo "    Link speed is \"$LINK_SPEED\" (\"$EXP_LINK_SPEED\" expected)."
    echo "    Duplex is \"$DUPLEX\" (\"$EXP_DUPLEX\" expected)."
    echo " "
    TEST_STATUS=0
    PREVIOUS_TEST=0
fi

# Ping the defined IP
RES=$(ping "$PING_IP" -c "$N_PING_PKG" -s "$PKT_SIZE" | grep 'transmitted' | awk '{print $4}' ) > /dev/null

if [[ "$RES" != "$N_PING_PKG" ]]
then
    echo "Ping to unit $PING_IP failed"
    TEST_STATUS=0
    PREVIOUS_TEST=0
else
    echo "Ping to unit $PING_IP OKAY"
fi

else
    echo "Skip ethernet test"
fi

sleep $SLEEP_BETWEEN_TESTS

# TEST 1 - Ethernet test - If was OK Writte  "1"  in logfile status byte
if [ $TEST_STATUS -eq 1 ]
then
    # TEST_GLOBAL_STATUS=$(($TEST_GLOBAL_STATUS+$TEST_VALUE_LED))
    LOGFILE_STATUS=$(($LOGFILE_STATUS+$TEST_VALUE))
    # $MONITOR $LED_ADDR w "$(printf '0x%02x' $TEST_GLOBAL_STATUS)"
fi
echo "**********************************************************************************"
echo " Current Tests status of: T8-T7-T6-T5-T4-T3-T2-T1-T0 -> ${D2B[$LOGFILE_STATUS]}   "
echo "**********************************************************************************"
#(LED1->TEST_VALUE_LED=2     is used for enviroment parameters test)
#(LED2->TEST_VALUE_LED=4     is used for ethernet, zyng temperature and voltages test)
#(LED3->TEST_VALUE_LED=8     is used for USB drive test)
#(LED4->TEST_VALUE_LED=16    is used for GPIO and SATA tests)
#(LED5->TEST_VALUE_LED=32    is used for slow ADC and DAC tests)
#(LED6->TEST_VALUE_LED=64    is used for fast ADC and DAC + bit analysis tests)
#(LED7->TEST_VALUE_LED=128   is used for calibration tests)

###############################################################################
# STEP 2: Temperature and Power supply voltages test
###############################################################################

echo
echo "--- TEST 2: Temperature and Power supply voltages test ---"
echo

TEST_STATUS=1
TEST_VALUE=4
#(LED1->TEST_VALUE_LED=2     is used for enviroment parameters test)
#(LED2->TEST_VALUE_LED=4     is used for ethernet, zyng temperature and voltages test)
#(LED3->TEST_VALUE_LED=8     is used for USB drive test)
#(LED4->TEST_VALUE_LED=16    is used for GPIO and SATA tests)
#(LED5->TEST_VALUE_LED=32    is used for slow ADC and DAC tests)
#(LED6->TEST_VALUE_LED=64    is used for fast ADC and DAC + bit analysis tests)
#(LED7->TEST_VALUE_LED=128   is used for calibration tests)

# Acquire temperature and voltages
IN_TEMP0_RAW="$(cat '/sys/bus/iio/devices/iio:device0/in_temp0_raw')"
IN_TEMP0_OFFSET="$(cat '/sys/bus/iio/devices/iio:device0/in_temp0_offset')"
IN_TEMP0_SCALE="$(cat '/sys/bus/iio/devices/iio:device0/in_temp0_scale')"
IN_VOLTAGE0_VCCINT_RAW="$(cat '/sys/bus/iio/devices/iio:device0/in_voltage0_vccint_raw')"
IN_VOLTAGE0_VCCINT_SCALE="$(cat '/sys/bus/iio/devices/iio:device0/in_voltage0_vccint_scale')"
IN_VOLTAGE1_VCCAUX_RAW="$(cat '/sys/bus/iio/devices/iio:device0/in_voltage1_vccaux_raw')"
IN_VOLTAGE1_VCCAUX_SCALE="$(cat '/sys/bus/iio/devices/iio:device0/in_voltage1_vccaux_scale')"
IN_VOLTAGE2_VCCBRAM_RAW="$(cat '/sys/bus/iio/devices/iio:device0/in_voltage2_vccbram_raw')"
IN_VOLTAGE2_VCCBRAM_SCALE="$(cat '/sys/bus/iio/devices/iio:device0/in_voltage2_vccbram_scale')"

TEMP=$(bc -l <<< "($IN_TEMP0_RAW + $IN_TEMP0_OFFSET) * $IN_TEMP0_SCALE / 1000" | awk '{ printf "%d\n", $1 }')
VCCINT=$(bc -l <<< "$IN_VOLTAGE0_VCCINT_RAW * $IN_VOLTAGE0_VCCINT_SCALE" | awk '{ printf "%d\n", $1 }')
VCCAUX=$(bc -l <<< "$IN_VOLTAGE1_VCCAUX_RAW * $IN_VOLTAGE1_VCCAUX_SCALE" | awk '{ printf "%d\n", $1 }')
VCCBRAM=$(bc -l <<< "$IN_VOLTAGE2_VCCBRAM_RAW * $IN_VOLTAGE2_VCCBRAM_SCALE" | awk '{ printf "%d\n", $1 }')

#Added, check if teh variable is empty > unsucsefull read will return empty variable. in this case set variable to "x".
if [ -z "$TEMP" ]
then
    TEMP="x"
    echo "Unsuccessful readout of TEMP"
else
# Check if the values are within expectations
BC_RESULT=$(bc -l <<< "($TEMP < $MIN_TEMP) || ($TEMP > $MAX_TEMP)")
if [[ "$BC_RESULT" = '1' ]]
then
    echo "Measured temperature ($TEMP [deg]) is outside expected range ($MIN_TEMP-$MAX_TEMP)"
    TEST_STATUS=0
else
    echo "Measured temperature ($TEMP [deg]) is within expectations"
fi
fi

#Added, check if teh variable is empty > unsucsefull read will return empty variable. in this case set variable to "x".
if [ -z "$VCCAUX" ]
then
    VCCAUX="x"
    echo "Unsuccessful readout of VCCAUX"
else
# Check if the values are within expectations
BC_RESULT=$(bc -l <<< "($VCCAUX < $MIN_VCCAUX) || ($VCCAUX > $MAX_VCCAUX)")
if [[ "$BC_RESULT" = '1' ]]
then
    echo "Measured VCCAUX ($VCCAUX [mV]) is outside expected range ($MIN_VCCAUX-$MAX_VCCAUX)"
    TEST_STATUS=0
else
    echo "Measured VCCAUX ($VCCAUX [mV]) is within expectations"
fi
fi


#Added, check if teh variable is empty > unsucsefull read will return empty variable. in this case set variable to "x".
if [ -z "$VCCBRAM" ]
then
    VCCBRAM="x"
    echo "Unsuccessful readout of VCCBRAM"
else
# Check if the values are within expectations
BC_RESULT=$(bc -l <<< "($VCCBRAM < $MIN_VCCBRAM) || ($VCCBRAM > $MAX_VCCBRAM)")
if [[ "$BC_RESULT" = '1' ]]
then
    echo "Measured VCCBRAM ($VCCBRAM [mV]) is outside expected range ($MIN_VCCBRAM-$MAX_VCCBRAM)"
    TEST_STATUS=0
else
    echo "Measured VCCBRAM ($VCCBRAM [mV]) is within expectations"
fi
fi

#Added, check if teh variable is empty > unsucsefull read will return empty variable. in this case set variable to "x".
if [ -z "$VCCINT" ]
then
    VCCINT="x"
    echo "Unsuccessful readout of VCCINT"
else
# Check if the values are within expectations
BC_RESULT=$(bc -l <<< "($VCCINT < $MIN_VCCINT) || ($VCCINT > $MAX_VCCINT)")
if [[ "$BC_RESULT" = '1' ]]
then
    echo "Measured VCCINT ($VCCINT [mV]) is outside expected range ($MIN_VCCINT-$MAX_VCCINT)"
    TEST_STATUS=0
else
    echo "Measured VCCINT ($VCCINT [mV]) is within expectations"
fi
fi

# Log temperature and voltages
LOG_VAR="$LOG_VAR $TEMP $VCCAUX $VCCBRAM $VCCINT"

                        echo " "
                        echo "-------------------------------Printing  Log variables  step 3 -> TEMP VCCAUX VCCBRAM VCCINT------------------------------------------------------"
                        echo "$LOG_VAR"
                        echo "--------------------------------------------------------------------------------------------------------------------------------------------------"
                        echo " "

sleep $SLEEP_BETWEEN_TESTS

# TEST 2 - Zyng temperature and voltages test If was OK Writte  "1"  in logfile status byte and set LED2 ON
if [ $TEST_STATUS -eq 1 ]
then
    if [ $PREVIOUS_TEST -eq 1 ]  # Sharing LEDs for more then one test
    then
    TEST_GLOBAL_STATUS=$(($TEST_GLOBAL_STATUS+$TEST_VALUE_LED))
    fi
    LOGFILE_STATUS=$(($LOGFILE_STATUS+$TEST_VALUE))
    $MONITOR $LED_ADDR w "$(printf '0x%02x' $TEST_GLOBAL_STATUS)"
fi
echo
echo
echo "**********************************************************************************"
echo " Current Tests status of: T8-T7-T6-T5-T4-T3-T2-T1-T0 -> ${D2B[$LOGFILE_STATUS]}   "
echo "**********************************************************************************"
echo
#(LED1->TEST_VALUE_LED=2     is used for enviroment parameters test)
#(LED2->TEST_VALUE_LED=4     is used for ethernet, zyng temperature and voltages test)
#(LED3->TEST_VALUE_LED=8     is used for USB drive test)
#(LED4->TEST_VALUE_LED=16    is used for GPIO and SATA tests)
#(LED5->TEST_VALUE_LED=32    is used for slow ADC and DAC tests)
#(LED6->TEST_VALUE_LED=64    is used for fast ADC and DAC + bit analysis tests)
#(LED7->TEST_VALUE_LED=128   is used for calibration tests)

###############################################################################
# STEP 3: USB drive test
###############################################################################

echo
echo "--- TEST 3: USB drive test ---"
echo

DRIVE_TEST_STATUS=1      # Special flag, will be checked at the and for logging purposes
TEST_VALUE=8
TEST_VALUE_LED=8
#(LED1->TEST_VALUE_LED=2     is used for enviroment parameters test)
#(LED2->TEST_VALUE_LED=4     is used for ethernet, zyng temperature and voltages test)
#(LED3->TEST_VALUE_LED=8     is used for USB drive test)
#(LED4->TEST_VALUE_LED=16    is used for GPIO and SATA tests)
#(LED5->TEST_VALUE_LED=32    is used for slow ADC and DAC tests)
#(LED6->TEST_VALUE_LED=64    is used for fast ADC and DAC + bit analysis tests)
#(LED7->TEST_VALUE_LED=128   is used for calibration tests)

# Check that the USB drive is plugged on the board
USB_IN=$(fdisk -l | grep $USB_DEVICE)

if [ $? -ne 0 ]
then
    echo "    USB device not found, check if USB device is connected"
    DRIVE_TEST_STATUS=0

else

    # USB_DEVICE="/dev/sda1"
    # USB_MOUNT_FOLDER="/mnt/usb"
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

# TEST 3 - USB drive test, if was OK Writte  "1"  in logfile status byte and set LED3 ON
if [ $DRIVE_TEST_STATUS -eq 1 ]
then
    TEST_GLOBAL_STATUS=$(($TEST_GLOBAL_STATUS+$TEST_VALUE_LED))
    LOGFILE_STATUS=$(($LOGFILE_STATUS+$TEST_VALUE))
    $MONITOR $LED_ADDR w "$(printf '0x%02x' $TEST_GLOBAL_STATUS)"
fi
echo
echo
echo "**********************************************************************************"
echo " Current Tests status of: T8-T7-T6-T5-T4-T3-T2-T1-T0 -> ${D2B[$LOGFILE_STATUS]}   "
echo "**********************************************************************************"
echo
#(LED1->TEST_VALUE_LED=2     is used for enviroment parameters test)
#(LED2->TEST_VALUE_LED=4     is used for ethernet, zyng temperature and voltages test)
#(LED3->TEST_VALUE_LED=8     is used for USB drive test)
#(LED4->TEST_VALUE_LED=16    is used for GPIO and SATA tests)
#(LED5->TEST_VALUE_LED=32    is used for slow ADC and DAC tests)
#(LED6->TEST_VALUE_LED=64    is used for fast ADC and DAC + bit analysis tests)
#(LED7->TEST_VALUE_LED=128   is used for calibration tests)


###############################################################################
# STEP 4: Serial Communication (SATA) test
###############################################################################
sleep 0.2
TEST_STATUS=1
TEST_VALUE=16
TEST_VALUE_LED=16
PREVIOUS_TEST=1
#(LED1->TEST_VALUE_LED=2     is used for enviroment parameters test)
#(LED2->TEST_VALUE_LED=4     is used for ethernet, zyng temperature and voltages test)
#(LED3->TEST_VALUE_LED=8     is used for USB drive test)
#(LED4->TEST_VALUE_LED=16    is used for GPIO and SATA tests)
#(LED5->TEST_VALUE_LED=32    is used for slow ADC and DAC tests)
#(LED6->TEST_VALUE_LED=64    is used for fast ADC and DAC + bit analysis tests)
#(LED7->TEST_VALUE_LED=128   is used for calibration tests)

# This test is disabled for Z020
# if  [[ "$READ_HWREV" == "STEM_125-10_v1.0" ]] || [[ "$READ_HWREV" == "STEM_10_B_v1.0" ]]
if true
then

echo
echo "--- TEST 4: Serial Communication (SATA) test is SKIPPED---"
echo "--- STEM version is $READ_HWREV---"
echo

else
echo
echo "--- TEST 4: Serial Communication (SATA) test---"
echo

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
            PREVIOUS_TEST=0
        fi

        # IMP: transferred words are stored in a 8bit register. Sometimes negative rate is shown due to register reset
        if [ $TX_RATE -gt $MAX_SATA_RATE ] || [ $TX_RATE -lt $MIN_SATA_RATE ]; then
            echo "$ind:   Tx rate $TX_RATE, between $TX_DEC and $OLD_TX_DEC, out of specification"
            TEST_STATUS=0
            PREVIOUS_TEST=0
        fi

    else
        echo "    $ind: Transfer count is $TX_DEC, error count is $ERR_DEC"
    fi

    OLD_TX_DEC=$TX_DEC

    # every cycle takes in total SEC_PER_CYC seconds, should wait SEC_PER_CYC-0.4
    sleep 1.6
done

fi

sleep $SLEEP_BETWEEN_TESTS

# TEST 4 - SATA test, if was OK Writte  "1"  in logfile status byte
if [ $TEST_STATUS -eq 1 ]
then
    # TEST_GLOBAL_STATUS=$(($TEST_GLOBAL_STATUS+$TEST_VALUE_LED))
    LOGFILE_STATUS=$(($LOGFILE_STATUS+$TEST_VALUE))
    # $MONITOR $LED_ADDR w "$(printf '0x%02x' $TEST_GLOBAL_STATUS)"
fi
echo
echo
echo "**********************************************************************************"
echo " Current Tests status of: T8-T7-T6-T5-T4-T3-T2-T1-T0 -> ${D2B[$LOGFILE_STATUS]}   "
echo "**********************************************************************************"
echo
#(LED1->TEST_VALUE_LED=2     is used for enviroment parameters test)
#(LED2->TEST_VALUE_LED=4     is used for ethernet, zyng temperature and voltages test)
#(LED3->TEST_VALUE_LED=8     is used for USB drive test)
#(LED4->TEST_VALUE_LED=16    is used for GPIO and SATA tests)
#(LED5->TEST_VALUE_LED=32    is used for slow ADC and DAC tests)
#(LED6->TEST_VALUE_LED=64    is used for fast ADC and DAC + bit analysis tests)
#(LED7->TEST_VALUE_LED=128   is used for calibration tests)

###############################################################################
# STEP 5: GPIO connection test
###############################################################################

echo
echo "--- TEST 5: GPIO connection test ---"
echo

TEST_STATUS=1
TEST_VALUE=32
#(LED1->TEST_VALUE_LED=2     is used for enviroment parameters test)
#(LED2->TEST_VALUE_LED=4     is used for ethernet, zyng temperature and voltages test)
#(LED3->TEST_VALUE_LED=8     is used for USB drive test)
#(LED4->TEST_VALUE_LED=16    is used for GPIO and SATA tests)
#(LED5->TEST_VALUE_LED=32    is used for slow ADC and DAC tests)
#(LED6->TEST_VALUE_LED=64    is used for fast ADC and DAC + bit analysis tests)
#(LED7->TEST_VALUE_LED=128   is used for calibration tests)

# Configure both ports as input ports (avoid N and P in output configuration)
# Here we change DIOx_N and DIOx_P so relays can switch uncontrolabre if the EARTH is not connected to the TEST BOARD.
# But even if this happens (relay switching) it will not affect on the TEST 5.
# After TEST 5 we will set DIOx_P to inputs and DIOx_N to outputs to prevent relay uncontrolabre switching
$MONITOR 0x40000010 w 0x00
sleep 0.2
$MONITOR 0x40000014 w 0x00
sleep 0.2

# P->N configuration -Configure P as output port
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

# N->P configuration - Configure P back as input port, and N as output port
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

sleep $SLEEP_BETWEEN_TESTS

# TEST 5 - GPIO test, if was OK Writte  "1"  in logfile status byte and set LED4 ON
if [ $TEST_STATUS -eq 1 ]
then
    if [ $PREVIOUS_TEST -eq 1 ]
    then
    TEST_GLOBAL_STATUS=$(($TEST_GLOBAL_STATUS+$TEST_VALUE_LED))
    fi
    LOGFILE_STATUS=$(($LOGFILE_STATUS+$TEST_VALUE))
    $MONITOR $LED_ADDR w "$(printf '0x%02x' $TEST_GLOBAL_STATUS)"
fi
echo
echo
echo "**********************************************************************************"
echo " Current Tests status of: T8-T7-T6-T5-T4-T3-T2-T1-T0 -> ${D2B[$LOGFILE_STATUS]}   "
echo "**********************************************************************************"
echo
#(LED1->TEST_VALUE_LED=2     is used for enviroment parameters test)
#(LED2->TEST_VALUE_LED=4     is used for ethernet, zyng temperature and voltages test)
#(LED3->TEST_VALUE_LED=8     is used for USB drive test)
#(LED4->TEST_VALUE_LED=16    is used for GPIO and SATA tests)
#(LED5->TEST_VALUE_LED=32    is used for slow ADC and DAC tests)
#(LED6->TEST_VALUE_LED=64    is used for fast ADC and DAC + bit analysis tests)
#(LED7->TEST_VALUE_LED=128   is used for calibration tests)

# DIO test is finished- Configure DIOx_P to inputs and DIOx_N to outputs to prevent Relay misbehaviour
$MONITOR 0x40000010 w 0x00 # -> Set P to inputs
sleep 0.2
$MONITOR 0x40000014 w 0xFF # -> Set N to outputs
sleep 0.2

###############################################################################
# STEP 6: Slow ADCs and DACs test
###############################################################################

echo
echo "--- TEST 6: Slow ADCs and DACs test ---"
echo

TEST_STATUS=1
TEST_VALUE=64
TEST_VALUE_LED=32
#(LED1->TEST_VALUE_LED=2     is used for enviroment parameters test)
#(LED2->TEST_VALUE_LED=4     is used for ethernet, zyng temperature and voltages test)
#(LED3->TEST_VALUE_LED=8     is used for USB drive test)
#(LED4->TEST_VALUE_LED=16    is used for GPIO and SATA tests)
#(LED5->TEST_VALUE_LED=32    is used for slow ADC and DAC tests)
#(LED6->TEST_VALUE_LED=64    is used for fast ADC and DAC + bit analysis tests)
#(LED7->TEST_VALUE_LED=128   is used for calibration tests)

# Load the Mercury firmware to control the analog inputs and outputs
cat /opt/redpitaya/fpga/mercury/fpga.bit > /dev/xdevcfg
sleep 2
$MONITOR 0x40000010 w 0x00 # -> Set P to inputs
sleep 0.2
$MONITOR 0x40000014 w 0x00 # -> Set N to inputs
sleep 0.2

# Set the half-scale output level of all four DACs (5M)
DAC_VALUE=0x4C4B40
$MONITOR 0x40020020 w $DAC_VALUE
sleep 0.2
$MONITOR 0x40020024 w $DAC_VALUE
sleep 0.2
$MONITOR 0x40020028 w $DAC_VALUE
sleep 0.2
$MONITOR 0x4002002C w $DAC_VALUE
sleep 0.2

getAiValue() {
    local AI_PATH='/sys/bus/iio/devices/iio:device1'
    local AI_RAW="$(cat ${AI_PATH}/${1}_raw)"
    local AI_SCALE="$(cat ${AI_PATH}/${1}_scale)"
    bc -l <<< "${AI_RAW} * ${AI_SCALE}"
}

# Get the input level of all four ADCs
ADC1_A="$(getAiValue in_voltage11_vaux8)"
ADC2_A="$(getAiValue in_voltage9_vaux0)"
ADC3_A="$(getAiValue in_voltage10_vaux1)"
ADC4_A="$(getAiValue in_voltage12_vaux9)"

echo "    ADC values - first acquisition - are $ADC1_A, $ADC2_A, $ADC3_A, $ADC4_A"

# Set almost full-scale output level of all four DACs (2x5M=10M)
DAC_VALUE=0x989680
$MONITOR 0x40020020 w $DAC_VALUE
sleep 0.2
$MONITOR 0x40020024 w $DAC_VALUE
sleep 0.2
$MONITOR 0x40020028 w $DAC_VALUE
sleep 0.2
$MONITOR 0x4002002C w $DAC_VALUE
sleep 0.2

# Get again the input level of all four DACs
ADC1_B="$(getAiValue in_voltage11_vaux8)"
ADC2_B="$(getAiValue in_voltage9_vaux0)"
ADC3_B="$(getAiValue in_voltage10_vaux1)"
ADC4_B="$(getAiValue in_voltage12_vaux9)"

echo "    ADC values - second acquisition - are $ADC1_B, $ADC2_B, $ADC3_B, $ADC4_B"

# Evaluate the ratios
ADC1_R=$(bc -l <<< "$ADC1_B / $ADC1_A")
ADC2_R=$(bc -l <<< "$ADC2_B / $ADC2_A")
ADC3_R=$(bc -l <<< "$ADC3_B / $ADC3_A")
ADC4_R=$(bc -l <<< "$ADC4_B / $ADC4_A")

BC_RESULT=$(bc -l <<< "\
($ADC1_R >= $MIN_RATIO) && \
($ADC1_R <= $MAX_RATIO) && \
($ADC2_R >= $MIN_RATIO) && \
($ADC2_R <= $MAX_RATIO) && \
($ADC3_R >= $MIN_RATIO) && \
($ADC3_R <= $MAX_RATIO) && \
($ADC4_R >= $MIN_RATIO) && \
($ADC4_R <= $MAX_RATIO) \
")

if [[ "$BC_RESULT" = '1' ]]
then
    echo "    Measured ratios after two slow ADC acquisitions match expected ratio (2)"
else
    echo "    Measured ratios after two slow ADC acquisitions ($ADC1_R, $ADC2_R, $ADC3_R, $ADC4_R) don't match expected ratio (2)"
    TEST_STATUS=0
fi

sleep $SLEEP_BETWEEN_TESTS


# TEST 6 - Slow ADC/DAC test, if was OK Writte  "1"  in logfile status byte and set LED5 ON
if [ $TEST_STATUS -eq 1 ]
then
    TEST_GLOBAL_STATUS=$(($TEST_GLOBAL_STATUS+$TEST_VALUE_LED))
    LOGFILE_STATUS=$(($LOGFILE_STATUS+$TEST_VALUE))
    $MONITOR $LED_ADDR w "$(printf '0x%02x' $TEST_GLOBAL_STATUS)"
fi
echo
echo
echo "**********************************************************************************"
echo " Current Tests status of: T8-T7-T6-T5-T4-T3-T2-T1-T0 -> ${D2B[$LOGFILE_STATUS]}   "
echo "**********************************************************************************"
echo
#(LED1->TEST_VALUE_LED=2     is used for enviroment parameters test)
#(LED2->TEST_VALUE_LED=4     is used for ethernet, zyng temperature and voltages test)
#(LED3->TEST_VALUE_LED=8     is used for USB drive test)
#(LED4->TEST_VALUE_LED=16    is used for GPIO and SATA tests)
#(LED5->TEST_VALUE_LED=32    is used for slow ADC and DAC tests)
#(LED6->TEST_VALUE_LED=64    is used for fast ADC and DAC + bit analysis tests)
#(LED7->TEST_VALUE_LED=128   is used for calibration tests)

# Restore FPGA firmware
cat /opt/redpitaya/fpga/fpga_0.94.bit > /dev/xdevcfg
sleep 2
$MONITOR 0x40000010 w 0x00 # -> Set P to inputs
sleep 0.2
$MONITOR 0x40000014 w 0x00 # -> Set N to inputs
sleep 0.2

# Restore LED state
$MONITOR $LED_ADDR w "$(printf '0x%02x' $TEST_GLOBAL_STATUS)"
sleep 0.2

###############################################################################
# STEP 7: Fast ADCs bit analysis
###############################################################################
echo
echo "--- TEST 7: Fast ADCs bit analysis test ---"
echo

echo "    Acquisition with DAC signal ($SIG_AMPL Vpp / $SIG_FREQ Hz) - ADCs with HIGH gain"
echo

PREVIOUS_TEST=0
TEST_STATUS=1
TEST_VALUE=128
#(LED1->TEST_VALUE_LED=2     is used for enviroment parameters test)
#(LED2->TEST_VALUE_LED=4     is used for ethernet, zyng temperature and voltages test)
#(LED3->TEST_VALUE_LED=8     is used for USB drive test)
#(LED4->TEST_VALUE_LED=16    is used for GPIO and SATA tests)
#(LED5->TEST_VALUE_LED=32    is used for slow ADC and DAC tests)
#(LED6->TEST_VALUE_LED=64    is used for fast ADC and DAC + bit analysis tests)
#(LED7->TEST_VALUE_LED=128   is used for calibration tests)


# Configure DIOx_P to inputs and DIOx_N to outputs to prevent Relay misbehaviour
$MONITOR 0x40000010 w 0x00 # -> Set P to inputs
sleep 0.2
$MONITOR 0x40000014 w 0xFF # -> Set N to outputs
sleep 0.2

# Set Jumper setting back to LV and set OUT1-IN1 , and OUT2-IN2
# Configure relays  DIO5_N = 1, DIO6_N = 0, DIO7_N = 1 (lv jumper settings)
$MONITOR 0x4000001C w 0x00 # -> Reset N
sleep 0.2
$MONITOR 0x4000001C w 0xA0 # -> Set DIO5_N=1 -> Configure the ADC in high-gain mode -> DIO7_N = 1
sleep 0.2

# Turn the DAC signal generator on on both channels
$GENERATE 1 $SIG_AMPL $SIG_FREQ
$GENERATE 2 $SIG_AMPL $SIG_FREQ

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

# TEST 8 - Fast ADC/DAC bit analysis test, if was OK Writte  "1"  in logfile status byte set LED6 ON
if [ $TEST_STATUS -eq 1 ]
then
    echo "    Bit analysis test was successfull"
    PREVIOUS_TEST=1
    TEST_GLOBAL_STATUS=$(($TEST_GLOBAL_STATUS+$TEST_VALUE_LED))
    LOGFILE_STATUS=$(($LOGFILE_STATUS+$TEST_VALUE))
    $MONITOR $LED_ADDR w "$(printf '0x%02x' $TEST_GLOBAL_STATUS)"
fi
echo
echo
echo "**********************************************************************************"
echo " Current Tests status of: T8-T7-T6-T5-T4-T3-T2-T1-T0 -> ${D2B[$LOGFILE_STATUS]}   "
echo "**********************************************************************************"
echo
#(LED1->TEST_VALUE_LED=2     is used for enviroment parameters test)
#(LED2->TEST_VALUE_LED=4     is used for ethernet, zyng temperature and voltages test)
#(LED3->TEST_VALUE_LED=8     is used for USB drive test)
#(LED4->TEST_VALUE_LED=16    is used for GPIO and SATA tests)
#(LED5->TEST_VALUE_LED=32    is used for slow ADC and DAC tests)
#(LED6->TEST_VALUE_LED=64    is used for fast ADC and DAC + bit analysis tests)
#(LED7->TEST_VALUE_LED=128   is used for calibration tests)

echo "    Restoring DAC signals and ADC gain to idle conditions..."
$GENERATE 1 0 $SIG_FREQ
$GENERATE 2 0 $SIG_FREQ
echo
sleep $SLEEP_BETWEEN_TESTS

###############################################################################
# STEP 8: SFDR spectrum test ---
###############################################################################
echo
echo "--- TEST 8: SFDR spectrum test ---"
echo

echo

TEST_STATUS=1
TEST_VALUE=256
#(LED1->TEST_VALUE_LED=2     is used for enviroment parameters test)
#(LED2->TEST_VALUE_LED=4     is used for ethernet, zyng temperature and voltages test)
#(LED3->TEST_VALUE_LED=8     is used for USB drive test)
#(LED4->TEST_VALUE_LED=16    is used for GPIO and SATA tests)
#(LED5->TEST_VALUE_LED=32    is used for slow ADC and DAC tests)
#(LED6->TEST_VALUE_LED=64    is used for fast ADC and DAC + bit analysis tests)
#(LED7->TEST_VALUE_LED=128   is used for calibration tests)

if [ $PREVIOUS_TEST -eq 1 ]
then

        exec 5>&1
        SFDR_VAL=$(/opt/redpitaya/bin/production_testing_script_z20_spectrum.sh|tee >(cat - >&5))
        SFDR_RESULT=$(gawk '{match($0, /SFDR_TEST_STATUS=[0-9]+/, a)}{split(a[0],b,"=")}{print b[2]}' <<< "${SFDR_VAL}")
        if [[ $SFDR_RESULT -eq 1 ]]
        then
        TEST_STATUS=0
        fi
        SFDR_VAL_RES=$(gawk '{match($0, /RES_SFDR=\s(.+);/, a)};{gsub("RES_SFDR=","",a[0])};{gsub(";","",a[0])};{print a[0]}' <<< "${SFDR_VAL}")
	SFDR_VAL_RES=$(gawk '{$1=$1};1' <<< "${SFDR_VAL_RES}")
	SFDR_VAL_RES=$(tr -dc '[:print:]' <<< "$SFDR_VAL_RES")
        LOG_VAR="$LOG_VAR $SFDR_VAL_RES"
else
echo "SFDR spectrum test skipped"
LOG_VAR="$LOG_VAR 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0"
TEST_STATUS=0
fi
sleep $SLEEP_BETWEEN_TESTS

echo "TEST_STATUS=$TEST_STATUS"
echo "$SFDR_VAL"

# TEST 8 - Fast ADC/DAC bit analysis test, if was OK Writte  "1"  in logfile status byte set LED6 ON
if [ $TEST_STATUS -eq 1 ]
then

    if [ $PREVIOUS_TEST -eq 1 ]
    then
    TEST_GLOBAL_STATUS=$(($TEST_GLOBAL_STATUS+$TEST_VALUE_LED))
    fi
    LOGFILE_STATUS=$(($LOGFILE_STATUS+$TEST_VALUE))
    $MONITOR $LED_ADDR w "$(printf '0x%02x' $TEST_GLOBAL_STATUS)"
fi
echo
echo
echo "**********************************************************************************"
echo " Current Tests status of: T8-T7-T6-T5-T4-T3-T2-T1-T0 -> ${D2B[$LOGFILE_STATUS]}   "
echo "**********************************************************************************"
echo
#(LED1->TEST_VALUE_LED=2     is used for enviroment parameters test)
#(LED2->TEST_VALUE_LED=4     is used for ethernet, zyng temperature and voltages test)
#(LED3->TEST_VALUE_LED=8     is used for USB drive test)
#(LED4->TEST_VALUE_LED=16    is used for GPIO and SATA tests)
#(LED5->TEST_VALUE_LED=32    is used for slow ADC and DAC tests)
#(LED6->TEST_VALUE_LED=64    is used for fast ADC and DAC + bit analysis tests)
#(LED7->TEST_VALUE_LED=128   is used for calibration tests)

echo
sleep $SLEEP_BETWEEN_TESTS

###################################################################################################################################################################################
# Fast ADCs and DACs test add missing values
###############################################################################

LOG_VAR="$LOG_VAR 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0"


###################################################################################################################################################################################
# STEP 9: Fast ADCs and DACs CALIBRATION
###############################################################################
if [ $LOGFILE_STATUS -eq 511 ]
then
echo
echo "--- TEST 9: Reset to default ADCs and DACs CALIBRATION ---"
echo
echo "Reseting cal parameters to unit gains and zerro DC offset..."

TEST_STATUS=1                # It is not used in calibration but i have left it in IF checking so it can be used later if needed.
CALIBRATION_STATUS=1

# Calibration parameters set during the process
FE_CH1_FS_G_HI=42949672
FE_CH2_FS_G_HI=42949672
FE_CH1_FS_G_LO=858993459
FE_CH2_FS_G_LO=858993459
FE_CH1_DC_offs=0
FE_CH2_DC_offs=0
BE_CH1_FS=42949672
BE_CH2_FS=42949672
BE_CH1_DC_offs=0
BE_CH2_DC_offs=0
SOME_eeprom_value=-143053289  #SOME_eeprom_value is some value in eeprom which is not used for anything but after Crt added hv offset calib values this value also appeard.
FE_CH1_DC_offs_HI=0
FE_CH2_DC_offs_HI=0

# Copy the NEW CALIBRATION PARAMETERS to the user EEPROM memory partition
NEW_CAL_PARAMS="$FE_CH1_FS_G_HI $FE_CH2_FS_G_HI $FE_CH1_FS_G_LO $FE_CH2_FS_G_LO $FE_CH1_DC_offs $FE_CH2_DC_offs $BE_CH1_FS $BE_CH2_FS $BE_CH1_DC_offs $BE_CH2_DC_offs $SOME_eeprom_value $FE_CH1_DC_offs_HI $FE_CH2_DC_offs_HI"
echo "Setting the gains = 1 and zerro DC offset calibration parameters into the user EEPROM space..."
echo
 # Set the CALIBRATION PARAMETERS to the FACTORY -wf EEPROM memory partition (factory parameters)
    echo
    echo "Setting the default calibration parameters into the EEPROM..."
    echo $NEW_CAL_PARAMS | $CALIB -wf
    sleep 0.2
    if [ $? -ne 0 ]
    then
        echo
        echo "Default calibration parameters are NOT correctly written in the factory EEPROM space"
	CALIBRATION_STATUS=0
	TEST_STATUS=0
        sleep 1
    fi

    # Copy the NEW CALIBRATION PARAMETERS to the user EEPROM memory partition
    echo "Setting the default calibration parameters into the user EEPROM space..."
    echo " "
        echo $NEW_CAL_PARAMS | $CALIB -w
        sleep 0.2
        if [ $? -ne 0 ]
        then
        echo
        echo "New calibration parameters are NOT correctly written in the user EEPROM space"
	CALIBRATION_STATUS=0
	TEST_STATUS=0
        sleep 1
        fi

sleep 0.4

# Verify the values of the factory calibration parameters
echo "Verifying the default calibration parameters in the EEPROM..."
echo
wr_FE_CH1_FS_G_HI=$($CALIB -rfv | grep FE_CH1_FS_G_HI | awk '{print $3}')
wr_FE_CH2_FS_G_HI=$($CALIB -rfv | grep FE_CH2_FS_G_HI | awk '{print $3}')
wr_FE_CH1_FS_G_LO=$($CALIB -rfv | grep FE_CH1_FS_G_LO | awk '{print $3}')
wr_FE_CH2_FS_G_LO=$($CALIB -rfv | grep FE_CH2_FS_G_LO | awk '{print $3}')
wr_FE_CH1_DC_offs=$($CALIB -rfv | grep FE_CH1_DC_offs | awk 'FNR == 1 {print $3}')
wr_FE_CH2_DC_offs=$($CALIB -rfv | grep FE_CH2_DC_offs | awk 'FNR == 1 {print $3}')
wr_BE_CH1_FS=$($CALIB -rfv | grep BE_CH1_FS | awk '{print $3}')
wr_BE_CH2_FS=$($CALIB -rfv | grep BE_CH2_FS | awk '{print $3}')
wr_BE_CH1_DC_offs=$($CALIB -rfv | grep BE_CH1_DC_offs | awk '{print $3}')
wr_BE_CH2_DC_offs=$($CALIB -rfv | grep BE_CH2_DC_offs | awk '{print $3}')
wr_SOME_eeprom_value=$($CALIB -rfv | grep Magic | awk '{print $3}')
# SOME_eeprom_value is some value in eeprom which is not used for anything but after Črt added hv offset calib values this value also appeard.
wr_FE_CH1_DC_offs_HI=$($CALIB -rfv | grep FE_CH1_DC_offs_HI | awk '{print $3}')
wr_FE_CH2_DC_offs_HI=$($CALIB -rfv | grep FE_CH2_DC_offs_HI | awk '{print $3}')


if [ "$wr_FE_CH1_FS_G_HI" -ne "$FE_CH1_FS_G_HI" ] || [ "$wr_FE_CH2_FS_G_HI" -ne "$FE_CH2_FS_G_HI" ] || [ "$wr_FE_CH1_FS_G_LO" -ne "$FE_CH1_FS_G_LO" ] || [ "$wr_FE_CH2_FS_G_LO" -ne "$FE_CH2_FS_G_LO" ] || [ "$wr_FE_CH1_DC_offs" -ne "$FE_CH1_DC_offs" ] || [ "$wr_FE_CH2_DC_offs" -ne "$FE_CH2_DC_offs" ] || [ "$wr_BE_CH1_FS" -ne "$BE_CH1_FS" ] || [ "$wr_BE_CH2_FS" -ne "$BE_CH2_FS" ] || [ "$wr_BE_CH1_DC_offs" -ne "$BE_CH1_DC_offs" ] || [ "$wr_BE_CH2_DC_offs" -ne "$BE_CH2_DC_offs" ] || [ "$wr_FE_CH1_DC_offs_HI" -ne "$FE_CH1_DC_offs_HI" ] || [ "$wr_FE_CH2_DC_offs_HI" -ne "$FE_CH2_DC_offs_HI" ]
then
    echo "Default calibration parameters are NOT correctly set into the EEPROM..."
        CALIBRATION_STATUS=0
	TEST_STATUS=0
    echo
else
    # Verify the values of the written calibration parameters
    echo "Verifying the user calibration parameters in the EEPROM..."
    echo
    $CALIB -rf > /tmp/read_factory_cal.txt
    sleep 0.2
    $CALIB -r > /tmp/read_user_cal.txt
    sleep 0.2
    PARAM_CMP=$(cmp /tmp/read_factory_cal.txt /tmp/read_user_cal.txt)

    if [ $? != 0 ]
    then
        echo "User calibration parameters are NOT correctly set into the EEPROM..."
        CALIBRATION_STATUS=0
	TEST_STATUS=0
        echo
    fi
fi

CALIBRATION_LOG_PARAMS="$FE_CH1_DC_offs $FE_CH2_DC_offs $FE_CH1_FS_G_LO $FE_CH2_FS_G_LO $FE_CH1_DC_offs_HI $FE_CH2_DC_offs_HI $FE_CH1_FS_G_HI $FE_CH2_FS_G_HI $BE_CH1_DC_offs $BE_CH2_DC_offs $BE_CH1_FS $BE_CH2_FS"
LOG_VAR="$LOG_VAR $CALIBRATION_LOG_PARAMS $CALIBRATION_STATUS" # If clibraation fail log factory cal data


else   # From if [ $LOGFILE_STATUS -eq 511 ] conditon
echo
echo "*******************************************************"
echo "       Test has faild and Calibration is SKIPPED       "
echo "*******************************************************"
echo
CALIBRATION_STATUS=2
CALIBRATION_SKIPPED_LOG_PARAMS="$FE_CH1_DC_offs $FE_CH2_DC_offs $FE_CH1_FS_G_LO $FE_CH2_FS_G_LO $FE_CH1_DC_offs_HI $FE_CH2_DC_offs_HI $FE_CH1_FS_G_HI $FE_CH2_FS_G_HI $BE_CH1_DC_offs $BE_CH2_DC_offs $BE_CH1_FS $BE_CH2_FS"
LOG_VAR="$LOG_VAR $CALIBRATION_SKIPPED_LOG_PARAMS $CALIBRATION_STATUS" # If clibraation fail log factory cal data
fi     # From if [ $LOGFILE_STATUS -eq 511 ] conditon






# Setting relay states, Set LV jumper settings, Connect OUT1&OUT2 to the IN1&IN2 - once more just in case
# Set Directions of DIO
$MONITOR 0x40000010 w 0x00   # - P line inputs
sleep 0.2
$MONITOR 0x40000014 w 0xff   # - N outputs
sleep 0.2
$MONITOR 0x4000001C w 0xA0   # -> Set N
sleep 0.2



########################################################################################
#END - LOGGING TEST RESULTS to Red Pitaya d.d SERVER - LOCAL PC - SD CARD and usb DRIVE
########################################################################################
echo "Testing and calibration are completed..."
echo

# Added HW REVISON to final LOG parameters.
# HW revision is read out from envirometn parameters
echo "Added HW_REV in final log file"
READ_HWREV_LOG="$(echo $READ_HWREV | tr _ -)"
echo "$READ_HWREV_LOG"


LOG_VAR="$(printf '0x%03x' $LOGFILE_STATUS) $LOG_VAR $READ_HWREV_LOG"

                        echo " "
                        echo "----------------------------******** FINAL LOG PARAMETERS ********--------------------------------------------------------------------------------"
                        echo "$LOG_VAR "
                        echo "--------------------------------------------------------------------------------------------------------------------------------------------------"
                        echo " "

echo
echo
echo "Test data logging is started..."
echo
echo

# Add date to for local pc, sd card and usb log
DATE=$(date)
LOG_VAR_DATE="$DATE $LOG_VAR"
LOGGING_STATUS=1

###############################################################################
# LOGGING TEST TO SD CARD
###############################################################################
echo "------------Logging test result to SD card------------------------------"
echo
# SAVE RESULTS INTO THE SD-CARD
# Remount the SD card with R&W rigths
mount -o remount,rw $SD_CARD_PATH

# Log information to the log file
echo "      Test data logging on the SD card was successfull"
echo $LOG_VAR_DATE >> "$SD_CARD_PATH/$LOG_FILENAME"

# Remount the SD card with Read only rigths
mount -o remount,ro $SD_CARD_PATH
echo
echo "------------------------------------------------------------------------"
echo
echo


###############################################################################
# LOGGING TEST TO  USB DRIVE
###############################################################################
# Check that the USB drive is plugged on the board
if [ $DRIVE_TEST_STATUS -eq 1 ]
then
    # Create the mounting point folder and mount the device
    mkdir $USB_MOUNT_FOLDER > /dev/null 2>&1
    sleep 0.2
    mount $USB_DEVICE $USB_MOUNT_FOLDER > /dev/null 2>&1
    sleep 0.2

    # Append the test result to the log file
    echo "-----------Logging test  results to USB drive------------------------"
    echo
    echo $LOG_VAR_DATE >> "$USB_MOUNT_FOLDER/$LOG_FILENAME"
    echo "      Test data logging on the USB drive was successfull"
    sleep 0.2
    # Umount the device and delete the folder
    umount $USB_MOUNT_FOLDER
    sleep 0.2
    rm -rf $USB_MOUNT_FOLDER
    sleep 0.2
else
    echo "      Not possible to log test statistics to USB drive"
fi
echo
echo
echo "-------------------------------------------------------------------------"
echo
echo

###############################################################################
# LOGGING TEST TO LOCAL PC
###############################################################################
# Create the mounting point folder and mount the device
RES=$(ping "$LOCAL_SERVER_IP" -c "$N_PING_PKG" | grep 'transmitted' | awk '{print $4}' ) > /dev/null

 if [[ "$RES" == "$N_PING_PKG" ]]
 then
    echo "----------Logging test statistics to PRODUCTION PC-------------------"
#ssh $LOCAL_USER@$LOCAL_SERVER_IP "cat >> $LOCAL_SERVER_DIR/$LOG_FILENAME"
    #echo 'sometext' | ssh zumy@192.168.178.123 "cat >> /home/zumy/Desktop/Test_LOGS/manuf_test.log"
    echo $LOG_VAR_DATE | sshpass -p "$LOCAL_SERVER_PASS"  ssh -oStrictHostKeyChecking=no -oUserKnownHostsFile=/dev/null $LOCAL_USER@$LOCAL_SERVER_IP "cat >> $LOCAL_SERVER_DIR/$LOG_FILENAME"
    echo
    echo "      Test data logging on the local PC was successfull"
    else
    echo "      Not possible to log test statistics to local PC"
    LOGGING_STATUS=1
fi
echo
echo
echo "-------------------------------------------------------------------------"
echo
echo


###############################################################################
# LOGGING TEST RESULTS TO SERVER
###############################################################################
echo
echo "------------Logging test result to the Red Pitaya d.d MAIN server---------"
echo
echo "      Checking if board is online and server is available.."
echo
CURL_RSP="$(curl -Is http://www.redpitaya.com | head -1)"
#echo $CURL_RSP
echo
if [ `echo $CURL_RSP | grep -c "HTTP/1.1" ` -gt 0 ]
then
echo "      Board is online & server is available."
else
echo "      Board is offline or server is not available. Logging test result to the Red Pitaya d.d MAIN server FAILD!!!"
LOGGING_STATUS=0
#exit
fi
echo
echo "      Sending test record data to server..."
# Prepare LOG_VAR data
LOG_VAR="${LOG_VAR// /_}"
#echo $LOG_VAR
echo
PASS="xBqRnS8r"
MD5IN="$LOG_VAR $PASS"
#echo $MD5IN
MD5OUT=`echo -n $MD5IN | md5sum | awk '{print $1}'`
#echo $MD5OUT
echo
URL="http://account.redpitaya.com/production.php?test_rec_data=$LOG_VAR"
URL+="&password=$MD5OUT"
#echo $URL
echo
#URL1="www.redpitaya.com"
#echo $URL1
#curl $URL1
CURL_RSP="$(curl $URL)"

status=$(curl -s -o /dev/null -w "%{http_code}" "$URL")

if [ "$status" == 200 ]
then
echo
echo "      Test record was successfully added to production database."
echo
else
echo
echo "      No response from SERVER!!!"
echo
#LOGGING_STATUS=1
fi

#if [ `echo $CURL_RSP | grep -c "OK" ` -gt 0 ]
#then
#echo
#echo "      Test record was successfully added to production database."
#echo
#elif [ `echo $CURL_RSP | grep -c "FAILED" ` -gt 0 ]
#then
#echo
#echo
#echo "      This board  (combination of MAC & DNA) already exists in production database with PASS(0x1ff = 111111111) status!!!"
#echo
#echo
#LOGGING_STATUS=1
#else
#echo
#echo "      No response from SERVER!!!"
#echo
#LOGGING_STATUS=1
#fi
#echo
echo
echo "---------------------------------------------------------------------------------------------------------------------------------------------"

if [ $LOGGING_STATUS -eq 1 ] && [ $LOGFILE_STATUS -eq 511 ] ## && [ $CALIBRATION_STATUS -eq 1 ] If calibration fails cal parameters of the board are set to factory  but calibration params of the board are sent to .
then                                                        ## production data base
    echo
    echo
    echo "                  ********************"
    echo "                  ********************"
    echo "                  *                  *"
    echo "                  *                  *"
    echo "                  *    BOARD PASS    *"
    echo "                  *                  *"
    echo "                  *                  *"
    echo "                  ********************"
    echo "                  ********************"
    echo
    echo
    echo "                  Test status is: T8-T7-T6-T5-T4-T3-T2-T1-T0 -> ${D2B[$LOGFILE_STATUS]} "
    echo "                  Calibraton status is: $CALIBRATION_STATUS "
    echo "                  Data logging status is: $LOGGING_STATUS "

else
    echo
    echo
    echo "                  *********************"
    echo "                  *********************"
    echo "                  *                   *"
    echo "                  *                   *"
    echo "                  *    BOARD FAILED   *"
    echo "                  *                   *"
    echo "                  *                   *"
    echo "                  *********************"
    echo "                  *********************"
    echo
    echo
    echo "                  Test status is: T8-T7-T6-T5-T4-T3-T2-T1-T0 -> ${D2B[$LOGFILE_STATUS]} "
    echo "                  Calibraton status is: $CALIBRATION_STATUS "
    echo "                  Data logging status is: $LOGGING_STATUS "

fi
