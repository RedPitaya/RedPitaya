/**
* $Id: $
*
* @brief Red Pitaya application library Impedance Analyzer module interface
*
* @Author Luka Golinar <luka.golinar@gmail.com>
*
* (c) Red Pitaya  http://www.redpitaya.com
*
* This part of code is written in C programming language.
* Please visit http://en.wikipedia.org/wiki/C_(programming_language)
* for more details on the language used herein.
*/

#ifndef __LCRMETER_H
#define __LCRMETER_H
#include <stdbool.h>
#include <stdint.h>
#include <string>

#include "lcrApp.h"
#include "rp.h"

#include "lcr_generator.h"
#include "lcr_hw.h"

#define LCR_AMPLITUDE 0.25

#define APP_PATH "/opt/redpitaya/www/apps/lcr_meter/"

/* Calibration params */
#define CALIB_STEPS 4

#define MAX_ADC_CHANNELS 4

/* Main lcr params structure */
typedef struct params_e {
    calib_t calibration;
    bool series;
    lcr_shunt_mode_t shunt_mode;
    int shunt;
} lcr_params_t;

static inline const char* stringFromCalib(enum calibration calib) {
    static std::string strings[] = {"CALIB_NONE", "CALIB_OPEN", "CALIB_SHORT"};

    return strings[(int)calib].c_str();
}

/* Resource managment functions */
int lcr_Init();
int lcr_Release();
int lcr_Reset();
int lcr_SetDefaultValues();

/* Main lcr functions */
int lcr_Run();
int lcr_Stop();
int lcr_SetPause(bool pause);
int lcr_GenRun();
int lcr_GenStop();
int lcr_GenSetSettings();
void lcr_MainThread();

/* Measurment functions */
int lcr_SafeThreadGen(rp_channel_t channel, float frequency);
int lcr_ThreadAcqData(buffers_t* data, int* dec, float* freq);
int lcr_getImpedance(buffers_t* data, int api_decimation, float freq);
void lcr_CheckShunt(const float* ch1, const float* ch2, const uint32_t _size);

// int lcr_Correction();
int lcr_CalculateData(float _Complex Z_measured, float phase_measured, float freq);
int lcr_CopyParams(lcr_main_data_t* params);

int lcr_data_analysis(buffers_t* data, lcr_shunt_t r_shunt, float sigFreq, int decimation);

/* Getters and Setters */
int lcr_SetFrequency(float frequency);
int lcr_GetFrequency(float* frequency);
int lcr_SetAmplitude(float volt);
int lcr_GetAmplitude(float* volt);
int lcr_SetOffset(float offset);
int lcr_GetOffset(float* offset);
int lcr_setRShunt(lcr_shunt_t r_shunt);
int lcr_getRShunt(lcr_shunt_t* r_shunt);
int lcr_setRShuntIsAuto(bool isAuto);
int lcr_SetCalibMode(calib_t mode);
int lcr_GetCalibMode(calib_t* mode);
int lcr_SetMeasSeries(bool serial);
int lcr_GetMeasSeries(bool* serial);
int lcr_SetCustomShunt(int shunt);
int lcr_GetCustomShunt(int* shunt);
int lcr_SetShuntMode(lcr_shunt_mode_t shunt_mode);
int lcr_GetShuntMode(lcr_shunt_mode_t* shunt_mode);

int lcr_CheckModuleConnection(bool _muteWarnings);
int lcr_IsModuleConnected(bool* state);

#endif  //__LCRMETER_H
