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

// Base Oscilloscope address
static const int OSC_BASE_ADDR = 0x40100000;
static const int OSC_BASE_SIZE = 0x30000;

// Oscilloscope Channel A input signal buffer offset
#define OSC_CHA_OFFSET 0x10000

// Oscilloscope Channel B input signal buffer offset
#define OSC_CHB_OFFSET 0x20000

// Oscilloscope structure declaration
typedef struct osc_control_s {

    /** @brief Offset 0x00 - configuration register
     *
     * Configuration register (offset 0x00):
     * bit [0] - arm_trigger
     * bit [1] - rst_wr_state_machine
     * bit [2] - trigger_status
     * bit [3] - arm_keep
     * bits [31:4] - reserved
     */
    uint32_t conf;

    /** @brief Offset 0x04 - trigger source register
     *
     * Trigger source register (offset 0x04):
     * bits [ 2 : 0] - trigger source:
     * 1 - trig immediately
     * 2 - ChA positive edge
     * 3 - ChA negative edge
     * 4 - ChB positive edge
     * 5 - ChB negative edge
     * 6 - External trigger 0
     * 7 - External trigger 1
     * bits [31 : 3] -reserved
     */
    uint32_t trig_source;

    /** @brief Offset 0x08 - Channel A threshold register
     *
     * Channel A threshold register (offset 0x08):
     * bits [13: 0] - ChA threshold
     * bits [31:14] - reserved
     */
    uint32_t cha_thr;

    /** @brief Offset 0x0C - Channel B threshold register
     *
     * Channel B threshold register (offset 0x0C):
     * bits [13: 0] - ChB threshold
     * bits [31:14] - reserved
     */
    uint32_t chb_thr;

    /** @brief Offset 0x10 - After trigger delay register
     *
     * After trigger delay register (offset 0x10)
     * bits [31: 0] - trigger delay
     * 32 bit number - how many decimated samples should be stored into a buffer.
     * (max 16k samples)
     */
    uint32_t trigger_delay;

    /** @brief Offset 0x14 - Data decimation register
     *
     * Data decimation register (offset 0x14):
     * bits [16: 0] - decimation factor, legal values:
     * 1, 8, 64, 1024, 8192 65536
     * If other values are written data is undefined
     * bits [31:17] - reserved
     */
    uint32_t data_dec;

    /** @brief Offset 0x18 - Current write pointer register
     *
     * Current write pointer register (offset 0x18), read only:
     * bits [13: 0] - current write pointer
     * bits [31:14] - reserved
     */
    uint32_t wr_ptr_cur;

    /** @brief Offset 0x1C - Trigger write pointer register
     *
     * Trigger write pointer register (offset 0x1C), read only:
     * bits [13: 0] - trigger pointer (pointer where trigger was detected)
     * bits [31:14] - reserved
     */
    uint32_t wr_ptr_trigger;

    /** @brief ChA & ChB hysteresis - both of the format:
     * bits [13: 0] - hysteresis threshold
     * bits [31:14] - reserved
     */
    uint32_t cha_hystersis;
    uint32_t chb_hystersis;

    /** @brief
     * bits [0] - enable signal average at decimation
     * bits [31:1] - reserved
     */
    uint32_t other;

    /** @brief - Pre Trigger counter
     *
     * Pre Trigger counter (offset 0x2C)
     * bits [31: 0] - Pre Trigger counter
     * 32 bit number - how many decimated samples have been stored into a buffer
     * before trigger arrived.
     */
    uint32_t pre_trigger_counter;

    /** @brief ChA Equalization filter
     * bits [17:0] - AA coefficient (pole)
     * bits [31:18] - reserved
     */
    uint32_t cha_filt_aa;

    /** @brief ChA Equalization filter
     * bits [24:0] - BB coefficient (zero)
     * bits [31:25] - reserved
     */
    uint32_t cha_filt_bb;

    /** @brief ChA Equalization filter
     * bits [24:0] - KK coefficient (gain)
     * bits [31:25] - reserved
     */
    uint32_t cha_filt_kk;

    /** @brief ChA Equalization filter
     * bits [24:0] - PP coefficient (pole)
     * bits [31:25] - reserved
     */
    uint32_t cha_filt_pp;

    /** @brief ChB Equalization filter
     * bits [17:0] - AA coefficient (pole)
     * bits [31:18] - reserved
     */
    uint32_t chb_filt_aa;

    /** @brief ChB Equalization filter
     * bits [24:0] - BB coefficient (zero)
     * bits [31:25] - reserved
     */
    uint32_t chb_filt_bb;

    /** @brief ChB Equalization filter
     * bits [24:0] - KK coefficient (gain)
     * bits [31:25] - reserved
     */
    uint32_t chb_filt_kk;

    /** @brief ChB Equalization filter
     * bits [24:0] - PP coefficient (pole)
     * bits [31:25] - reserved
     */
    uint32_t chb_filt_pp;

    /** @brief ChA AXI lower address
    * bits [31:0] - starting writing address
    */
    uint32_t cha_axi_low;

    /** @brief ChA AXI High address
    * bits [31:0] - starting writing address
    */
    uint32_t cha_axi_high;

    /** @brief ChA AXI delay after trigger
    * bits [31:0] - Number of decimated data 
    * after trig written into memory
    */
    uint32_t cha_trig_delay;

    /**@brief ChB AXI enable master
    * bits [0] Enable AXI master
    * bits [31:0] reserved
    */
    uint32_t cha_enable_axi_m;

    /**@brief ChA AXI write pointer trigger
    * Write pointer at time the trigger arrived
    */
    uint32_t cha_w_ptr_trig;

    /**@brief ChA AXI write pointer current
    * Current write pointer
    */
    uint32_t cha_w_ptr_curr;

    /* Reserved 0x68 & 0x6C */
    uint32_t reserved_2;
    uint32_t reserved_3;

    /** @brief ChB AXI lower address
    * bits [31:0] - starting writing address
    */
    uint32_t chb_axi_low;

    /** @brief ChB AXI High address
    * bits [31:0] - starting writing address
    */
    uint32_t chb_axi_high;

    /** @brief ChB AXI delay after trigger
    * bits [31:0] - Number of decimated data 
    * after trig written into memory
    */
    uint32_t chb_trig_delay;

    /**@brief ChB AXI enable master
    * bits [0] Enable AXI master
    * bits [31:0] reserved
    */
    uint32_t chb_enable_axi_m;

    /**@brief ChB AXI write pointer trigger
    * Write pointer at time the trigger arrived
    */
    uint32_t chb_w_ptr_trig;

    /**@brief ChB AXI write pointer current
    * Current write pointer
    */
    uint32_t chb_w_ptr_curr;

    /* Reserved 0x88 & 0x8C */
    uint32_t reserved_4;
    uint32_t reserved_5;

    /**@brief Trigger debuncer time
    * bits [19:0] Number of ADC clock periods 
    * trigger is disabled after activation
    * reset value is decimal 62500 
    * or equivalent to 0.5ms
    */
    uint32_t trig_dbc_t;

    /* ChA & ChB data - 14 LSB bits valid starts from 0x10000 and
     * 0x20000 and are each 16k samples long */
} osc_control_t;


// The FPGA register structure for oscilloscope
static volatile osc_control_t *osc_reg = NULL;

// The FPGA input signal buffer pointer for channel A
static volatile uint32_t *osc_cha = NULL;

// The FPGA input signal buffer pointer for channel B
static volatile uint32_t *osc_chb = NULL;


static const uint32_t DATA_DEC_MASK         = 0x1FFFF;      // (17 bits)
static const uint32_t DATA_AVG_MASK         = 0x1;          // (1 bit)
static const uint32_t TRIG_SRC_MASK         = 0xF;          // (4 bits)
static const uint32_t START_DATA_WRITE_MASK = 0x1;          // (1 bit)
static const uint32_t THRESHOLD_MASK        = 0x3FFF;       // (14 bits)
static const uint32_t HYSTERESIS_MASK       = 0x3FFF;       // (14 bits)
static const uint32_t TRIG_DELAY_MASK       = 0xFFFFFFFF;   // (32 bits)
static const uint32_t WRITE_POINTER_MASK    = 0x3FFF;       // (14 bits)
static const uint32_t EQ_FILTER_AA          = 0x3FFFF;      // (18 bits)
static const uint32_t EQ_FILTER             = 0x1FFFFFF;    // (25 bits)
static const uint32_t RST_WR_ST_MCH_MASK    = 0x2;          // (1st bit)
static const uint32_t TRIG_ST_MCH_MASK      = 0x4;          // (2st bit)
static const uint32_t PRE_TRIGGER_COUNTER   = 0xFFFFFFFF;   // (32 bit)
static const uint32_t ARM_KEEP_MASK         = 0xF;          // (4 bit)


/**
 * general
 */

int osc_Init()
{
//    ECHECK(cmn_Init());
    ECHECK(cmn_Map(OSC_BASE_SIZE, OSC_BASE_ADDR, (void**)&osc_reg));
    osc_cha = (uint32_t*)((char*)osc_reg + OSC_CHA_OFFSET);
    osc_chb = (uint32_t*)((char*)osc_reg + OSC_CHB_OFFSET);
    return RP_OK;
}

int osc_Release()
{
    ECHECK(cmn_Unmap(OSC_BASE_SIZE, (void**)&osc_reg));
    osc_cha = NULL;
    osc_chb = NULL;
//    ECHECK(cmn_Release());
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
    ECHECK(cmn_SetValue(&osc_reg->cha_filt_aa, coef_aa, EQ_FILTER_AA));
    ECHECK(cmn_SetValue(&osc_reg->cha_filt_bb, coef_bb, EQ_FILTER));
    ECHECK(cmn_SetValue(&osc_reg->cha_filt_kk, coef_kk, EQ_FILTER));
    ECHECK(cmn_SetValue(&osc_reg->cha_filt_pp, coef_pp, EQ_FILTER));
    return RP_OK;
}

int osc_GetEqFiltersChA(uint32_t* coef_aa, uint32_t* coef_bb, uint32_t* coef_kk, uint32_t* coef_pp)
{
    ECHECK(cmn_GetValue(&osc_reg->cha_filt_aa, coef_aa, EQ_FILTER_AA));
    ECHECK(cmn_GetValue(&osc_reg->cha_filt_bb, coef_bb, EQ_FILTER));
    ECHECK(cmn_GetValue(&osc_reg->cha_filt_kk, coef_kk, EQ_FILTER));
    ECHECK(cmn_GetValue(&osc_reg->cha_filt_pp, coef_pp, EQ_FILTER));
    return RP_OK;
}

int osc_SetEqFiltersChB(uint32_t coef_aa, uint32_t coef_bb, uint32_t coef_kk, uint32_t coef_pp)
{
    ECHECK(cmn_SetValue(&osc_reg->chb_filt_aa, coef_aa, EQ_FILTER_AA));
    ECHECK(cmn_SetValue(&osc_reg->chb_filt_bb, coef_bb, EQ_FILTER));
    ECHECK(cmn_SetValue(&osc_reg->chb_filt_kk, coef_kk, EQ_FILTER));
    ECHECK(cmn_SetValue(&osc_reg->chb_filt_pp, coef_pp, EQ_FILTER));
    return RP_OK;
}

int osc_GetEqFiltersChB(uint32_t* coef_aa, uint32_t* coef_bb, uint32_t* coef_kk, uint32_t* coef_pp)
{
    ECHECK(cmn_GetValue(&osc_reg->chb_filt_aa, coef_aa, EQ_FILTER_AA));
    ECHECK(cmn_GetValue(&osc_reg->chb_filt_bb, coef_bb, EQ_FILTER));
    ECHECK(cmn_GetValue(&osc_reg->chb_filt_kk, coef_kk, EQ_FILTER));
    ECHECK(cmn_GetValue(&osc_reg->chb_filt_pp, coef_pp, EQ_FILTER));
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
