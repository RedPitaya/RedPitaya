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

# Main commands shortcuts
MONITOR="$SD_CARD_PATH/bin/monitor_old"
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

###############################################################################
# Test variables
###############################################################################

LOG_VAR=''
TEST_GLOBAL_STATUS=0
LED_ADDR=0x40000030

# fast ADCs and DACs data acquisitions
ADC_BUFF_SIZE=16384

# Configure DIOx_P to inputs and DIOx_N to outputs to prevent Relay misbehaviour
# During DIO test this will be changed and after DIO test set back to this condition
$MONITOR 0x40000010 w 0x00 # -> Set P to inputs
sleep 0.2
$MONITOR 0x40000014 w 0xFF # -> Set N to outputs

# Use new FPGA image for ADC test and calibration
cat /opt/redpitaya/fpga/fpga_0.94.bit > /dev/xdevcfg
sleep 2



#####################################################################################
#####################################################################################


echo $FACTORY_CAL | $CALIB -wf
    status=$?
    sleep 0.2
    if [ $status -ne 0 ]
    then
        echo
        echo "Default calibration parameters are NOT correctly written in the factory EEPROM space"
        sleep 1
    fi
# Copy the NEW CALIBRATION PARAMETERS to the user EEPROM memory partition
echo $FACTORY_CAL | $CALIB -w
    status=$?
    sleep 0.2
    if [ $status -ne 0 ]
    then
        echo
        echo "Default calibration parameters are NOT correctly written in the user EEPROM space"
        sleep 1
    fi



echo "Fast ADCs and DACs CALIBRATION ---"
echo
echo "Reseting cal parameters to unit gains and zerro DC offset..."

TEST_STATUS=1                # It is not used in calibration but i have left it in IF checking so it can be used later if needed.
CALIBRATION_STATUS=1
TEST_VALUE_LED=128
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
        status=$?
        sleep 0.1
        if [ $status -ne 0 ]
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
                        echo "------------------Printing  Log variables  step 1: Calibration parameters FE_CH1_DC_offs, FE_CH2_DC_offs -----------------------------------------"
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
                        echo "------------------Printing  Log variables  step 2: Calibration parameters FE_CH1_FS_G_LO, FE_CH2_FS_G_LO -----------------------------------------"
                        echo "$LOG_VAR"
                        echo "--------------------------------------------------------------------------------------------------------------------------------------------------"
                        echo

# COPY NEW CALIBRATION PARAMETERS IN TO USER  EPROM SPACE/PARTITION
NEW_CAL_PARAMS="$FE_CH1_FS_G_HI $FE_CH2_FS_G_HI $FE_CH1_FS_G_LO $FE_CH2_FS_G_LO $FE_CH1_DC_offs $FE_CH2_DC_offs $BE_CH1_FS $BE_CH2_FS $BE_CH1_DC_offs $BE_CH2_DC_offs $SOME_eeprom_value $FE_CH1_DC_offs_HI $FE_CH2_DC_offs_HI"

# Copy the NEW CALIBRATION PARAMETERS to the user EEPROM memory partition
echo "Setting the new calibration parameters into the user EEPROM space..."
echo
        echo $NEW_CAL_PARAMS | $CALIB -w
        status=$?
        sleep 0.1
        if [ $status -ne 0 ]
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
                        echo "------------------Printing  Log variables  step 3: Calibration parameters FE_CH1_DC_offs_HI, FE_CH2_DC_offs_HI -----------------------------------------"
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
                        echo "------------------Printing  Log variables  step 4: Calibration parameters FE_CH1_FS_G_HI,  FE_CH2_FS_G_HI -----------------------------------------"
                        echo "$LOG_VAR"
                        echo "--------------------------------------------------------------------------------------------------------------------------------------------------"
                        echo


# COPY NEW CALIBRATION PARAMETERS IN TO USER EPROM SPACE/PARTITION  #
NEW_CAL_PARAMS="$FE_CH1_FS_G_HI $FE_CH2_FS_G_HI $FE_CH1_FS_G_LO $FE_CH2_FS_G_LO $FE_CH1_DC_offs $FE_CH2_DC_offs $BE_CH1_FS $BE_CH2_FS $BE_CH1_DC_offs $BE_CH2_DC_offs $SOME_eeprom_value $FE_CH1_DC_offs_HI $FE_CH2_DC_offs_HI"

# Copy the NEW CALIBRATION PARAMETERS to the user EEPROM memory partition
echo "Setting the new calibration parameters into the user EEPROM space..."
echo
        echo $NEW_CAL_PARAMS | $CALIB -w
        status=$?
        sleep 0.1
        if [ $status -ne 0 ]
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
                        echo "------------------Printing  Log variables  step 5: Calibration parameters BE_CH1_DC_offs, BE_CH2_DC_offs -----------------------------------------"
                        echo "$LOG_VAR"
                        echo "--------------------------------------------------------------------------------------------------------------------------------------------------"
                        echo

# COPY NEW CALIBRATION PARAMETERS IN TO USER EPROM SPACE/PARTITION  #
NEW_CAL_PARAMS="$FE_CH1_FS_G_HI $FE_CH2_FS_G_HI $FE_CH1_FS_G_LO $FE_CH2_FS_G_LO $FE_CH1_DC_offs $FE_CH2_DC_offs $BE_CH1_FS $BE_CH2_FS $BE_CH1_DC_offs $BE_CH2_DC_offs $SOME_eeprom_value $FE_CH1_DC_offs_HI $FE_CH2_DC_offs_HI"
echo "Setting the new calibration parameters into the user EEPROM space..."
echo
        echo $NEW_CAL_PARAMS | $CALIB -w
        status=$?
        sleep 0.1
        if [ $status -ne 0 ]
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
                        echo "------------------Printing  Log variables  step 6: Calibration parameters BE_CH1_FS, BE_CH2_FS --------------------------------------------------"
                        echo "$LOG_VAR"
                        echo "--------------------------------------------------------------------------------------------------------------------------------------------------"
                        echo




if [[ $CALIBRATION_STATUS -eq 0 ]]
then
                    echo "*********************************************************************************"
                    echo "*                                                                               *"
                    echo "*          Calibration has faild...Restoring factory calibration parameters...  *"
                    echo "*                                                                               *"
                    echo "*                                                                               *"
                    echo "*********************************************************************************"
                                echo $FACTORY_CAL | $CALIB -wf
                                    status=$?
                                    sleep 0.2
                                    if [ $status -ne 0 ]
                                    then
                                        echo
                                        echo "Default calibration parameters are NOT correctly written in the factory EEPROM space"
                                        sleep 1
                                    fi
                                # Copy the NEW CALIBRATION PARAMETERS to the user EEPROM memory partition
                                echo $FACTORY_CAL | $CALIB -w
                                    status=$?
                                    sleep 0.2
                                    if [ $status -ne 0 ]
                                    then
                                        echo
                                        echo "Default calibration parameters are NOT correctly written in the user EEPROM space"
                                        sleep 1
                                    fi
                    echo
                    echo

else

                    echo "*********************************************************************************"
                    echo "*                                                                               *"
                    echo "*                                                                               *"
                    echo "                    Calibration was successfull                                 *"
                    echo "*                                                                               *"
                    echo "*                                                                               *"
                    echo "*********************************************************************************"

TEST_GLOBAL_STATUS=$(($TEST_GLOBAL_STATUS+$TEST_VALUE_LED))
$MONITOR $LED_ADDR w "$(printf '0x%02x' $TEST_GLOBAL_STATUS)"
echo
echo

                    # COPY NEW CALIBRATION PARAMETERS IN TO USER EPROM SPACE/PARTITION  #
                    NEW_CAL_PARAMS="$FE_CH1_FS_G_HI $FE_CH2_FS_G_HI $FE_CH1_FS_G_LO $FE_CH2_FS_G_LO $FE_CH1_DC_offs $FE_CH2_DC_offs $BE_CH1_FS $BE_CH2_FS $BE_CH1_DC_offs $BE_CH2_DC_offs $SOME_eeprom_value $FE_CH1_DC_offs_HI $FE_CH2_DC_offs_HI"
                    echo "Setting the new calibration parameters into the user EEPROM space..."
                    echo
                            echo $NEW_CAL_PARAMS | $CALIB -w
                            status=$?
                            sleep 0.1
                            if [ $status -ne 0 ]
                            then
                            echo
                            echo "New calibration parameters are NOT correctly written in the user EEPROM space"
                            sleep 1
                            fi
                    echo -ne '\n' | $CALIB -r

                    echo "Setting the new calibration parameters into the FACTORY EEPROM space..."
                    echo
                            echo $NEW_CAL_PARAMS | $CALIB -wf
                            status=$?
                            sleep 0.1
                            if [ $status -ne 0 ]
                            then
                            echo
                            echo "New calibration parameters are NOT correctly written in the FACTORY EEPROM space"
                            sleep 1
                            fi
                    echo -ne '\n' | $CALIB -rf

fi

LOG_VAR="$LOG_VAR $CALIBRATION_STATUS"

                        echo " "
                        echo "----------------------------******** FINAL CALIBRATION PARAMETERS ********------------------------------------------------------------------------"
                        echo "$LOG_VAR "
                        echo "--------------------------------------------------------------------------------------------------------------------------------------------------"
                        echo " "



# Setting relay states, Set LV jumper settings, Connect OUT1&OUT2 to the IN1&IN2 - once more just in case
# Set Directions of DIO
$MONITOR 0x40000010 w 0x00   # - P line inputs
sleep 0.2
$MONITOR 0x40000014 w 0xff   # - N outputs
sleep 0.2
$MONITOR 0x4000001C w 0xA0   # -> Set N
sleep 0.2

