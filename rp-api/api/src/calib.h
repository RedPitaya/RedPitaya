/**
 * $Id: $
 *
 * @brief Red Pitaya Calibration Module.
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef __CALIB_H
#define __CALIB_H

#include <stdint.h>
#include "rp_cross.h"

#define CONSTANT_SIGNAL_AMPLITUDE 0.8


              int calib_Init();
              int calib_Release();

rp_calib_params_t calib_GetParams();
rp_calib_params_t calib_GetDefaultCalib();
              int calib_WriteParams(rp_calib_params_t calib_params,bool use_factory_zone);
              int calib_SetParams(rp_calib_params_t calib_params);
             void calib_SetToZero();
              int calib_LoadFromFactoryZone();
            

              int calib_SetFrontEndOffset(rp_channel_t channel, rp_pinState_t gain, rp_calib_params_t* out_params);
              int calib_SetFrontEndScaleLV(rp_channel_t channel, float referentialVoltage, rp_calib_params_t* out_params);
              int calib_SetFrontEndScaleHV(rp_channel_t channel, float referentialVoltage, rp_calib_params_t* out_params);

              int calib_SetBackEndOffset(rp_channel_t channel);
              int calib_SetBackEndScale(rp_channel_t channel);
              int calib_CalibrateBackEnd(rp_channel_t channel, rp_calib_params_t* out_params);

              int calib_Reset();

          int32_t calib_GetDataMedian(rp_channel_t channel, rp_pinState_t gain);
            float calib_GetDataMedianFloat(rp_channel_t channel, rp_pinState_t gain);
              int calib_GetDataMinMaxFloat(rp_channel_t channel, rp_pinState_t gain, float* min, float* max);

              int calib_setCachedParams();  
#ifndef Z20_250_12 
         uint32_t calib_GetFrontEndScale(rp_channel_t channel, rp_pinState_t gain);
          int32_t calib_getOffset(rp_channel_t channel, rp_pinState_t gain);
          int32_t calib_getGenOffset(rp_channel_t channel);
         uint32_t calib_getGenScale(rp_channel_t channel);
#if defined Z10 || defined Z20_125
              int calib_SetFilterCoff(rp_channel_t channel, rp_pinState_t gain, rp_eq_filter_cof_t coff , uint32_t value);
         uint32_t calib_GetFilterCoff(rp_channel_t channel, rp_pinState_t gain, rp_eq_filter_cof_t coff);
#endif
#else
         uint32_t calib_GetFrontEndScale(rp_channel_t channel, rp_pinState_t gain, rp_acq_ac_dc_mode_t power_mode);
          int32_t calib_getOffset(rp_channel_t channel, rp_pinState_t gain, rp_acq_ac_dc_mode_t power_mode);
          int32_t calib_getGenOffset(rp_channel_t channel, rp_gen_gain_t gain);
         uint32_t calib_getGenScale(rp_channel_t channel, rp_gen_gain_t gain);
#endif

#endif //__CALIB_H
