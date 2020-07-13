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
FE_FS_G_HI_MAX=$(printf %.0f $(bc -l <<< "scale=0; $OSC_CH1_G_20_DC * 1.3")) # default FE_CH1_FS_G_HI + 30 %
FE_FS_G_LO_MAX=$(printf %.0f $(bc -l <<< "scale=0; $OSC_CH1_G_1_DC * 1.3")) # default FE_CH1_FS_G_LO + 30 %
FE_DC_offs_MAX=300
BE_FS_MAX_1=$(printf %.0f $(bc -l <<< "scale=0; $GEN_CH1_G_1 * 1.3"))
BE_FS_MAX_5=$(printf %.0f $(bc -l <<< "scale=0; $GEN_CH1_G_5 * 1.3"))
BE_DC_offs_MAX=300
FE_DC_offs_HI_MAX=400

FE_FS_G_HI_MIN=$(printf %.0f $(bc -l <<< "scale=0; $OSC_CH1_G_20_DC * 0.7")) # default FE_CH1_FS_G_HI - 30 %
FE_FS_G_LO_MIN=$(printf %.0f $(bc -l <<< "scale=0; $OSC_CH1_G_1_DC * 0.7")) # default FE_CH1_FS_G_LO - 30 %
FE_DC_offs_MIN=-300
BE_FS_MIN_1=$(printf %.0f $(bc -l <<< "scale=0; $GEN_CH1_G_1 * 0.7"))
BE_FS_MIN_5=$(printf %.0f $(bc -l <<< "scale=0; $GEN_CH1_G_5 * 0.7"))
BE_DC_offs_MIN=-300
FE_DC_offs_HI_MIN=-400

FE_AC_offs_MAX=300
FE_AC_offs_HI_MAX=400

FE_AC_offs_MIN=-300
FE_AC_offs_HI_MIN=-400


DECIMATION=1024
ADC_BUFF_SIZE=16000
OUT_AMP_LO_cnt=1843 # 0.45V  VOLTS VPP reference voltage
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
disableAllDIOPin
print_ok
sleep 0.5

# get data from adc
echo -n "  * Get data from ADC "
ADC_PARAM="-d B"
acquireData
print_ok
# Print out the measurements
echo "      IN1 DC offset value is $ADC_A"
echo "      IN2 DC offset value is $ADC_B"

checkValue $ADC_A $ADC_B $FE_DC_offs_MIN $FE_DC_offs_MAX

OSC_CH1_OFF_1_DC=$ADC_A
OSC_CH2_OFF_1_DC=$ADC_B
N1_LV_DC=$ADC_A
N2_LV_DC=$ADC_B



##########################################################################################
##########################################################################################
#  Calibrate DC (0.45V) mode (1:1)
##########################################################################################
##########################################################################################

echo "INPUT DC gain calibration; (1:1); IN=0.45V"


sleep 0.5
echo -n "  * Connect (0.45V) to OUT "
# connect in to out 
enableK3Pin
print_ok
sleep 0.5
# get data from adc
echo -n "  * Get data from ADC "
ADC_PARAM="-d B"
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

OSC_CH1_G_1_DC=$(awk -v GAIN1_LV=$GAIN1_LV_DC -v X=$OSC_CH1_G_1_DC 'BEGIN { print sprintf("%d", int((X*GAIN1_LV))) }')
OSC_CH2_G_1_DC=$(awk -v GAIN2_LV=$GAIN2_LV_DC -v X=$OSC_CH2_G_1_DC 'BEGIN { print sprintf("%d", int((X*GAIN2_LV))) }')

# Print out the measurements
echo
echo "      NEW IN1 (1:1) gain cal param >>OSC_CH1_G_1_DC<< is $OSC_CH1_G_1_DC"
echo "      NEW IN2 (1:1) gain cal param >>OSC_CH2_G_1_DC<< is $OSC_CH2_G_1_DC"


# Check if the values are within expectations
checkValue $OSC_CH1_G_1_DC $OSC_CH2_G_1_DC $FE_FS_G_LO_MIN $FE_FS_G_LO_MAX

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
$C_ACQUIRE -d B $ADC_BUFF_SIZE $DECIMATION > /tmp/adc.txt   # WORKAROUND: First acquisition is thrown away
sleep 0.4
$C_ACQUIRE -d B $ADC_BUFF_SIZE $DECIMATION > /tmp/adc.txt
cat /tmp/adc.txt | awk '{print $1}' > /tmp/adc_a.txt
cat /tmp/adc.txt | awk '{print $2}' > /tmp/adc_b.txt

# Calculate mean value
ADC_A_MEAN=$(awk -v Y=$OSC_CH1_OFF_1_DC -v X=$GAIN1_LV_DC '{sum+=$1} END { print int( ((sum/NR)*X)-Y)}' /tmp/adc_a.txt)
ADC_B_MEAN=$(awk -v Y=$OSC_CH2_OFF_1_DC -v X=$GAIN2_LV_DC '{sum+=$1} END { print int( ((sum/NR)*X)-Y)}' /tmp/adc_b.txt)


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

echo "INPUT DC offset calibration; (1:20); IN=GND"
echo

sleep 0.5
echo -n "  * Disable generator "
# turn off generator 
disableGenerator
print_ok

sleep 0.5
echo -n "  * Connect IN to GND "
# connect in to out 
disableAllDIOPin
print_ok
sleep 0.5

# get data from adc
echo -n "  * Get data from ADC "
ADC_PARAM="-d B -1 20 -2 20"
acquireData
print_ok
# Print out the measurements
echo "      IN1 DC offset value is $ADC_A"
echo "      IN2 DC offset value is $ADC_B"

# Check if the values are within expectations
checkValue $ADC_A $ADC_B $FE_DC_offs_MIN $FE_DC_offs_MAX

OSC_CH1_OFF_20_DC=$ADC_A
OSC_CH2_OFF_20_DC=$ADC_B
N1_HV_DC=$ADC_A
N2_HV_DC=$ADC_B

# Print out the new cal parameters
echo "      NEW IN1 (1:20) DC offset cal param >>OSC_CH1_OFF_20_DC<< is $OSC_CH1_OFF_20_DC"
echo "      NEW IN2 (1:20) DC offset cal param >>OSC_CH2_OFF_20_DC<< is $OSC_CH2_OFF_20_DC"
echo



##########################################################################################
##########################################################################################
#  Calibrate DC (9V) mode (1:20)
##########################################################################################
##########################################################################################

echo "INPUT DC gain calibration; (1:20); IN=9V"


sleep 0.5
echo -n "  * Connect (9V) to OUT "
# connect in to out 
enableK4Pin
print_ok
sleep 0.5

# get data from adc
echo -n "  * Get data from ADC "
ADC_PARAM="-d B -1 20 -2 20"
acquireData
print_ok
# Print out the measurements
echo "      IN1 mean value is $ADC_A"
echo "      IN2 mean value is $ADC_B"

getHighRefValue
REF_VALUE_HV=$REF_V

GAIN1_HV_DC=$(awk -v N1_HV=$N1_HV_DC -v REF_VALUE_HV=$REF_VALUE_HV -v ADC_A=$ADC_A 'BEGIN { print ( ( REF_VALUE_HV) / ( ADC_A-N1_HV ) ) }')
GAIN2_HV_DC=$(awk -v N2_HV=$N2_HV_DC -v REF_VALUE_HV=$REF_VALUE_HV -v ADC_B=$ADC_B 'BEGIN { print ( ( REF_VALUE_HV) / ( ADC_B-N2_HV ) ) }')

# Print out the measurements
echo "      IN1_Gain is $GAIN1_HV_DC"
echo "      IN2_Gain is $GAIN2_HV_DC"

OSC_CH1_G_20_DC=$(awk -v X=$GAIN1_HV_DC -v Y=$OSC_CH1_G_20_DC 'BEGIN { print sprintf("%d", int((Y*X))) }')
OSC_CH2_G_20_DC=$(awk -v X=$GAIN2_HV_DC -v Y=$OSC_CH2_G_20_DC 'BEGIN { print sprintf("%d", int((Y*X))) }')

# Print out the measurements
echo
echo "      NEW IN1 gain cal param >>OSC_CH1_G_20_DC<< is $OSC_CH1_G_20_DC"
echo "      NEW IN2 gain cal param >>OSC_CH2_G_20_DC<< is $OSC_CH2_G_20_DC"


# Check if the values are within expectations
checkValue $OSC_CH1_G_20_DC $OSC_CH2_G_20_DC $FE_FS_G_HI_MIN $FE_FS_G_HI_MAX

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
$C_ACQUIRE -d B -1 20 -2 20 $ADC_BUFF_SIZE $DECIMATION > /tmp/adc.txt   # WORKAROUND: First acquisition is thrown away
sleep 0.4
$C_ACQUIRE -d B -1 20 -2 20 $ADC_BUFF_SIZE $DECIMATION > /tmp/adc.txt
cat /tmp/adc.txt | awk '{print $1}' > /tmp/adc_a.txt
cat /tmp/adc.txt | awk '{print $2}' > /tmp/adc_b.txt

# Calculate mean value
ADC_A_MEAN=$(awk -v Y=$OSC_CH1_OFF_20_DC -v X=$GAIN1_HV_DC '{sum+=$1} END { print int( ((sum/NR)*X)-Y)}' /tmp/adc_a.txt)
ADC_B_MEAN=$(awk -v Y=$OSC_CH2_OFF_20_DC -v X=$GAIN2_HV_DC '{sum+=$1} END { print int( ((sum/NR)*X)-Y)}' /tmp/adc_b.txt)

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
FACTORY_NEW_CAL="$GEN_CH1_G_1 $GEN_CH2_G_1 $GEN_CH1_OFF_1 $GEN_CH2_OFF_1 $GEN_CH1_G_5 $GEN_CH2_G_5 $GEN_CH1_OFF_5 $GEN_CH2_OFF_5 $OSC_CH1_G_1_AC $OSC_CH2_G_1_AC $OSC_CH1_OFF_1_AC $OSC_CH2_OFF_1_AC $OSC_CH1_G_1_DC $OSC_CH2_G_1_DC $OSC_CH1_OFF_1_DC $OSC_CH2_OFF_1_DC $OSC_CH1_G_20_AC $OSC_CH2_G_20_AC $OSC_CH1_OFF_20_AC $OSC_CH2_OFF_20_AC $OSC_CH1_G_20_DC $OSC_CH2_G_20_DC $OSC_CH1_OFF_20_DC $OSC_CH2_OFF_20_DC"
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
enableK1Pin
print_ok
sleep 0.5

# get data from adc
echo -n "  * Get data from ADC "
sleep 0.5
ADC_PARAM="-d B"
acquireData
print_ok


echo "      IN1 offset value is $ADC_A"
echo "      IN2 offset value is $ADC_B"

ADC_A=$(printf %.4f $(bc -l <<< "-1 * ($ADC_A - $N1_LV_DC )  * $GAIN1_LV_DC"))
ADC_B=$(printf %.4f $(bc -l <<< "-1 * ($ADC_B - $N2_LV_DC )  * $GAIN2_LV_DC"))


# Print out the measurements
echo "      IN1 offset value is $ADC_A"
echo "      IN2 offset value is $ADC_B"

GEN_CH1_OFF_1=$(printf %.0f $(bc -l <<< "($ADC_A + $GEN_CH1_OFF_1)"))
GEN_CH2_OFF_1=$(printf %.0f $(bc -l <<< "($ADC_B + $GEN_CH2_OFF_1)"))

# Print out the measurements
echo "      NEW OUT1 DC offset cal param >>GEN_CH1_OFF_1<<  is $GEN_CH1_OFF_1"
echo "      NEW OUT2 DC offset cal param >>GEN_CH2_OFF_1<<  is $GEN_CH2_OFF_1"
echo

# Check if the values are within expectations
checkValue $GEN_CH1_OFF_1 $GEN_CH2_OFF_1 $BE_DC_offs_MIN $BE_DC_offs_MAX

##########################################################################################
#  Set ADC parameters
##########################################################################################

echo "  * Set new Generator offset calibration"
FACTORY_NEW_CAL="$GEN_CH1_G_1 $GEN_CH2_G_1 $GEN_CH1_OFF_1 $GEN_CH2_OFF_1 $GEN_CH1_G_5 $GEN_CH2_G_5 $GEN_CH1_OFF_5 $GEN_CH2_OFF_5 $OSC_CH1_G_1_AC $OSC_CH2_G_1_AC $OSC_CH1_OFF_1_AC $OSC_CH2_OFF_1_AC $OSC_CH1_G_1_DC $OSC_CH2_G_1_DC $OSC_CH1_OFF_1_DC $OSC_CH2_OFF_1_DC $OSC_CH1_G_20_AC $OSC_CH2_G_20_AC $OSC_CH1_OFF_20_AC $OSC_CH2_OFF_20_AC $OSC_CH1_G_20_DC $OSC_CH2_G_20_DC $OSC_CH1_OFF_20_DC $OSC_CH2_OFF_20_DC"
FACTORY_CAL=$FACTORY_NEW_CAL
export FACTORY_CAL
./sub_test/set_calibration.sh
echo

##########################################################################################
#  Calibrate Generator x5 on 0
##########################################################################################


echo
echo "OUTPUT DC offset calibration; Gain x5; OUT=0V "
sleep 0.5
echo -n "  * Disable generator "
# turn off generator 
disableGeneratorX5

print_ok

sleep 0.5
echo -n "  * Connect IN to OUT "
# connect in to out 
enableK1Pin
print_ok
sleep 0.5

# get data from adc
echo -n "  * Get data from ADC "
sleep 0.5
ADC_PARAM="-d B"
acquireData
print_ok


ADC_A=$(printf %.4f $(bc -l <<< "-1 * ($ADC_A - $N1_LV_DC )  * $GAIN1_LV_DC / 5.0"))
ADC_B=$(printf %.4f $(bc -l <<< "-1 * ($ADC_B - $N2_LV_DC )  * $GAIN2_LV_DC / 5.0"))


# Print out the measurements
echo "      IN1 offset value is $ADC_A"
echo "      IN2 offset value is $ADC_B"

GEN_CH1_OFF_5=$(printf %.0f $(bc -l <<< "($ADC_A + $GEN_CH1_OFF_5)"))
GEN_CH2_OFF_5=$(printf %.0f $(bc -l <<< "($ADC_B + $GEN_CH2_OFF_5)"))


# Print out the measurements
echo "      NEW OUT1 DC offset cal param >>GEN_CH1_OFF_5<<  is $GEN_CH1_OFF_5"
echo "      NEW OUT2 DC offset cal param >>GEN_CH2_OFF_5<<  is $GEN_CH2_OFF_5"
echo

# Check if the values are within expectations
checkValue $GEN_CH1_OFF_5 $GEN_CH2_OFF_5 $BE_DC_offs_MIN $BE_DC_offs_MAX

##########################################################################################
#  Set ADC parameters
##########################################################################################

echo "  * Set new Generator offset calibration"
FACTORY_NEW_CAL="$GEN_CH1_G_1 $GEN_CH2_G_1 $GEN_CH1_OFF_1 $GEN_CH2_OFF_1 $GEN_CH1_G_5 $GEN_CH2_G_5 $GEN_CH1_OFF_5 $GEN_CH2_OFF_5 $OSC_CH1_G_1_AC $OSC_CH2_G_1_AC $OSC_CH1_OFF_1_AC $OSC_CH2_OFF_1_AC $OSC_CH1_G_1_DC $OSC_CH2_G_1_DC $OSC_CH1_OFF_1_DC $OSC_CH2_OFF_1_DC $OSC_CH1_G_20_AC $OSC_CH2_G_20_AC $OSC_CH1_OFF_20_AC $OSC_CH2_OFF_20_AC $OSC_CH1_G_20_DC $OSC_CH2_G_20_DC $OSC_CH1_OFF_20_DC $OSC_CH2_OFF_20_DC"
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
enableK1Pin
print_ok

# get data from adc
echo -n "  * Get data from ADC "
ADC_PARAM="-d B"
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


GEN_CH1_G_1=$(printf %.0f $(bc -l <<< "($GAIN1_OUT  * $GEN_CH1_G_1 )")) 
GEN_CH2_G_1=$(printf %.0f $(bc -l <<< "($GAIN2_OUT  * $GEN_CH2_G_1 )")) 

# Print out the measurements
echo
echo "      NEW OUT1 gain cal param >>GEN_CH1_G_1<< is $GEN_CH1_G_1"
echo "      NEW OUT2 gain cal param >>GEN_CH2_G_1<< is $GEN_CH2_G_1"


# Check if the values are within expectations
checkValue $GEN_CH1_G_1 $GEN_CH2_G_1 $BE_FS_MIN_1 $BE_FS_MAX_1


# ##########################################################################################
# #  Set DAC parameters
# ##########################################################################################

echo "  * Set new Generator offset calibration"
FACTORY_NEW_CAL="$GEN_CH1_G_1 $GEN_CH2_G_1 $GEN_CH1_OFF_1 $GEN_CH2_OFF_1 $GEN_CH1_G_5 $GEN_CH2_G_5 $GEN_CH1_OFF_5 $GEN_CH2_OFF_5 $OSC_CH1_G_1_AC $OSC_CH2_G_1_AC $OSC_CH1_OFF_1_AC $OSC_CH2_OFF_1_AC $OSC_CH1_G_1_DC $OSC_CH2_G_1_DC $OSC_CH1_OFF_1_DC $OSC_CH2_OFF_1_DC $OSC_CH1_G_20_AC $OSC_CH2_G_20_AC $OSC_CH1_OFF_20_AC $OSC_CH2_OFF_20_AC $OSC_CH1_G_20_DC $OSC_CH2_G_20_DC $OSC_CH1_OFF_20_DC $OSC_CH2_OFF_20_DC"
FACTORY_CAL=$FACTORY_NEW_CAL
export FACTORY_CAL
./sub_test/set_calibration.sh
echo



# ##########################################################################################
# ##########################################################################################
# #  Calibrate Output (2V) x5 mode
# ##########################################################################################
# ##########################################################################################


echo
echo "OUTPUT DC gain calibration; Gain x5; OUT=1V "

sleep 0.5
echo -n "  * Start generator in DC (1V) " 
generate_with_api -d2
print_ok

sleep 0.5
echo -n "  * Connect IN to OUT "
# connect in to out 
enableK1Pin
print_ok

# get data from adc
echo -n "  * Get data from ADC "
ADC_PARAM="-d B"
acquireData
print_ok


ADC_A=$(printf %.4f $(bc -l <<< "($ADC_A  - $N1_LV_DC ) * $GAIN1_LV_DC"))
ADC_B=$(printf %.4f $(bc -l <<< "($ADC_B  - $N2_LV_DC ) * $GAIN2_LV_DC"))

echo "ADC_A is $ADC_A"
echo "ADC_B is $ADC_B"

OUT1_VOLTAGE_HI=$(printf %.4f $(bc -l <<< "($ADC_A/4096)")) 
OUT2_VOLTAGE_HI=$(printf %.4f $(bc -l <<< "($ADC_B/4096)"))
#Print out the measurements
echo "OUT1_VOLTAGE_HI is $OUT1_VOLTAGE_HI"
echo "OUT2_VOLTAGE_HI is $OUT2_VOLTAGE_HI"

echo
GAIN1_OUT=$(printf %.4f $(bc -l <<< "($ADC_A/$OUT_AMP_HI_cnt)")) 
GAIN2_OUT=$(printf %.4f $(bc -l <<< "($ADC_B/$OUT_AMP_HI_cnt)")) 

echo "GAIN1_OUT is $GAIN1_OUT"
echo "GAIN2_OUT is $GAIN2_OUT"
echo

GEN_CH1_G_5=$(printf %.0f $(bc -l <<< "($GAIN1_OUT  * $GEN_CH1_G_5 )")) 
GEN_CH2_G_5=$(printf %.0f $(bc -l <<< "($GAIN2_OUT  * $GEN_CH2_G_5 )")) 

# Print out the measurements
echo "      NEW OUT1 gain cal param >>GEN_CH1_G_5<< is $GEN_CH1_G_5"
echo "      NEW OUT2 gain cal param >>GEN_CH2_G_5<< is $GEN_CH2_G_5"
echo

# Check if the values are within expectations
checkValue $GEN_CH1_G_5 $GEN_CH2_G_5 $BE_FS_MIN_5 $BE_FS_MAX_5

##########################################################################################
#  Set DAC parameters
##########################################################################################

echo "  * Set new Generator offset calibration"
FACTORY_NEW_CAL="$GEN_CH1_G_1 $GEN_CH2_G_1 $GEN_CH1_OFF_1 $GEN_CH2_OFF_1 $GEN_CH1_G_5 $GEN_CH2_G_5 $GEN_CH1_OFF_5 $GEN_CH2_OFF_5 $OSC_CH1_G_1_AC $OSC_CH2_G_1_AC $OSC_CH1_OFF_1_AC $OSC_CH2_OFF_1_AC $OSC_CH1_G_1_DC $OSC_CH2_G_1_DC $OSC_CH1_OFF_1_DC $OSC_CH2_OFF_1_DC $OSC_CH1_G_20_AC $OSC_CH2_G_20_AC $OSC_CH1_OFF_20_AC $OSC_CH2_OFF_20_AC $OSC_CH1_G_20_DC $OSC_CH2_G_20_DC $OSC_CH1_OFF_20_DC $OSC_CH2_OFF_20_DC"
FACTORY_CAL=$FACTORY_NEW_CAL
export FACTORY_CAL
./sub_test/set_calibration.sh
echo



##########################################################################################
##########################################################################################
#  Calibrate AC mode (1:1)
##########################################################################################
##########################################################################################
echo "##########################################################################################"
echo "Calibrate ADC in AC mode"
echo "##########################################################################################"
echo
echo "INPUT AC offset calibration; (1:1); IN=0.45V"
echo

sleep 0.5
echo -n "  * Enable generator; Signal=Sin; Amp=0.45V "
generate_with_api -s1
print_ok

sleep 0.5
echo -n "  * Connect IN to OUT "
# connect in to out 
enableK1Pin
print_ok
sleep 0.5

# get data from adc
echo -n "  * Get data from ADC "
ADC_PARAM=""
acquireData_AC
print_ok
# Print out the measurements
echo "      IN1 value is $CH1_MIN - $CH1_MAX"
echo "      IN2 value is $CH2_MIN - $CH2_MAX"

ADC_A=$(printf %f $(bc -l <<< "($CH1_MAX + $CH1_MIN) * 0.5"))
ADC_B=$(printf %f $(bc -l <<< "($CH2_MAX + $CH2_MIN) * 0.5"))

echo "      IN1 AC offset value is $ADC_A"
echo "      IN2 AC offset value is $ADC_B"

checkValue $ADC_A $ADC_B $FE_AC_offs_MIN $FE_AC_offs_MAX

OSC_CH1_OFF_1_AC=$ADC_A
OSC_CH2_OFF_1_AC=$ADC_B
N1_LV_AC=$ADC_A
N2_LV_AC=$ADC_B


##########################################################################################
##########################################################################################
#  Calibrate AC (0.45V) mode (1:1)
##########################################################################################
##########################################################################################

echo "INPUT AC gain calibration; (1:1); IN=0.45V"

sleep 0.5
echo -n "  * Enable generator; Signal=Sin; Amp=0.45V "
generate_with_api -s1
print_ok

sleep 0.5
echo -n "  * Connect IN to OUT "
# connect in to out 
enableK1Pin
print_ok
sleep 0.5

# get data from adc
echo -n "  * Get data from ADC "
ADC_PARAM=""
acquireData_AC
print_ok

echo "      IN1 value is $CH1_MIN - $CH1_MAX"
echo "      IN2 value is $CH2_MIN - $CH2_MAX"

ADC_A=$CH1_MAX
ADC_B=$CH2_MAX

# Print out the measurements
echo "      IN1 mean value is $ADC_A"
echo "      IN2 mean value is $ADC_B"

REF_VALUE_LV=$OUT_AMP_LO_cnt

GAIN1_LV_AC=$(awk -v X=$N1_LV_AC -v REF_VALUE_LV=$REF_VALUE_LV -v ADC_A=$ADC_A 'BEGIN { print ( ( REF_VALUE_LV) / ( ADC_A-X ) ) }')
GAIN2_LV_AC=$(awk -v X=$N2_LV_AC -v REF_VALUE_LV=$REF_VALUE_LV -v ADC_B=$ADC_B 'BEGIN { print ( ( REF_VALUE_LV) / ( ADC_B-X ) ) }')

# Print out the measurements
echo "      IN1 Gain is $GAIN1_LV_AC"
echo "      IN2 Gain is $GAIN2_LV_AC"

OSC_CH1_G_1_AC=$(awk -v GAIN1_LV=$GAIN1_LV_AC -v X=$OSC_CH1_G_1_AC 'BEGIN { print sprintf("%d", int((X*GAIN1_LV))) }')
OSC_CH2_G_1_AC=$(awk -v GAIN2_LV=$GAIN2_LV_AC -v X=$OSC_CH2_G_1_AC 'BEGIN { print sprintf("%d", int((X*GAIN2_LV))) }')

# Print out the measurements
echo "      NEW IN1 1:1 AC gain cal param >>OSC_CH1_G_1_AC<< is $OSC_CH1_G_1_AC"
echo "      NEW IN2 1:1 AC gain cal param >>OSC_CH2_G_1_AC<< is $OSC_CH2_G_1_AC"

# Check if the values are within expectations
checkValue $OSC_CH1_G_1_AC $OSC_CH2_G_1_AC $FE_FS_G_LO_MIN $FE_FS_G_LO_MAX


##########################################################################################
##########################################################################################
#  Recalculaed AC offset by Gain mode (1:1)
##########################################################################################
##########################################################################################

# OSC_CH1_OFF_1_AC=$(bc -l <<< "$N1_LV_AC * $GAIN1_LV_AC")
# OSC_CH2_OFF_1_AC=$(bc -l <<< "$N2_LV_AC * $GAIN2_LV_AC")
# # round value
# OSC_CH1_OFF_1_AC=${OSC_CH1_OFF_1_AC%.*}
# OSC_CH2_OFF_1_AC=${OSC_CH2_OFF_1_AC%.*}
# # Print out the new cal parameters
# echo "      NEW IN1 1:1 AC offset cal param >>OSC_CH1_OFF_1_AC<< is $OSC_CH1_OFF_1_AC"
# echo "      NEW IN2 1:1 AC offset cal param >>OSC_CH2_OFF_1_AC<< is $OSC_CH2_OFF_1_AC"
# echo


##########################################################################################
##########################################################################################
#  Calibrate AC mode (1:20)
##########################################################################################
##########################################################################################

echo "INPUT AC offset calibration; (1:20); IN=4V"
echo

sleep 0.5
echo -n "  * Enable generator; Signal=Sin; Amp=4V "
# turn off generator 
generate_with_api -s2
print_ok

sleep 0.5
echo -n "  * Connect IN to OUT "
# connect in to out 
enableK1Pin
print_ok
sleep 0.5

# get data from adc
echo -n "  * Get data from ADC "
ADC_PARAM="-1 20 -2 20"
acquireData_AC
print_ok
# Print out the measurements
echo "      IN1 value is $CH1_MIN - $CH1_MAX"
echo "      IN2 value is $CH2_MIN - $CH2_MAX"

ADC_A=$(printf %.0f $(bc -l <<< "scale=0; ($CH1_MAX + $CH1_MIN) * 0.5"))
ADC_B=$(printf %.0f $(bc -l <<< "scale=0; ($CH2_MAX + $CH2_MIN) * 0.5"))

echo "      IN1 DC offset value is $ADC_A"
echo "      IN2 DC offset value is $ADC_B"

checkValue $ADC_A $ADC_B $FE_AC_offs_MIN $FE_AC_offs_MAX

OSC_CH1_OFF_20_AC=$(( ADC_A ))
OSC_CH2_OFF_20_AC=$(( ADC_B ))
N1_HV_AC=$ADC_A
N2_HV_AC=$ADC_B

# Print out the new cal parameters
echo "      NEW IN1 (1:20) AC offset cal param >>OSC_CH1_OFF_20_AC<< is $OSC_CH1_OFF_20_AC"
echo "      NEW IN2 (1:20) AC offset cal param >>OSC_CH2_OFF_20_AC<< is $OSC_CH2_OFF_20_AC"
echo

##########################################################################################
##########################################################################################
#  Calibrate AC (4V) mode (1:20)
##########################################################################################
##########################################################################################


echo "INPUT AC gain calibration; (1:20); IN=4V"


sleep 0.5
echo -n "  * Connect IN to OUT "
# connect in to out 
enableK1Pin
print_ok
sleep 0.5

# get data from adc
echo -n "  * Get data from ADC "
ADC_PARAM="-1 20 -2 20"
acquireData_AC
print_ok


echo "      IN1 value is $CH1_MIN - $CH1_MAX"
echo "      IN2 value is $CH2_MIN - $CH2_MAX"

ADC_A=$CH1_MAX
ADC_B=$CH2_MAX

# Print out the measurements
echo "      IN1 mean value is $ADC_A"
echo "      IN2 mean value is $ADC_B"

REF_VALUE_HV=$OUT_AMP_HI_AC_cnt

GAIN1_HV_AC=$(awk -v X=$N1_HV_AC -v Y=$REF_VALUE_HV -v ADC_A=$ADC_A 'BEGIN { print (Y / (ADC_A - X )) }')
GAIN2_HV_AC=$(awk -v X=$N2_HV_AC -v Y=$REF_VALUE_HV -v ADC_B=$ADC_B 'BEGIN { print (Y / (ADC_B - X )) }')

# Print out the measurements
echo "      IN1 Gain is $GAIN1_HV_AC"
echo "      IN2 Gain is $GAIN2_HV_AC"

OSC_CH1_G_20_AC=$(awk -v Y=$GAIN1_HV_AC -v X=$OSC_CH1_G_20_AC 'BEGIN { print sprintf("%d", int(( X * Y ))) }')
OSC_CH2_G_20_AC=$(awk -v Y=$GAIN2_HV_AC -v X=$OSC_CH2_G_20_AC 'BEGIN { print sprintf("%d", int(( X * Y ))) }')

# Print out the measurements
echo "      NEW IN1 1:20 AC gain cal param >>OSC_CH1_G_20_AC<< is $OSC_CH1_G_20_AC"
echo "      NEW IN2 1:20 AC gain cal param >>OSC_CH2_G_20_AC<< is $OSC_CH2_G_20_AC"


# Check if the values are within expectations
checkValue $OSC_CH1_G_20_AC $OSC_CH2_G_20_AC $FE_FS_G_HI_MIN $FE_FS_G_HI_MAX

##########################################################################################
##########################################################################################
#  Recalculaed AC offset by Gain mode (1:1)
##########################################################################################
##########################################################################################

# OSC_CH1_OFF_20_AC=$(bc -l <<< "$N1_HV_AC * $GAIN1_HV_AC")
# OSC_CH2_OFF_20_AC=$(bc -l <<< "$N2_HV_AC * $GAIN2_HV_AC")
# # round value
# OSC_CH1_OFF_20_AC=${OSC_CH1_OFF_20_AC%.*}
# OSC_CH2_OFF_20_AC=${OSC_CH2_OFF_20_AC%.*}
# # Print out the new cal parameters
# echo "      NEW IN1 1:1  AC offset cal param >>OSC_CH1_OFF_20_AC<< is $OSC_CH1_OFF_20_AC"
# echo "      NEW IN2 1:1  AC offset cal param >>OSC_CH2_OFF_20_AC<< is $OSC_CH2_OFF_20_AC"
# echo


##########################################################################################
#  Set ADC parameters
##########################################################################################

echo "  * Set new ADC calibration"
FACTORY_NEW_CAL="$GEN_CH1_G_1 $GEN_CH2_G_1 $GEN_CH1_OFF_1 $GEN_CH2_OFF_1 $GEN_CH1_G_5 $GEN_CH2_G_5 $GEN_CH1_OFF_5 $GEN_CH2_OFF_5 $OSC_CH1_G_1_AC $OSC_CH2_G_1_AC $OSC_CH1_OFF_1_AC $OSC_CH2_OFF_1_AC $OSC_CH1_G_1_DC $OSC_CH2_G_1_DC $OSC_CH1_OFF_1_DC $OSC_CH2_OFF_1_DC $OSC_CH1_G_20_AC $OSC_CH2_G_20_AC $OSC_CH1_OFF_20_AC $OSC_CH2_OFF_20_AC $OSC_CH1_G_20_DC $OSC_CH2_G_20_DC $OSC_CH1_OFF_20_DC $OSC_CH2_OFF_20_DC"
FACTORY_CAL=$FACTORY_NEW_CAL
export FACTORY_CAL
./sub_test/set_calibration.sh
./sub_test/set_calibration_fw.sh
echo

# set light of calibration complited
#recover light on RP
$C_MONITOR 0x40000030 w $LIGHT_STATUS
RPLight3

# Calibrate capacitors
./sub_test/calibration_capacitors.sh

if [[ $STATUS == 0 ]]
then
    SetBitState 0x2000
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
