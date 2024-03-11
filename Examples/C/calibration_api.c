/* @brief This is a simple application for testing calibration api
* (c) Red Pitaya  http://www.redpitaya.com
*
* This part of code is written in C programming language.
* Please visit http://en.wikipedia.org/wiki/C_(programming_language)
* for more details on the language used herein.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "rp_hw-calib.h"


int main(int argc, char *argv[]){

    // Read calibrations from EEPROM to RAM
    int res = rp_CalibInit();
    printf("Init result: %d\n",res);

    // Backup current calibrations
    rp_calib_params_t current_calib = rp_GetCalibrationSettings();

    // Retrieves calibration coefficients
    double gain;
    int32_t offset;
    res = rp_CalibGetFastADCCalibValue(RP_CH_1_CALIB,RP_DC_CALIB,&gain,&offset);
    printf("Calibration factors for channel 1 GAIN %f OFFSET %d result = %d\n",gain,offset,res);
    res = rp_CalibGetFastADCCalibValue(RP_CH_2_CALIB,RP_DC_CALIB,&gain,&offset);
    printf("Calibration factors for channel 2 GAIN %f OFFSET %d result = %d\n",gain,offset,res);

    printf("\nCurrent calibration in user space\n");
    rp_CalibPrint(&current_calib);

    // Returns the default calibration for the current board version. It may be different on different boards.
    rp_calib_params_t def_calib = rp_GetDefaultCalibrationSettings();
    printf("\nDefailt calibration\n");
    rp_CalibPrint(&def_calib);

    // Write default calibration in EEPROM user space.
    // The previously loaded in RAM calibration will not change.
    rp_CalibrationWriteParams(def_calib,false);

    // Sets calibration parameters into RAM.
    rp_CalibrationSetParams(def_calib);

    res = rp_CalibGetFastADCCalibValue(RP_CH_1_CALIB,RP_DC_CALIB,&gain,&offset);
    printf("Calibration factors for channel 1 GAIN %f OFFSET %d result = %d\n",gain,offset,res);
    res = rp_CalibGetFastADCCalibValue(RP_CH_2_CALIB,RP_DC_CALIB,&gain,&offset);
    printf("Calibration factors for channel 2 GAIN %f OFFSET %d result = %d\n",gain,offset,res);

    // Resets calibration values in EEPROM
    rp_CalibrationWriteParams(current_calib,false);

    // Read calibrations from EEPROM to RAM
    res = rp_CalibInit();
    printf("\nRestored calibration\n");
    res = rp_CalibGetFastADCCalibValue(RP_CH_1_CALIB,RP_DC_CALIB,&gain,&offset);
    printf("Calibration factors for channel 1 GAIN %f OFFSET %d result = %d\n",gain,offset,res);
    res = rp_CalibGetFastADCCalibValue(RP_CH_2_CALIB,RP_DC_CALIB,&gain,&offset);
    printf("Calibration factors for channel 2 GAIN %f OFFSET %d result = %d\n",gain,offset,res);

    // Read calibration direct from eeprom
    uint8_t *data = NULL;
    uint16_t size;
    rp_calib_params_t eeprom_calib;
    rp_CalibGetEEPROM(&data,&size,false);
    // Convert bytes to rp_calib_params_t
    rp_CalibConvertEEPROM(data,size,&eeprom_calib);
    printf("\nCalibration read directly\n");
    rp_CalibPrint(&eeprom_calib);

    return 0;
}