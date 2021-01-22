#!/bin/bash
source ./sub_test/common_func.sh
source ./sub_test/default_calibration_values.sh


function acquireData(){
    #Acquire data with $DECIMATION decimation factor
    $C_ACQUIRE $ADC_PARAM -c $ADC_BUFF_SIZE $DECIMATION > /tmp/adc.txt   # WORKAROUND: First acquisition is thrown away
    sleep 0.4
    $C_ACQUIRE $ADC_PARAM -c $ADC_BUFF_SIZE $DECIMATION > /tmp/adc.txt
    cat /tmp/adc.txt | awk '{print $1}' > /tmp/adc_a.txt
    cat /tmp/adc.txt | awk '{print $2}' > /tmp/adc_b.txt

    # Calculate mean value
    ADC_A=$(awk '{sum+=$1} END { print sum/NR}' /tmp/adc_a.txt)
    ADC_B=$(awk '{sum+=$1} END { print sum/NR}' /tmp/adc_b.txt)
}

function acquireData_AC(){
    #Acquire data with $DECIMATION decimation factor
    $C_ACQUIRE $ADC_PARAM -c $ADC_BUFF_SIZE $DECIMATION > /tmp/adc.txt   # WORKAROUND: First acquisition is thrown away
    sleep 0.4
    $C_ACQUIRE $ADC_PARAM -c $ADC_BUFF_SIZE $DECIMATION > /tmp/adc.txt
    VAR=$($C_A_SIGNAL -f /tmp/adc.txt)
    # Calculate mean value
    #ADC_A_MAX=$(awk '{sum+=$1} END { print int(sum/NR)}' /tmp/adc_a.txt)
    #ADC_B=$(awk '{sum+=$1} END { print int(sum/NR)}' /tmp/adc_b.txt)
    CH1_MAX=$(gawk 'match($0, /^CH1Max=(.+) CH2Max=(.+) CH1Min=(.+) CH2Min=(.+)$/, a) {print a[1]}' <<< "${VAR}")
    CH2_MAX=$(gawk 'match($0, /^CH1Max=(.+) CH2Max=(.+) CH1Min=(.+) CH2Min=(.+)$/, a) {print a[2]}' <<< "${VAR}")
    CH1_MIN=$(gawk 'match($0, /^CH1Max=(.+) CH2Max=(.+) CH1Min=(.+) CH2Min=(.+)$/, a) {print a[3]}' <<< "${VAR}")
    CH2_MIN=$(gawk 'match($0, /^CH1Max=(.+) CH2Max=(.+) CH1Min=(.+) CH2Min=(.+)$/, a) {print a[4]}' <<< "${VAR}")
}


# arg1 value_a 
# arg2 value_b 
# arg3 min value
# arg4 max value
function checkValue(){
        # Check if the values are within expectations
    if [[ $(echo "$1 < $3" |bc -l) -eq 1 ]] || [[ $(echo "$1 > $4" |bc -l) -eq 1 ]]
    then
        echo
        echo -n "      Measured IN1 value ($1) is outside expected range ($3 - $4)"
        print_fail
        echo
        STATUS=1
        CALIBRATION_STATUS=1
    fi

    if [[ $(echo "$2 < $3" |bc -l) -eq 1 ]] || [[ $(echo "$2 > $4" |bc -l) -eq 1 ]]
    then
        echo
        echo -n "      Measured IN2 value ($2) is outside expected range ($3 - $4)"
        print_fail
        echo
        STATUS=1
        CALIBRATION_STATUS=1
    fi
}

# args
# 1 - value
# 2 - offset
# 3 - gain

function convertMeasure(){
    CALC_MEASURE=$(printf %f $(bc -l <<< "$1 * $3 + $2"))
}



getDefCalibValues
# MAX/MIN calibration parameters
FE_FS_G_HI_MAX=$(printf %.0f $(bc -l <<< "scale=0; $FE_CH1_FS_G_HI * 1.3")) # default FE_CH1_FS_G_HI + 30 %
FE_FS_G_LO_MAX=$(printf %.0f $(bc -l <<< "scale=0; $FE_CH1_FS_G_LO * 1.3")) # default FE_CH1_FS_G_LO + 30 %
FE_DC_offs_MAX=300
BE_FS_MAX=$(printf %.0f $(bc -l <<< "scale=0; $BE_CH1_FS * 1.3"))
BE_DC_offs_MAX=300
FE_DC_offs_HI_MAX=400

FE_FS_G_HI_MIN=$(printf %.0f $(bc -l <<< "scale=0; $FE_CH1_FS_G_HI * 0.7")) # default FE_CH1_FS_G_HI - 30 %
FE_FS_G_LO_MIN=$(printf %.0f $(bc -l <<< "scale=0; $FE_CH1_FS_G_LO * 0.7")) # default FE_CH1_FS_G_LO - 30 %
FE_DC_offs_MIN=-300
BE_FS_MIN=$(printf %.0f $(bc -l <<< "scale=0; $BE_CH1_FS * 0.7"))
BE_DC_offs_MIN=-300
FE_DC_offs_HI_MIN=-400

FE_AC_offs_MAX=300
FE_AC_offs_HI_MAX=400

FE_AC_offs_MIN=-300
FE_AC_offs_HI_MIN=-400


DECIMATION=1024
ADC_BUFF_SIZE=16000
OUT_AMP_LO_cnt=3686 # 0.45V  VOLTS VPP reference voltage
OUT_AMP_HI_cnt=4096
OUT_AMP_HI_AC_cnt=819 # 4V 

echo
echo -e "\e[94m########################################################################\e[0m"
echo -e "\e[94m#            Fast ADCs and DACs CALIBRATION                            #\e[0m"
echo -e "\e[94m########################################################################\e[0m"
echo

STATUS=0
CALIBRATION_STATUS=0
LIGHT_STATUS=$($C_MONITOR 0x40000030)

echo "  * Set default calibration"
export FACTORY_CAL
./sub_test/set_calibration.sh


##########################################################################################
##########################################################################################
#  Calibrate DC mode (1:1)
##########################################################################################
##########################################################################################
echo "##########################################################################################"
echo "Calibrate ADC in DC mode"
echo "##########################################################################################"
echo
echo "INPUT DC offset calibration; (1:1); IN=GND"
echo

sleep 0.5
echo -n "  * Disable generator "
# turn off generator 
disableGenerator
print_ok

sleep 0.5
echo -n "  * Connect IN to GND "

# connect in to out 
# Set Directions of DIO
$C_MONITOR 0x40000010 w 0x00   # - P line inputs
sleep 0.4
$C_MONITOR 0x40000014 w 0xff   # - N outputs
sleep 0.4
# Set LV jumper settings Connect IN1&IN2 to the GND, LV jumper settings
# DIO5_N = 0
# DIO6_N = 0
# DIO7_N = 1
$C_MONITOR 0x4000001C w 0x80 # -> Set N
sleep 0.4
print_ok
sleep 0.5

# get data from adc
echo -n "  * Get data from ADC "
ADC_PARAM="-c -s -e -1 lv -2 lv"
acquireData
print_ok
# Print out the measurements
echo "      IN1 DC offset value is $ADC_A"
echo "      IN2 DC offset value is $ADC_B"

checkValue $ADC_A $ADC_B $FE_DC_offs_MIN $FE_DC_offs_MAX

FE_CH1_DC_offs=$ADC_A
FE_CH2_DC_offs=$ADC_B
N1_LV_DC=$ADC_A
N2_LV_DC=$ADC_B


##########################################################################################
##########################################################################################
#  Calibrate DC (0.9V) mode (1:1)
##########################################################################################
##########################################################################################

echo "INPUT DC gain calibration; (1:1); IN=$G_CALIBRATION_REF_LV_VALUE V"


sleep 0.5
echo -n "  * Connect ($G_CALIBRATION_REF_LV_VALUE V) to OUT "

# connect in to out 
# Set Directions of DIO
$C_MONITOR 0x40000010 w 0x00   # - P line inputs
sleep 0.4
$C_MONITOR 0x40000014 w 0xff   # - N outputs
sleep 0.4
# Set LV jumper settings Connect IN1&IN2 to the REF VALUE -> Set relay states  DIO6_N, DIO7_N and DIO7_P
# DIO5_N = 1
# DIO6_N = 1
# DIO7_N = 1
$C_MONITOR 0x4000001C w 0xE0 # -> Set N
sleep 0.4

print_ok
sleep 0.5
# get data from adc
echo -n "  * Get data from ADC "
ADC_PARAM=" -s -e -1 lv -2 lv"
acquireData
print_ok

# Print out the measurements
echo "      IN1 mean value is $ADC_A"
echo "      IN2 mean value is $ADC_B"

getLowRefValue
REF_VALUE_LV=$REF_V

GAIN1_LV_DC=$(awk -v N1_LV=$N1_LV_DC -v REF_VALUE_LV=$REF_VALUE_LV -v ADC_A=$ADC_A 'BEGIN { print ( ( REF_VALUE_LV) / ( ADC_A-N1_LV ) ) }')
GAIN2_LV_DC=$(awk -v N2_LV=$N2_LV_DC -v REF_VALUE_LV=$REF_VALUE_LV -v ADC_B=$ADC_B 'BEGIN { print ( ( REF_VALUE_LV) / ( ADC_B-N2_LV ) ) }')


# Print out the measurements
echo "      IN1 Gain is $GAIN1_LV_DC"
echo "      IN2 Gain is $GAIN2_LV_DC"

FE_CH1_FS_G_LO=$(awk -v GAIN1_LV=$GAIN1_LV_DC -v X=$FE_CH1_FS_G_LO 'BEGIN { print sprintf("%d", int((X*GAIN1_LV))) }')
FE_CH2_FS_G_LO=$(awk -v GAIN2_LV=$GAIN2_LV_DC -v X=$FE_CH2_FS_G_LO 'BEGIN { print sprintf("%d", int((X*GAIN2_LV))) }')

# Print out the measurements
echo
echo "      NEW IN1 (1:1) LV gain cal param >>FE_CH1_FS_G_LO<< is $FE_CH1_FS_G_LO"
echo "      NEW IN2 (1:1) LV gain cal param >>FE_CH2_FS_G_LO<< is $FE_CH2_FS_G_LO"


# Check if the values are within expectations
checkValue $FE_CH1_FS_G_LO $FE_CH2_FS_G_LO $FE_FS_G_LO_MIN $FE_FS_G_LO_MAX

##########################################################################################
##########################################################################################
#  Recalculaed DC offset by Gain mode (1:1)
##########################################################################################
##########################################################################################

# OSC_CH1_OFF_1_DC=$(bc -l <<< "$N1_LV_DC * $GAIN1_LV_DC")
# OSC_CH2_OFF_1_DC=$(bc -l <<< "$N2_LV_DC * $GAIN2_LV_DC")
# # round value
# OSC_CH1_OFF_1_DC=${OSC_CH1_OFF_1_DC%.*}
# OSC_CH2_OFF_1_DC=${OSC_CH2_OFF_1_DC%.*}
# # Print out the new cal parameters
# echo "      NEW IN1 1:1 DC offset cal param >>OSC_CH1_OFF_1_DC<< is $OSC_CH1_OFF_1_DC"
# echo "      NEW IN2 1:1 DC offset cal param >>OSC_CH2_OFF_1_DC<< is $OSC_CH2_OFF_1_DC"
# echo

##########################################################################################
# CHECK CALIBRATION
##########################################################################################

# Acquire data with $DECIMATION decimation factor
$C_ACQUIRE $ADC_BUFF_SIZE $DECIMATION > /tmp/adc.txt   # WORKAROUND: First acquisition is thrown away
sleep 0.4
$C_ACQUIRE $ADC_BUFF_SIZE $DECIMATION > /tmp/adc.txt
cat /tmp/adc.txt | awk '{print $1}' > /tmp/adc_a.txt
cat /tmp/adc.txt | awk '{print $2}' > /tmp/adc_b.txt

# Calculate mean value
ADC_A_MEAN=$(awk -v Y=$FE_CH1_DC_offs -v X=$GAIN1_LV_DC '{sum+=$1} END { print int( ((sum/NR)*X)-Y)}' /tmp/adc_a.txt)
ADC_B_MEAN=$(awk -v Y=$FE_CH2_DC_offs -v X=$GAIN2_LV_DC '{sum+=$1} END { print int( ((sum/NR)*X)-Y)}' /tmp/adc_b.txt)


IN1_ERROR_LV=$(awk -v X=$ADC_A_MEAN -v Y=$REF_VALUE_LV 'BEGIN { print (((X-Y)/Y)*100) }')
IN2_ERROR_LV=$(awk -v X=$ADC_B_MEAN -v Y=$REF_VALUE_LV 'BEGIN { print (((X-Y)/Y)*100) }')

# Print out the measurements
echo
echo "      IN1 Error after the calibration is $IN1_ERROR_LV %"
echo "      IN2 Error after the calibration is $IN2_ERROR_LV %"
echo 


##########################################################################################
##########################################################################################
#  Calibrate DC mode (1:20)
##########################################################################################
##########################################################################################

echo "INPUT offset calibration; (1:20); IN=GND"
echo

sleep 0.5
echo -n "  * Disable generator "
# turn off generator 
disableGenerator
print_ok

sleep 0.5
echo -n "  * Connect IN to GND "
# connect in to out 
# Set Directions of DIO
$C_MONITOR 0x40000010 w 0x00   # - P line inputs
sleep 0.4
$C_MONITOR 0x40000014 w 0xff   # - N outputs
sleep 0.4
# Setting relay states, Set HV jumper setting,s onnect IN1&IN2 to the GND.
# DIO5_N = 0
# DIO6_N = 0
# DIO7_N = 0
$C_MONITOR 0x4000001C w 0x00 # -> Set N
sleep 0.4
print_ok
sleep 0.5

# get data from adc
echo -n "  * Get data from ADC "
ADC_PARAM=" -s -e -1 hv -2 hv"
acquireData
print_ok
# Print out the measurements
echo "      IN1 DC offset value is $ADC_A"
echo "      IN2 DC offset value is $ADC_B"

# Check if the values are within expectations
checkValue $ADC_A $ADC_B $FE_DC_offs_MIN $FE_DC_offs_MAX

FE_CH1_DC_offs_HI=$ADC_A
FE_CH2_DC_offs_HI=$ADC_B
N1_HV_DC=$ADC_A
N2_HV_DC=$ADC_B

# Print out the new cal parameters
echo "      NEW IN1 (1:20) HV offset cal param >>FE_CH1_DC_offs_HI<< is $FE_CH1_DC_offs_HI"
echo "      NEW IN2 (1:20) HV offset cal param >>FE_CH2_DC_offs_HI<< is $FE_CH2_DC_offs_HI"
echo


##########################################################################################
##########################################################################################
#  Calibrate DC (9V) mode (1:20)
##########################################################################################
##########################################################################################

echo "INPUT gain calibration; (1:20); IN=$G_CALIBRATION_REF_HV_VALUE V"


sleep 0.5
echo -n "  * Connect ($G_CALIBRATION_REF_HV_VALUE V) to OUT "
# connect in to out 
# Set Directions of DIO
$C_MONITOR 0x40000010 w 0x00   # - P line inputs
sleep 0.4
$C_MONITOR 0x40000014 w 0xff   # - N outputs
sleep 0.4
# Setting relay states, Set HV jumper setting, connect IN1&IN2 to the REF_VALUE_HV.
# DIO5_N = 1
# DIO6_N = 1
# DIO7_N = 0
$C_MONITOR 0x4000001C w 0x60 # -> Set N
sleep 0.4
print_ok
sleep 0.5

# get data from adc
echo -n "  * Get data from ADC "
ADC_PARAM=" -s -e -1 hv -2 hv"
acquireData
print_ok
# Print out the measurements
echo "      IN1 mean value is $ADC_A"
echo "      IN2 mean value is $ADC_B"

getHighRefValue
REF_VALUE_HV=$REF_V
#echo $REF_VALUE_HV
GAIN1_HV_DC=$(awk -v N1_HV=$N1_HV_DC -v REF_VALUE_HV=$REF_VALUE_HV -v ADC_A=$ADC_A 'BEGIN { print ( ( REF_VALUE_HV) / ( ADC_A-N1_HV ) ) }')
GAIN2_HV_DC=$(awk -v N2_HV=$N2_HV_DC -v REF_VALUE_HV=$REF_VALUE_HV -v ADC_B=$ADC_B 'BEGIN { print ( ( REF_VALUE_HV) / ( ADC_B-N2_HV ) ) }')

# Print out the measurements
echo "      IN1_Gain is $GAIN1_HV_DC"
echo "      IN2_Gain is $GAIN2_HV_DC"

FE_CH1_FS_G_HI=$(awk -v X=$GAIN1_HV_DC -v Y=$FE_CH1_FS_G_HI 'BEGIN { print sprintf("%d", int((Y*X))) }')
FE_CH2_FS_G_HI=$(awk -v X=$GAIN2_HV_DC -v Y=$FE_CH2_FS_G_HI 'BEGIN { print sprintf("%d", int((Y*X))) }')

# Print out the measurements
echo
echo "      NEW IN1 gain cal param >>FE_CH1_FS_G_HI<< is $FE_CH1_FS_G_HI"
echo "      NEW IN2 gain cal param >>FE_CH2_FS_G_HI<< is $FE_CH2_FS_G_HI"


# Check if the values are within expectations
checkValue $FE_CH1_FS_G_HI $FE_CH2_FS_G_HI $FE_FS_G_HI_MIN $FE_FS_G_HI_MAX



##########################################################################################
##########################################################################################
#  Recalculaed DC offset by Gain mode (1:1)
##########################################################################################
##########################################################################################

# OSC_CH1_OFF_20_DC=$(bc -l <<< "$N1_HV_DC * $GAIN1_HV_DC")
# OSC_CH2_OFF_20_DC=$(bc -l <<< "$N2_HV_DC * $GAIN2_HV_DC")
# OSC_CH1_OFF_20_DC=${OSC_CH1_OFF_20_DC%.*}
# OSC_CH2_OFF_20_DC=${OSC_CH2_OFF_20_DC%.*}
# # Print out the new cal parameters
# echo "      NEW IN1 1:1 DC offset cal param >>OSC_CH1_OFF_1_DC<< is $OSC_CH1_OFF_20_DC"
# echo "      NEW IN2 1:1 DC offset cal param >>OSC_CH2_OFF_1_DC<< is $OSC_CH2_OFF_20_DC"
# echo

##########################################################################################
# CHECK CALIBRATION
##########################################################################################

# Acquire data with $DECIMATION decimation factor
$C_ACQUIRE -1 hv -2 hv $ADC_BUFF_SIZE $DECIMATION > /tmp/adc.txt   # WORKAROUND: First acquisition is thrown away
sleep 0.4
$C_ACQUIRE -1 hv -2 hv $ADC_BUFF_SIZE $DECIMATION > /tmp/adc.txt
cat /tmp/adc.txt | awk '{print $1}' > /tmp/adc_a.txt
cat /tmp/adc.txt | awk '{print $2}' > /tmp/adc_b.txt

# Calculate mean value
ADC_A_MEAN=$(awk -v Y=$FE_CH1_DC_offs_HI -v X=$GAIN1_HV_DC '{sum+=$1} END { print int( ((sum/NR)*X)-Y)}' /tmp/adc_a.txt)
ADC_B_MEAN=$(awk -v Y=$FE_CH2_DC_offs_HI -v X=$GAIN2_HV_DC '{sum+=$1} END { print int( ((sum/NR)*X)-Y)}' /tmp/adc_b.txt)

IN1_ERROR_HV=$(awk -v X=$ADC_A_MEAN -v REF_VALUE_HV=$REF_VALUE_HV 'BEGIN { print (((X-REF_VALUE_HV)/REF_VALUE_HV)*100) }')
IN2_ERROR_HV=$(awk -v X=$ADC_B_MEAN -v REF_VALUE_HV=$REF_VALUE_HV 'BEGIN { print (((X-REF_VALUE_HV)/REF_VALUE_HV)*100) }')

# Print out the measurements
echo
echo "      IN1 Error after the calibration is $IN1_ERROR_HV %"
echo "      IN2 Error after the calibration is $IN2_ERROR_HV %"
echo 



##########################################################################################
#  Set ADC parameters
##########################################################################################

echo "  * Set new ADC calibration"
FACTORY_NEW_CAL="$FE_CH1_FS_G_HI $FE_CH2_FS_G_HI $FE_CH1_FS_G_LO $FE_CH2_FS_G_LO $FE_CH1_DC_offs $FE_CH2_DC_offs $BE_CH1_FS $BE_CH2_FS $BE_CH1_DC_offs $BE_CH2_DC_offs $SOME_eeprom_value $FE_CH1_DC_offs_HI $FE_CH2_DC_offs_HI"
FACTORY_CAL=$FACTORY_NEW_CAL
export FACTORY_CAL
./sub_test/set_calibration.sh
echo

##########################################################################################
##########################################################################################
#  Calibrate Generator
##########################################################################################
##########################################################################################
echo "##########################################################################################"
echo "Calibrate generator"
echo "##########################################################################################"
echo
echo "Outputs DC offset calibration is started..."
echo
echo "OUTPUT DC offset calibration; Gain x1; OUT=0V"
sleep 0.5
echo -n "  * Disable generator "
# turn off generator 
disableGenerator

print_ok

sleep 0.5
echo -n "  * Connect IN to OUT "
# connect in to out 
# Set Directions of DIO
$C_MONITOR 0x40000010 w 0x00   # - P line inputs
sleep 0.4
$C_MONITOR 0x40000014 w 0xff   # - N outputs
sleep 0.4
# Setting relay states, Set LV jumper settings, Connect OUT1&OUT2 to the IN1&IN2
# DIO5_N = 1
# DIO6_N = 0
# DIO7_N = 1
$C_MONITOR 0x4000001C w 0xA0 # -> Set N
print_ok
sleep 0.5

# get data from adc
echo -n "  * Get data from ADC "
sleep 0.5
ADC_PARAM="-s -e -1 lv -2 lv"
acquireData
print_ok


echo "      IN1 offset value is $ADC_A"
echo "      IN2 offset value is $ADC_B"

ADC_A=$(printf %.4f $(bc -l <<< "-1 * ($ADC_A - $N1_LV_DC )  * $GAIN1_LV_DC"))
ADC_B=$(printf %.4f $(bc -l <<< "-1 * ($ADC_B - $N2_LV_DC )  * $GAIN2_LV_DC"))


# Print out the measurements
echo "      IN1 offset value is $ADC_A"
echo "      IN2 offset value is $ADC_B"

BE_CH1_DC_offs=$(printf %.0f $(bc -l <<< "($ADC_A + $BE_CH1_DC_offs)"))
BE_CH2_DC_offs=$(printf %.0f $(bc -l <<< "($ADC_B + $BE_CH2_DC_offs)"))

# Print out the measurements
echo "      NEW OUT1 DC offset cal param >>BE_CH1_DC_offs<<  is $BE_CH1_DC_offs"
echo "      NEW OUT2 DC offset cal param >>BE_CH2_DC_offs<<  is $BE_CH2_DC_offs"
echo

# Check if the values are within expectations
checkValue $BE_CH1_DC_offs $BE_CH2_DC_offs $BE_DC_offs_MIN $BE_DC_offs_MAX

##########################################################################################
#  Set ADC parameters
##########################################################################################

echo "  * Set new Generator offset calibration"
FACTORY_NEW_CAL="$FE_CH1_FS_G_HI $FE_CH2_FS_G_HI $FE_CH1_FS_G_LO $FE_CH2_FS_G_LO $FE_CH1_DC_offs $FE_CH2_DC_offs $BE_CH1_FS $BE_CH2_FS $BE_CH1_DC_offs $BE_CH2_DC_offs $SOME_eeprom_value $FE_CH1_DC_offs_HI $FE_CH2_DC_offs_HI"
FACTORY_CAL=$FACTORY_NEW_CAL
export FACTORY_CAL
./sub_test/set_calibration.sh
echo

# ##########################################################################################
# ##########################################################################################
# #  Calibrate Output (0.45V) x1 mode
# ##########################################################################################
# ##########################################################################################

echo
echo "OUTPUT DC gain calibration; Gain x1; OUT=0.45V "

sleep 0.5
echo -n "  * Start generator in DC (0.45V) " 
generate_with_api -d1
print_ok

sleep 0.5
echo -n "  * Connect IN to OUT "
# connect in to out 
$C_MONITOR 0x40000010 w 0x00   # - P line inputs
sleep 0.4
$C_MONITOR 0x40000014 w 0xff   # - N outputs
sleep 0.4
# Setting relay states, Set LV jumper settings, Connect OUT1&OUT2 to the IN1&IN2
# DIO5_N = 1
# DIO6_N = 0
# DIO7_N = 1
$C_MONITOR 0x4000001C w 0xA0 # -> Set N
print_ok
sleep 0.5

# get data from adc
echo -n "  * Get data from ADC "
ADC_PARAM="-s -e -1 lv -2 lv"
acquireData
print_ok


ADC_A=$(printf %.4f $(bc -l <<< "($ADC_A  - $N1_LV_DC ) * $GAIN1_LV_DC"))
ADC_B=$(printf %.4f $(bc -l <<< "($ADC_B  - $N2_LV_DC ) * $GAIN2_LV_DC"))

echo "ADC_A is $ADC_A"
echo "ADC_B is $ADC_B"

OUT1_VOLTAGE_LO=$(printf %.4f $(bc -l <<< "($ADC_A/4096)")) 
OUT2_VOLTAGE_LO=$(printf %.4f $(bc -l <<< "($ADC_B/4096)"))
#Print out the measurements
echo "OUT1_VOLTAGE_LO is $OUT1_VOLTAGE_LO"
echo "OUT2_VOLTAGE_LO is $OUT2_VOLTAGE_LO"
echo
GAIN1_OUT=$(printf %.4f $(bc -l <<< "($ADC_A/$OUT_AMP_LO_cnt)")) 
GAIN2_OUT=$(printf %.4f $(bc -l <<< "($ADC_B/$OUT_AMP_LO_cnt)")) 

echo "GAIN1_OUT is $GAIN1_OUT"
echo "GAIN2_OUT is $GAIN2_OUT"
echo


BE_CH1_FS=$(printf %.0f $(bc -l <<< "($GAIN1_OUT  * $BE_CH1_FS )")) 
BE_CH2_FS=$(printf %.0f $(bc -l <<< "($GAIN2_OUT  * $BE_CH2_FS )")) 

# Print out the measurements
echo
echo "      NEW OUT1 gain cal param >>BE_CH1_FS<< is $BE_CH1_FS"
echo "      NEW OUT2 gain cal param >>BE_CH2_FS<< is $BE_CH2_FS"


# Check if the values are within expectations
checkValue $BE_CH1_FS $BE_CH2_FS $BE_FS_MIN $BE_FS_MAX



##########################################################################################
#  Set ADC parameters
##########################################################################################

echo "  * Set new ADC calibration"
FACTORY_NEW_CAL="$FE_CH1_FS_G_HI $FE_CH2_FS_G_HI $FE_CH1_FS_G_LO $FE_CH2_FS_G_LO $FE_CH1_DC_offs $FE_CH2_DC_offs $BE_CH1_FS $BE_CH2_FS $BE_CH1_DC_offs $BE_CH2_DC_offs $SOME_eeprom_value $FE_CH1_DC_offs_HI $FE_CH2_DC_offs_HI"
FACTORY_CAL=$FACTORY_NEW_CAL
export FACTORY_CAL
./sub_test/set_calibration.sh
./sub_test/set_calibration_fw.sh
echo

$C_MONITOR 0x40000030 w $LIGHT_STATUS

if [[ $STATUS == 0 ]]
then
    RPLight7
else 
    echo -n "  * Set calibration"
    print_fail
    getDefCalibValues
    export FACTORY_CAL
    ./sub_test/set_calibration.sh
    ./sub_test/set_calibration_fw.sh
    echo    "  * Set default calibration"
fi

exit $STATUS
