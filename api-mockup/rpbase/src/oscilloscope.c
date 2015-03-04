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

#include <stdio.h>
#include <sys/types.h>

#include "version.h"
#include "common.h"
#include "oscilloscope.h"

// Base Oscilloscope address
static const int OSC_BASE_ADDR = 0x40100000;
static const int OSC_BASE_SIZE = 0x50000;

// Oscilloscope Channel A input signal buffer offset
#define OSC_CHA_OFFSET 0x10000

// Oscilloscope Channel B input signal buffer offset
#define OSC_CHB_OFFSET 0x20000

//Oscilloscope Channel A accumulated signal buffer offset
#define OSC_ACC_CHA_OFFSET 0x30000

//Oscilloscope Channel B accumulated signal buffer offset
#define OSC_ACC_CHB_OFFSET 0x40000

// Oscilloscope signal A and B length
#define OSC_SIG_LEN (16*1024)

// Oscilloscope structure declaration
typedef struct osc_control_s {

    /** @brief Offset 0x00 - configuration register
     *
     * Configuration register (offset 0x00):
     * bit [0] - arm_trigger
     * bit [1] - rst_wr_state_machine
     * bits [31:2] - reserved
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

    uint32_t reseved; // Empty space...

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

    /**@brief Acumulation contro/status:
    * bits [31:2] reserved
    * bits [1] Accumulation run:
    * writing an 1 will start accumulating data, 
    * writing 0 should not be used, the main 
    * configuration register should be used for reset
    * reading will show the status of the 
    * accumulation process (1 – running, 0 - finished)
    */
    uint32_t ac_ctrl_stat;

    /**@brief Accumulation counter:
    * bits [31:0] 
    * on write: the number of accumulated 
    * samples is specified
    * on read: the status of the increment counter
    * is provided for monitoring 
    * (0x0 – accumulate 1 sample, 0xffffffff – accumulate 4294967296 samples)
    */
    uint32_t ac_count;

    /**@brief Accumulator output shift:
    * bits [31:4] reserved 
    * bits [3:0] specifies how many LSBbits from the
    * accumulated storaged (up to 48bits) are removed
    * before reaching software, which is limited to 32 bits
    */
    uint32_t ac_out_sft;

    /**@brief Accumulator data sequence length
    * bits [31:14] reserved
    * bits [13:0] specifies the number of samples accumulated after a
    * trigger. It is limited by the length of the accumulation buffer to
    * 2^14 locations (0x0 - sequence of 1 sample, 0xffffffff - sequence
    * of 16384 samples)
    */
    uint32_t ac_data_seq_len;

    /**@brief Accumulator data offset corection ChA
    * bits [31:14] reserved
    * bits [13:0] signed offset value
    */
    uint32_t ac_off_cha;

    /**@brief Accumulator data offset corection ChB
    * bits [31:14] reserved
    * bits [13:0] signed offset value
    */
    uint32_t ac_off_chb;

    /* ChA & ChB data - 14 LSB bits valid starts from 0x10000 and
     * 0x20000 and are each 16k samples long */

    /* ChA & ChB accumulated memory data. Starts from 0x30000 and 
     * 0x40000 and are each 16k samples long */ 
} osc_control_t;


// The FPGA register structure for oscilloscope
static volatile osc_control_t *osc_reg = NULL;

// The FPGA input signal buffer pointer for channel A
static volatile uint32_t *osc_cha = NULL;

// The FPGA input signal buffer pointer for channel B
static volatile uint32_t *osc_chb = NULL;

// The FPGA accumulated signal buffer pointer for channel A
static volatile uint32_t *osc_dp_avg_cha = NULL;

// The FPGA accumulated signal buffer pointer for channel B
static volatile uint32_t *osc_dp_avg_chb = NULL;


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
static const uint32_t AC_DATA_SEQ_MASK      = 0x3FFF;       // (14 bits)
static const uint32_t SHIFT_MASK            = 0xF;          // (4 bits)
static const uint32_t COUNT_MASK            = 0xFFFFFFFF;   // (32 bits)
static const uint32_t DEB_TIM_MASK          = 0xFFFFF;      // (20 bits)
static const uint32_t AC_CTRL_STAT          = 0x3;          // (2 bits)


/**
 * general
 */

int osc_Init()
{
    ECHECK(cmn_Init());
    ECHECK(cmn_Map(OSC_BASE_SIZE, OSC_BASE_ADDR, (void**)&osc_reg));
    osc_cha        = (uint32_t*)((char*)osc_reg + OSC_CHA_OFFSET);
    osc_chb        = (uint32_t*)((char*)osc_reg + OSC_CHB_OFFSET);
    osc_dp_avg_cha = (uint32_t*)((char*)osc_reg + OSC_ACC_CHA_OFFSET);
    osc_dp_avg_chb = (uint32_t*)((char*)osc_reg + OSC_ACC_CHB_OFFSET);

    //Test
    printf("osc_reg address: %p || dp avg channel a address: %p\n", &osc_reg->conf, osc_dp_avg_cha);
    return RP_OK;
}

int osc_Release()
{
    ECHECK(cmn_Unmap(OSC_BASE_SIZE, (void**)&osc_reg));
    osc_cha = NULL;
    osc_chb = NULL;
    osc_dp_avg_cha = NULL;
    osc_dp_avg_chb = NULL;

    ECHECK(cmn_Release());
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
 * Deep averaging
 */
int osc_SetDeepAvgCount(uint32_t count){
    cmn_SetValue(&osc_reg->ac_count, count, COUNT_MASK);
    return RP_OK;
}

int osc_SetDeepAvgShift(uint32_t shift){
    ECHECK(cmn_SetValue(&osc_reg->ac_out_sft, shift, SHIFT_MASK));
    return RP_OK;
}

int osc_SetDeepDataSeqLen(uint32_t len){
    //Writing len-1 because we write one bit less in register.
    ECHECK(cmn_SetValue(&osc_reg->ac_data_seq_len, len, AC_DATA_SEQ_MASK));
    return RP_OK;
}

int osc_SetDeepAvgDebTim(uint32_t deb_t){
    ECHECK(cmn_SetValue(&osc_reg->trig_dbc_t, deb_t, DEB_TIM_MASK));
    return RP_OK;
}

int osc_GetDeepAvgCount(uint32_t *count){
    return cmn_GetValue(&osc_reg->ac_count, count, COUNT_MASK);
}

int osc_GetDeepAvgShift(uint32_t *shift){
    return cmn_GetValue(&osc_reg->ac_out_sft, shift, SHIFT_MASK);
}

int osc_GetDeepDataSeqLen(uint32_t *len){
    return cmn_GetValue(&osc_reg->ac_data_seq_len, len, AC_DATA_SEQ_MASK);
}

int osc_GetDeepAvgDebTim(uint32_t *deb_t){
    return cmn_GetValue(&osc_reg->trig_dbc_t, deb_t, DEB_TIM_MASK);
}

int osc_GetDeepAvgRunState(uint32_t *run){
    return cmn_GetValue(&osc_reg->ac_ctrl_stat, run, AC_CTRL_STAT);
}


/* osc_WriteDataIntoMemoryDeepAvg
 * First write enable and then write run
 * Must be set separately 
 */
int osc_WriteDataIntoMemoryDeepAvg(bool enable){

    if(enable){
        cmn_SetBits(&osc_reg->ac_ctrl_stat, 0x1, AC_CTRL_STAT);
        cmn_SetBits(&osc_reg->ac_ctrl_stat, 0x2, AC_CTRL_STAT);
    }else{
        cmn_UnsetBits(&osc_reg->ac_ctrl_stat, 0x1, AC_CTRL_STAT);
        cmn_UnsetBits(&osc_reg->ac_ctrl_stat, 0x2, AC_CTRL_STAT);
    }
    return RP_OK;
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

const volatile uint32_t* osc_GetDeepAvgDataBufferChA(){
    return osc_dp_avg_cha;
}

const volatile uint32_t* osc_GetDeepAvgDataBufferChB(){
    return osc_dp_avg_chb;
}
