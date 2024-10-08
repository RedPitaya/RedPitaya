/**
 * $Id: $
 *
 * @brief Red Pitaya Applications library common module interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef COMMON_APP_H_
#define COMMON_APP_H_

/* @brief ADC buffer size is 16 k samples. */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <cstring>

#include "rpApp.h"
#include "rp_hw-calib.h"

#define MILLI_TO_NANO               1000000.0

#define MAX_ADC_CHANNELS 4
#define MAX_DAC_CHANNELS 2

#define ECHECK_APP(x) { \
    int retval = (x); \
    if (retval != RP_OK) { \
        ERROR_LOG("%s returned \"%s\"", #x, rpApp_GetError(retval)); \
        return retval; \
    } \
}


#define ECHECK_APP_NO_RET(x) { \
    int retval = (x); \
    if (retval != RP_OK) { \
        ERROR_LOG("%s returned \"%s\"", #x, rpApp_GetError(retval)); \
    } \
}

#define CHANNEL_ACTION(CHANNEL, CHANNEL_1_ACTION, CHANNEL_2_ACTION) \
if ((CHANNEL) == RP_CH_1) { \
    CHANNEL_1_ACTION; \
} \
else if ((CHANNEL) == RP_CH_2) { \
    CHANNEL_2_ACTION; \
} \
else { \
    return RP_EPN; \
}

#define CHANNEL_ACTION_4CH(CHANNEL, CHANNEL_1_ACTION, CHANNEL_2_ACTION, CHANNEL_3_ACTION, CHANNEL_4_ACTION) \
if ((CHANNEL) == RP_CH_1) { \
    CHANNEL_1_ACTION; \
} \
else if ((CHANNEL) == RP_CH_2) { \
    CHANNEL_2_ACTION; \
} \
else if ((CHANNEL) == RP_CH_3) { \
    CHANNEL_3_ACTION; \
} \
else if ((CHANNEL) == RP_CH_4) { \
    CHANNEL_4_ACTION; \
} \
else { \
    return RP_EPN; \
}

#define SOURCE_ACTION(SOURCE, SOURCE_1_ACTION, SOURCE_2_ACTION, SOURCE_3_ACTION) \
switch ((SOURCE)) { \
    case RPAPP_OSC_SOUR_CH1: \
        (SOURCE_1_ACTION); \
        break; \
    case RPAPP_OSC_SOUR_CH2: \
        (SOURCE_2_ACTION); \
        break; \
    case RPAPP_OSC_SOUR_MATH: \
        (SOURCE_3_ACTION); \
        break; \
    default: \
        return RP_EPN; \
        break; \
}

#define SOURCE_ACTION_4CH(SOURCE, SOURCE_1_ACTION, SOURCE_2_ACTION, SOURCE_3_ACTION,SOURCE_4_ACTION,SOURCE_5_ACTION) \
switch ((SOURCE)) { \
    case RPAPP_OSC_SOUR_CH1: \
        (SOURCE_1_ACTION); \
        break; \
    case RPAPP_OSC_SOUR_CH2: \
        (SOURCE_2_ACTION); \
        break; \
    case RPAPP_OSC_SOUR_CH3: \
        (SOURCE_3_ACTION); \
        break; \
    case RPAPP_OSC_SOUR_CH4: \
        (SOURCE_4_ACTION); \
        break; \
    case RPAPP_OSC_SOUR_MATH: \
        (SOURCE_5_ACTION); \
        break; \
    default: \
        return RP_EPN; \
        break; \
}


#define CHECK_CHANNEL() \
    uint8_t channels_rp_HPGetFastADCChannelsCount = 0; \
    if (rp_HPGetFastADCChannelsCount(&channels_rp_HPGetFastADCChannelsCount) != RP_HP_OK){ \
        ERROR_LOG("Can't get fast ADC channels count"); \
        return RP_NOTS; \
    } \
    if (channel >= channels_rp_HPGetFastADCChannelsCount){ \
        ERROR_LOG("Channel is larger than allowed"); \
        return RP_NOTS; \
    }

#define EXECUTE_ATOMICALLY(MUTEX, ACTION) \
    (MUTEX).lock();\
    (ACTION); \
    (MUTEX).unlock();

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

int cmn_Init();
int cmn_Release();

int intCmp(const void *a, const void *b);

auto indexToTime(int64_t index) -> float;

auto timeToIndexD(float time) -> double;
auto timeToIndexI(float time) -> int64_t;

auto osc_adc_sign(uint32_t cnts, uint8_t bits) -> int32_t;

auto getADCChannels() -> uint8_t;
auto getDACChannels() -> uint8_t;
auto getDACRate() -> uint32_t;
auto getADCRate() -> uint32_t;
auto getModel() -> rp_HPeModels_t;
auto getDACDevider() -> double;
auto getModelName() -> std::string;

auto convertToVoltSigned(uint32_t cnts, uint8_t bits, float fullScale, uint32_t gain, uint32_t base, int32_t offset) -> float;
auto calibCntsSigned(uint32_t cnts, uint8_t bits, uint32_t gain, uint32_t base, int32_t offset) -> int32_t;
auto getADCSamplePeriod(double *value) -> int;

auto convertCh(rp_channel_t ch) -> rp_channel_calib_t;
auto convertChFromIndex(uint8_t index) -> rp_channel_t;
auto convertPower(rp_acq_ac_dc_mode_t ch) -> rp_acq_ac_dc_mode_calib_t;
auto convertCh(rpApp_osc_trig_source_t ts) -> int;

template<typename X>
inline X scaleAmplitude(X _volts, X _ampScale, X _probeAtt, X _ampOffset, X _invertFactor) {
    // return (_volts * _invertFactor * _probeAtt + _ampOffset) / _ampScale;
    return _volts * (_invertFactor * _probeAtt) / _ampScale + _ampOffset / _ampScale;
}

template<typename X>
inline X scaleAmplitude(X _volts, X coff1, X coff2) {
    // return (_volts * _invertFactor * _probeAtt + _ampOffset) / _ampScale;
    return _volts * coff1 + coff2;
}

template<typename X>
inline X unscaleAmplitude(X _value, X _ampScale, X _probeAtt, X _ampOffset, X _invertFactor) {
    // return ((_value * _ampScale) - _ampOffset) / _probeAtt / _invertFactor;
    return ((_value * _ampScale * _invertFactor) - _ampOffset * _invertFactor) / _probeAtt;
}

template<typename X>
inline X unscaleAmplitude(X _value, X coff1, X coff2) {
    return _value * coff1 - coff2;
}

template<typename X>
inline X unOffsetAmplitude(X _value, X _ampScale, X _ampOffset) {
    return _value - (_ampOffset / _ampScale);
}

template<typename X>
inline X unOffsetAmplitude(X _value, X coff1) {
    return _value - coff1;
}

#endif /* COMMON_APP_H_ */
