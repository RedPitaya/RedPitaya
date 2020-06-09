/**
 * $Id: $
 *
 * @brief Red Pitaya library oscilloscope module implementation
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include "common.h"
#include "oscilloscope.h"
#include "rp_cross.h"
// The FPGA register structure for oscilloscope
static volatile osc_control_t *osc_reg = NULL;

// The FPGA input signal buffer pointer for channel A
static volatile uint32_t *osc_cha = NULL;

// The FPGA input signal buffer pointer for channel B
static volatile uint32_t *osc_chb = NULL;


/**
 * general
 */

int osc_Init()
{
    cmn_Map(OSC_BASE_SIZE, OSC_BASE_ADDR, (void**)&osc_reg);
    osc_cha = (uint32_t*)((char*)osc_reg + OSC_CHA_OFFSET);
    osc_chb = (uint32_t*)((char*)osc_reg + OSC_CHB_OFFSET);
    return RP_OK;
}

int osc_Release()
{
    cmn_Unmap(OSC_BASE_SIZE, (void**)&osc_reg);
    osc_cha = NULL;
    osc_chb = NULL;
    return RP_OK;
}


/**
 * decimation
 */

int osc_SetDecimation(uint32_t decimation)
{
    return cmn_SetValue(&osc_reg->data_dec, decimation, DATA_DEC_MASK);
}

int osc_GetDecimation(uint32_t* decimation)
{
    return cmn_GetValue(&osc_reg->data_dec, decimation, DATA_DEC_MASK);
}

int osc_SetAveraging(bool enable)
{
    if (enable) {
        return cmn_SetBits(&osc_reg->other, 0x1, DATA_AVG_MASK);
    }
    else {
        return cmn_UnsetBits(&osc_reg->other, 0x1, DATA_AVG_MASK);
    }
}

int osc_GetAveraging(bool* enable)
{
    return cmn_AreBitsSet(osc_reg->other, 0x1, DATA_AVG_MASK, enable);
}

/**
 * trigger source
 */

int osc_SetTriggerSource(uint32_t source)
{
    return cmn_SetValue(&osc_reg->trig_source, source, TRIG_SRC_MASK);
}

int osc_GetTriggerSource(uint32_t* source)
{
    return cmn_GetValue(&osc_reg->trig_source, source, TRIG_SRC_MASK);
}

int osc_WriteDataIntoMemory(bool enable)
{
    if (enable) {
        return cmn_SetBits(&osc_reg->conf, 0x1, START_DATA_WRITE_MASK);
    }
    else {
        return cmn_UnsetBits(&osc_reg->conf, 0x1, START_DATA_WRITE_MASK);
    }
}

int osc_ResetWriteStateMachine()
{
    return cmn_SetBits(&osc_reg->conf, (0x1 << 1), RST_WR_ST_MCH_MASK);
}

int osc_SetArmKeep(bool enable)
{
    if (enable)
        return cmn_SetBits(&osc_reg->conf, 0x8, ARM_KEEP_MASK);
    else
        return cmn_UnsetBits(&osc_reg->conf, 0x8, ARM_KEEP_MASK);
}

int osc_GetArmKeep(bool *state){
    return cmn_AreBitsSet(osc_reg->conf, 0x8 , ARM_KEEP_MASK, state);
}

int osc_GetBufferFillState(bool *state){
    return cmn_AreBitsSet(osc_reg->conf, 0x10 , FILL_STATE_MASK, state);
}

int osc_GetTriggerState(bool *received)
{
    return cmn_AreBitsSet(osc_reg->conf, (0x1 << 2), TRIG_ST_MCH_MASK, received);
}

int osc_GetPreTriggerCounter(uint32_t *value)
{
    return cmn_GetValue(&osc_reg->pre_trigger_counter, value, PRE_TRIGGER_COUNTER);
}

/**
 * trigger delay
 */

int osc_SetTriggerDelay(uint32_t decimated_data_num)
{
    return cmn_SetValue(&osc_reg->trigger_delay, decimated_data_num, TRIG_DELAY_MASK);
}

int osc_GetTriggerDelay(uint32_t* decimated_data_num)
{
    return cmn_GetValue(&osc_reg->trigger_delay, decimated_data_num, TRIG_DELAY_MASK);
}

/**
 * Threshold
 */

int osc_SetThresholdChA(uint32_t threshold)
{
    return cmn_SetValue(&osc_reg->cha_thr, threshold, THRESHOLD_MASK);
}

int osc_GetThresholdChA(uint32_t* threshold)
{
    return cmn_GetValue(&osc_reg->cha_thr, threshold, THRESHOLD_MASK);
}

int osc_SetThresholdChB(uint32_t threshold)
{
    return cmn_SetValue(&osc_reg->chb_thr, threshold, THRESHOLD_MASK);
}

int osc_GetThresholdChB(uint32_t* threshold)
{
    return cmn_GetValue(&osc_reg->chb_thr, threshold, THRESHOLD_MASK);
}

/**
 * Hysteresis
 */
int osc_SetHysteresisChA(uint32_t hysteresis)
{
    return cmn_SetValue(&osc_reg->cha_hystersis, hysteresis, HYSTERESIS_MASK);
}

int osc_GetHysteresisChA(uint32_t* hysteresis)
{
    return cmn_GetValue(&osc_reg->cha_hystersis, hysteresis, HYSTERESIS_MASK);
}

int osc_SetHysteresisChB(uint32_t hysteresis)
{
    return cmn_SetValue(&osc_reg->chb_hystersis, hysteresis, HYSTERESIS_MASK);
}

int osc_GetHysteresisChB(uint32_t* hysteresis)
{
    return cmn_GetValue(&osc_reg->chb_hystersis, hysteresis, HYSTERESIS_MASK);
}

/**
 * Equalization filters
 */
int osc_SetEqFiltersChA(uint32_t coef_aa, uint32_t coef_bb, uint32_t coef_kk, uint32_t coef_pp)
{
    cmn_SetValue(&osc_reg->cha_filt_aa, coef_aa, EQ_FILTER_AA);
    cmn_SetValue(&osc_reg->cha_filt_bb, coef_bb, EQ_FILTER);
    cmn_SetValue(&osc_reg->cha_filt_kk, coef_kk, EQ_FILTER);
    cmn_SetValue(&osc_reg->cha_filt_pp, coef_pp, EQ_FILTER);
    return RP_OK;
}

int osc_GetEqFiltersChA(uint32_t* coef_aa, uint32_t* coef_bb, uint32_t* coef_kk, uint32_t* coef_pp)
{
    cmn_GetValue(&osc_reg->cha_filt_aa, coef_aa, EQ_FILTER_AA);
    cmn_GetValue(&osc_reg->cha_filt_bb, coef_bb, EQ_FILTER);
    cmn_GetValue(&osc_reg->cha_filt_kk, coef_kk, EQ_FILTER);
    cmn_GetValue(&osc_reg->cha_filt_pp, coef_pp, EQ_FILTER);
    return RP_OK;
}

int osc_SetEqFiltersChB(uint32_t coef_aa, uint32_t coef_bb, uint32_t coef_kk, uint32_t coef_pp)
{
    cmn_SetValue(&osc_reg->chb_filt_aa, coef_aa, EQ_FILTER_AA);
    cmn_SetValue(&osc_reg->chb_filt_bb, coef_bb, EQ_FILTER);
    cmn_SetValue(&osc_reg->chb_filt_kk, coef_kk, EQ_FILTER);
    cmn_SetValue(&osc_reg->chb_filt_pp, coef_pp, EQ_FILTER);
    return RP_OK;
}

int osc_GetEqFiltersChB(uint32_t* coef_aa, uint32_t* coef_bb, uint32_t* coef_kk, uint32_t* coef_pp)
{
    cmn_GetValue(&osc_reg->chb_filt_aa, coef_aa, EQ_FILTER_AA);
    cmn_GetValue(&osc_reg->chb_filt_bb, coef_bb, EQ_FILTER);
    cmn_GetValue(&osc_reg->chb_filt_kk, coef_kk, EQ_FILTER);
    cmn_GetValue(&osc_reg->chb_filt_pp, coef_pp, EQ_FILTER);
    return RP_OK;
}

/**
 * Write pointer
 */
int osc_GetWritePointer(uint32_t* pos)
{
    return cmn_GetValue(&osc_reg->wr_ptr_cur, pos, WRITE_POINTER_MASK);
}

int osc_GetWritePointerAtTrig(uint32_t* pos)
{
    return cmn_GetValue(&osc_reg->wr_ptr_trigger, pos, WRITE_POINTER_MASK);
}

/**
 * Raw buffers
 */
const volatile uint32_t* osc_GetDataBufferChA()
{
    return osc_cha;
}

const volatile uint32_t* osc_GetDataBufferChB()
{
    return osc_chb;
}
