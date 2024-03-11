/**
 * $Id: worker.h 881 2013-12-16 05:37:34Z rp_jmenart $
 *
 * @brief Red Pitaya Spectrum Analyzer worker.
 *
 * @Author Jure Menart <juremenart@gmail.com>
 *         
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef __SPECTROMETERAPP_H
#define __SPECTROMETERAPP_H

#include "rpApp.h"
#include "rp_dsp.h"

int spec_run();

int spec_stop();

int spec_running(); // true/false

int spec_reset();

int spec_getViewData(float **signals, size_t size);

int spec_getViewSize(size_t *size);

int spec_getPeakPower(rp_channel_t channel, float* power);

int spec_getPeakFreq(rp_channel_t channel, float* freq);

int spec_setFreqRange(float _freq_min, float freq);

int spec_setWindow(rp_dsp_api::window_mode_t mode);

int spec_getWindow(rp_dsp_api::window_mode_t *mode);

int spec_setADCBufferSize(size_t size);

int spec_getADCBufferSize();

int spec_getGetADCFreq();

int spec_setRemoveDC(bool state);

int spec_getRemoveDC();

int spec_getFpgaFreq(float* freq);

int spec_getFreqMax(float* freq);

int spec_getFreqMin(float* freq);

int spec_getVoltMode(rp_dsp_api::mode_t *mode);

int spec_setVoltMode(rp_dsp_api::mode_t mode);

int spec_setImpedance(double value);

int spec_getImpedance(double *value);



#endif /* __SPECTROMETERAPP_H*/
