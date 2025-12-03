/**
* $Id: $
*
* @brief Red Pitaya application impedance module application
*
* @Author Luka Golinar
*
* (c) Red Pitaya  http://www.redpitaya.com
*/

#ifndef __LCR_APP_H
#define __LCR_APP_H

typedef enum calibration {
    CALIB_NONE,
    CALIB_OPEN,
    CALIB_SHORT,
} calib_t;

typedef enum lcr_shunt { RP_LCR_S_NOT_INIT = -1, RP_LCR_S_10 = 0, RP_LCR_S_100 = 1, RP_LCR_S_1k = 2, RP_LCR_S_10k = 3, RP_LCR_S_100k = 4, RP_LCR_S_1M = 5 } lcr_shunt_t;

typedef enum lcr_shunt_mode { RP_LCR_S_EXTENSION = 0, RP_LCR_S_CUSTOM = 1 } lcr_shunt_mode_t;

/* Main lcr measurment data */
typedef struct data_e {
    double lcr_freq;
    double lcr_amplitude;
    double lcr_phase;
    double lcr_D;
    double lcr_Q;
    double lcr_ESR;
    double lcr_L;
    double lcr_C;
    double lcr_R;
    // values for console tool
    double lcr_L_s;
    double lcr_C_s;
    double lcr_R_s;
    double lcr_L_p;
    double lcr_C_p;
    double lcr_R_p;
    double lcr_D_s;
    double lcr_Q_s;
    double lcr_D_p;
    double lcr_Q_p;
    double lcr_X_s;
    double lcr_G_p;
    double lcr_B_p;
    double lcr_Y_abs;
    double lcr_Phase_Y;
    double lcr_P_p_amp;

} lcr_main_data_t;

typedef enum lcr_error {
    RP_LCR_OK = 0,
    RP_LCR_HW_CANT_OPEN = 1,
    RP_LCR_HW_MISSING_DEVICE = 2,
    RP_LCR_HW_ERROR = 3,
    RP_LCR_HW_ERROR_DETECT = 4,
    RP_LCR_ERROR_INVALID_VALUE = 5,
    RP_LCR_UERROR = 6,
    RP_LCR_NOT_STARTED = 7
} lcr_error_t;

/** @name General
*/
///@{

int lcrApp_lcrInit();
int lcrApp_LcrRelease();

int lcrApp_LcrRun();
int lcrApp_LcrStop();
int lcrApp_LcrSetPause(bool pause);
int lcrApp_LcrReset();

int lcrApp_GenRun();
int lcrApp_GenStop();
int lcrApp_GenSetSettings();
// int lcrApp_LcrStartCorrection();
int lcrApp_LcrCopyParams(lcr_main_data_t* data);

//Getters, setters
int lcrApp_LcrSetFrequency(float frequency);
int lcrApp_LcrGetFrequency(float* frequency);
int lcrApp_LcrSetAmplitude(float volt);
int lcrApp_LcrGetAmplitude(float* volt);
int lcrApp_LcrSetOffset(float offset);
int lcrApp_LcrGetOffset(float* offset);
int lcrApp_LcrSetShunt(lcr_shunt_t shunt);
int lcrApp_LcrGetShunt(lcr_shunt_t* shunt);
int lcrApp_LcrSetCustomShunt(int shunt);
int lcrApp_LcrGetCustomShunt(int* shunt);
int lcrApp_LcrSetShuntMode(lcr_shunt_mode_t shunt_mode);
int lcrApp_LcrGetShuntMode(lcr_shunt_mode_t* shunt_mode);

int lcrApp_LcrSetShuntIsAuto(bool isShuntAuto);
int lcrApp_LcrSetCalibMode(calib_t calib_mode);
int lcrApp_LcrGetCalibMode(calib_t* calib_mode);
int lcrApp_LcrSetMeasSeries(bool series);
int lcrApp_LcrGetMeasSeries(bool* series);
int lcrApp_LcrCheckExtensionModuleConnection(bool _muteWarnings);
int lcrApp_LcrIsModuleConnected(bool* state);

const char* lcrApp_LcrGetError(lcr_error_t errorCode);

#endif  //__LCR_APP_H
