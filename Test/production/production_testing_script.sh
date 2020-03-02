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

hexToDec() {
    local VALUE
    read VALUE
    printf "%d\n" "$VALUE"
}

# Path variables
SD_CARD_PATH='/opt/redpitaya'
USB_DEVICE="/dev/sda1"
USB_MOUNT_FOLDER="/mnt/usb"

# TEST Log variables
LOG_FILENAME='manuf_test.log'

# Production PC/SERVER variables
#LOCAL_SERVER_IP='192.168.1.200'
LOCAL_SERVER_IP='192.168.178.121'
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
SOME_eeprom_value=-1430532899 #SOME_eeprom_value is some value in eeprom which is not used for anything but after Crt added hv offset calib values this value also appeard.
FE_CH1_DC_offs_HI=100
FE_CH2_DC_offs_HI=100          #FE_CHx_DC_offs_HI  are dc offset parameters for HV jumper settings
#All calibration parameters in one string
FACTORY_CAL="$FE_CH1_FS_G_HI $FE_CH2_FS_G_HI $FE_CH1_FS_G_LO $FE_CH2_FS_G_LO $FE_CH1_DC_offs $FE_CH2_DC_offs $BE_CH1_FS $BE_CH2_FS $BE_CH1_DC_offs $BE_CH2_DC_offs $SOME_eeprom_value $FE_CH1_DC_offs_HI $FE_CH2_DC_offs_HI"

# I2C test configuration
TEST_LABEL='I2C_test'
I2C_TEST_CONFIG='/sys/bus/i2c/devices/0-0051/eeprom 0x1800 0x0400'

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
SIG_FREQ=1000
SIG_AMPL=2
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
N_ADC_BITS=14
HALF_ADC_RANGE=8192

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

# Configure DIOx_P to inputs and DIOx_N to outputs to prevent Relay misbehaviour
# During DIO test this will be changed and after DIO test set back to this condition
$MONITOR 0x40000010 w 0x00 # -> Set P to inputs
sleep 0.2
$MONITOR 0x40000014 w 0xFF # -> Set N to outputs


# USE OLD FPGA for all up to TEST 7
cat /opt/redpitaya/fpga/fpga_0.94.bit > /dev/xdevcfg
sleep 2
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
fi

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

sleep $SLEEP_BETWEEN_TESTS

# TEST 1 - Ethernet test - If was OK Writte  "1"  in logfile status byte
if [ $TEST_STATUS -eq 1 ]
then
    # TEST_GLOBAL_STATUS=$(($TEST_GLOBAL_STATUS+$TEST_VALUE_LED))
    LOGFILE_STATUS=$(($LOGFILE_STATUS+$TEST_VALUE))
    # $MONITOR $LED_ADDR w "$(printf '0x%02x' $TEST_GLOBAL_STATUS)"
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
fi

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
if [[ $VCCAUX -lt $MIN_VCCAUX ]] || [[ $VCCAUX -gt $MAX_VCCAUX ]]
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
if [[ $VCCBRAM -lt $MIN_VCCBRAM ]] || [[ $VCCBRAM -gt $MAX_VCCBRAM ]]
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
if [[ $VCCINT -lt $MIN_VCCINT ]] || [[ $VCCINT -gt $MAX_VCCINT ]]
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
fi

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
fi


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

if  [[ "$READ_HWREV" == "STEM_125-10_v1.0" ]] || [[ "$READ_HWREV" == "STEM_10_B_v1.0" ]]
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
fi

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
fi

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
ADC1_A=$($MONITOR 0x40400000 | hexToDec)
sleep 0.2
ADC2_A=$($MONITOR 0x40400004 | hexToDec)
sleep 0.2
ADC3_A=$($MONITOR 0x40400008 | hexToDec)
sleep 0.2
ADC4_A=$($MONITOR 0x4040000C | hexToDec)
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
ADC1_B=$($MONITOR 0x40400000 | hexToDec)
sleep 0.2
ADC2_B=$($MONITOR 0x40400004 | hexToDec)
sleep 0.2
ADC3_B=$($MONITOR 0x40400008 | hexToDec)
sleep 0.2
ADC4_B=$($MONITOR 0x4040000C | hexToDec)
sleep 0.2

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
fi

echo
echo "          Loading fpga_0.94.bit all LEDs 0-7 will go OFF for 2 seconds...."
echo
# Use new FPGA image for ADC test and calibration
cat /opt/redpitaya/fpga/fpga_0.94.bit > /dev/xdevcfg
sleep 2
# Restore LED status after fpga_0.94.bit replacement
$MONITOR $LED_ADDR w "$(printf '0x%02x' $TEST_GLOBAL_STATUS)"

###############################################################################
# STEP 7: Fast ADCs and DACs test
###############################################################################

echo
echo "--- TEST 7: Fast ADCs and DACs test ---"
echo
echo "    Acquisition without DAC signal - ADCs with HIGH gain"
echo

TEST_STATUS=1
TEST_VALUE=128
TEST_VALUE_LED=64
PREVIOUS_TEST=1
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

# Configure relays  DIO5_N = 1, DIO6_N = 0, DIO7_N = 1 (lv jumper settings)
$MONITOR 0x4000001C w 0x00 # -> Set N outputs to zero - > reseet
sleep 0.2
$MONITOR 0x4000001C w 0xA0 # -> Set DIO5_N=1 -> Configure the ADC in high-gain mode -> DIO7_N = 1
sleep 1

# Assure tht DAC signals (ch 1 & 2) are OFF
$GENERATE 1 0 $SIG_FREQ
$GENERATE 2 0 $SIG_FREQ

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
ADC_A_STD=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)*(sum/NR)))}' /tmp/adc_a.txt)
ADC_B_STD=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)*(sum/NR)))}' /tmp/adc_b.txt)
ADC_A_PP=$(awk 'BEGIN{max=-1000;min=1000} { if (max < $1){ max = $1 }; if(min > $1){ min = $1 } } END{ print max-min}' /tmp/adc_a.txt)
ADC_B_PP=$(awk 'BEGIN{max=-1000;min=1000} { if (max < $1){ max = $1 }; if(min > $1){ min = $1 } } END{ print max-min}' /tmp/adc_b.txt)
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
                        echo "----------------------------Printing  Log variables  step 4 -> IN1 and IN2 parameters (LV jumper settings) -> OUT1 and OUT2 disabled--------------"
                        echo "$LOG_VAR"
                        echo "--------------------------------------------------------------------------------------------------------------------------------------------------"
                        echo " "

# Check if the values are within expectations
if [[ $ADC_A_MEAN -gt $MAX_ABS_OFFS_HIGH_GAIN ]] || [[ $ADC_A_MEAN -lt $((-$MAX_ABS_OFFS_HIGH_GAIN)) ]]
then
    echo "    Measured ch.A mean value ($ADC_A_MEAN) is outside expected range (+/- $MAX_ABS_OFFS_HIGH_GAIN)"
    TEST_STATUS=0
    PREVIOUS_TEST=0
fi

if [[ $ADC_B_MEAN -gt $MAX_ABS_OFFS_HIGH_GAIN ]] || [[ $ADC_B_MEAN -lt $((-$MAX_ABS_OFFS_HIGH_GAIN)) ]]
then
    echo "    Measured ch.B mean value ($ADC_B_MEAN) is outside expected range (+/- $MAX_ABS_OFFS_HIGH_GAIN)"
    TEST_STATUS=0
    PREVIOUS_TEST=0
fi

if [[ $ADC_A_STD -gt $MAX_NOISE_STD ]]
then
    echo "    Measured ch.A std deviation value ($ADC_A_STD) is outside expected range (0-$MAX_NOISE_STD)"
    TEST_STATUS=0
    PREVIOUS_TEST=0
fi

if [[ $ADC_B_STD -gt $MAX_NOISE_STD ]]
then
    echo "    Measured ch.B std deviation value ($ADC_B_STD) is outside expected range (0-$MAX_NOISE_STD)"
    TEST_STATUS=0
    PREVIOUS_TEST=0
fi

if [[ $ADC_A_STD_NO_DEC -gt $MAX_NOISE_STD_NO_DEC ]]
then
    echo "    Measured ch.A std deviation value with no decimation ($ADC_A_STD_NO_DEC) is outside expected range (0-$MAX_NOISE_STD_NO_DEC)"
    TEST_STATUS=0
    PREVIOUS_TEST=0
fi

if [[ $ADC_B_STD_NO_DEC -gt $MAX_NOISE_STD_NO_DEC ]]
then
    echo "    Measured ch.B std deviation value with no decimation ($ADC_B_STD_NO_DEC) is outside expected range (0-$MAX_NOISE_STD_NO_DEC)"
    TEST_STATUS=0
    PREVIOUS_TEST=0
fi

if [[ $ADC_A_PP -gt $MAX_NOISE_P2P ]]
then
    echo "    Measured ch.A p2p value ($ADC_A_PP) is outside expected range (0-$MAX_NOISE_P2P)"
    TEST_STATUS=0
    PREVIOUS_TEST=0
fi

if [[ $ADC_B_PP -gt $MAX_NOISE_P2P ]]
then
    echo "    Measured ch.B p2p value ($ADC_B_PP) is outside expected range (0-$MAX_NOISE_P2P)"
    TEST_STATUS=0
    PREVIOUS_TEST=0
fi

# Set DAC value to proper counts / frequency for both channels
echo
echo "    Acquisition with DAC signal ($SIG_AMPL Vpp / $SIG_FREQ Hz) - ADCs with HIGH gain"
echo

# Turn the DAC signal generator on on both channels
$GENERATE 1 $SIG_AMPL $SIG_FREQ
$GENERATE 2 $SIG_AMPL $SIG_FREQ

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
ADC_A_STD=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)*(sum/NR)))}' /tmp/adc_a.txt)
ADC_B_STD=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)*(sum/NR)))}' /tmp/adc_b.txt)
ADC_A_PP=$(awk 'BEGIN{max=-1000;min=1000} { if (max < $1){ max = $1 }; if(min > $1){ min = $1 } } END{ print max-min}' /tmp/adc_a.txt)
ADC_B_PP=$(awk 'BEGIN{max=-1000;min=1000} { if (max < $1){ max = $1 }; if(min > $1){ min = $1 } } END{ print max-min}' /tmp/adc_b.txt)
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
                        echo "----------------------------Printing  Log variables  step 5 -> IN1 and IN2 parameters (LV jumper settings) -> OUT1 and OUT2 enabled---------------"
                        echo "$LOG_VAR"
                        echo "--------------------------------------------------------------------------------------------------------------------------------------------------"
                        echo " "

# Check if the values are within expectations
if [[ $ADC_A_MEAN -gt $MAX_ABS_OFFS_HIGH_GAIN ]] || [[ $ADC_A_MEAN -lt $((-$MAX_ABS_OFFS_HIGH_GAIN)) ]]
then
    echo "    Measured ch.A mean value ($ADC_A_MEAN) is outside expected range (+/- $MAX_ABS_OFFS_HIGH_GAIN)"
    TEST_STATUS=0
    PREVIOUS_TEST=0
fi

if [[ $ADC_B_MEAN -gt $MAX_ABS_OFFS_HIGH_GAIN ]] || [[ $ADC_B_MEAN -lt $((-$MAX_ABS_OFFS_HIGH_GAIN)) ]]
then
    echo "    Measured ch.B mean value ($ADC_B_MEAN) is outside expected range (+/- $MAX_ABS_OFFS_HIGH_GAIN)"
    TEST_STATUS=0
    PREVIOUS_TEST=0
fi

if [[ $ADC_A_STD -lt $MIN_SIG_STD_HIGH_GAIN ]] || [[ $ADC_A_STD -gt $MAX_SIG_STD_HIGH_GAIN ]]
then
    echo "    Measured ch.A std deviation value ($ADC_A_STD) is outside expected range ($MIN_SIG_STD_HIGH_GAIN-$MAX_SIG_STD_HIGH_GAIN)"
    TEST_STATUS=0
    PREVIOUS_TEST=0
fi

if [[ $ADC_B_STD -lt $MIN_SIG_STD_HIGH_GAIN ]] || [[ $ADC_B_STD -gt $MAX_SIG_STD_HIGH_GAIN ]]
then
    echo "    Measured ch.B std deviation value ($ADC_B_STD) is outside expected range ($MIN_SIG_STD_HIGH_GAIN-$MAX_SIG_STD_HIGH_GAIN)"
    TEST_STATUS=0
    PREVIOUS_TEST=0
fi

if [[ $ADC_A_PP -lt $MIN_SIG_P2P_HIGH_GAIN ]] || [[ $ADC_A_PP -gt $MAX_SIG_P2P_HIGH_GAIN ]]
then
    echo "    Measured ch.A p2p value ($ADC_A_PP) is outside expected range ($MIN_SIG_P2P_HIGH_GAIN-$MAX_SIG_P2P_HIGH_GAIN)"
    TEST_STATUS=0
    PREVIOUS_TEST=0
fi

if [[ $ADC_B_PP -lt $MIN_SIG_P2P_HIGH_GAIN ]] || [[ $ADC_B_PP -gt $MAX_SIG_P2P_HIGH_GAIN ]]
then
    echo "    Measured ch.B p2p value ($ADC_B_PP) is outside expected range ($MIN_SIG_P2P_HIGH_GAIN-$MAX_SIG_P2P_HIGH_GAIN)"
    TEST_STATUS=0
    PREVIOUS_TEST=0
fi


echo
echo "    Acquisition without DAC signal - ADCs with LOW gain(HV jumper settings)"
echo

# Turn the DAC signal generator OFF on both channels (ch 1 & 2)
$GENERATE 1 0 $SIG_FREQ
$GENERATE 2 0 $SIG_FREQ
sleep 0.2

# Configure relays  DIO5_N = 1, DIO6_N = 0, DIO7_N = 0 (hv jumper settings)
$MONITOR 0x4000001C w 0x00 # -> Set N outputs to zero - > reseet
sleep 0.2
$MONITOR 0x4000001C w 0x20 # -> Set DIO5_N=1 -> Configure the ADC in low-gain mode -> DIO7_N = 0
sleep 0.2

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
ADC_A_STD=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)*(sum/NR)))}' /tmp/adc_a.txt)
ADC_B_STD=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)*(sum/NR)))}' /tmp/adc_b.txt)
ADC_A_PP=$(awk 'BEGIN{max=-1000;min=1000} { if (max < $1){ max = $1 }; if(min > $1){ min = $1 } } END{ print max-min}' /tmp/adc_a.txt)
ADC_B_PP=$(awk 'BEGIN{max=-1000;min=1000} { if (max < $1){ max = $1 }; if(min > $1){ min = $1 } } END{ print max-min}' /tmp/adc_b.txt)
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
                        echo "----------------------------Printing  Log variables  step 6 -> IN1 and IN2 parameters (HV jumper settings) -> OUT1 and OUT2 disabled--------------"
                        echo "$LOG_VAR"
                        echo "--------------------------------------------------------------------------------------------------------------------------------------------------"
                        echo " "


# Check if the values are within expectations
if [[ $ADC_A_MEAN -gt $MAX_ABS_OFFS_LOW_GAIN ]] || [[ $ADC_A_MEAN -lt $((-$MAX_ABS_OFFS_LOW_GAIN)) ]]
then
    echo "    Measured ch.A mean value ($ADC_A_MEAN) is outside expected range (+/- $MAX_ABS_OFFS_LOW_GAIN)"
    TEST_STATUS=0
    PREVIOUS_TEST=0
fi

if [[ $ADC_B_MEAN -gt $MAX_ABS_OFFS_LOW_GAIN ]] || [[ $ADC_B_MEAN -lt $((-$MAX_ABS_OFFS_LOW_GAIN)) ]]
then
    echo "    Measured ch.B mean value ($ADC_B_MEAN) is outside expected range (+/- $MAX_ABS_OFFS_LOW_GAIN)"
    TEST_STATUS=0
    PREVIOUS_TEST=0
fi

if [[ $ADC_A_STD -gt $MAX_NOISE_STD ]]
then
    echo "    Measured ch.A std deviation value ($ADC_A_STD) is outside expected range (0-$MAX_NOISE_STD)"
    TEST_STATUS=0
    PREVIOUS_TEST=0
fi

if [[ $ADC_B_STD -gt $MAX_NOISE_STD ]]
then
    echo "    Measured ch.B std deviation value ($ADC_B_STD) is outside expected range (0-$MAX_NOISE_STD)"
    TEST_STATUS=0
    PREVIOUS_TEST=0
fi

if [[ $ADC_A_STD_NO_DEC -gt $MAX_NOISE_STD_NO_DEC ]]
then
    echo "    Measured ch.A std deviation value with no decimation ($ADC_A_STD_NO_DEC) is outside expected range (0-$MAX_NOISE_STD_NO_DEC)"
    TEST_STATUS=0
    PREVIOUS_TEST=0
fi

if [[ $ADC_B_STD_NO_DEC -gt $MAX_NOISE_STD_NO_DEC ]]
then
    echo "    Measured ch.B std deviation value with no decimation ($ADC_B_STD_NO_DEC) is outside expected range (0-$MAX_NOISE_STD_NO_DEC)"
    TEST_STATUS=0
    PREVIOUS_TEST=0
fi

if [[ $ADC_A_PP -gt $MAX_NOISE_P2P ]]
then
    echo "    Measured ch.A p2p value ($ADC_A_PP) is outside expected range (0-$MAX_NOISE_P2P)"
    TEST_STATUS=0
    PREVIOUS_TEST=0
fi

if [[ $ADC_B_PP -gt $MAX_NOISE_P2P ]]
then
    echo "    Measured ch.B p2p value ($ADC_B_PP) is outside expected range (0-$MAX_NOISE_P2P)"
    TEST_STATUS=0
    PREVIOUS_TEST=0
fi


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
ADC_A_STD=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)*(sum/NR)))}' /tmp/adc_a.txt)
ADC_B_STD=$(awk '{sum+=$1; sumsq+=$1*$1} END {print int(sqrt(sumsq/NR - (sum/NR)*(sum/NR)))}' /tmp/adc_b.txt)
ADC_A_PP=$(awk 'BEGIN{max=-1000;min=1000} { if (max < $1){ max = $1 }; if(min > $1){ min = $1 } } END{ print max-min}' /tmp/adc_a.txt)
ADC_B_PP=$(awk 'BEGIN{max=-1000;min=1000} { if (max < $1){ max = $1 }; if(min > $1){ min = $1 } } END{ print max-min}' /tmp/adc_b.txt)
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
                        echo "----------------------------Printing  Log variables  step 7 -> IN1 and IN2 parameters (HV jumper settings) -> OUT1 and OUT2 enabled---------------"
                        echo "$LOG_VAR"
                        echo "--------------------------------------------------------------------------------------------------------------------------------------------------"
                        echo " "

# Check if the values are within expectations
if [[ $ADC_A_MEAN -gt $MAX_ABS_OFFS_LOW_GAIN ]] || [[ $ADC_A_MEAN -lt $((-$MAX_ABS_OFFS_LOW_GAIN)) ]]
then
    echo "    Measured ch.A mean value ($ADC_A_MEAN) is outside expected range (+/- $MAX_ABS_OFFS_LOW_GAIN)"
    TEST_STATUS=0
    PREVIOUS_TEST=0
fi

if [[ $ADC_B_MEAN -gt $MAX_ABS_OFFS_LOW_GAIN ]] || [[ $ADC_B_MEAN -lt $((-$MAX_ABS_OFFS_LOW_GAIN)) ]]
then
    echo "    Measured ch.B mean value ($ADC_B_MEAN) is outside expected range (+/- $MAX_ABS_OFFS_LOW_GAIN)"
    TEST_STATUS=0
    PREVIOUS_TEST=0
fi

if [[ $ADC_A_STD -lt $MIN_SIG_STD_LOW_GAIN ]] || [[ $ADC_A_STD -gt $MAX_SIG_STD_LOW_GAIN ]]
then
    echo "    Measured ch.A std deviation value ($ADC_A_STD) is outside expected range ($MIN_SIG_STD_LOW_GAIN-$MAX_SIG_STD_LOW_GAIN)"
    TEST_STATUS=0
    PREVIOUS_TEST=0
fi

if [[ $ADC_B_STD -lt $MIN_SIG_STD_LOW_GAIN ]] || [[ $ADC_B_STD -gt $MAX_SIG_STD_LOW_GAIN ]]
then
    echo "    Measured ch.B std deviation value ($ADC_B_STD) is outside expected range ($MIN_SIG_STD_LOW_GAIN-$MAX_SIG_STD_LOW_GAIN)"
    TEST_STATUS=0
    PREVIOUS_TEST=0
fi

if [[ $ADC_A_PP -lt $MIN_SIG_P2P_LOW_GAIN ]] || [[ $ADC_A_PP -gt $MAX_SIG_P2P_LOW_GAIN ]]
then
    echo "    Measured ch.A p2p value ($ADC_A_PP) is outside expected range ($MIN_SIG_P2P_LOW_GAIN-$MAX_SIG_P2P_LOW_GAIN)"
    TEST_STATUS=0
    PREVIOUS_TEST=0
fi

if [[ $ADC_B_PP -lt $MIN_SIG_P2P_LOW_GAIN ]] || [[ $ADC_B_PP -gt $MAX_SIG_P2P_LOW_GAIN ]]
then
    echo "    Measured ch.B p2p value ($ADC_B_PP) is outside expected range ($MIN_SIG_P2P_LOW_GAIN-$MAX_SIG_P2P_LOW_GAIN)"
    TEST_STATUS=0
    PREVIOUS_TEST=0
fi


# Set DAC value to 0 for both channels (1 & 2)
echo
echo "    Restoring DAC signals and ADC gain to idle conditions"
$GENERATE 1 0 $SIG_FREQ
$GENERATE 2 0 $SIG_FREQ
echo

sleep $SLEEP_BETWEEN_TESTS

# TEST 7 - Fast ADC/DAC test, if was OK Writte  "1"  in logfile status byte
if [ $TEST_STATUS -eq 1 ]
then
    #TEST_GLOBAL_STATUS=$(($TEST_GLOBAL_STATUS+$TEST_VALUE_LED))
    LOGFILE_STATUS=$(($LOGFILE_STATUS+$TEST_VALUE))
    #$MONITOR $LED_ADDR w "$(printf '0x%02x' $TEST_GLOBAL_STATUS)"
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
fi

# Set Jumper setting back to LV and set OUT1-IN1 , and OUT2-IN2
# Configure relays  DIO5_N = 1, DIO6_N = 0, DIO7_N = 1 (lv jumper settings)
$MONITOR 0x4000001C w 0x00 # -> Reset N
sleep 0.2
$MONITOR 0x4000001C w 0xA0 # -> Set DIO5_N=1 -> Configure the ADC in high-gain mode -> DIO7_N = 1
sleep 0.2

###############################################################################
# STEP 8: Fast ADCs bit analysis
###############################################################################
echo
echo "--- TEST 8: Fast ADCs bit analysis test ---"
echo

echo "    Acquisition with DAC signal ($SIG_AMPL Vpp / $SIG_FREQ Hz) - ADCs with HIGH gain"
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
    if [ $PREVIOUS_TEST -eq 1 ]
    then
    TEST_GLOBAL_STATUS=$(($TEST_GLOBAL_STATUS+$TEST_VALUE_LED))
    fi
    LOGFILE_STATUS=$(($LOGFILE_STATUS+$TEST_VALUE))
    $MONITOR $LED_ADDR w "$(printf '0x%02x' $TEST_GLOBAL_STATUS)"
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

fi
echo "    Restoring DAC signals and ADC gain to idle conditions..."
$GENERATE 1 0 $SIG_FREQ
$GENERATE 2 0 $SIG_FREQ
echo
sleep $SLEEP_BETWEEN_TESTS

###################################################################################################################################################################################
# STEP 9: Fast ADCs and DACs CALIBRATION
###############################################################################
if [ $LOGFILE_STATUS -eq 511 ]
then
echo
echo "--- TEST 9: Fast ADCs and DACs CALIBRATION ---"
echo
echo "Reseting cal parameters to unit gains and zerro DC offset..."

TEST_STATUS=1                # It is not used in calibration but i have left it in IF checking so it can be used later if needed.
CALIBRATION_STATUS=1
TEST_VALUE_LED=128
#(LED1->TEST_VALUE_LED=2     is used for enviroment parameters test)
#(LED2->TEST_VALUE_LED=4     is used for ethernet, zyng temperature and voltages test)
#(LED3->TEST_VALUE_LED=8     is used for USB drive test)
#(LED4->TEST_VALUE_LED=16    is used for GPIO and SATA tests)
#(LED5->TEST_VALUE_LED=32    is used for slow ADC and DAC tests)
#(LED6->TEST_VALUE_LED=64    is used for fast ADC and DAC + bit analysis tests)
#(LED7->TEST_VALUE_LED=128   is used for calibration tests)

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
echo "Setting the unit gains and zerro DC offset calibration parameters into the user EEPROM space..."
echo
        echo $NEW_CAL_PARAMS | $CALIB -w
        sleep 0.1
        if [ $? -ne 0 ]
        then
        echo
        echo "Unit gains and zerro DC offset calibration parameters are NOT correctly written in the user EEPROM space"
        sleep 1
        fi
echo -ne '\n' | $CALIB -r
sleep 0.4

# MAX/MIN calibration parameters
FE_FS_G_HI_MAX=55834573                      # 42949672 + 30 %
FE_FS_G_LO_MAX=1116691496                    # 858993459 + 30 %
FE_DC_offs_MAX=300
BE_FS_MAX=55834573
BE_DC_offs_MAX=300
FE_DC_offs_HI_MAX=400

FE_FS_G_HI_MIN=33038209                      # 42949672 - 30 %
FE_FS_G_LO_MIN=660764199                     # 858993459 - 30 %
FE_DC_offs_MIN=-300
BE_FS_MIN=33038209
BE_DC_offs_MIN=-300
FE_DC_offs_HI_MIN=-400

DECIMATION=1024

# Assure tht DAC signals (ch 1 & 2) are OFF
$GENERATE 1 0 0 sqr
$GENERATE 2 0 0 sqr

# Set Directions of DIO
$MONITOR 0x40000010 w 0x00   # - P line inputs
sleep 0.4
$MONITOR 0x40000014 w 0xff   # - N outputs
sleep 0.4

echo "LV jumper settings DC offset calibration is started..."
echo "Setting relay states...-> Set LV jumper settings... ->Connect IN1&IN2 to the GND..."

# Set LV jumper settings Connect IN1&IN2 to the GND, LV jumper settings
# DIO5_N = 0
# DIO6_N = 0
# DIO7_N = 1
$MONITOR 0x4000001C w 0x80 # -> Set N
sleep 0.4

#Acquire data with $DECIMATION decimation factor
$ACQUIRE -e -s $ADC_BUFF_SIZE $DECIMATION > /tmp/adc.txt   # WORKAROUND: First acquisition is thrown away
sleep 0.4
$ACQUIRE -e -s $ADC_BUFF_SIZE $DECIMATION > /tmp/adc.txt
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
if [[ $ADC_A_MEAN -gt $FE_DC_offs_MAX ]] || [[ $ADC_A_MEAN -lt $FE_DC_offs_MIN ]]
then
    echo "      Measured IN1 LV DC offset value ($ADC_A_MEAN) is outside expected range (+/- $FE_DC_offs_MAX)"
    TEST_STATUS=0
    CALIBRATION_STATUS=0
fi

if [[ $ADC_B_MEAN -gt $FE_DC_offs_MAX ]] || [[ $ADC_B_MEAN -lt $FE_DC_offs_MIN ]]
then
    echo "      Measured IN2 LV DC offset value ($ADC_B_MEAN) is outside expected range (+/- $FE_DC_offs_MAX)"
    TEST_STATUS=0
    CALIBRATION_STATUS=0
fi

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
                        echo "------------------Printing  Log variables  step 7: Calibration parameters FE_CH1_DC_offs, FE_CH2_DC_offs -----------------------------------------"
                        echo "$LOG_VAR"
                        echo "--------------------------------------------------------------------------------------------------------------------------------------------------"
                        echo " "

echo "Inputs  LV GAIN calibration is started..."
echo "Setting relay states...-> Set LV jumper settings... ->Connect IN1&IN2 to the REF_VALUE_LV..."


# Set Directions of DIO
$MONITOR 0x40000010 w 0x00   # - P line inputs
sleep 0.4
$MONITOR 0x40000014 w 0xff   # - N outputs
sleep 0.4
# Set LV jumper settings Connect IN1&IN2 to the REF VALUE -> Set relay states  DIO6_N, DIO7_N and DIO7_P
# DIO5_N = 1
# DIO6_N = 1
# DIO7_N = 1
$MONITOR 0x4000001C w 0xE0 # -> Set N
sleep 0.4

echo "Connecting reference voltage 0.9V..."

sleep 0.4
#Acquire data with $DECIMATION decimation factor
$ACQUIRE -e -s $ADC_BUFF_SIZE $DECIMATION > /tmp/adc.txt   # WORKAROUND: First acquisition is thrown away
sleep 0.4
$ACQUIRE -e -s $ADC_BUFF_SIZE $DECIMATION > /tmp/adc.txt
cat /tmp/adc.txt | awk '{print $1}' > /tmp/adc_a.txt
cat /tmp/adc.txt | awk '{print $2}' > /tmp/adc_b.txt

# Calculate mean value
ADC_A_MEAN=$(awk '{sum+=$1} END { print int(sum/NR)}' /tmp/adc_a.txt)
ADC_B_MEAN=$(awk '{sum+=$1} END { print int(sum/NR)}' /tmp/adc_b.txt)

# Print out the measurements
echo "IN1 mean value is $ADC_A_MEAN"
echo "IN2 mean value is $ADC_B_MEAN"
echo

#Gain calibration y=xk+n
REF_VALUE_LV=7373     #0.9 VOLTS reference voltage in ADC counts

GAIN1_LV=$(awk -v N1_LV=$N1_LV -v REF_VALUE_LV=$REF_VALUE_LV -v ADC_A_MEAN=$ADC_A_MEAN 'BEGIN { print ( ( REF_VALUE_LV) / ( ADC_A_MEAN-N1_LV ) ) }')
GAIN2_LV=$(awk -v N2_LV=$N2_LV -v REF_VALUE_LV=$REF_VALUE_LV -v ADC_B_MEAN=$ADC_B_MEAN 'BEGIN { print ( ( REF_VALUE_LV) / ( ADC_B_MEAN-N2_LV ) ) }')

# Print out the measurements
echo "IN1_LV_Gain is $GAIN1_LV"
echo "IN2_LV_Gain is $GAIN2_LV"
echo

FE_CH1_FS_G_LO=$(awk -v GAIN1_LV=$GAIN1_LV 'BEGIN { print sprintf("%d", int((858993459*GAIN1_LV))) }')
FE_CH2_FS_G_LO=$(awk -v GAIN2_LV=$GAIN2_LV 'BEGIN { print sprintf("%d", int((858993459*GAIN2_LV))) }')

# Print out the measurements
echo "      NEW IN1 LV gain cal param >>FE_CH1_FS_G_LO<< is $FE_CH1_FS_G_LO"
echo "      NEW IN2 LV gain cal param >>FE_CH2_FS_G_LO<< is $FE_CH2_FS_G_LO"
echo

# Check if the values are within expectations
if [[ $FE_CH1_FS_G_LO -gt $FE_FS_G_LO_MAX ]] || [[ $FE_CH1_FS_G_LO -lt $FE_FS_G_LO_MIN ]]
then
    echo "      Measured IN1 LV gain ($FE_CH1_FS_G_LO) is outside expected range ( 858993459 +/- 30 %)"
    TEST_STATUS=0
    CALIBRATION_STATUS=0
fi

if [[ $FE_CH2_FS_G_LO -gt $FE_FS_G_LO_MAX ]] || [[ $FE_CH2_FS_G_LO -lt $FE_FS_G_LO_MIN ]]
then
    echo "      Measured IN2 LV gain ($FE_CH2_FS_G_LO) is outside expected range ( 858993459 +/- 30 %)"
    TEST_STATUS=0
    CALIBRATION_STATUS=0
fi

LOG_VAR="$LOG_VAR $FE_CH1_FS_G_LO $FE_CH2_FS_G_LO"

                        echo
                        echo "------------------Printing  Log variables  step 8: Calibration parameters FE_CH1_FS_G_LO, FE_CH2_FS_G_LO -----------------------------------------"
                        echo "$LOG_VAR"
                        echo "--------------------------------------------------------------------------------------------------------------------------------------------------"
                        echo

# COPY NEW CALIBRATION PARAMETERS IN TO USER  EPROM SPACE/PARTITION
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
# Acquire data with $DECIMATION decimation factor
$ACQUIRE -e -s $ADC_BUFF_SIZE $DECIMATION > /tmp/adc.txt   # WORKAROUND: First acquisition is thrown away
sleep 0.4
$ACQUIRE -e -s $ADC_BUFF_SIZE $DECIMATION > /tmp/adc.txt
cat /tmp/adc.txt | awk '{print $1}' > /tmp/adc_a.txt
cat /tmp/adc.txt | awk '{print $2}' > /tmp/adc_b.txt

# Calculate mean value
ADC_A_MEAN=$(awk -v N1_LV=$N1_LV -v GAIN1_LV=$GAIN1_LV '{sum+=$1} END { print int( ((sum/NR)*GAIN1_LV)-N1_LV)}' /tmp/adc_a.txt)
ADC_B_MEAN=$(awk -v N2_LV=$N2_LV -v GAIN2_LV=$GAIN2_LV '{sum+=$1} END { print int( ((sum/NR)*GAIN2_LV)-N2_LV)}' /tmp/adc_b.txt)

IN1_ERROR_LV=$(awk -v ADC_A_MEAN=$ADC_A_MEAN -v REF_VALUE_LV=$REF_VALUE_LV 'BEGIN { print (((ADC_A_MEAN-REF_VALUE_LV)/REF_VALUE_LV)*100) }')
IN2_ERROR_LV=$(awk -v ADC_B_MEAN=$ADC_B_MEAN -v REF_VALUE_LV=$REF_VALUE_LV 'BEGIN { print (((ADC_B_MEAN-REF_VALUE_LV)/REF_VALUE_LV)*100) }')

# Print out the measurements
echo "      IN1 LV Error after the calibration is $IN1_ERROR_LV %"
echo "      IN2 LV Error after the calibration is $IN2_ERROR_LV %"
echo

# HV jumper settings
echo "HV jumper settings DC offset calibration is started..."
echo "Setting relay states...-> Set HV jumper settings... ->Connect IN1&IN2 to the GND..."

# Set Directions of DIO
$MONITOR 0x40000010 w 0x00   # - P line inputs
sleep 0.4
$MONITOR 0x40000014 w 0xff   # - N outputs
sleep 0.4
# Setting relay states, Set HV jumper setting,s onnect IN1&IN2 to the GND.
# DIO5_N = 0
# DIO6_N = 0
# DIO7_N = 0
$MONITOR 0x4000001C w 0x00 # -> Set N
sleep 0.4

#Acquire data with $DECIMATION decimation factor
$ACQUIRE -e -s -1 hv -2 hv $ADC_BUFF_SIZE $DECIMATION > /tmp/adc.txt   # WORKAROUND: First acquisition is thrown away
sleep 0.4
$ACQUIRE -e -s -1 hv -2 hv $ADC_BUFF_SIZE $DECIMATION > /tmp/adc.txt
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
if [[ $ADC_A_MEAN -gt $FE_DC_offs_HI_MAX ]] || [[ $ADC_A_MEAN -lt $FE_DC_offs_HI_MIN ]]
then
    echo "      Measured IN1 HV DC offset value ($ADC_A_MEAN) is outside expected range (+/- $FE_DC_offs_HI_MAX)"
    TEST_STATUS=0
    CALIBRATION_STATUS=0
fi

if [[ $ADC_B_MEAN -gt $FE_DC_offs_HI_MAX ]] || [[ $ADC_B_MEAN -lt $FE_DC_offs_HI_MIN ]]
then
    echo "      Measured IN2 HV DC offset value ($ADC_B_MEAN) is outside expected range (+/- $FE_DC_offs_HI_MAX)"
    TEST_STATUS=0
    CALIBRATION_STATUS=0
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
                        echo "------------------Printing  Log variables  step 9: Calibration parameters FE_CH1_DC_offs_HI, FE_CH2_DC_offs_HI -----------------------------------------"
                        echo "$LOG_VAR"
                        echo "--------------------------------------------------------------------------------------------------------------------------------------------------"
                        echo

echo "Inputs  HV GAIN calibration is started..."
echo "Setting relay states...-> Set HV jumper settings... ->Connect IN1&IN2 to the REF_VALUE_HV..."
echo "Connecting reference voltage 10.9V..."

sleep 0.4

# Set Directions of DIO
$MONITOR 0x40000010 w 0x00   # - P line inputs
sleep 0.4
$MONITOR 0x40000014 w 0xff   # - N outputs
sleep 0.4
# Setting relay states, Set HV jumper setting, connect IN1&IN2 to the REF_VALUE_HV.
# DIO5_N = 1
# DIO6_N = 1
# DIO7_N = 0
$MONITOR 0x4000001C w 0x60 # -> Set N
sleep 0.4

#Acquire data with $DECIMATION decimation factor
$ACQUIRE -e -s -1 hv -2 hv $ADC_BUFF_SIZE $DECIMATION > /tmp/adc.txt   # WORKAROUND: First acquisition is thrown away
sleep 0.4
$ACQUIRE -e -s -1 hv -2 hv $ADC_BUFF_SIZE $DECIMATION > /tmp/adc.txt
cat /tmp/adc.txt | awk '{print $1}' > /tmp/adc_a.txt
cat /tmp/adc.txt | awk '{print $2}' > /tmp/adc_b.txt

# Calculate mean value
ADC_A_MEAN=$(awk '{sum+=$1} END { print int(sum/NR)}' /tmp/adc_a.txt)
ADC_B_MEAN=$(awk '{sum+=$1} END { print int(sum/NR)}' /tmp/adc_b.txt)

# Print out the measurements
echo "      IN1 mean value is $ADC_A_MEAN"
echo "      IN2 mean value is $ADC_B_MEAN"
echo

#Gain calibration y=xk+n
REF_VALUE_HV=4465 # 10.9 VOLTS reference voltage IN COUNTS
GAIN1_HV=$(awk -v N1_HV=$N1_HV -v REF_VALUE_HV=$REF_VALUE_HV -v ADC_A_MEAN=$ADC_A_MEAN 'BEGIN { print ( ( REF_VALUE_HV) / ( ADC_A_MEAN-N1_HV ) ) }')
GAIN2_HV=$(awk -v N2_HV=$N2_HV -v REF_VALUE_HV=$REF_VALUE_HV -v ADC_B_MEAN=$ADC_B_MEAN 'BEGIN { print ( ( REF_VALUE_HV) / ( ADC_B_MEAN-N2_HV ) ) }')

# Print out the measurements
echo "IN1_HV_Gain is $GAIN1_HV"
echo "IN2_HV_Gain is $GAIN2_HV"
echo

FE_CH1_FS_G_HI=$(awk -v GAIN1_HV=$GAIN1_HV 'BEGIN { print sprintf("%d", int((42949673*GAIN1_HV))) }')
FE_CH2_FS_G_HI=$(awk -v GAIN2_HV=$GAIN2_HV 'BEGIN { print sprintf("%d", int((42949673*GAIN2_HV))) }')

# Print out the measurements
echo "      NEW IN1 HV gain cal param >>FE_CH1_FS_G_HI<< is $FE_CH1_FS_G_HI"
echo "      NEW IN2 HV gain cal param >>FE_CH2_FS_G_HI<< is $FE_CH2_FS_G_HI"
echo

# Check if the values are within expectations
if [[ $FE_CH1_FS_G_HI -gt $FE_FS_G_HI_MAX ]] || [[ $FE_CH1_FS_G_HI -lt $FE_FS_G_HI_MIN ]]
then
    echo "      Measured IN1 HV gain ($FE_CH1_FS_G_HI) is outside expected range ( 42949673 +/- 30 %)"
    TEST_STATUS=0
    CALIBRATION_STATUS=0
fi

if [[ $FE_CH2_FS_G_HI -gt $FE_FS_G_HI_MAX ]] || [[ $FE_CH2_FS_G_HI -lt $FE_FS_G_HI_MIN ]]
then
    echo "      Measured IN2 HV gain ($FE_CH2_FS_G_HI) is outside expected range ( 42949673 +/- 30 %)"
    TEST_STATUS=0
    CALIBRATION_STATUS=0
fi

LOG_VAR="$LOG_VAR $FE_CH1_FS_G_HI $FE_CH2_FS_G_HI"

                        echo
                        echo "------------------Printing  Log variables  step 9: Calibration parameters FE_CH1_FS_G_HI,  FE_CH2_FS_G_HI -----------------------------------------"
                        echo "$LOG_VAR"
                        echo "--------------------------------------------------------------------------------------------------------------------------------------------------"
                        echo


# COPY NEW CALIBRATION PARAMETERS IN TO USER EPROM SPACE/PARTITION  #
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
# Acquire data with $DECIMATION decimation factor
$ACQUIRE -e -s -1 hv -2 hv $ADC_BUFF_SIZE $DECIMATION > /tmp/adc.txt   # WORKAROUND: First acquisition is thrown away
sleep 0.4
$ACQUIRE -e -s -1 hv -2 hv $ADC_BUFF_SIZE $DECIMATION > /tmp/adc.txt
cat /tmp/adc.txt | awk '{print $1}' > /tmp/adc_a.txt
cat /tmp/adc.txt | awk '{print $2}' > /tmp/adc_b.txt

# Calculate mean value
ADC_A_MEAN=$(awk -v N1_HV=$N1_HV -v GAIN1_HV=$GAIN1_HV '{sum+=$1} END { print int( ((sum/NR)*GAIN1_HV)-N1_HV)}' /tmp/adc_a.txt)
ADC_B_MEAN=$(awk -v N2_HV=$N2_HV -v GAIN2_HV=$GAIN2_HV '{sum+=$1} END { print int( ((sum/NR)*GAIN2_HV)-N2_HV)}' /tmp/adc_b.txt)

IN1_ERROR_HV=$(awk -v ADC_A_MEAN=$ADC_A_MEAN -v REF_VALUE_HV=$REF_VALUE_HV 'BEGIN { print (((ADC_A_MEAN-REF_VALUE_HV)/REF_VALUE_HV)*100) }')
IN2_ERROR_HV=$(awk -v ADC_B_MEAN=$ADC_B_MEAN -v REF_VALUE_HV=$REF_VALUE_HV 'BEGIN { print (((ADC_B_MEAN-REF_VALUE_HV)/REF_VALUE_HV)*100) }')

# Print out the measurements
echo
echo "      IN1 HV Error after the calibration is $IN1_ERROR_HV %"
echo "      IN2 HV Error after the calibration is $IN2_ERROR_HV %"
echo " "


# OUTPUTS CALIBRATION
echo
echo "Outputs DC offset calibration is started..."
echo "Setting relay states...-> Set LV jumper settings... ->Connect OUT1&OUT2 to the IN1&IN2"

# Variables
OUT_AMP_LO_cnt=3686 # 0.45V  VOLTS VPP reference voltage
OUT_AMP_HI_cnt=7373 # 0.9V  VOLTS VPP reference voltage
# Old generate.c has hardcoded DC osffset -155 /Test/generate/generate.c line 206 >>dcoffs<<
BE_CH1_DC_offs=-150
BE_CH2_DC_offs=-150


# Set Directions of DIO
$MONITOR 0x40000010 w 0x00   # - P line inputs
sleep 0.4
$MONITOR 0x40000014 w 0xff   # - N outputs
sleep 0.4
# Setting relay states, Set LV jumper settings, Connect OUT1&OUT2 to the IN1&IN2
# DIO5_N = 1
# DIO6_N = 0
# DIO7_N = 1
$MONITOR 0x4000001C w 0xA0 # -> Set N
sleep 0.4

#Generate DC output signal with amplitude
$GENERATE 1 0 0 sqr
$GENERATE 2 0 0 sqr
sleep 0.4

#Acquire data with $DECIMATION decimation factor
$ACQUIRE -e -s $ADC_BUFF_SIZE $DECIMATION > /tmp/adc.txt   # WORKAROUND: First acquisition is thrown away
sleep 0.4
$ACQUIRE -e -s $ADC_BUFF_SIZE $DECIMATION > /tmp/adc.txt
cat /tmp/adc.txt | awk '{print $1}' > /tmp/adc_a.txt
cat /tmp/adc.txt | awk '{print $2}' > /tmp/adc_b.txt

# Calculate mean value
ADC_A_MEAN=$(awk -v N1_LV=$N1_LV '{sum+=$1} END { print int((sum/NR)-N1_LV)}' /tmp/adc_a.txt)
ADC_B_MEAN=$(awk -v N2_LV=$N2_LV '{sum+=$1} END { print int((sum/NR)-N2_LV)}' /tmp/adc_b.txt)
OUT1_DC_offs=$(awk -v ADC_A_MEAN=$ADC_A_MEAN -v BE_CH1_DC_offs=$BE_CH1_DC_offs 'BEGIN { print sprintf("%d", int(BE_CH1_DC_offs-ADC_A_MEAN))}')
OUT2_DC_offs=$(awk -v ADC_B_MEAN=$ADC_B_MEAN -v BE_CH2_DC_offs=$BE_CH2_DC_offs 'BEGIN { print sprintf("%d", int(BE_CH2_DC_offs-ADC_B_MEAN))}')

BE_CH1_DC_offs=$OUT1_DC_offs
BE_CH2_DC_offs=$OUT2_DC_offs
# Print out the measurements
echo "      NEW OUT1 DC offset cal param >>BE_CH1_DC_offs<<  is $BE_CH1_DC_offs"
echo "      NEW OUT2 DC offset cal param >>BE_CH2_DC_offs<<  is $BE_CH2_DC_offs"
echo

# Check if the values are within expectations
if [[ $BE_CH1_DC_offs -gt $BE_DC_offs_MAX ]] || [[ $BE_CH1_DC_offs -lt $BE_DC_offs_MIN ]]
then
    echo "      OUT1 DC offset calibration parameter ($OUT1_DC_offs) is outside expected range (+/- $BE_DC_offs_MAX)"
    TEST_STATUS=0
    CALIBRATION_STATUS=0
fi

if [[ $BE_CH2_DC_offs -gt $BE_DC_offs_MAX ]] || [[ $BE_CH2_DC_offs -lt $BE_DC_offs_MIN ]]
then
    echo "      OUT2 DC offset calibration parameter ($OUT2_DC_offs) is outside expected range (+/- $BE_DC_offs_MAX)"
    TEST_STATUS=0
    CALIBRATION_STATUS=0
fi

sleep 0.4
LOG_VAR="$LOG_VAR $BE_CH1_DC_offs $BE_CH2_DC_offs"
                        echo
                        echo "------------------Printing  Log variables  step 10: Calibration parameters BE_CH1_DC_offs, BE_CH2_DC_offs -----------------------------------------"
                        echo "$LOG_VAR"
                        echo "--------------------------------------------------------------------------------------------------------------------------------------------------"
                        echo

# COPY NEW CALIBRATION PARAMETERS IN TO USER EPROM SPACE/PARTITION  #
NEW_CAL_PARAMS="$FE_CH1_FS_G_HI $FE_CH2_FS_G_HI $FE_CH1_FS_G_LO $FE_CH2_FS_G_LO $FE_CH1_DC_offs $FE_CH2_DC_offs $BE_CH1_FS $BE_CH2_FS $BE_CH1_DC_offs $BE_CH2_DC_offs $SOME_eeprom_value $FE_CH1_DC_offs_HI $FE_CH2_DC_offs_HI"
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


echo
echo "---Output gain calibration is started---"

# Set Directions of DIO
$MONITOR 0x40000010 w 0x00   # - P line inputs
sleep 0.4
$MONITOR 0x40000014 w 0xff   # - N outputs
sleep 0.4
# Setting relay states, Set LV jumper settings, Connect OUT1&OUT2 to the IN1&IN2
# DIO5_N = 1
# DIO6_N = 0
# DIO7_N = 1
$MONITOR 0x4000001C w 0xA0 # -> Set N
sleep 0.4

#Generate DC output signal
LD_LIBRARY_PATH=/opt/redpitaya/lib  generate_DC_LO
sleep 1
# Restore LED status Calling generate_DC will reset LEDs. generate_DC.c has rp_Init() inside.
$MONITOR $LED_ADDR w "$(printf '0x%02x' $TEST_GLOBAL_STATUS)"

# Caling LD_LIBRARY_PATH=/opt/redpitaya/lib  generate_DC_LO RESETS DIO so set it agin
# Set Directions of DIO
$MONITOR 0x40000010 w 0x00   # - P line inputs
sleep 0.2
$MONITOR 0x40000014 w 0xff   # - N outputs
sleep 0.2
# Setting relay states, Set LV jumper settings, Connect OUT1&OUT2 to the IN1&IN2
# DIO5_N = 1
# DIO6_N = 0
# DIO7_N = 1
$MONITOR 0x4000001C w 0xA0 # -> Set N
sleep 0.4

#Acquire data with $DECIMATION decimation factor
$ACQUIRE -e -s $ADC_BUFF_SIZE $DECIMATION > /tmp/adc.txt   # WORKAROUND: First acquisition is thrown away
sleep 0.4
$ACQUIRE -e -s $ADC_BUFF_SIZE $DECIMATION > /tmp/adc.txt
cat /tmp/adc.txt | awk '{print $1}' > /tmp/adc_a.txt
cat /tmp/adc.txt | awk '{print $2}' > /tmp/adc_b.txt

# Calculate mean value and correct it using CALIBRATED input gains
ADC_A_MEAN=$(awk -v N1_LV=$N1_LV -v GAIN1_LV=$GAIN1_LV '{sum+=$1} END { print int(((sum/NR)*GAIN1_LV)-N1_LV)}' /tmp/adc_a.txt)
ADC_B_MEAN=$(awk -v N2_LV=$N2_LV -v GAIN2_LV=$GAIN1_LV '{sum+=$1} END { print int(((sum/NR)*GAIN2_LV)-N2_LV)}' /tmp/adc_b.txt)

OUT1_VOLTAGE_LO=$(awk -v ADC_A_MEAN=$ADC_A_MEAN 'BEGIN { print ((ADC_A_MEAN/8192))}' )
OUT2_VOLTAGE_LO=$(awk -v ADC_B_MEAN=$ADC_B_MEAN 'BEGIN { print ((ADC_B_MEAN/8192))}' )
#Print out the measurements
echo "OUT1_VOLTAGE_LO is $OUT1_VOLTAGE_LO"
echo "OUT2_VOLTAGE_LO is $OUT2_VOLTAGE_LO"
echo

GAIN1_OUT_LO=$(awk -v BE_CH1_DC_offs=$BE_CH1_DC_offs -v OUT_AMP_LO_cnt=$OUT_AMP_LO_cnt -v ADC_A_MEAN=$ADC_A_MEAN 'BEGIN {print ((ADC_A_MEAN)/OUT_AMP_LO_cnt) }')
GAIN2_OUT_LO=$(awk -v BE_CH2_DC_offs=$BE_CH2_DC_offs -v OUT_AMP_LO_cnt=$OUT_AMP_LO_cnt -v ADC_B_MEAN=$ADC_B_MEAN 'BEGIN {print ((ADC_B_MEAN)/OUT_AMP_LO_cnt) }')

# Caling LD_LIBRARY_PATH=/opt/redpitaya/lib  generate_DC_LO RESETS DIO so set it agin
# Set Directions of DIO
$MONITOR 0x40000010 w 0x00   # - P line inputs
sleep 0.2
$MONITOR 0x40000014 w 0xff   # - N outputs
sleep 0.2
# Setting relay states, Set LV jumper settings, Connect OUT1&OUT2 to the IN1&IN2
# DIO5_N = 1
# DIO6_N = 0
# DIO7_N = 1
$MONITOR 0x4000001C w 0xA0 # -> Set N
sleep 0.4

#Generate DC output signal HI
LD_LIBRARY_PATH=/opt/redpitaya/lib generate_DC
sleep 1

# Restore LED status Calling generate_DC will reset LEDs. generate_DC.c has rp_Init() inside.
$MONITOR $LED_ADDR w "$(printf '0x%02x' $TEST_GLOBAL_STATUS)"

# Caling LD_LIBRARY_PATH=/opt/redpitaya/lib  generate_DC_LO RESETS DIO so set it agin
# Set Directions of DIO
$MONITOR 0x40000010 w 0x00   # - P line inputs
sleep 0.2
$MONITOR 0x40000014 w 0xff   # - N outputs
sleep 0.2
# Setting relay states, Set LV jumper settings, Connect OUT1&OUT2 to the IN1&IN2
# DIO5_N = 1
# DIO6_N = 0
# DIO7_N = 1
$MONITOR 0x4000001C w 0xA0 # -> Set N
sleep 0.4

#Acquire data with $DECIMATION decimation factor
$ACQUIRE -e -s $ADC_BUFF_SIZE $DECIMATION > /tmp/adc.txt   # WORKAROUND: First acquisition is thrown away
sleep 0.4
$ACQUIRE -e -s $ADC_BUFF_SIZE $DECIMATION > /tmp/adc.txt
cat /tmp/adc.txt | awk '{print $1}' > /tmp/adc_a.txt
cat /tmp/adc.txt | awk '{print $2}' > /tmp/adc_b.txt

# Calculate mean value and correct it using CALIBRATED input gains
ADC_A_MEAN=$(awk -v N1_LV=$N1_LV -v GAIN1_LV=$GAIN1_LV '{sum+=$1} END { print int(((sum/NR)*GAIN1_LV)-N1_LV)}' /tmp/adc_a.txt)
ADC_B_MEAN=$(awk -v N2_LV=$N2_LV -v GAIN2_LV=$GAIN1_LV '{sum+=$1} END { print int(((sum/NR)*GAIN2_LV)-N2_LV)}' /tmp/adc_b.txt)


OUT1_VOLTAGE_HI=$(awk -v ADC_A_MEAN=$ADC_A_MEAN 'BEGIN { print ((ADC_A_MEAN/8192))}' )
OUT2_VOLTAGE_HI=$(awk -v ADC_B_MEAN=$ADC_B_MEAN 'BEGIN { print ((ADC_B_MEAN/8192))}' )
#Print out the measurements
echo "OUT1_VOLTAGE_HI is $OUT1_VOLTAGE_HI"
echo "OUT2_VOLTAGE_HI is $OUT2_VOLTAGE_HI"
echo


GAIN1_OUT_HI=$(awk -v BE_CH1_DC_offs=$BE_CH1_DC_offs -v OUT_AMP_HI_cnt=$OUT_AMP_HI_cnt -v ADC_A_MEAN=$ADC_A_MEAN 'BEGIN {print ((ADC_A_MEAN)/OUT_AMP_HI_cnt) }')
GAIN2_OUT_HI=$(awk -v BE_CH2_DC_offs=$BE_CH2_DC_offs -v OUT_AMP_HI_cnt=$OUT_AMP_HI_cnt -v ADC_B_MEAN=$ADC_B_MEAN 'BEGIN {print ((ADC_B_MEAN)/OUT_AMP_HI_cnt) }')

GAIN1_OUT=$(awk -v GAIN1_OUT_LO=$GAIN1_OUT_LO  -v GAIN1_OUT_HI=$GAIN1_OUT_HI 'BEGIN { print ((GAIN1_OUT_LO+GAIN1_OUT_HI)/2) }')
GAIN2_OUT=$(awk -v GAIN2_OUT_LO=$GAIN2_OUT_LO  -v GAIN2_OUT_HI=$GAIN2_OUT_HI 'BEGIN { print ((GAIN2_OUT_LO+GAIN2_OUT_HI)/2) }')
echo "GAIN1_OUT is $GAIN1_OUT"
echo "GAIN2_OUT is $GAIN2_OUT"
echo


BE_CH1_FS=$(awk -v GAIN1_OUT=$GAIN1_OUT 'BEGIN { print sprintf("%d", int((42949673*GAIN1_OUT))) }')
BE_CH2_FS=$(awk -v GAIN2_OUT=$GAIN2_OUT 'BEGIN { print sprintf("%d", int((42949673*GAIN2_OUT))) }')

# Print out the measurements
echo "      NEW OUT1 gain cal param >>BE_CH1_FS<< is $BE_CH1_FS"
echo "      NEW OUT2 gain cal param >>BE_CH2_FS<< is $BE_CH2_FS"
echo

# Trun OFF outputs
$GENERATE 1 0 0 sqr
$GENERATE 2 0 0 sqr

# Check if the values are within expectations
if [[ $BE_CH1_FS -gt $BE_FS_MAX ]] || [[ $BE_CH1_FS -lt $BE_FS_MIN ]]
then
    echo "      OUT1 gain calibration parameter ($BE_CH1_FS) is outside expected range (42949673 +/- 30%)"
    TEST_STATUS=0
    CALIBRATION_STATUS=0
fi

if [[ $BE_CH2_FS -gt $BE_FS_MAX ]] || [[ $BE_CH2_FS -lt $BE_FS_MIN ]]
then
    echo "      OUT2 gain calibration parameter ($BE_CH2_FS) is outside expected range (42949673 +/- 30%)"
    TEST_STATUS=0
    CALIBRATION_STATUS=0
fi

LOG_VAR="$LOG_VAR $BE_CH1_FS $BE_CH2_FS"

                        echo
                        echo "------------------Printing  Log variables  step 11: Calibration parameters BE_CH1_FS, BE_CH2_FS --------------------------------------------------"
                        echo "$LOG_VAR"
                        echo "--------------------------------------------------------------------------------------------------------------------------------------------------"
                        echo




if [[ $CALIBRATION_STATUS -eq 0 ]]
then

                    echo
                    echo "          Calibration has faild...Restoring factory calibration parameters..."
                    echo
                    echo
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
                    echo
                    echo

else
echo "          Calibration was successfull"
TEST_GLOBAL_STATUS=$(($TEST_GLOBAL_STATUS+$TEST_VALUE_LED))
$MONITOR $LED_ADDR w "$(printf '0x%02x' $TEST_GLOBAL_STATUS)"
echo
echo

                    # COPY NEW CALIBRATION PARAMETERS IN TO USER EPROM SPACE/PARTITION  #
                    NEW_CAL_PARAMS="$FE_CH1_FS_G_HI $FE_CH2_FS_G_HI $FE_CH1_FS_G_LO $FE_CH2_FS_G_LO $FE_CH1_DC_offs $FE_CH2_DC_offs $BE_CH1_FS $BE_CH2_FS $BE_CH1_DC_offs $BE_CH2_DC_offs $SOME_eeprom_value $FE_CH1_DC_offs_HI $FE_CH2_DC_offs_HI"
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

                    echo "Setting the new calibration parameters into the FACTORY EEPROM space..."
                    echo
                            echo $NEW_CAL_PARAMS | $CALIB -wf
                            sleep 0.1
                            if [ $? -ne 0 ]
                            then
                            echo
                            echo "New calibration parameters are NOT correctly written in the FACTORY EEPROM space"
                            sleep 1
                            fi
                    echo -ne '\n' | $CALIB -rf

fi


LOG_VAR="$LOG_VAR $CALIBRATION_STATUS"


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

    #echo 'sometext' | ssh zumy@192.168.178.123 "cat >> /home/zumy/Desktop/Test_LOGS/manuf_test.log"
    echo $LOG_VAR_DATE | ssh $LOCAL_USER@$LOCAL_SERVER_IP "cat >> $LOCAL_SERVER_DIR/$LOG_FILENAME"
    echo
    echo "      Test data logging on the local PC was successfull"
    else
    echo "      Not possible to log test statistics to local PC"
    LOGGING_STATUS=0
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
if [ `echo $CURL_RSP | grep -c "OK" ` -gt 0 ]
then
echo
echo "      Test record was successfully added to production database."
echo
elif [ `echo $CURL_RSP | grep -c "FAILED" ` -gt 0 ]
then
echo
echo
echo "      This board  (combination of MAC & DNA) already exists in production database with PASS(0x1ff = 111111111) status!!!"
echo
echo
LOGGING_STATUS=0
else
echo
echo "      No response from SERVER!!!"
echo
LOGGING_STATUS=0
fi
echo
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

