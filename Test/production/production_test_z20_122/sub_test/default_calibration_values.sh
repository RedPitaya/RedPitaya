#!/bin/bash

function getDefCalibValues(){
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
}