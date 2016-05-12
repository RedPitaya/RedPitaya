#!/bin/bash

# path to IIO device
XADC_PATH=/sys/bus/iio/devices/iio:device1/

################################################################################
# temperature
################################################################################

OFF=`cat $XADC_PATH/in_temp0_offset`
RAW=`cat $XADC_PATH/in_temp0_raw`
SCL=`cat $XADC_PATH/in_temp0_scale`

FORMULA="(($OFF+$RAW)*$SCL)/1000.0"
VAL=`echo "scale=2;${FORMULA}" | bc`
echo "in_temp0 = ${VAL} °C"

################################################################################
# power supply voltages (predefined scaling)
################################################################################

voltages=( "in_voltage0_vccint" \
           "in_voltage1_vccaux" \
           "in_voltage2_vccbram" \
           "in_voltage3_vccpint" \
           "in_voltage4_vccpaux" \
           "in_voltage5_vccoddr" \
           "in_voltage6_vrefp" \
           "in_voltage7_vrefn" )

for voltage in "${voltages[@]}"
do
  RAW=`cat ${XADC_PATH}/${voltage}_raw`
  SCL=`cat ${XADC_PATH}/${voltage}_scale`
   
  FORMULA="(${RAW}*${SCL})/1000.0"
  VAL=`echo "scale=2;${FORMULA}" | bc`
  echo "${voltage} = ${VAL} V"
done

################################################################################
# input pin voltages (custom scaling)
# for in_voltage8          full range is 12.2  V
# for in_voltage9/10/11/12 full range is  7.01 V
################################################################################

voltages=( "in_voltage8_vpvn" )

for voltage in "${voltages[@]}"
do
  RAW=`cat ${XADC_PATH}/${voltage}_raw`
   
  FORMULA="(${RAW}*12.2)/4095"
  VAL=`echo "scale=2;${FORMULA}" | bc`
  echo "${voltage} = ${VAL} V"
done


voltages=( "in_voltage9_vaux0" \
           "in_voltage10_vaux1" \
           "in_voltage11_vaux8" \
           "in_voltage12_vaux9" )

for voltage in "${voltages[@]}"
do
  RAW=`cat ${XADC_PATH}/${voltage}_raw`
   
  FORMULA="(${RAW}*7.01)/4095"
  VAL=`echo "scale=2;${FORMULA}" | bc`
  echo "${voltage} = ${VAL} V"
done
