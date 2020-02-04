#!/bin/bash
source ./sub_test/common_func.sh
source ./sub_test/default_calibration_values.sh

function acquireData(){
    #Acquire data with $DECIMATION decimation factor
    $C_ACQUIRE $ADC_PARAM $ADC_BUFF_SIZE $DECIMATION > /tmp/adc.txt   # WORKAROUND: First acquisition is thrown away
    sleep 0.4
    $C_ACQUIRE $ADC_PARAM $ADC_BUFF_SIZE $DECIMATION > /tmp/adc.txt
    cat /tmp/adc.txt | awk '{print $1}' > /tmp/adc_a.txt
    cat /tmp/adc.txt | awk '{print $2}' > /tmp/adc_b.txt

    # Calculate mean value
    ADC_A=$(awk '{sum+=$1} END { print int(sum/NR)}' /tmp/adc_a.txt)
    ADC_B=$(awk '{sum+=$1} END { print int(sum/NR)}' /tmp/adc_b.txt)
}

function acquireData_AC(){
    #Acquire data with $DECIMATION decimation factor
    $C_ACQUIRE $ADC_PARAM $ADC_BUFF_SIZE $DECIMATION > /tmp/adc.txt   # WORKAROUND: First acquisition is thrown away
    sleep 0.4
    $C_ACQUIRE $ADC_PARAM $ADC_BUFF_SIZE $DECIMATION > /tmp/adc.txt
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
    if [[ $1 -gt $4 ]] || [[ $1 -lt $3 ]]
    then
        echo "      Measured IN1 value ($1) is outside expected range ($3 - $4)"
        STATUS=1
        CALIBRATION_STATUS=1
    fi

    if [[ $2 -gt $4 ]] || [[ $2 -lt $3 ]]
    then
        echo "      Measured IN2 value ($2) is outside expected range ($3 - $4)"
        STATUS=1
        CALIBRATION_STATUS=1
    fi
}



getDefCalibValues
# MAX/MIN calibration parameters
FE_FS_G_HI_MAX=$(printf %.$2f $(bc -l <<< "scale=0; $OSC_CH1_G_20_DC * 1.3")) # default FE_CH1_FS_G_HI + 30 %
FE_FS_G_LO_MAX=$(printf %.$2f $(bc -l <<< "scale=0; $OSC_CH1_G_1_DC * 1.3")) # default FE_CH1_FS_G_LO + 30 %
FE_DC_offs_MAX=300
BE_FS_MAX=$(printf %.$2f $(bc -l <<< "scale=0; $GEN_CH1_G_1 * 1.3"))
BE_DC_offs_MAX=300
FE_DC_offs_HI_MAX=400

FE_FS_G_HI_MIN=$(printf %.$2f $(bc -l <<< "scale=0; $OSC_CH1_G_20_DC * 0.7")) # default FE_CH1_FS_G_HI - 30 %
FE_FS_G_LO_MIN=$(printf %.$2f $(bc -l <<< "scale=0; $OSC_CH1_G_1_DC * 0.7")) # default FE_CH1_FS_G_LO - 30 %
FE_DC_offs_MIN=-300
BE_FS_MIN=$(printf %.$2f $(bc -l <<< "scale=0; $GEN_CH1_G_1 * 0.7"))
BE_DC_offs_MIN=-300
FE_DC_offs_HI_MIN=-400

FE_AC_offs_MAX=300
FE_AC_offs_HI_MAX=400

FE_AC_offs_MIN=-300
FE_AC_offs_HI_MIN=-400


DECIMATION=1024
ADC_BUFF_SIZE=16384
OUT_AMP_LO_cnt=3686 # 0.45V  VOLTS VPP reference voltage
OUT_AMP_HI_cnt=1638 

echo
echo -e "\e[94m########################################################################\e[0m"
echo -e "\e[94m#            Fast ADCs and DACs CALIBRATION                            #\e[0m"
echo -e "\e[94m########################################################################\e[0m"
echo

STATUS=0
CALIBRATION_STATUS=0


echo "  * Set default calibration"
export FACTORY_CAL
./sub_test/set_calibration.sh

##########################################################################################
##########################################################################################
#  Calibrate DC mode (1:1)
##########################################################################################
##########################################################################################

echo
echo "Calibrate in LV/DC mode"
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

OSC_CH1_OFF_1_DC=$(( ADC_A ))
OSC_CH2_OFF_1_DC=$(( ADC_B ))
N1_LV_DC=$ADC_A
N2_LV_DC=$ADC_B

# Print out the new cal parameters
echo "      NEW IN1 LV DC offset cal param >>OSC_CH1_OFF_1_DC<< is $OSC_CH1_OFF_1_DC"
echo "      NEW IN2 LV DC offset cal param >>OSC_CH2_OFF_1_DC<< is $OSC_CH2_OFF_1_DC"
echo



##########################################################################################
##########################################################################################
#  Calibrate DC (0.45V) mode (1:1)
##########################################################################################
##########################################################################################

echo "Calibrate in DC (0.45V) mode (1:1)"


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
echo "      NEW IN1 LV gain cal param >>OSC_CH1_G_1_DC<< is $OSC_CH1_G_1_DC"
echo "      NEW IN2 LV gain cal param >>OSC_CH2_G_1_DC<< is $OSC_CH2_G_1_DC"
echo

# Check if the values are within expectations
checkValue $OSC_CH1_G_1_DC $OSC_CH2_G_1_DC $FE_FS_G_LO_MIN $FE_FS_G_LO_MAX

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
ADC_A_MEAN=$(awk -v Y=$N1_LV_DC -v X=$GAIN1_LV_DC '{sum+=$1} END { print int( ((sum/NR)*X)-Y)}' /tmp/adc_a.txt)
ADC_B_MEAN=$(awk -v Y=$N2_LV_DC -v X=$GAIN2_LV_DC '{sum+=$1} END { print int( ((sum/NR)*X)-Y)}' /tmp/adc_b.txt)


IN1_ERROR_LV=$(awk -v X=$ADC_A_MEAN -v Y=$REF_VALUE_LV 'BEGIN { print (((X-Y)/Y)*100) }')
IN2_ERROR_LV=$(awk -v X=$ADC_B_MEAN -v Y=$REF_VALUE_LV 'BEGIN { print (((X-Y)/Y)*100) }')

# Print out the measurements
echo
echo "      IN1 Error after the claibration is $IN1_ERROR_LV %"
echo "      IN2 Error after the claibration is $IN2_ERROR_LV %"
echo 


##########################################################################################
##########################################################################################
#  Calibrate DC mode (1:20)
##########################################################################################
##########################################################################################

echo "Calibrate in DC mode (1:20)"
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

OSC_CH1_OFF_20_DC=$(( ADC_A ))
OSC_CH2_OFF_20_DC=$(( ADC_B ))
N1_HV_DC=$ADC_A
N2_HV_DC=$ADC_B

# Print out the new cal parameters
echo "      NEW IN1 DC offset cal param >>OSC_CH1_OFF_20_DC<< is $OSC_CH1_OFF_20_DC"
echo "      NEW IN2 DC offset cal param >>OSC_CH2_OFF_20_DC<< is $OSC_CH2_OFF_20_DC"
echo



##########################################################################################
##########################################################################################
#  Calibrate DC (9V) mode (1:20)
##########################################################################################
##########################################################################################

echo "Calibrate in DC (9V) mode (1:20)"


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
echo "      NEW IN1 gain cal param >>OSC_CH1_G_20_DC<< is $OSC_CH1_G_20_DC"
echo "      NEW IN2 gain cal param >>OSC_CH2_G_20_DC<< is $OSC_CH2_G_20_DC"
echo

# Check if the values are within expectations
checkValue $OSC_CH1_G_20_DC $OSC_CH2_G_20_DC $FE_FS_G_HI_MIN $FE_FS_G_HI_MAX

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
ADC_A_MEAN=$(awk -v Y=$N1_HV_DC -v X=$GAIN1_HV_DC '{sum+=$1} END { print int( ((sum/NR)*X)-Y)}' /tmp/adc_a.txt)
ADC_B_MEAN=$(awk -v Y=$N2_HV_DC -v X=$GAIN2_HV_DC '{sum+=$1} END { print int( ((sum/NR)*X)-Y)}' /tmp/adc_b.txt)

IN1_ERROR_HV=$(awk -v X=$ADC_A_MEAN -v REF_VALUE_HV=$REF_VALUE_HV 'BEGIN { print (((X-REF_VALUE_HV)/REF_VALUE_HV)*100) }')
IN2_ERROR_HV=$(awk -v X=$ADC_B_MEAN -v REF_VALUE_HV=$REF_VALUE_HV 'BEGIN { print (((X-REF_VALUE_HV)/REF_VALUE_HV)*100) }')

# Print out the measurements
echo
echo "      IN1 Error after the claibration is $IN1_ERROR_HV %"
echo "      IN2 Error after the claibration is $IN2_ERROR_HV %"
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

echo
echo "Outputs DC offset calibration is started..."
echo
echo "Calibrate generator in x1 gain mode"
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
ADC_PARAM="-d B -r"
acquireData
print_ok

ADC_A=$(printf %.$2f $(bc -l <<< "$ADC_A - ($N1_LV_DC)"))
ADC_B=$(printf %.$2f $(bc -l <<< "$ADC_B - ($N2_LV_DC)"))

# Print out the measurements
echo "      IN1 offset value is $ADC_A"
echo "      IN2 offset value is $ADC_B"

OUT1_DC_offs=$(awk -v ADC_A_MEAN=$ADC_A -v X=$GEN_CH1_OFF_1 'BEGIN { print sprintf("%d", int(X+ADC_A_MEAN))}')
OUT2_DC_offs=$(awk -v ADC_B_MEAN=$ADC_B -v X=$GEN_CH2_OFF_1 'BEGIN { print sprintf("%d", int(X+ADC_B_MEAN))}')

GEN_CH1_OFF_1=$OUT1_DC_offs
GEN_CH2_OFF_1=$OUT2_DC_offs
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
echo "Calibrate generator in x5 gain mode"
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
ADC_PARAM="-d B -r"
acquireData
print_ok

ADC_A=$(printf %.$2f $(bc -l <<< "($ADC_A - $N1_HV_DC)/5"))
ADC_B=$(printf %.$2f $(bc -l <<< "($ADC_B - $N2_HV_DC)/5"))

# Print out the measurements
echo "      IN1 offset value is $ADC_A"
echo "      IN2 offset value is $ADC_B"

OUT1_DC_offs=$(awk -v ADC_A_MEAN=$ADC_A -v X=$GEN_CH1_OFF_5 'BEGIN { print sprintf("%d", int(X+ADC_A_MEAN))}')
OUT2_DC_offs=$(awk -v ADC_B_MEAN=$ADC_B -v X=$GEN_CH2_OFF_5 'BEGIN { print sprintf("%d", int(X+ADC_B_MEAN))}')

GEN_CH1_OFF_5=$OUT1_DC_offs
GEN_CH2_OFF_5=$OUT2_DC_offs
# Print out the measurements
echo "      NEW OUT1 DC offset cal param >>GEN_CH1_OFF_5<<  is $GEN_CH1_OFF_5"
echo "      NEW OUT2 DC offset cal param >>GEN_CH2_OFF_5<<  is $GEN_CH2_OFF_5"
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



# ##########################################################################################
# ##########################################################################################
# #  Calibrate Output (0.45V) x1 mode
# ##########################################################################################
# ##########################################################################################


echo
echo "Calibrate generator in x1 gain mode"

sleep 0.5
echo -n "  * Start generator in DC (0.45V)" 
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

echo "ADC_A is $ADC_A"
echo "ADC_B is $ADC_B"


ADC_A=$(printf %.$2f $(bc -l <<< "2 * ($ADC_A  - $N1_LV_DC) * $GAIN1_LV_DC"))
ADC_B=$(printf %.$2f $(bc -l <<< "2 * ($ADC_B  - $N2_LV_DC) * $GAIN2_LV_DC"))


OUT1_VOLTAGE_LO=$(awk -v ADC_A_MEAN=$ADC_A 'BEGIN { print ((ADC_A_MEAN/8192))}' )
OUT2_VOLTAGE_LO=$(awk -v ADC_B_MEAN=$ADC_B 'BEGIN { print ((ADC_B_MEAN/8192))}' )
#Print out the measurements
echo "OUT1_VOLTAGE_LO is $OUT1_VOLTAGE_LO"
echo "OUT2_VOLTAGE_LO is $OUT2_VOLTAGE_LO"
echo
GAIN1_OUT=$(awk -v  X=$OUT_AMP_LO_cnt -v ADC_A_MEAN=$ADC_A 'BEGIN {print ((ADC_A_MEAN)/X) }')
GAIN2_OUT=$(awk -v  X=$OUT_AMP_LO_cnt -v ADC_B_MEAN=$ADC_B 'BEGIN {print ((ADC_B_MEAN)/X) }')

echo "GAIN1_OUT is $GAIN1_OUT"
echo "GAIN2_OUT is $GAIN2_OUT"
echo


GEN_CH1_G_1=$(awk -v X=$GAIN1_OUT -v Y=$GEN_CH1_G_1 'BEGIN { print sprintf("%d", int((Y*X))) }')
GEN_CH2_G_1=$(awk -v X=$GAIN2_OUT -v Y=$GEN_CH2_G_1 'BEGIN { print sprintf("%d", int((Y*X))) }')

# Print out the measurements
echo "      NEW OUT1 gain cal param >>GEN_CH1_G_1<< is $GEN_CH1_G_1"
echo "      NEW OUT2 gain cal param >>GEN_CH2_G_1<< is $GEN_CH2_G_1"
echo

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
# #  Calibrate Output (4V) x5 mode
# ##########################################################################################
# ##########################################################################################


echo
echo "Calibrate generator in x5 gain mode"

sleep 0.5
echo -n "  * Start generator in DC (4V)" 
generate_with_api -d2
print_ok

sleep 0.5
echo -n "  * Connect IN to OUT "
# connect in to out 
enableK1Pin
print_ok

# get data from adc
echo -n "  * Get data from ADC "
ADC_PARAM="-d B -1 20 -2 20"
acquireData
print_ok
ADC_A=$(printf %.$2f $(bc -l <<< "2 * ($ADC_A * $GAIN1_HV_DC - $N1_HV_DC)"))
ADC_B=$(printf %.$2f $(bc -l <<< "2 * ($ADC_B * $GAIN2_HV_DC - $N2_HV_DC)"))
echo "ADC_A is $ADC_A"
echo "ADC_B is $ADC_B"

OUT1_VOLTAGE_HI=$(awk -v ADC_A_MEAN=$ADC_A 'BEGIN { print (20 * (ADC_A_MEAN/8192))}' )
OUT2_VOLTAGE_HI=$(awk -v ADC_B_MEAN=$ADC_B 'BEGIN { print (20 * (ADC_B_MEAN/8192))}' )
#Print out the measurements
echo "OUT1_VOLTAGE_HI is $OUT1_VOLTAGE_HI"
echo "OUT2_VOLTAGE_HI is $OUT2_VOLTAGE_HI"
echo
GAIN1_OUT=$(awk -v X=$OUT_AMP_HI_cnt -v ADC_A_MEAN=$ADC_A 'BEGIN {print ((ADC_A_MEAN)/X) }')
GAIN2_OUT=$(awk -v X=$OUT_AMP_HI_cnt -v ADC_B_MEAN=$ADC_B 'BEGIN {print ((ADC_B_MEAN)/X) }')

echo "GAIN1_OUT is $GAIN1_OUT"
echo "GAIN2_OUT is $GAIN2_OUT"
echo


GEN_CH1_G_5=$(awk -v X=$GAIN1_OUT -v Y=$GEN_CH1_G_5 'BEGIN { print sprintf("%d", int((Y*X))) }')
GEN_CH2_G_5=$(awk -v X=$GAIN2_OUT -v Y=$GEN_CH2_G_5 'BEGIN { print sprintf("%d", int((Y*X))) }')

# Print out the measurements
echo "      NEW OUT1 gain cal param >>GEN_CH1_G_5<< is $GEN_CH1_G_5"
echo "      NEW OUT2 gain cal param >>GEN_CH2_G_5<< is $GEN_CH2_G_5"
echo

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

echo
echo "Calibrate in LV/AC mode"
echo

sleep 0.5
echo -n "  * Enable generator "
# turn off generator 
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

ADC_A=$(printf %.$2f $(bc -l <<< "scale=0; ($CH1_MAX + $CH1_MIN) * 0.5"))
ADC_B=$(printf %.$2f $(bc -l <<< "scale=0; ($CH2_MAX + $CH2_MIN) * 0.5"))

echo "      IN1 DC offset value is $ADC_A"
echo "      IN2 DC offset value is $ADC_B"

checkValue $ADC_A $ADC_B $FE_AC_offs_MIN $FE_AC_offs_MAX

OSC_CH1_OFF_1_AC=$(( ADC_A ))
OSC_CH2_OFF_1_AC=$(( ADC_B ))
N1_LV_AC=$ADC_A
N2_LV_AC=$ADC_B

# Print out the new cal parameters
echo "      NEW IN1 LV DC offset cal param >>OSC_CH1_OFF_1_AC<< is $OSC_CH1_OFF_1_AC"
echo "      NEW IN2 LV DC offset cal param >>OSC_CH2_OFF_1_AC<< is $OSC_CH2_OFF_1_AC"
echo

##########################################################################################
##########################################################################################
#  Calibrate AC (0.45V) mode (1:1)
##########################################################################################
##########################################################################################

echo "Calibrate in AC (0.45V) mode (1:1)"


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
ADC_A=$(printf %.$2f $(bc -l <<< "scale=0; (2 * $ADC_A - $N1_LV_AC)"))
ADC_B=$(printf %.$2f $(bc -l <<< "scale=0; (2 * $ADC_B - $N2_LV_AC)"))

# Print out the measurements
echo "      IN1 mean value is $ADC_A"
echo "      IN2 mean value is $ADC_B"

REF_VALUE_LV=$OUT_AMP_LO_cnt

GAIN1_LV_AC=$(awk -v N1_LV=$N1_LV_AC -v REF_VALUE_LV=$REF_VALUE_LV -v ADC_A=$ADC_A 'BEGIN { print ( ( REF_VALUE_LV) / ( ADC_A-N1_LV ) ) }')
GAIN2_LV_AC=$(awk -v N2_LV=$N2_LV_AC -v REF_VALUE_LV=$REF_VALUE_LV -v ADC_B=$ADC_B 'BEGIN { print ( ( REF_VALUE_LV) / ( ADC_B-N2_LV ) ) }')

# Print out the measurements
echo "      IN1 Gain is $GAIN1_LV_AC"
echo "      IN2 Gain is $GAIN2_LV_AC"

OSC_CH1_G_1_AC=$(awk -v GAIN1_LV=$GAIN1_LV_AC -v X=$OSC_CH1_G_1_AC 'BEGIN { print sprintf("%d", int((X*GAIN1_LV))) }')
OSC_CH2_G_1_AC=$(awk -v GAIN2_LV=$GAIN2_LV_AC -v X=$OSC_CH2_G_1_AC 'BEGIN { print sprintf("%d", int((X*GAIN2_LV))) }')

# Print out the measurements
echo "      NEW IN1 LV gain cal param >>OSC_CH1_G_1_AC<< is $OSC_CH1_G_1_AC"
echo "      NEW IN2 LV gain cal param >>OSC_CH2_G_1_AC<< is $OSC_CH2_G_1_AC"
echo

# Check if the values are within expectations
checkValue $OSC_CH1_G_1_AC $OSC_CH2_G_1_AC $FE_FS_G_LO_MIN $FE_FS_G_LO_MAX

##########################################################################################
##########################################################################################
#  Calibrate AC mode (1:20)
##########################################################################################
##########################################################################################

echo "Calibrate in AC mode (1:20)"
echo

sleep 0.5
echo -n "  * Enable generator "
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

ADC_A=$(printf %.$2f $(bc -l <<< "scale=0; ($CH1_MAX + $CH1_MIN) * 0.5"))
ADC_B=$(printf %.$2f $(bc -l <<< "scale=0; ($CH2_MAX + $CH2_MIN) * 0.5"))

echo "      IN1 DC offset value is $ADC_A"
echo "      IN2 DC offset value is $ADC_B"

checkValue $ADC_A $ADC_B $FE_AC_offs_MIN $FE_AC_offs_MAX

OSC_CH1_OFF_20_AC=$(( ADC_A ))
OSC_CH2_OFF_20_AC=$(( ADC_B ))
N1_HV_AC=$ADC_A
N2_HV_AC=$ADC_B

# Print out the new cal parameters
echo "      NEW IN1 DC offset cal param >>OSC_CH1_OFF_20_AC<< is $OSC_CH1_OFF_20_AC"
echo "      NEW IN2 DC offset cal param >>OSC_CH2_OFF_20_AC<< is $OSC_CH2_OFF_20_AC"
echo

##########################################################################################
##########################################################################################
#  Calibrate AC (9V) mode (1:20)
##########################################################################################
##########################################################################################


echo "Calibrate in AC (4V) mode (1:20)"


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

ADC_A=$(printf %.$2f $(bc -l <<< "(2 * $ADC_A - $N1_HV_AC)"))
ADC_B=$(printf %.$2f $(bc -l <<< "(2 * $ADC_B - $N2_HV_AC)"))

# Print out the measurements
echo "      IN1 mean value is $ADC_A"
echo "      IN2 mean value is $ADC_B"

REF_VALUE_HV=$OUT_AMP_HI_cnt

GAIN1_HV_AC=$(awk -v X=$N1_HV_AC -v Y=$REF_VALUE_HV -v ADC_A=$ADC_A 'BEGIN { print ( ( Y) / ( ADC_A-X ) ) }')
GAIN2_HV_AC=$(awk -v X=$N2_HV_AC -v Y=$REF_VALUE_HV -v ADC_B=$ADC_B 'BEGIN { print ( ( Y) / ( ADC_B-X ) ) }')

# Print out the measurements
echo "      IN1 Gain is $GAIN1_HV_AC"
echo "      IN2 Gain is $GAIN2_HV_AC"

OSC_CH1_G_20_AC=$(awk -v GAIN1_LV=$GAIN1_HV_AC -v X=$OSC_CH1_G_20_AC 'BEGIN { print sprintf("%d", int((X*GAIN1_LV))) }')
OSC_CH2_G_20_AC=$(awk -v GAIN2_LV=$GAIN2_HV_AC -v X=$OSC_CH2_G_20_AC 'BEGIN { print sprintf("%d", int((X*GAIN2_LV))) }')

# Print out the measurements
echo "      NEW IN1 gain cal param >>OSC_CH1_G_20_AC<< is $OSC_CH1_G_20_AC"
echo "      NEW IN2 gain cal param >>OSC_CH2_G_20_AC<< is $OSC_CH2_G_20_AC"
echo

# Check if the values are within expectations
checkValue $OSC_CH1_G_20_AC $OSC_CH2_G_20_AC $FE_FS_G_HI_MIN $FE_FS_G_HI_MAX


##########################################################################################
#  Set ADC parameters
##########################################################################################

echo "  * Set new ADC calibration"
FACTORY_NEW_CAL="$GEN_CH1_G_1 $GEN_CH2_G_1 $GEN_CH1_OFF_1 $GEN_CH2_OFF_1 $GEN_CH1_G_5 $GEN_CH2_G_5 $GEN_CH1_OFF_5 $GEN_CH2_OFF_5 $OSC_CH1_G_1_AC $OSC_CH2_G_1_AC $OSC_CH1_OFF_1_AC $OSC_CH2_OFF_1_AC $OSC_CH1_G_1_DC $OSC_CH2_G_1_DC $OSC_CH1_OFF_1_DC $OSC_CH2_OFF_1_DC $OSC_CH1_G_20_AC $OSC_CH2_G_20_AC $OSC_CH1_OFF_20_AC $OSC_CH2_OFF_20_AC $OSC_CH1_G_20_DC $OSC_CH2_G_20_DC $OSC_CH1_OFF_20_DC $OSC_CH2_OFF_20_DC"
FACTORY_CAL=$FACTORY_NEW_CAL
export FACTORY_CAL
./sub_test/set_calibration.sh
echo

# Calibrate capacitors
./sub_test/calibration_capacitors.sh