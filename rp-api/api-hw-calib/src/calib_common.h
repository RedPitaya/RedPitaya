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

#ifndef __CALIB_COMMON_H
#define __CALIB_COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include "rp_hw-profiles.h"
#include "calib_structs.h"
#include "rp_hw-calib.h"

// Used for old board like 125-10 where missing HV calibaration
#define CALIB_MAGIC 0xAABBCCDD
#define CALIB_MAGIC_FILTER 0xDDCCBBAA

#define DEFAULT_1_1_FILT_AA 0x7D93
#define DEFAULT_1_1_FILT_BB 0x437C7
#define DEFAULT_1_1_FILT_PP 0x2666
#define DEFAULT_1_1_FILT_KK 0xd9999a
#define DEFAULT_1_20_FILT_AA 0x4205
#define DEFAULT_1_20_FILT_BB 0x2F38B
#define DEFAULT_1_20_FILT_PP 0x2666
#define DEFAULT_1_20_FILT_KK 0xd9999a


uint8_t* readFromEpprom(uint16_t *size);
uint8_t* readFromFactoryEpprom(uint16_t *size);

uint16_t writeToEpprom(uint8_t* buffer,uint16_t size);
uint16_t writeToFactoryEpprom(uint8_t* buffer,uint16_t size);

bool convertV1(rp_calib_params_t *param,rp_calib_params_v1_t *out);
bool convertV2(rp_calib_params_t *param,rp_calib_params_v2_t *out);
bool convertV3(rp_calib_params_t *param,rp_calib_params_v3_t *out);

rp_calib_params_t convertV1toCommon(rp_calib_params_v1_t *param);
rp_calib_params_t convertV2toCommon(rp_calib_params_v2_t *param);
rp_calib_params_t convertV3toCommon(rp_calib_params_v3_t *param);

rp_calib_params_t getDefault(rp_HPeModels_t model);

uint_gain_calib_t convertFloatToInt(channel_calib_t *param,uint8_t precision);

bool recalculateGain(rp_calib_params_t *param);
bool recalculateCalibValue(rp_calib_params_t *param);

#endif //__CALIB_H