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

static int fd = -1;

int cmn_Init()
{
    if (fd == -1) {
        if((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) {
            return RP_EOMD;
        }
    }
    return RP_OK;
}

int cmn_Release()
{
    if (fd != -1) {
        if(close(fd) < 0) {
            return RP_ECMD;
        }
        fd = -1;
    }

    return RP_OK;
}

int cmn_Map(size_t size, size_t offset, void** mapped)
{
    *mapped = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);

    if(mapped == (void *) -1) {
        return RP_EMMD;
    }

    return RP_OK;
}

int cmn_Unmap(size_t size, void** mapped)
{
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

/*----------------------------------------------------------------------------*/
/**
 * @brief Calibrates ADC/DAC/Buffer counts and checks for limits
 *
 * Function is used to publish captured signal data to external world in calibrated +- units.
 * Calculation is based on ADC/DAC inputs and calibrated and user defined DC offsets.
 *
 * @param[in] field_len Number of field (ADC/DAC/Buffer) bits
 * @param[in] cnts Captured Signal Value, expressed in ADC/DAC counts
 * @param[in] calib_dc_off Calibrated DC offset, specified in ADC/DAC counts
 * @retval Calibrated counts
 */

int32_t cmn_CalibCnts(uint32_t field_len, uint32_t cnts, int calib_dc_off)
{
    int32_t m;

    /* check sign */
    if(cnts & (1 << (field_len - 1))) {
        /* negative number */
        m = -1 *((cnts ^ ((1 << field_len) - 1)) + 1);
    } else {
        /* positive number */
        m = cnts;
    }

    /* adopt ADC count with calibrated DC offset */
    m += calib_dc_off;

    /* check limits */
    if(m < (-1 * (1 << (field_len - 1))))
        m = (-1 * (1 << (field_len - 1)));
    else if(m > (1 << (field_len - 1)))
        m = (1 << (field_len - 1));

    return m;
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
 * @param[in] user_dc_off User specified DC offset, specified in [V]
 * @retval float Signal Value, expressed in user units [V]
 */

float cmn_CnvCalibCntToV(uint32_t field_len, int32_t calib_cnts, float adc_max_v, float user_dc_off)
{
    /* map ADC counts into user units */
    float ret_val = (calib_cnts * adc_max_v / (float)(1 << (field_len - 1)));

    /* and adopt the calculation with user specified DC offset */
    ret_val += user_dc_off;

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
 * @param[in] calib_dc_off Calibrated DC offset, specified in ADC/DAC counts
 * @param[in] user_dc_off User specified DC offset, specified in [V]
 * @retval float Signal Value, expressed in user units [V]
 */

float cmn_CnvCntToV(uint32_t field_len, uint32_t cnts, float adc_max_v, int calib_dc_off, float user_dc_off)
{
    int32_t calib_cnts = cmn_CalibCnts(field_len, cnts, calib_dc_off);
    return cmn_CnvCalibCntToV(field_len, calib_cnts, adc_max_v, user_dc_off);
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
 * @param[in] calib_dc_off Calibrated DC offset, specified in ADC/DAC counts
 * @param[in] user_dc_off User specified DC offset, , specified in [V]
 * @retval int ADC/DAC counts
 */
uint32_t cmn_CnvVToCnt(uint32_t field_len, float voltage, float adc_max_v, int calib_dc_off, float user_dc_off)
{
    int adc_cnts = 0;

    /* check and limit the specified voltage arguments towards */
    /* maximal voltages which can be applied on ADC inputs */
    if(voltage > adc_max_v)
        voltage = adc_max_v;
    else if(voltage < -adc_max_v)
        voltage = -adc_max_v;

    /* adopt the specified voltage with user defined DC offset */
    voltage -= user_dc_off;

    /* map voltage units into FPGA adc counts */
    adc_cnts = (int)round(voltage * (float)((int)(1 << field_len)) / (2 * adc_max_v));

    /* clip to the highest value (we are dealing with 14 bits only) */
    if((voltage > 0) && (adc_cnts & (1 << (field_len - 1))))
        adc_cnts = (1 << (field_len - 1)) - 1;
    else
        adc_cnts = adc_cnts & ((1 << (field_len)) - 1);

    /* adopt calculated ADC counts with calibration DC offset */
    adc_cnts -= calib_dc_off;

    return (uint32_t)adc_cnts;
}
