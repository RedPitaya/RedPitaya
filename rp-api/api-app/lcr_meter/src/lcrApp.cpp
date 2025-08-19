/**
* $Id: $
*
* @brief Red Pitaya application Impedance Analyzer module interface
*
* @Author Luka Golinar <luka.golinar@redpitaya.com>
*
* (c) Red Pitaya  http://www.redpitaya.com
*
* This part of code is written in C programming language.
* Please visit http://en.wikipedia.org/wiki/C_(programming_language)
* for more details on the language used herein.
*/

#include <stdio.h>

#include "lcrApp.h"
#include "lcr_meter.h"
#include "utils.h"

/* Resource managment functions */
int lcrApp_lcrInit() {
    return lcr_Init();
}

int lcrApp_LcrRelease() {
    return lcr_Release();
}

int lcrApp_LcrReset() {
    return lcr_Reset();
}

int lcrApp_LcrRun() {
    return lcr_Run();
}

int lcrApp_LcrStop() {
    return lcr_Stop();
}

int lcrApp_LcrSetPause(bool pause) {
    return lcr_SetPause(pause);
}

int lcrApp_GenRun() {
    return lcr_GenRun();
}

int lcrApp_GenStop() {
    return lcr_GenStop();
}

int lcrApp_GenSetSettings() {
    return lcr_GenSetSettings();
}

int lcrApp_LcrCopyParams(lcr_main_data_t* data) {
    return lcr_CopyParams(data);
}

// int lcrApp_LcrStartCorrection(){
// 	return lcr_Correction();
// }

int lcrApp_LcrSetFrequency(float frequency) {
    return lcr_SetFrequency(frequency);
}

int lcrApp_LcrGetFrequency(float* frequency) {
    return lcr_GetFrequency(frequency);
}

int lcrApp_LcrSetAmplitude(float volt) {
    return lcr_SetAmplitude(volt);
}

int lcrApp_LcrGetAmplitude(float* volt) {
    return lcr_GetAmplitude(volt);
}

int lcrApp_LcrSetOffset(float offset) {
    return lcr_SetOffset(offset);
}

int lcrApp_LcrGetOffset(float* offset) {
    return lcr_GetOffset(offset);
}

int lcrApp_LcrSetShunt(lcr_shunt_t shunt) {
    return lcr_setRShunt(shunt);
}

int lcrApp_LcrGetShunt(lcr_shunt_t* shunt) {
    return lcr_getRShunt(shunt);
}

int lcrApp_LcrSetShuntIsAuto(bool isShuntAuto) {
    return lcr_setRShuntIsAuto(isShuntAuto);
}

int lcrApp_LcrSetCalibMode(calib_t calib_mode) {
    return lcr_SetCalibMode(calib_mode);
}

int lcrApp_LcrGetCalibMode(calib_t* calib_mode) {
    return lcr_GetCalibMode(calib_mode);
}

int lcrApp_LcrSetMeasSeries(bool series) {
    return lcr_SetMeasSeries(series);
}

int lcrApp_LcrGetMeasSeries(bool* series) {
    return lcr_GetMeasSeries(series);
}

int lcrApp_LcrCheckExtensionModuleConnection(bool _muteWarnings) {
    return lcr_CheckModuleConnection(_muteWarnings);
}

int lcrApp_LcrIsModuleConnected(bool* state) {
    return lcr_IsModuleConnected(state);
}

const char* lcrApp_LcrGetError(lcr_error_t errorCode) {
    switch (errorCode) {
        case RP_LCR_OK:
            return "OK";
        case RP_LCR_HW_CANT_OPEN:
            return "Can't open i2c device";
        case RP_LCR_HW_MISSING_DEVICE:
            return "LCR extension not connected";
        case RP_LCR_HW_ERROR:
            return "LCR undefined hardware error";
        case RP_LCR_HW_ERROR_DETECT:
            return "LCR extension detection error";
        case RP_LCR_ERROR_INVALID_VALUE:
            return "Invalid value";
        case RP_LCR_UERROR:
            return "LCR undefined error";
        case RP_LCR_NOT_STARTED:
            return "Api is not running";
        default:
            break;
    }
    return "Undefined error";
}

int lcrApp_LcrSetCustomShunt(int shunt) {
    return lcr_SetCustomShunt(shunt);
}

int lcrApp_LcrGetCustomShunt(int* shunt) {
    return lcr_GetCustomShunt(shunt);
}

int lcrApp_LcrSetShuntMode(lcr_shunt_mode_t shunt_mode) {
    return lcr_SetShuntMode(shunt_mode);
}

int lcrApp_LcrGetShuntMode(lcr_shunt_mode_t* shunt_mode) {
    return lcr_GetShuntMode(shunt_mode);
}
