/**
 * $Id: worker.h 881 2013-12-16 05:37:34Z rp_jmenart $
 *
 * @brief Red Pitaya Spectrum Analyzer worker.
 *
 * @Author Jure Menart <juremenart@gmail.com>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#ifndef __SPECTROMETERAPP_H
#define __SPECTROMETERAPP_H

#include "math/rp_dsp.h"
#include "rpApp.h"

int spec_run();

int spec_stop();

int spec_running();  // true/false

int spec_reset();

int spec_lockData();

int spec_unlockData();

int spec_SetEnable(rp_channel_t channel, bool state);

int spec_GetEnable(rp_channel_t channel, bool* state);

int spec_getViewData(const rp_dsp_api::rp_dsp_result_t** data);

int spec_getViewSize(size_t* size);

int spec_setWindow(rp_dsp_api::window_mode_t mode);

int spec_getWindow(rp_dsp_api::window_mode_t* mode);

int spec_setProbe(rp_channel_t channel, uint32_t probe);

int spec_getProbe(rp_channel_t channel, uint32_t* probe);

int spec_setFreqRange(float max_freq);

int spec_setADCBufferSize(size_t size);

int spec_getADCBufferSize();

int spec_getGetADCFreq();

int spec_setRemoveDC(bool state);

int spec_getRemoveDC();

int spec_getFpgaFreq(float* freq);

int spec_setImpedance(double value);

int spec_getImpedance(double* value);

#endif /* __SPECTROMETERAPP_H*/
