#!/bin/bash
source ./sub_test/common_func.sh
source ./sub_test/default_calibration_values.sh

function acquireData(){
    #Acquire data with $DECIMATION decimation factor
    $C_ACQUIRE -d B $ADC_PARAM $ADC_BUFF_SIZE $DECIMATION > /tmp/adc.txt   # WORKAROUND: First acquisition is thrown away
    sleep 0.4
    $C_ACQUIRE -d B $ADC_PARAM $ADC_BUFF_SIZE $DECIMATION > /tmp/adc.txt
    cat /tmp/adc.txt | awk '{print $1}' > /tmp/adc_a.txt
    cat /tmp/adc.txt | awk '{print $2}' > /tmp/adc_b.txt

    # Calculate mean value
    ADC_A=$(awk '{sum+=$1} END { print int(sum/NR)}' /tmp/adc_a.txt)
    ADC_B=$(awk '{sum+=$1} END { print int(sum/NR)}' /tmp/adc_b.txt)
}



getDefCalibValues
# MAX/MIN calibration parameters
FE_FS_G_HI_MAX=$(printf %.$2f $(bc -l <<< "scale=0; $FE_CH1_FS_G_HI * 1.3")) # default FE_CH1_FS_G_HI + 30 %
FE_FS_G_LO_MAX=$(printf %.$2f $(bc -l <<< "scale=0; $FE_CH1_FS_G_LO * 1.3")) # default FE_CH1_FS_G_LO + 30 %
FE_DC_offs_MAX=300
BE_FS_MAX=$(printf %.$2f $(bc -l <<< "scale=0; $BE_CH1_FS * 1.3"))
BE_DC_offs_MAX=300
FE_DC_offs_HI_MAX=400

FE_FS_G_HI_MIN=$(printf %.$2f $(bc -l <<< "scale=0; $FE_CH1_FS_G_HI * 0.7")) # default FE_CH1_FS_G_HI - 30 %
FE_FS_G_LO_MIN=$(printf %.$2f $(bc -l <<< "scale=0; $FE_CH1_FS_G_LO * 0.7")) # default FE_CH1_FS_G_LO - 30 %
FE_DC_offs_MIN=-300
BE_FS_MIN=$(printf %.$2f $(bc -l <<< "scale=0; $BE_CH1_FS * 0.7"))
BE_DC_offs_MIN=-300
FE_DC_offs_HI_MIN=-400


DECIMATION=1024
ADC_BUFF_SIZE=16384

echo
echo -e "\e[94m########################################################################\e[0m"
echo -e "\e[94m#            Fast ADCs and DACs CALIBRATION                            #\e[0m"
echo -e "\e[94m########################################################################\e[0m"
echo

STATUS=0
CALIBRATION_STATUS=0


##########################################################################################
##########################################################################################
#  Calibrate LV/DC mode
##########################################################################################
##########################################################################################

echo "  * Set default calibration"
export FACTORY_CAL
./sub_test/set_calibration.sh
echo

echo "Calibrate in LV/DC mode"
echo

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

# get data from adc
echo -n "  * Get data from ADC "
ADC_PARAM=""
acquireData
print_ok
# Print out the measurements
echo "      IN1 LV DC offset value is $ADC_A"
echo "      IN2 LV DC offset value is $ADC_B"

# Check if the values are within expectations
if [[ $ADC_A -gt $FE_DC_offs_MAX ]] || [[ $ADC_A -lt $FE_DC_offs_MIN ]]
then
    echo "      Measured IN1 LV DC offset value ($ADC_A) is outside expected range ($FE_DC_offs_MIN - $FE_DC_offs_MAX)"
    STATUS=1
    CALIBRATION_STATUS=1
fi

if [[ $ADC_B -gt $FE_DC_offs_MAX ]] || [[ $ADC_B -lt $FE_DC_offs_MIN ]]
then
    echo "      Measured IN2 LV DC offset value ($ADC_B) is outside expected range ($FE_DC_offs_MIN - $FE_DC_offs_MAX)"
    STATUS=1
    CALIBRATION_STATUS=1
fi

FE_CH1_DC_offs=$(( ADC_A ))
FE_CH2_DC_offs=$(( ADC_B ))
N1_LV=$ADC_A
N2_LV=$ADC_B

# Print out the new cal parameters
echo "      NEW IN1 LV DC offset cal param >>FE_CH1_DC_offs<< is $FE_CH1_DC_offs"
echo "      NEW IN2 LV DC offset cal param >>FE_CH2_DC_offs<< is $FE_CH2_DC_offs"
echo


##########################################################################################
##########################################################################################
#  Calibrate LV/DC (0.45V) mode
##########################################################################################
##########################################################################################

echo "Calibrate in LV/DC (0.45V) mode"


sleep 0.5
echo -n "  * Connect (0.45V) to OUT "
# connect in to out 
enableK3Pin
print_ok

# get data from adc
echo -n "  * Get data from ADC "
ADC_PARAM=""
acquireData
print_ok
# Print out the measurements
echo "      IN1 mean value is $ADC_A"
echo "      IN2 mean value is $ADC_B"

getLowRefValue
REF_VALUE_LV=$REF_V

GAIN1_LV=$(awk -v N1_LV=$N1_LV -v REF_VALUE_LV=$REF_VALUE_LV -v ADC_A=$ADC_A 'BEGIN { print ( ( REF_VALUE_LV) / ( ADC_A-N1_LV ) ) }')
GAIN2_LV=$(awk -v N2_LV=$N2_LV -v REF_VALUE_LV=$REF_VALUE_LV -v ADC_B=$ADC_B 'BEGIN { print ( ( REF_VALUE_LV) / ( ADC_B-N2_LV ) ) }')

# Print out the measurements
echo "      IN1_LV_Gain is $GAIN1_LV"
echo "      IN2_LV_Gain is $GAIN2_LV"

FE_CH1_FS_G_LO=$(awk -v GAIN1_LV=$GAIN1_LV -v FE_CH1_FS_G_LO=$FE_CH1_FS_G_LO 'BEGIN { print sprintf("%d", int((FE_CH1_FS_G_LO*GAIN1_LV))) }')
FE_CH2_FS_G_LO=$(awk -v GAIN2_LV=$GAIN2_LV -v FE_CH2_FS_G_LO=$FE_CH2_FS_G_LO 'BEGIN { print sprintf("%d", int((FE_CH2_FS_G_LO*GAIN2_LV))) }')

# Print out the measurements
echo "      NEW IN1 LV gain cal param >>FE_CH1_FS_G_LO<< is $FE_CH1_FS_G_LO"
echo "      NEW IN2 LV gain cal param >>FE_CH2_FS_G_LO<< is $FE_CH2_FS_G_LO"
echo

# Check if the values are within expectations
if [[ $FE_CH1_FS_G_LO -gt $FE_FS_G_LO_MAX ]] || [[ $FE_CH1_FS_G_LO -lt $FE_FS_G_LO_MIN ]]
then
    echo "      Measured IN1 LV gain ($FE_CH1_FS_G_LO) is outside expected range ( $FE_FS_G_LO_MIN - $FE_FS_G_LO_MAX )"
    STATUS=1
    CALIBRATION_STATUS=1
fi

if [[ $FE_CH2_FS_G_LO -gt $FE_FS_G_LO_MAX ]] || [[ $FE_CH2_FS_G_LO -lt $FE_FS_G_LO_MIN ]]
then
    echo "      Measured IN2 LV gain ($FE_CH2_FS_G_LO) is outside expected range ( $FE_FS_G_LO_MIN - $FE_FS_G_LO_MAX )"
    STATUS=1
    CALIBRATION_STATUS=1
fi


##########################################################################################
##########################################################################################
#  Calibrate HV/DC mode
##########################################################################################
##########################################################################################

echo "Calibrate in HV/DC mode"
echo

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

# get data from adc
echo -n "  * Get data from ADC "
ADC_PARAM="-1 20 -2 20"
acquireData
print_ok
# Print out the measurements
echo "      IN1 HV DC offset value is $ADC_A"
echo "      IN2 HV DC offset value is $ADC_B"

# Check if the values are within expectations
if [[ $ADC_A -gt $FE_DC_offs_MAX ]] || [[ $ADC_A -lt $FE_DC_offs_MIN ]]
then
    echo "      Measured IN1 HV DC offset value ($ADC_A) is outside expected range ($FE_DC_offs_MIN - $FE_DC_offs_MAX)"
    STATUS=1
    CALIBRATION_STATUS=1
fi

if [[ $ADC_B -gt $FE_DC_offs_MAX ]] || [[ $ADC_B -lt $FE_DC_offs_MIN ]]
then
    echo "      Measured IN2 HV DC offset value ($ADC_B) is outside expected range ($FE_DC_offs_MIN - $FE_DC_offs_MAX)"
    STATUS=1
    CALIBRATION_STATUS=1
fi

FE_CH1_DC_offs_HI=$(( ADC_A ))
FE_CH2_DC_offs_HI=$(( ADC_B ))
N1_HV=$ADC_A
N2_HV=$ADC_B

# Print out the new cal parameters
echo "      NEW IN1 HV DC offset cal param >>FE_CH1_DC_offs_HI<< is $FE_CH1_DC_offs_HI"
echo "      NEW IN2 HV DC offset cal param >>FE_CH2_DC_offs_HI<< is $FE_CH2_DC_offs_HI"
echo


##########################################################################################
##########################################################################################
#  Calibrate HV/DC (9V) mode
##########################################################################################
##########################################################################################

echo "Calibrate in HV/DC (9V) mode"


sleep 0.5
echo -n "  * Connect (9V) to OUT "
# connect in to out 
enableK4Pin
print_ok

# get data from adc
echo -n "  * Get data from ADC "
ADC_PARAM="-1 20 -2 20"
acquireData
print_ok
# Print out the measurements
echo "      IN1 mean value is $ADC_A"
echo "      IN2 mean value is $ADC_B"

getHighRefValue
REF_VALUE_HV=$REF_V

GAIN1_HV=$(awk -v N1_HV=$N1_HV -v REF_VALUE_HV=$REF_VALUE_HV -v ADC_A=$ADC_A 'BEGIN { print ( ( REF_VALUE_HV) / ( ADC_A-N1_HV ) ) }')
GAIN2_HV=$(awk -v N2_HV=$N2_HV -v REF_VALUE_HV=$REF_VALUE_HV -v ADC_B=$ADC_B 'BEGIN { print ( ( REF_VALUE_HV) / ( ADC_B-N2_HV ) ) }')

# Print out the measurements
echo "      IN1_HV_Gain is $GAIN1_HV"
echo "      IN2_HV_Gain is $GAIN2_HV"

FE_CH1_FS_G_HI=$(awk -v GAIN1_LV=$GAIN1_LV -v FE_CH1_FS_G_HI=$FE_CH1_FS_G_HI 'BEGIN { print sprintf("%d", int((FE_CH1_FS_G_HI*GAIN1_LV))) }')
FE_CH2_FS_G_HI=$(awk -v GAIN2_LV=$GAIN2_LV -v FE_CH2_FS_G_HI=$FE_CH2_FS_G_HI 'BEGIN { print sprintf("%d", int((FE_CH2_FS_G_HI*GAIN2_LV))) }')

# Print out the measurements
echo "      NEW IN1 HV gain cal param >>FE_CH1_FS_G_HI<< is $FE_CH1_FS_G_HI"
echo "      NEW IN2 HV gain cal param >>FE_CH2_FS_G_HI<< is $FE_CH2_FS_G_HI"
echo

# Check if the values are within expectations
if [[ $FE_CH1_FS_G_HI -gt $FE_FS_G_HI_MAX ]] || [[ $FE_CH1_FS_G_HI -lt $FE_FS_G_HI_MIN ]]
then
    echo "      Measured IN1 HV gain ($FE_CH1_FS_G_HI) is outside expected range ( $FE_FS_G_HI_MIN - $FE_FS_G_LO_MAX )"
    STATUS=1
    CALIBRATION_STATUS=1
fi

if [[ $FE_CH2_FS_G_HI -gt $FE_FS_G_HI_MAX ]] || [[ $FE_CH2_FS_G_HI -lt $FE_FS_G_HI_MIN ]]
then
    echo "      Measured IN2 HV gain ($FE_CH2_FS_G_HI) is outside expected range ( $FE_FS_G_HI_MIN - $FE_FS_G_HI_MAX )"
    STATUS=1
    CALIBRATION_STATUS=1
fi


##########################################################################################
#  Set ADC parameters
##########################################################################################

echo "  * Set new ADC calibration"
FACTORY_NEW_CAL="$FE_CH1_FS_G_HI $FE_CH2_FS_G_HI $FE_CH1_FS_G_LO $FE_CH2_FS_G_LO $FE_CH1_DC_offs $FE_CH2_DC_offs $BE_CH1_FS $BE_CH2_FS $BE_CH1_DC_offs $BE_CH2_DC_offs $Magic $FE_CH1_DC_offs_HI $FE_CH2_DC_offs_HI"
FACTORY_CAL=$FACTORY_NEW_CAL
export FACTORY_CAL
./sub_test/set_calibration.sh
echo

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
ADC_A_MEAN=$(awk -v N1_HV=$N1_HV -v GAIN1_HV=$GAIN1_HV '{sum+=$1} END { print int( ((sum/NR)*GAIN1_HV)-N1_HV)}' /tmp/adc_a.txt)
ADC_B_MEAN=$(awk -v N2_HV=$N2_HV -v GAIN2_HV=$GAIN2_HV '{sum+=$1} END { print int( ((sum/NR)*GAIN2_HV)-N2_HV)}' /tmp/adc_b.txt)

IN1_ERROR_HV=$(awk -v ADC_A_MEAN=$ADC_A_MEAN -v REF_VALUE_HV=$REF_VALUE_HV 'BEGIN { print (((ADC_A_MEAN-REF_VALUE_HV)/REF_VALUE_HV)*100) }')
IN2_ERROR_HV=$(awk -v ADC_B_MEAN=$ADC_B_MEAN -v REF_VALUE_HV=$REF_VALUE_HV 'BEGIN { print (((ADC_B_MEAN-REF_VALUE_HV)/REF_VALUE_HV)*100) }')

# Print out the measurements
echo
echo "      IN1 HV Error after the claibration is $IN1_ERROR_HV %"
echo "      IN2 HV Error after the claibration is $IN2_ERROR_HV %"
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

# get data from adc
echo -n "  * Get data from ADC "
ADC_PARAM=""
acquireData
print_ok

ADC_A=$(printf %.$2f $(bc -l <<< "$ADC_A-($N1_LV)"))
ADC_B=$(printf %.$2f $(bc -l <<< "$ADC_B-($N2_LV)"))

# Print out the measurements
echo "      IN1 offset value is $ADC_A"
echo "      IN2 offset value is $ADC_B"

OUT1_DC_offs=$(awk -v ADC_A_MEAN=$ADC_A -v BE_CH1_DC_offs=$BE_CH1_DC_offs 'BEGIN { print sprintf("%d", int(BE_CH1_DC_offs-ADC_A_MEAN))}')
OUT2_DC_offs=$(awk -v ADC_B_MEAN=$ADC_A -v BE_CH2_DC_offs=$BE_CH2_DC_offs 'BEGIN { print sprintf("%d", int(BE_CH2_DC_offs-ADC_B_MEAN))}')

BE_CH1_DC_offs=$OUT1_DC_offs
BE_CH2_DC_offs=$OUT2_DC_offs
# Print out the measurements
echo "      NEW OUT1 DC offset cal param >>BE_CH1_DC_offs<<  is $BE_CH1_DC_offs"
echo "      NEW OUT2 DC offset cal param >>BE_CH2_DC_offs<<  is $BE_CH2_DC_offs"
echo

# Check if the values are within expectations
if [[ $BE_CH1_DC_offs -gt $BE_DC_offs_MAX ]] || [[ $BE_CH1_DC_offs -lt $BE_DC_offs_MIN ]]
then
    echo "      OUT1 DC offset calibration parameter ($OUT1_DC_offs) is outside expected range ($BE_DC_offs_MIN-$BE_DC_offs_MAX)"
    STATUS=1
    CALIBRATION_STATUS=1
fi

if [[ $BE_CH2_DC_offs -gt $BE_DC_offs_MAX ]] || [[ $BE_CH2_DC_offs -lt $BE_DC_offs_MIN ]]
then
    echo "      OUT2 DC offset calibration parameter ($OUT2_DC_offs) is outside expected range ($BE_DC_offs_MIN-$BE_DC_offs_MAX)"
    STATUS=1
    CALIBRATION_STATUS=1
fi
