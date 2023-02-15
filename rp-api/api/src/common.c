/**
 * $Id: $
 *
 * @brief Red Pitaya library common module implementation
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include "common.h"
#include "redpitaya/rp.h"

static int fd = 0;

int cmn_Init()
{
    if (!fd) {
        if((fd = open("/dev/uio/api", O_RDWR | O_SYNC)) == -1) {
            return RP_EOMD;
        }
    }
    return RP_OK;
}

int cmn_Release()
{
    if (fd) {
        if(close(fd) < 0) {
            return RP_ECMD;
        }
    }

    return RP_OK;
}

int cmn_Map(size_t size, size_t offset, void** mapped)
{
    if(fd == -1) {
        return RP_EMMD;
    }

    offset = (offset >> 20) * sysconf(_SC_PAGESIZE);

    *mapped = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);

    if(mapped == (void *) -1) {
        return RP_EMMD;
    }

    return RP_OK;
}

int cmn_Unmap(size_t size, void** mapped)
{
    if(fd == -1) {
        return RP_EUMD;
    }

    if((mapped == (void *) -1) || (mapped == NULL)) {
        return RP_EUMD;
    }

    if((*mapped == (void *) -1) || (*mapped == NULL)) {
        return RP_EUMD;
    }

    if(munmap(*mapped, size) < 0){
        return RP_EUMD;
    }
    *mapped = NULL;
    return RP_OK;
}

void cmn_DebugReg(const char* msg,uint32_t value){
    fprintf(stderr,"\tSet %s 0x%X\n",msg,value);
}

void cmn_DebugRegCh(const char* msg,int ch,uint32_t value){
    rp_channel_t chV = (rp_channel_t)ch;
    switch(chV){
        case RP_CH_1:
            fprintf(stderr,"\tSet %s [CH1] 0x%X\n",msg,value);
        break;
        case RP_CH_2:
            fprintf(stderr,"\tSet %s [CH2] 0x%X\n",msg,value);
        break;
        case RP_CH_3:
            fprintf(stderr,"\tSet %s [CH2] 0x%X\n",msg,value);
        break;
        case RP_CH_4:
            fprintf(stderr,"\tSet %s [CH2] 0x%X\n",msg,value);
        break;
        default:
            fprintf(stderr,"\tSet %s [Error channel] 0x%X\n",msg,value);
        break;
    }
}


int cmn_SetShiftedValue(volatile uint32_t* field, uint32_t value, uint32_t mask, uint32_t bitsToSetShift,uint32_t *settedValue)
{
    VALIDATE_BITS(value, mask);
    cmn_GetValue(field, settedValue, 0xffffffff);
    *settedValue &=  ~(mask << bitsToSetShift); // Clear all bits at specified location
    *settedValue +=  (value << bitsToSetShift); // Set value at specified location
    SET_VALUE(*field, *settedValue);
    return RP_OK;
}

int cmn_SetValue(volatile uint32_t* field, uint32_t value, uint32_t mask,uint32_t *settedValue)
{
    return cmn_SetShiftedValue(field, value, mask, 0, settedValue);
}

int cmn_GetShiftedValue(volatile uint32_t* field, uint32_t* value, uint32_t mask, uint32_t bitsToSetShift)
{
    *value = (*field >> bitsToSetShift) & mask;
    return RP_OK;
}

int cmn_GetValue(volatile uint32_t* field, uint32_t* value, uint32_t mask)
{
    return cmn_GetShiftedValue(field, value, mask, 0);
}

int cmn_SetBits(volatile uint32_t* field, uint32_t bits, uint32_t mask)
{
    VALIDATE_BITS(bits, mask);
    SET_BITS(*field, bits);
    return RP_OK;
}

int cmn_UnsetBits(volatile uint32_t* field, uint32_t bits, uint32_t mask)
{
    VALIDATE_BITS(bits, mask);
    UNSET_BITS(*field, bits);
    return RP_OK;
}

int cmn_AreBitsSet(volatile uint32_t field, uint32_t bits, uint32_t mask, bool* result)
{
    VALIDATE_BITS(bits, mask);
    *result = ARE_BITS_SET(field, bits);
    return RP_OK;
}

/* 32 bit integer comparator */
int intcmp(const void *v1, const void *v2)
{
    return (*(int *)v1 - *(int *)v2);
}

/* 16 bit integer comparator */
int int16cmp(const void *aa, const void *bb)
{
    const int16_t *a = aa, *b = bb;
    return (*a < *b) ? -1 : (*a > *b);
}

/* Float comparator */
int floatCmp(const void *a, const void *b) {
    float fa = *(const float*) a, fb = *(const float*) b;
    return (fa > fb) - (fa < fb);
}

rp_channel_calib_t convertCh(rp_channel_t ch){
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
        fprintf(stderr,"[FATAL ERROR] Convert from %d\n",ch);
        assert(false);
    }
    return RP_EOOR;
}

rp_channel_t convertChFromIndex(uint8_t index){
    if (index == 0)  return RP_CH_1;
    if (index == 1)  return RP_CH_2;
    if (index == 2)  return RP_CH_3;
    if (index == 3)  return RP_CH_4;

    fprintf(stderr,"[FATAL ERROR] Convert from %d\n",index);
    assert(false);
    return RP_CH_1;
}

rp_channel_calib_t convertPINCh(rp_apin_t pin){
    switch (pin)
    {
    case RP_AIN0:
    case RP_AOUT0:
        return RP_CH_1_CALIB;
    case RP_AIN1:
    case RP_AOUT1:
        return RP_CH_2_CALIB;
    case RP_AIN2:
    case RP_AOUT2:
        return RP_CH_3_CALIB;
    case RP_AIN3:
    case RP_AOUT3:
        return RP_CH_4_CALIB;

    default:
        fprintf(stderr,"[FATAL ERROR] Convert from PIN %d\n",pin);
        assert(false);
    }
    return RP_EOOR;
}

rp_acq_ac_dc_mode_calib_t convertPower(rp_acq_ac_dc_mode_t ch){
    switch (ch)
    {
    case RP_AC:
        return RP_AC_CALIB;
    case RP_DC:
        return RP_DC_CALIB;
    default:
        fprintf(stderr,"[FATAL ERROR] Convert from %d\n",ch);
        assert(false);
    }
    return RP_EOOR;
}


uint32_t cmn_convertToCnt(float voltage,uint8_t bits,float fullScale,bool is_signed,double gain, int32_t offset){
    uint32_t mask = ((uint64_t)1 << bits) - 1;

    if (gain == 0){
        fprintf(stderr,"[FATAL ERROR] convertToCnt devide by zero\n");
        assert(false);
    }

    if (fullScale == 0){
        fprintf(stderr,"[FATAL ERROR] convertToCnt devide by zero\n");
        assert(false);
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

float cmn_convertToVoltSigned(uint32_t cnts, uint8_t bits, float fullScale, uint32_t gain, uint32_t base, int32_t offset){
    int32_t calib_cnts = cmn_CalibCntsSigned(cnts, bits, gain, base, offset);
    float ret_val = ((float)calib_cnts * fullScale / (float)(1 << (bits - 1)));
    return ret_val;
}

float cmn_convertToVoltUnsigned(uint32_t cnts, uint8_t bits, float fullScale, uint32_t gain, uint32_t base, int32_t offset){
    uint32_t calib_cnts = cmn_CalibCntsUnsigned(cnts, bits, gain, base, offset);
    float ret_val = ((float)calib_cnts * fullScale / (float)(1 << bits));
    return ret_val;
}

int32_t cmn_CalibCntsSigned(uint32_t cnts, uint8_t bits, uint32_t gain, uint32_t base, int32_t offset){
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
    if(m < -(1 << (bits - 1)))
        m = -(1 << (bits - 1));
    else if(m > (1 << (bits - 1)))
        m = (1 << (bits - 1));

    return m;
}

uint32_t cmn_CalibCntsUnsigned(uint32_t cnts, uint8_t bits, uint32_t gain, uint32_t base, int32_t offset){
    int32_t m = cnts;

    /* adopt ADC count with calibrated DC offset */
    m -= offset;

    m = (gain * m) / base;

    /* check limits */
    if(m < 0)
        m = 0;
    else if(m > (1 << bits))
        m = (1 << bits);

    return m;
}

/*----------------------------------------------------------------------------*/

// /**
// * @brief Converts calibration Full scale to volts. Scale is usually read from EPROM calibration parameters.
// * If parameter is 0, a factor 1 is returned -> no scaling.
// *
// * @param[in] fullScaleGain value of full voltage scale
// * @retval Scale in volts
// */
// float cmn_CalibFullScaleToVoltage(uint32_t fullScaleGain) {
//     /* no scale */
//     if (fullScaleGain == 0) {
//         return 1;
//     }
//     return (float) ((float)fullScaleGain  * 100.0 / ((uint64_t)1<<32));
// }

// float rp_cmn_CalibFullScaleToVoltage(uint32_t fullScaleGain) {
// 	return cmn_CalibFullScaleToVoltage(fullScaleGain);
// }

// /**
// * @brief Converts scale voltage to calibration Full scale. Result is usually written to EPROM calibration parameters.
// *
// * @param[in] voltageScale Scale value in voltage
// * @retval Scale in volts
// */
// uint32_t cmn_CalibFullScaleFromVoltage(float voltageScale) {
//     return (uint32_t) (voltageScale / 100.0 * ((uint64_t)1<<32));
// }

// uint32_t rp_cmn_CalibFullScaleFromVoltage(float voltageScale) {
//     return cmn_CalibFullScaleFromVoltage(voltageScale);
// }

// /**
//  * @brief Calibrates ADC/DAC/Buffer counts and checks for limits
//  *
//  * Function is used to publish captured signal data to external world in calibrated +- units.
//  * Calculation is based on ADC/DAC inputs and calibrated and user defined DC offsets.
//  *
//  * @param[in] field_len Number of field (ADC/DAC/Buffer) bits
//  * @param[in] cnts Captured Signal Value, expressed in ADC/DAC counts
//  * @param[in] calib_dc_off Calibrated DC offset, specified in ADC/DAC counts
//  * @retval Calibrated counts
//  */

// int32_t cmn_CalibCnts(uint32_t field_len, uint32_t cnts, int calib_dc_off)
// {
//     int32_t m;

//     /* check sign */
//     if(cnts & (1 << (field_len - 1))) {
//         /* negative number */
//         m = -1 *((cnts ^ ((1 << field_len) - 1)) + 1);
//     } else {
//         /* positive number */
//         m = cnts;
//     }

//     /* adopt ADC count with calibrated DC offset */
//     m -= calib_dc_off;

//     /* check limits */
//     if(m < (-1 * (1 << (field_len - 1))))
//         m = (-1 * (1 << (field_len - 1)));
//     else if(m > (1 << (field_len - 1)))
//         m = (1 << (field_len - 1));

//     return m;
// }

// /*----------------------------------------------------------------------------*/
// /**
//  * @brief Converts ADC/DAC/Buffer counts to voltage [V]
//  *
//  * Function is used to publish captured signal data to external world in user units.
//  * Calculation is based on maximal voltage, which can be applied on ADC/DAC inputs and
//  * calibrated and user defined DC offsets.
//  *
//  * @param[in] field_len Number of field (ADC/DAC/Buffer) bits
//  * @param[in] cnts Captured Signal Value, expressed in ADC/DAC counts
//  * @param[in] adc_max_v Maximal ADC/DAC voltage, specified in [V]
//  * @param[in] calibScale Calibration scale factor, specified in [V]
//  * @param[in] user_dc_off User specified DC offset, specified in [V]
//  * @retval float Signal Value, expressed in user units [V]
//  */

// float cmn_CnvCalibCntToV(uint32_t field_len, int32_t calib_cnts, float adc_max_v, float calibScale, float user_dc_off,double full_scale_norm)
// {
//     /* map ADC counts into user units */
//     double ret_val = ((double)calib_cnts * adc_max_v / (double)(1 << (field_len - 1)));

//     /* and adopt the calculation with user specified DC offset */
//     ret_val += user_dc_off;
//     /* adopt the calculation with calibration scaling */
//     ret_val *= (double)calibScale / (full_scale_norm/(double)adc_max_v);

//     return ret_val;
// }

// /*----------------------------------------------------------------------------*/
// /**
//  * @brief Converts ADC/DAC/Buffer counts to voltage [V]
//  *
//  * Function is used to publish captured signal data to external world in user units.
//  * Calculation is based on maximal voltage, which can be applied on ADC/DAC inputs and
//  * calibrated and user defined DC offsets.
//  *
//  * @param[in] field_len Number of field (ADC/DAC/Buffer) bits
//  * @param[in] cnts Captured Signal Value, expressed in ADC/DAC counts
//  * @param[in] adc_max_v Maximal ADC/DAC voltage, specified in [V]
//  * @param[in] calibScale Calibration scale factor, specified in [full scale] - EPROM calibration parameter storage format
//  * @param[in] calib_dc_off Calibrated DC offset, specified in ADC/DAC counts
//  * @param[in] user_dc_off User specified DC offset, specified in [V]
//  * @retval float Signal Value, expressed in user units [V]
//  */

// float cmn_CnvCntToV(uint32_t field_len, uint32_t cnts, float adc_max_v, uint32_t calibScale, int calib_dc_off, float user_dc_off)
// {
//     int32_t calib_cnts = cmn_CalibCnts(field_len, cnts, calib_dc_off);

//     return cmn_CnvCalibCntToV(field_len, calib_cnts, adc_max_v, cmn_CalibFullScaleToVoltage(calibScale), user_dc_off,FULL_SCALE_NORM);
// }

// float cmn_CnvNormCntToV(uint32_t field_len, uint32_t cnts, float adc_max_v, uint32_t calibScale, int calib_dc_off, float user_dc_off,double full_scale_norm){
//     int32_t calib_cnts = cmn_CalibCnts(field_len, cnts, calib_dc_off);

//     return cmn_CnvCalibCntToV(field_len, calib_cnts, adc_max_v, cmn_CalibFullScaleToVoltage(calibScale), user_dc_off,full_scale_norm);
// }

// float rp_cmn_CnvCntToV(uint32_t field_len, uint32_t cnts, float adc_max_v, uint32_t calibScale, int calib_dc_off, float user_dc_off) {
// 	return cmn_CnvCntToV(field_len, cnts, adc_max_v, calibScale, calib_dc_off, user_dc_off);
// }
// /**
//  * @brief Converts voltage in [V] to ADC/DAC/Buffer counts
//  *
//  * Function is used for setting up trigger threshold value, which is written into
//  * appropriate FPGA register. This value needs to be specified in ADC/DAC counts, while
//  * user specifies this information in Voltage. The resulting value is based on the
//  * specified threshold voltage, maximal ADC/DAC voltage, calibrated and user specified
//  * DC offsets.
//  *
//  * @param[in] field_len Number of field (ADC/DAC/Buffer) bits
//  * @param[in] voltage Voltage, specified in [V]
//  * @param[in] adc_max_v Maximal ADC/DAC voltage, specified in [V]
//  * @param[in] calibFS_LO True if calibrating for front size (out) low voltage
//  * @param[in] calibScale Calibration scale factor. If zero -> no scaling, specified in [full scale] - EPROM calibration parameter storage format
//  * @param[in] calib_dc_off Calibrated DC offset, specified in ADC/DAC counts
//  * @param[in] user_dc_off User specified DC offset, , specified in [V]
//  * @retval int ADC/DAC counts
//  */
// uint32_t cmn_CnvVToCnt(uint32_t field_len, float voltage, float adc_max_v, bool calibFS_LO, uint32_t calib_scale, int calib_dc_off, float user_dc_off)
// {
//     int adc_cnts = 0;

//     /* adopt the calculation with calibration scaling. If 0 ->  no calibration */
//     if (calib_scale != 0) {
//         voltage /= (float) cmn_CalibFullScaleToVoltage(calib_scale) / (float)((!calibFS_LO) ? 1.f : (FULL_SCALE_NORM/adc_max_v));
// //        voltage /= (float) cmn_CalibFullScaleToVoltage(calib_scale) / (float)(FULL_SCALE_NORM/adc_max_v);
//     }

//     /* check and limit the specified voltage arguments towards */
//     /* maximal voltages which can be applied on ADC inputs */
//     if(voltage > adc_max_v)
//         voltage = adc_max_v;
//     else if(voltage < -adc_max_v)
//         voltage = -adc_max_v;

//     /* adopt the specified voltage with user defined DC offset */
//     voltage -= user_dc_off;

//     /* map voltage units into FPGA adc counts */
//     adc_cnts = (int)round(voltage * (float) (1 << field_len) / (2 * adc_max_v));

//     /* adopt calculated ADC counts with calibration DC offset */
//     adc_cnts += calib_dc_off;

//     /* check and limit the specified cnt towards */
//     /* maximal cnt which can be applied on ADC inputs */
//     if(adc_cnts > (1 << (field_len - 1)) - 1)
//         adc_cnts = (1 << (field_len - 1)) - 1;
//     else if(adc_cnts < -(1 << (field_len - 1)))
//         adc_cnts = -1 << (field_len - 1);

//     /* if negative remove higher bits that represent negative number */
//     if (adc_cnts < 0)
//         adc_cnts = adc_cnts & ((1<<field_len)-1);

//     return (uint32_t)adc_cnts;
// }

// uint32_t rp_cmn_CnvVToCnt(uint32_t field_len, float voltage, float adc_max_v, bool calibFS_LO, uint32_t calib_scale, int calib_dc_off, float user_dc_off) {
// 	return cmn_CnvVToCnt(field_len, voltage, adc_max_v, calibFS_LO, calib_scale, calib_dc_off, user_dc_off);
// }
