/**
 * $Id: $
 *
 * @brief Red Pitaya Calibration Module.
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#ifndef __CALIB_COMMON_H
#define __CALIB_COMMON_H

#include <stdbool.h>
#include <stdint.h>
#include "calib_structs.h"
#include "rp_hw-profiles.h"
#include "rp_hw_calib.h"

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

#define DEFAULT_FILT_AA_NEW 0x82d5
#define DEFAULT_FILT_BB_NEW 0x45eaa
#define DEFAULT_FILT_PP_NEW 0x1040
#define DEFAULT_FILT_KK_NEW 0xdfffff

#define DISABLE_FILT_AA 0
#define DISABLE_FILT_BB 0
#define DISABLE_FILT_PP 0
#define DISABLE_FILT_KK 0xffffff

#define CHECK_GAIN_LIMIT(X) (X < 0 ? 0 : (X > 50 ? 50 : X))
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define CHECK_VALID_GAIN_LIMIT(X) (X >= 0.5 && X <= 1.5)

uint8_t* readFromEpprom(uint16_t* size);
uint8_t* readFromFactoryEpprom(uint16_t* size);
uint8_t* readHeader(uint16_t* size, bool use_factory_zone);

uint16_t writeToEpprom(uint8_t* buffer, uint16_t size);
uint16_t writeToFactoryEpprom(uint8_t* buffer, uint16_t size);

bool convertV1(rp_calib_params_t* param, rp_calib_params_v1_t* out);
bool convertV2(rp_calib_params_t* param, rp_calib_params_v2_t* out);
bool convertV3(rp_calib_params_t* param, rp_calib_params_v3_t* out);
bool convertV4(rp_calib_params_t* param, rp_calib_params_v1_t* out);
bool convertGen2(rp_calib_params_t* param, rp_calib_params_v1_t* out);

rp_calib_params_t convertV1toCommon(rp_calib_params_v1_t* param, bool adjust);
rp_calib_params_t convertV2toCommon(rp_calib_params_v2_t* param, bool adjust);
rp_calib_params_t convertV3toCommon(rp_calib_params_v3_t* param, bool adjust);
rp_calib_params_t convertV4toCommon(rp_calib_params_v1_t* param, bool adjust);
rp_calib_params_t convertGen2toCommon(rp_calib_params_v1_t* param, bool adjust);

rp_calib_params_t getDefault(rp_HPeModels_t model, bool setFilterZero);

uint_gain_calib_t convertFloatToInt(channel_calib_t* param, uint8_t precision);

bool recalculateGain(rp_calib_params_t* param);
bool recalculateCalibValue(rp_calib_params_t* param);

rp_calib_error adjustingBaseScaleEx(float* baseScale, int32_t* offset, bool adjust, uint32_t* calibValue);
rp_calib_error adjustingBaseScale(channel_calib_t* calib, bool adjust);

uint32_t calibBaseScaleFromVoltage(float voltageScale, bool uni_is_calib);

bool isUniversalCalib(uint16_t dataStructureId);

#endif  //__CALIB_H