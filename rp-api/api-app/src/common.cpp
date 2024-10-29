/**
 * $Id: $
 *
 * @brief Red Pitaya Applications library common module implementation
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */


#include <math.h>
#include <assert.h>

#include "common.h"
#include "rp.h"


auto cmn_Init() -> int {
    return RP_OK;
}

auto cmn_Release() -> int {
    return RP_OK;
}

auto intCmp(const void *a, const void *b) -> int {
    const int *ia = (const int *)a, *ib = (const int *)b;
    return (*ia < *ib) ? -1 : (*ia > *ib);
}

// Returns time in milliseconds
auto indexToTime(int64_t index) -> float {
    float samplingRate;
    ECHECK_APP(rp_AcqGetSamplingRateHz(&samplingRate));
    return (float) (index * 1000.0 / samplingRate);
}

// Parameter time is in milliseconds
auto timeToIndexI(float time) -> int64_t {
    float samplingRate;
    ECHECK_APP(rp_AcqGetSamplingRateHz(&samplingRate));
    return (int64_t) round(samplingRate * time / 1000.0);
}

auto timeToIndexD(float time) -> double {
    float samplingRate;
    ECHECK_APP(rp_AcqGetSamplingRateHz(&samplingRate));
    return (double)samplingRate * time / 1000.0;
}


auto getADCChannels() -> uint8_t{
    uint8_t c = 0;
    if (rp_HPGetFastADCChannelsCount(&c) != RP_HP_OK){
        ERROR_LOG("Can't get fast ADC channels count");
    }
    if (c > MAX_ADC_CHANNELS){
        ERROR_LOG("The number of channels is more than allowed");
        exit(-1);
    }
    return c;
}

auto getDACChannels() -> uint8_t{
    uint8_t c = 0;

    if (rp_HPGetFastDACChannelsCount(&c) != RP_HP_OK){
        ERROR_LOG("Can't get fast DAC channels count");
    }

    if (c > MAX_DAC_CHANNELS){
        ERROR_LOG("The number of channels is more than allowed");
        exit(-1);
    }
    return c;
}

auto getDACRate() -> uint32_t{
    uint32_t c = 0;
    if (rp_HPGetBaseFastDACSpeedHz(&c) != RP_HP_OK){
        ERROR_LOG("Can't get fast DAC channels count");
    }
    return c;
}

auto getADCRate() -> uint32_t{
    uint32_t c = 0;
    if (rp_HPGetBaseFastADCSpeedHz(&c) != RP_HP_OK){
        ERROR_LOG("Can't get fast ADC channels count");
    }
    return c;
}

 auto getModel() -> rp_HPeModels_t{
    rp_HPeModels_t c = STEM_125_14_v1_0;
    if (rp_HPGetModel(&c) != RP_HP_OK){
        ERROR_LOG("Can't get board model");
    }
    return c;
}

auto getADCSamplePeriod(double *value) -> int{
    *value = 0;
    uint32_t speed = 0;
    int ret = rp_HPGetBaseFastADCSpeedHz(&speed);
    if (ret == RP_HP_OK){
        *value = (double)1e9/speed;
    }else{
        ERROR_LOG("Can't get FAST ADC Rate");
    }
    return ret;
}

auto getDACDevider() -> double{
    if (getDACRate() > 125e6)
        return 4;
    return 2;
}

auto getModelName() -> std::string{
    auto model = getModel();
    switch (model)
    {
        case STEM_125_10_v1_0:
        case STEM_125_14_v1_0:
        case STEM_125_14_v1_1:
        case STEM_125_14_LN_v1_1:
        case STEM_125_14_LN_BO_v1_1:
        case STEM_125_14_LN_CE1_v1_1:
        case STEM_125_14_LN_CE2_v1_1:
        case STEM_125_14_v2_0:
        case STEM_125_14_Pro_v2_0:
            return "Z10";
        case STEM_125_14_Z7020_v1_0:
        case STEM_125_14_Z7020_LN_v1_1:
        case STEM_125_14_Z7020_Pro_v2_0:
        case STEM_125_14_Z7020_Ind_v2_0:
            return "Z20_125";
        case STEM_122_16SDR_v1_0:
        case STEM_122_16SDR_v1_1:
            return "Z20";
        case STEM_125_14_Z7020_4IN_v1_0:
        case STEM_125_14_Z7020_4IN_v1_2:
        case STEM_125_14_Z7020_4IN_v1_3:
            return "Z20_125_4CH";
        case STEM_250_12_v1_0:
        case STEM_250_12_v1_1:
        case STEM_250_12_v1_2:
        case STEM_250_12_v1_2a:
        case STEM_250_12_v1_2b:
            return "Z20_250_12";
        case STEM_250_12_120:
            return "Z20_250_12_120";
        default:{
            ERROR_LOG("Unknown model: %d.",model);
            return "";
        }
    }
    return "";
}


auto convertToVoltSigned(uint32_t cnts, uint8_t bits, float fullScale, uint32_t gain, uint32_t base, int32_t offset) -> float {
    int32_t calib_cnts = calibCntsSigned(cnts, bits, gain, base, offset);
    float ret_val = ((float)calib_cnts * fullScale / (float)(1 << (bits - 1)));
    return ret_val;
}

auto calibCntsSigned(uint32_t cnts, uint8_t bits, uint32_t gain, uint32_t base, int32_t offset) -> int32_t {
    int32_t m;
    cnts &= ((1 << bits) - 1);
    /* check sign */
    if(cnts & (1 << (bits - 1))) {
        /* negative number */
        m = -1 *((cnts ^ ((1 << bits) - 1)) + 1);
    } else {
        /* positive number */
        m = cnts;
    }
    /* adopt ADC count with calibrated DC offset */
    m -= offset;

    m = ((int32_t)gain * m) / (int32_t)base;
    /* check limits */
    if(m < -(1 << (bits - 1)))
        m = -(1 << (bits - 1));
    else if(m > (1 << (bits - 1)))
        m = (1 << (bits - 1));

    return m;
}

auto convertCh(rp_channel_t ch) -> rp_channel_calib_t{
    switch (ch)
    {
    case RP_CH_1:
        return RP_CH_1_CALIB;
    case RP_CH_2:
        return RP_CH_2_CALIB;
    case RP_CH_3:
        return RP_CH_3_CALIB;
    case RP_CH_4:
        return RP_CH_4_CALIB;

    default:
        ERROR_LOG("Convert from %d",ch);
        assert(false);
    }
    return RP_CH_1_CALIB;
}

auto convertChFromIndex(uint8_t index) -> rp_channel_t{
    if (index == 0)  return RP_CH_1;
    if (index == 1)  return RP_CH_2;
    if (index == 2)  return RP_CH_3;
    if (index == 3)  return RP_CH_4;

    ERROR_LOG("Convert from %d",index);
    assert(false);
    return RP_CH_1;
}

auto convertPower(rp_acq_ac_dc_mode_t ch) -> rp_acq_ac_dc_mode_calib_t{
    switch (ch)
    {
    case RP_AC:
        return RP_AC_CALIB;
    case RP_DC:
        return RP_DC_CALIB;
    default:
        ERROR_LOG("Convert from %d",ch);
        assert(false);
    }
    return RP_DC_CALIB;
}

auto osc_adc_sign(uint32_t cnts, uint8_t bits) -> int32_t{
    int32_t m;
    cnts &= ((1 << bits) - 1);
    /* check sign */
    if(cnts & (1 << (bits - 1))) {
        /* negative number */
        m = -1 *((cnts ^ ((1 << bits) - 1)) + 1);
    } else {
        /* positive number */
        m = cnts;
    }
    return m;
}

auto convertCh(rpApp_osc_trig_source_t ts) -> int{
    switch (ts)
    {
    case RPAPP_OSC_TRIG_SRC_CH1:
        return RP_CH_1;
    case RPAPP_OSC_TRIG_SRC_CH2:
        return RP_CH_2;
    case RPAPP_OSC_TRIG_SRC_CH3:
        return RP_CH_3;
    case RPAPP_OSC_TRIG_SRC_CH4:
        return RP_CH_4;

    default:
        break;
    }
    return -1;
}