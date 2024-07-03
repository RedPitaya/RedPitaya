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
#include "rp.h"

int fd = -1;

bool g_DebugReg = false;

int cmn_Init()
{
    if (fd == -1) {
        if((fd = open("/dev/uio/api", O_RDWR | O_SYNC)) == -1) {
            return RP_EOMD;
        }
    }
    return RP_OK;
}

int cmn_Release()
{
    if (fd == -1) {
        if(close(fd) < 0) {
            return RP_ECMD;
        }
    }
    fd = -1;
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

void cmn_enableDebugReg(){
    g_DebugReg = true;
}

int  cmn_isEnableDebugReg(){
    return g_DebugReg;
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
        ERROR("Convert from %d",ch);
        assert(false);
    }
    return RP_EOOR;
}

rp_channel_t convertChFromIndex(uint8_t index){
    if (index == 0)  return RP_CH_1;
    if (index == 1)  return RP_CH_2;
    if (index == 2)  return RP_CH_3;
    if (index == 3)  return RP_CH_4;

    ERROR("Convert from %d",index);
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
        ERROR("Convert from PIN %d",pin);
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
        ERROR("Convert from %d",ch);
        assert(false);
    }
    return RP_EOOR;
}


uint32_t cmn_convertToCnt(float voltage,uint8_t bits,float fullScale,bool is_signed,double gain, int32_t offset){
    uint32_t mask = ((uint64_t)1 << bits) - 1;

    if (gain == 0){
        ERROR("convertToCnt devide by zero");
        assert(false);
    }

    if (fullScale == 0){
        ERROR("convertToCnt devide by zero");
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

int cmn_GetReservedMemory(uint32_t *_startAddress,uint32_t *_size){
    *_startAddress = 0;
    *_size = 0;
    int fd = 0;
    if((fd = open("/sys/firmware/devicetree/base/reserved-memory/buffer@1000000/reg", O_RDONLY)) == -1) {
        fprintf(stderr,"[FATAL ERROR] Error open: /sys/firmware/devicetree/base/reserved-memory/buffer@1000000/reg\n");
        return RP_EOMD;
    }
    char data[8];
    int sz = read(fd, &data, 8);

    if (close(fd) < 0) {
        return RP_EOMD;
    }
    if (sz == 8){
        *_startAddress = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
        *_size = data[4] << 24 | data[5] << 16 | data[6] << 8 | data[7];
    }else{
        return RP_EOMD;
    }
    return RP_OK;
}
