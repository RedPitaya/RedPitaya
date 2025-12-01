/**
 * $Id: $
 *
 * @brief Red Pitaya library
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef CONVERT_HPP_
#define CONVERT_HPP_

#include <stdint.h>
#include <cstdlib>
#include <math.h>
#include "common/rp_log.h"

inline uint32_t cmn_convertToCnt(float voltage,uint8_t bits,float fullScale,bool is_signed,double gain, int32_t offset){
    uint32_t mask = ((uint64_t)1 << bits) - 1;

    if (gain == 0){
        FATAL("convertToCnt devide by zero")
    }

    if (fullScale == 0){
        FATAL("convertToCnt devide by zero")
    }

    voltage /= gain;

    /* check and limit the specified voltage arguments towards */
    /* maximal voltages which can be applied on ADC inputs */

    if(voltage > fullScale)
        voltage = fullScale;
    else if(voltage < -fullScale)
        voltage = -fullScale;

    if (!is_signed && voltage < 0){
        voltage = 0;
    }

    int32_t  cnts = (int)round(voltage * (float) (1 << (bits - (is_signed ? 1 : 0))) / fullScale);
    cnts += offset;

    /* check and limit the specified cnt towards */
    /* maximal cnt which can be applied on ADC inputs */
    if(cnts > (1 << (bits - (is_signed ? 1 : 0))) - 1)
        cnts = (1 << (bits - (is_signed ? 1 : 0))) - 1;
    else if(cnts < -(1 << (bits - (is_signed ? 1 : 0))))
        cnts = -(1 << (bits - (is_signed ? 1 : 0)));

    /* if negative remove higher bits that represent negative number */
    if (cnts < 0)
        cnts = cnts & mask;
    return (uint32_t)cnts;
}

inline int32_t cmn_CalibCntsSigned(uint32_t cnts, uint8_t bits, uint32_t gain, uint32_t base, int32_t offset){
    int32_t m;

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
    // if(m < -(1 << (bits - 1)))
    //     m = -(1 << (bits - 1));
    // else if(m > (1 << (bits - 1)))
    //     m = (1 << (bits - 1));

    return m;
}

inline uint32_t cmn_CalibCntsUnsigned(uint32_t cnts, uint8_t bits, uint32_t gain, uint32_t base, int32_t offset){
    int32_t m = cnts;

    /* adopt ADC count with calibrated DC offset */
    m -= offset;

    m = (gain * m) / base;

    /* check limits */
    // if(m < 0)
    //     m = 0;
    // else if(m > (1 << bits))
    //     m = (1 << bits);

    return m;
}

inline float cmn_convertToVoltSigned(uint32_t cnts, uint8_t bits, float fullScale, uint32_t gain, uint32_t base, int32_t offset){
    int32_t calib_cnts = cmn_CalibCntsSigned(cnts, bits, gain, base, offset);
    float ret_val = ((float)calib_cnts * fullScale / (float)(1 << (bits - 1)));
    return ret_val;
}

inline float cmn_convertToVoltUnsigned(uint32_t cnts, uint8_t bits, float fullScale, uint32_t gain, uint32_t base, int32_t offset){
    uint32_t calib_cnts = cmn_CalibCntsUnsigned(cnts, bits, gain, base, offset);
    float ret_val = ((float)calib_cnts * fullScale / (float)(1 << bits));
    return ret_val;
}

#endif