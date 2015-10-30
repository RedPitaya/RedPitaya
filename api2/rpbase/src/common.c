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

#include "common.h"

static int fd = NULL;

int cmn_Init()
{
    if (!fd) {
        if((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) {
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

int cmn_SetShiftedValue(volatile uint32_t* field, uint32_t value, uint32_t mask, uint32_t bitsToSetShift)
{
    VALIDATE_BITS(value, mask);
    uint32_t currentValue;
    cmn_GetValue(field, &currentValue, 0xffffffff);
    currentValue &=  ~(mask << bitsToSetShift); // Clear all bits at specified location
    currentValue +=  (value << bitsToSetShift); // Set value at specified location
    SET_VALUE(*field, currentValue);
    return RP_OK;
}

int cmn_SetValue(volatile uint32_t* field, uint32_t value, uint32_t mask)
{
    return cmn_SetShiftedValue(field, value, mask, 0);
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

/*----------------------------------------------------------------------------*/

/**
* @brief Converts scale voltage to calibration Full scale. Result is usually written to EPROM calibration parameters.
*
* @param[in] voltageScale Scale value in voltage
* @retval Scale in volts
*/
uint32_t cmn_CalibFullScaleFromVoltage(float voltageScale) {
    return (uint32_t) (voltageScale / 100.0 * ((uint64_t)1<<32));
}

/*----------------------------------------------------------------------------*/
/**
 * @brief Converts ADC/DAC/Buffer counts to voltage [V]
 *
 * Function is used to publish captured signal data to external world in user units.
 * Calculation is based on maximal voltage, which can be applied on ADC/DAC inputs and
 * calibrated and user defined DC offsets.
 *
 * @param[in] field_len Number of field (ADC/DAC/Buffer) bits
 * @param[in] cnts Captured Signal Value, expressed in ADC/DAC counts
 * @param[in] adc_max_v Maximal ADC/DAC voltage, specified in [V]
 * @retval float Signal Value, expressed in user units [V]
 */

static float cmn_CnvCalibCntToV(uint32_t field_len, int32_t calib_cnts, float adc_max_v)
{
    /* map ADC counts into user units */
    double ret_val = ((double)calib_cnts * adc_max_v / (double)(1 << (field_len - 1)));
    /* adopt the calculation with calibration scaling */
    ret_val *= 1 / ((double)FULL_SCALE_NORM/(double)adc_max_v);

    return ret_val;
}

/*----------------------------------------------------------------------------*/
/**
 * @brief Converts ADC/DAC/Buffer counts to voltage [V]
 *
 * Function is used to publish captured signal data to external world in user units.
 * Calculation is based on maximal voltage, which can be applied on ADC/DAC inputs and
 * calibrated and user defined DC offsets.
 *
 * @param[in] field_len Number of field (ADC/DAC/Buffer) bits
 * @param[in] cnts Captured Signal Value, expressed in ADC/DAC counts
 * @param[in] adc_max_v Maximal ADC/DAC voltage, specified in [V]
 * @retval float Signal Value, expressed in user units [V]
 */

float cmn_CnvCntToV(uint32_t field_len, uint32_t cnts, float adc_max_v)
{
    return cmn_CnvCalibCntToV(field_len, cnts, adc_max_v);
}

/**
 * @brief Converts voltage in [V] to ADC/DAC/Buffer counts
 *
 * Function is used for setting up trigger threshold value, which is written into
 * appropriate FPGA register. This value needs to be specified in ADC/DAC counts, while
 * user specifies this information in Voltage. The resulting value is based on the
 * specified threshold voltage, maximal ADC/DAC voltage, calibrated and user specified
 * DC offsets.
 *
 * @param[in] field_len Number of field (ADC/DAC/Buffer) bits
 * @param[in] voltage Voltage, specified in [V]
 * @param[in] adc_max_v Maximal ADC/DAC voltage, specified in [V]
 * @param[in] calibFS_LO True if calibrating for front size (out) low voltage
 * @retval int ADC/DAC counts
 */
uint32_t cmn_CnvVToCnt(uint32_t field_len, float voltage, float adc_max_v, bool calibFS_LO)
{
    int adc_cnts = 0;

    voltage /= 1.0 / (float)((!calibFS_LO) ? 1.f : (FULL_SCALE_NORM/adc_max_v));

    /* check and limit the specified voltage arguments towards */
    /* maximal voltages which can be applied on ADC inputs */
    if(voltage > adc_max_v)
        voltage = adc_max_v;
    else if(voltage < -adc_max_v)
        voltage = -adc_max_v;

    /* map voltage units into FPGA adc counts */
    adc_cnts = (int)round(voltage * (float) (1 << field_len) / (2 * adc_max_v));

    /* check and limit the specified cnt towards */
    /* maximal cnt which can be applied on ADC inputs */
    if(adc_cnts > (1 << (field_len - 1)) - 1)
        adc_cnts = (1 << (field_len - 1)) - 1;
    else if(adc_cnts < -(1 << (field_len - 1)))
        adc_cnts = -1 << (field_len - 1);

    /* if negative remove higher bits that represent negative number */
    if (adc_cnts < 0)
        adc_cnts = adc_cnts & ((1<<field_len)-1);

    return (uint32_t)adc_cnts;
}

