/**
 * $Id: $
 *
 * @brief Red Pitaya library oscilloscope module interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef SRC_OSCILLOSCOPE_H_
#define SRC_OSCILLOSCOPE_H_

#include <stdbool.h>
#include <stdint.h>
#include "common.h"

// Base Oscilloscope address
static const int OSC_BASE_ADDR = 0x00100000;
static const int OSC_BASE_SIZE = 0x30000;
static const int OSC_BASE_ADDR_4CH = 0x00200000;

// Oscilloscope Channel A input signal buffer offset
#define OSC_CHA_OFFSET 0x10000

// Oscilloscope Channel B input signal buffer offset
#define OSC_CHB_OFFSET 0x20000

typedef struct {
    uint8_t start_write : 1;           // (W) start write
    uint8_t reset_state_machine : 1;   // (W) rst_wr_state_machine
    uint8_t trigger_status : 1;        // (R) trigger_status
    uint8_t arm_keep : 1;              // (W) arm_keep
    uint8_t all_data_written : 1;      // (R) All data written to buffer
    uint8_t enable_split_trigger : 1;  // Work only first channel
    uint8_t : 2;
    void print() volatile {
        printRegBit(" - %-39s = 0x%08X (%d)\n", "start_write", start_write);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "reset_state_machine", reset_state_machine);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "trigger_status", trigger_status);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "arm_keep", arm_keep);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "all_data_written", all_data_written);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "enable_split_trigger", enable_split_trigger);
    };
} config_ch_t;

typedef struct {
    config_ch_t config_ch[4];
} config_t;

typedef union {
    uint32_t reg_full;
    config_t reg;
} config_u_t;

typedef struct {
    uint8_t trig_source : 4;
    uint8_t trig_lock : 1;
    uint8_t : 3;
    void print() volatile {
        printRegBit(" - %-39s = 0x%08X (%d)\n", "trig_source", trig_source);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "trig_lock", trig_lock);
    };
} trig_source_t;

typedef union {
    uint32_t reg_full;
    trig_source_t reg[4];
} trig_source_u_t;

typedef struct {
    uint8_t trig_lock : 1;
    uint8_t : 7;
    void print() volatile { printRegBit(" - %-39s = 0x%08X (%d)\n", "trig_lock", trig_lock); };
} trig_lock_control_t;

typedef union {
    uint32_t reg_full;
    trig_lock_control_t reg[4];
} trig_lock_control_u_t;

typedef struct {
    uint8_t average : 1;
    uint8_t : 7;
    void print() volatile { printRegBit(" - %-39s = 0x%08X (%d)\n", "average", average); };
} trig_average_t;

typedef union {
    uint32_t reg_full;
    trig_average_t reg[4];
} trig_average_u_t;

typedef struct {
    uint32_t trig_armed_ch1 : 1;
    uint32_t : 1;
    uint32_t trig_has_arrived_ch1 : 1;
    uint32_t trig_remines_armed_ch1 : 1;
    uint32_t acq_delay_passed_ch1 : 1;
    uint32_t : 11;
    uint32_t trig_armed_ch2 : 1;
    uint32_t : 1;
    uint32_t trig_has_arrived_ch2 : 1;
    uint32_t trig_remines_armed_ch2 : 1;
    uint32_t acq_delay_passed_ch2 : 1;
    uint32_t : 11;

    void print() volatile {
        printRegBit(" - %-39s = 0x%08X (%d)\n", "trig_armed_ch1", trig_armed_ch1);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "trig_has_arrived_ch1", trig_has_arrived_ch1);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "trig_remines_armed_ch1", trig_remines_armed_ch1);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "acq_delay_passed_ch1", acq_delay_passed_ch1);

        printRegBit(" - %-39s = 0x%08X (%d)\n", "trig_armed_ch2", trig_armed_ch2);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "trig_has_arrived_ch2", trig_has_arrived_ch2);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "trig_remines_armed_ch2", trig_remines_armed_ch2);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "acq_delay_passed_ch2", acq_delay_passed_ch2);
    };
} axi_state_t;

typedef union {
    uint32_t reg_full;
    axi_state_t reg;
} axi_state_u_t;

typedef struct {
    uint32_t ext_trig_dbc : 20;
    uint32_t : 12;
    void print() volatile { printRegBit(" - %-39s = 0x%08X (%d)\n", "ext_trig_dbc", ext_trig_dbc); };
} ext_trig_dbc_t;

typedef union {
    uint32_t reg_full;
    ext_trig_dbc_t reg;
} ext_trig_dbc_u_t;

// Oscilloscope structure declaration
typedef struct osc_control_s {

    /** @brief Offset 0x00 - configuration register
     *
     * Configuration register (offset 0x00):
     * bit [0] - (W) arm_trigger  - ch1/common
     * bit [1] - (W) rst_wr_state_machine
     * bit [2] - (R) trigger_status
     * bit [3] - (W) arm_keep
     * bit [4] - (R) All data written to buffer
     * bit [5] - (R/W) Independent mode on
     * bits [7:6] - reserved
     * bit [8] - (W) arm_trigger - ch2
     * bit [9] - (W) rst_wr_state_machine
     * bit [10] - (R) trigger_status
     * bit [11] - (W) arm_keep
     * bit [12] - (R) All data written to buffer
     * bit [13] - (R/W) Independent mode on
     * bits [15:14] - reserved
     * bit [16] - (W) arm_trigger - ch3
     * bit [17] - (W) rst_wr_state_machine
     * bit [18] - (R) trigger_status
     * bit [19] - (W) arm_keep
     * bit [20] - (R) All data written to buffer
     * bit [21] - (R/W) Independent mode on
     * bits [23:22] - reserved
     * bit [24] - (W) arm_trigger  - ch4
     * bit [25] - (W) rst_wr_state_machine
     * bit [26] - (R) trigger_status
     * bit [27] - (W) arm_keep
     * bit [28] - (R) All data written to buffer
     * bit [29] - (R/W) Independent mode on
     * bits [31:30] - reserved
     */
    uint32_t config;  // Can cast to config_u_t

    /** @brief Offset 0x04 - trigger source register
     *
     * Trigger source register (offset 0x04):
     * bits [3 : 0] - trigger source ch1/common:
        Trigger source
        1 - trig immediately
        2 - ch A threshold positive edge
        3 - ch A threshold negative edge
        4 - ch B threshold positive edge
        5 - ch B threshold negative edge
        6 - external trigger positive edge - DIO0_P pin
        7 - external trigger negative edge
        8 - arbitrary wave generator application       positive edge
        9 - arbitrary wave generator application
        negative edge
        10- ch C threshold positive edge
        11- ch C threshold negative edge
        12- ch D threshold positive edge
        13- ch D threshold negative edge
     * bits [4] - Trigger lock state
     * bits [7 : 5] -reserved
     * bits [11 : 8] - trigger source ch2:
     * bits [12] - Trigger lock state
     * bits [15 : 13] -reserved
     * bits [19 : 16] - trigger source ch3:
     * bits [20] - Trigger lock state
     * bits [23 : 21] -reserved
     * bits [27 : 24] - trigger source ch4:
     * bits [28] - Trigger lock state
     * bits [31 : 29] -reserved
     */
    uint32_t trig_source;

    /** @brief Offset 0x08 - Channel A threshold register
     *
     * Channel A threshold register (offset 0x08):
     * for 125 and 250
     * bits [13: 0] - ChA threshold
     * bits [31:14] - reserved
     * for 122
     * bits [15: 0] - ChA threshold
     * bits [31:16] - reserved
     */
    uint32_t cha_thr;

    /** @brief Offset 0x0C - Channel B threshold register
     *
     * Channel B threshold register (offset 0x0C):
     * for 125 and 250
     * bits [13: 0] - ChB threshold
     * bits [31:14] - reserved
     * for 122
     * bits [15: 0] - ChB threshold
     * bits [31:16] - reserved
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

    /** @brief Offset 0x20 - ChA & ChB hysteresis - both of the format:
     * for 125 and 250
     * bits [13: 0] - ChB threshold
     * bits [31:14] - reserved
     * for 122
     * bits [15: 0] - ChB threshold
     * bits [31:16] - reserved
     */
    uint32_t cha_hystersis;
    uint32_t chb_hystersis;

    /** @brief Offset 0x28
     * bits [0] - enable signal average at decimation ch1
     * bits [7:1] - reserved
     * bits [8] - enable signal average at decimation ch2
     * bits [9:15] - reserved
     * bits [16] - enable signal average at decimation ch3
     * bits [23:17] - reserved
     * bits [24] - enable signal average at decimation ch4
     * bits [31:25] - reserved
     */
    uint32_t average;

    /** @brief Offset 0x2C - Pre Trigger counter
     *
     * Pre Trigger counter (offset 0x2C)
     * bits [31: 0] - Pre Trigger counter
     * 32 bit number - how many decimated samples have been stored into a buffer
     * before trigger arrived.
     */
    uint32_t pre_trigger_counter;

    /** @brief Offset 0x30 - ChA Equalization filter
     * bits [17:0] - AA coefficient (pole)
     * bits [31:18] - reserved
     */
    uint32_t cha_filt_aa;

    /** @brief Offset 0x34 - ChA Equalization filter
     * bits [24:0] - BB coefficient (zero)
     * bits [31:25] - reserved
     */
    uint32_t cha_filt_bb;

    /** @brief Offset 0x38 - ChA Equalization filter
     * bits [24:0] - KK coefficient (gain)
     * bits [31:25] - reserved
     */
    uint32_t cha_filt_kk;

    /** @brief Offset 0x3C - ChA Equalization filter
     * bits [24:0] - PP coefficient (pole)
     * bits [31:25] - reserved
     */
    uint32_t cha_filt_pp;

    /** @brief Offset 0x40 - ChB Equalization filter
     * bits [17:0] - AA coefficient (pole)
     * bits [31:18] - reserved
     */
    uint32_t chb_filt_aa;

    /** @brief Offset 0x44 - ChB Equalization filter
     * bits [24:0] - BB coefficient (zero)
     * bits [31:25] - reserved
     */
    uint32_t chb_filt_bb;

    /** @brief Offset 0x48 - ChB Equalization filter
     * bits [24:0] - KK coefficient (gain)
     * bits [31:25] - reserved
     */
    uint32_t chb_filt_kk;

    /** @brief Offset 0x4C - ChB Equalization filter
     * bits [24:0] - PP coefficient (pole)
     * bits [31:25] - reserved
     */
    uint32_t chb_filt_pp;

    /** @brief Offset 0x50 - CH A AXI lower address
     * bits [31:0] - start address of CH A AXI buffer
     */
    uint32_t cha_axi_addr_low;

    /** @brief Offset 0x54 - CH A AXI upper address
     * bits [31:0] - end address of CH A AXI buffer
     */
    uint32_t cha_axi_addr_high;

    /** @brief Offset 0x58 - CH A AXI delay after trigger
     * bits [31:0] - number of decimated data after the trigger is written to memory
     */
    uint32_t cha_axi_delay;

    /** @brief Offset 0x5C - CH A AXI enable master
     * bits [0] - enable AXI master
     * bits [31:1] - reserved
     */
    uint32_t cha_axi_enable;

    /** @brief Offset 0x60 - CH A AXI write pointer - trigger
     * bits [31:0] - write pointer at the moment the trigger arrives
     */
    uint32_t cha_axi_wr_ptr_trigger;

    /** @brief Offset 0x64 - CH A AXI write pointer - current
     * bits [31:0] - current write pointer
     */
    uint32_t cha_axi_wr_ptr_cur;

    /** @brief Offset 0x68 - reserved
     */
    uint32_t reserved_68[2];

    /** @brief Offset 0x70 - CH B AXI lower address
     * bits [31:0] - start address of CH B AXI buffer
     */
    uint32_t chb_axi_addr_low;

    /** @brief Offset 0x74 - CH B AXI upper address
     * bits [31:0] - end address of CH B AXI buffer
     */
    uint32_t chb_axi_addr_high;

    /** @brief Offset 0x78 - CH B AXI delay after trigger
     * bits [31:0] - number of decimated data after the trigger is written to memory
     */
    uint32_t chb_axi_delay;

    /** @brief Offset 0x7C - CH B AXI enable master
     * bits [0] - enable AXI master
     * bits [31:1] - reserved
     */
    uint32_t chb_axi_enable;

    /** @brief Offset 0x80 - CH B AXI write pointer - trigger
     * bits [31:0] - write pointer at the moment the trigger arrives
     */
    uint32_t chb_axi_wr_ptr_trigger;

    /** @brief Offset 0x84 - CH B AXI write pointer - current
     * bits [31:0] - current write pointer
     */
    uint32_t chb_axi_wr_ptr_cur;

    /** @brief Offset 0x88 - AXI state register
     *
     * Configuration register (offset 0x00):
     * bit [0]      - (R) CH A AXI - Trigger armed
     * bit [1]      - Reserved
     * bit [2]      - (R) CH A AXI - Trigger has arrived
     * bit [3]      - (R) CH A AXI - Trigger remines armed
     * bit [4]      - (R) CH A AXI - ACQ delay has passed
     * bit [15:5]   - reserved
     * bit [16]     - (R) CH B AXI - Trigger armed
     * bit [17]     - Reserved
     * bit [18]     - (R) CH B AXI - Trigger has arrived
     * bit [19]     - (R) CH B AXI - Trigger remines armed
     * bit [20]     - (R) CH B AXI - ACQ delay has passed
     * bits [31:21]  - reserved
     */
    uint32_t axi_state;

    /* Reserved */
    uint32_t reserved_8C;

    /**@brief Offset 0x90 - External trigger debuncer time
    * bits [19:0] Number of ADC clock periods
    * trigger is disabled after activation
    * reset value is decimal 62500
    * or equivalent to 0.5ms
    */
    uint32_t ext_trig_dbc;  // 0x90

    /**@brief Offset 0x94 - Trigger lock control
     * bit[0] - (W) Write 1 for unlock trigger - ch1/common
     * bit[7:1] - reserved
     * bit[8] - (W) Write 1 for unlock trigger - ch2
     * bit[15:9] - reserved
     * bit[16] - (W) Write 1 for unlock trigger - ch3
     * bit[23:17] - reserved
     * bit[24] - (W) Write 1 for unlock trigger - ch4
     * bit[31:25] - reserved
    */
    uint32_t trigger_lock_ctr;  // 0x94

    /** @brief Offset 0x98 - reserved
     */
    uint32_t reserved_98[30];

    /** @brief Offset 0x110 - After trigger delay register
     *
     * After trigger delay register (offset 0x10)
     * bits [31: 0] - trigger delay
     * 32 bit number - how many decimated samples should be stored into a buffer.
     * (max 16k samples)
     */
    uint32_t trigger_delay_ch2;

    /** @brief Offset 0x114 - Data decimation register
     *
     * Data decimation register (offset 0x14):
     * bits [16: 0] - decimation factor, legal values:
     * 1, 8, 64, 1024, 8192 65536
     * If other values are written data is undefined
     * bits [31:17] - reserved
     */
    uint32_t data_dec_ch2;

    /** @brief Offset 0x118 - Current write pointer register
     *
     * Current write pointer register (offset 0x18), read only:
     * bits [13: 0] - current write pointer
     * bits [31:14] - reserved
     */
    uint32_t wr_ptr_cur_ch2;

    /** @brief Offset 0x11C - Trigger write pointer register
     *
     * Trigger write pointer register (offset 0x1C), read only:
     * bits [13: 0] - trigger pointer (pointer where trigger was detected)
     * bits [31:14] - reserved
     */
    uint32_t wr_ptr_trigger_ch2;

    /** @brief Offset 0x120 - reserved
     */
    uint32_t reserved_120[3];

    /** @brief Offset 0x12C - Pre Trigger counter
     *
     * Pre Trigger counter (offset 0x2C)
     * bits [31: 0] - Pre Trigger counter
     * 32 bit number - how many decimated samples have been stored into a buffer
     * before trigger arrived.
     */
    uint32_t pre_trigger_counter_ch2;

    /* ChA & ChB data - 14 LSB bits valid starts from 0x10000 and
     * 0x20000 and are each 16k samples long */
} osc_control_t;

static const uint32_t DATA_DEC_MASK = 0x1FFFF;           // (17 bits)
static const uint32_t DATA_AVG_MASK = 0x1;               // (1 bit)
static const uint32_t TRIG_SRC_MASK = 0xF;               // (4 bits)
static const uint32_t START_DATA_WRITE_MASK = 0x1;       // (1 bit)
static const uint32_t THRESHOLD_MASK = 0xFFFF;           // (16 bits)
static const uint32_t HYSTERESIS_MASK = 0xFFFF;          // (16 bits)
static const uint32_t TRIG_DELAY_MASK = 0xFFFFFFFF;      // (32 bits)
static const uint32_t WRITE_POINTER_MASK = 0x3FFF;       // (14 bits)
static const uint32_t EQ_FILTER_AA = 0x3FFFF;            // (18 bits)
static const uint32_t EQ_FILTER = 0x1FFFFFF;             // (25 bits)
static const uint32_t RST_WR_ST_MCH_MASK = 0x2;          // (1st bit)
static const uint32_t TRIG_ST_MCH_MASK = 0x4;            // (2nd bit)
static const uint32_t PRE_TRIGGER_COUNTER = 0xFFFFFFFF;  // (32 bit)
static const uint32_t ARM_KEEP_MASK = 0x8;               // (4 bit)
static const uint32_t FILL_STATE_MASK = 0x10;            // (1 bit)
static const uint32_t TRIG_UNLOCK_MASK = 0x10;           // (1 bit)

static const uint32_t AXI_ENABLE_MASK = 0x1;          // (1 bit)
static const uint32_t AXI_CHA_FILL_STATE = 0x10;      // (1 bit)
static const uint32_t AXI_CHB_FILL_STATE = 0x100000;  // (1 bit)
static const uint32_t FULL_MASK = 0xFFFFFFFF;         // (32 bit)

static const uint32_t DEBAUNCER_MASK = 0xFFFFF;  // (20 bit)

int osc_Init(int channels);
int osc_Release();

int osc_printRegset();

int osc_SetDecimation(rp_channel_t channel, uint32_t decimation);
int osc_GetDecimation(rp_channel_t channel, uint32_t* decimation);
int osc_SetAveraging(rp_channel_t channel, bool enable);
int osc_GetAveraging(rp_channel_t channel, bool* enable);
int osc_SetTriggerSource(rp_channel_t channel, uint32_t source);
int osc_GetTriggerSource(rp_channel_t channel, uint32_t* source);
int osc_SetSplitTriggerMode(bool enable);
int osc_GetSplitTriggerMode(bool* enable);
int osc_SetUnlockTrigger(rp_channel_t channel);
int osc_GetUnlockTrigger(rp_channel_t channel, bool* state);
int osc_WriteDataIntoMemory(rp_channel_t channel, bool enable);
int osc_ResetWriteStateMachine(rp_channel_t channel);
int osc_SetArmKeep(rp_channel_t channel, bool enable);
int osc_GetArmKeep(rp_channel_t channel, bool* state);
int osc_GetBufferFillState(rp_channel_t channel, bool* state);
int osc_GetTriggerState(rp_channel_t channel, bool* received);
int osc_GetPreTriggerCounter(rp_channel_t channel, uint32_t* value);
int osc_SetThresholdChA(uint32_t threshold);
int osc_GetThresholdChA(uint32_t* threshold);
int osc_SetThresholdChB(uint32_t threshold);
int osc_GetThresholdChB(uint32_t* threshold);
int osc_SetThresholdChC(uint32_t threshold);
int osc_GetThresholdChC(uint32_t* threshold);
int osc_SetThresholdChD(uint32_t threshold);
int osc_GetThresholdChD(uint32_t* threshold);
int osc_SetHysteresisChA(uint32_t hysteresis);
int osc_GetHysteresisChA(uint32_t* hysteresis);
int osc_SetHysteresisChB(uint32_t hysteresis);
int osc_GetHysteresisChB(uint32_t* hysteresis);
int osc_SetHysteresisChC(uint32_t hysteresis);
int osc_GetHysteresisChC(uint32_t* hysteresis);
int osc_SetHysteresisChD(uint32_t hysteresis);
int osc_GetHysteresisChD(uint32_t* hysteresis);
int osc_SetTriggerDelay(rp_channel_t channel, uint32_t decimated_data_num);
int osc_GetTriggerDelay(rp_channel_t channel, uint32_t* decimated_data_num);
int osc_GetWritePointer(rp_channel_t channel, uint32_t* pos);
int osc_GetWritePointerAtTrig(rp_channel_t channel, uint32_t* pos);
int osc_SetEqFiltersChA(uint32_t coef_aa, uint32_t coef_bb, uint32_t coef_kk, uint32_t coef_pp);
int osc_GetEqFiltersChA(uint32_t* coef_aa, uint32_t* coef_bb, uint32_t* coef_kk, uint32_t* coef_pp);
int osc_SetEqFiltersChB(uint32_t coef_aa, uint32_t coef_bb, uint32_t coef_kk, uint32_t coef_pp);
int osc_GetEqFiltersChB(uint32_t* coef_aa, uint32_t* coef_bb, uint32_t* coef_kk, uint32_t* coef_pp);
int osc_SetEqFiltersChC(uint32_t coef_aa, uint32_t coef_bb, uint32_t coef_kk, uint32_t coef_pp);
int osc_GetEqFiltersChC(uint32_t* coef_aa, uint32_t* coef_bb, uint32_t* coef_kk, uint32_t* coef_pp);
int osc_SetEqFiltersChD(uint32_t coef_aa, uint32_t coef_bb, uint32_t coef_kk, uint32_t coef_pp);
int osc_GetEqFiltersChD(uint32_t* coef_aa, uint32_t* coef_bb, uint32_t* coef_kk, uint32_t* coef_pp);

int osc_SetExtTriggerDebouncer(uint32_t value);
int osc_GetExtTriggerDebouncer(uint32_t* value);

const volatile uint32_t* osc_GetDataBufferChA();
const volatile uint32_t* osc_GetDataBufferChB();
const volatile uint32_t* osc_GetDataBufferChC();
const volatile uint32_t* osc_GetDataBufferChD();

int osc_axi_EnableChA(bool enable);
int osc_axi_EnableChB(bool enable);
int osc_axi_EnableChC(bool enable);
int osc_axi_EnableChD(bool enable);

int osc_axi_SetAddressStartChA(uint32_t address);
int osc_axi_SetAddressStartChB(uint32_t address);
int osc_axi_SetAddressStartChC(uint32_t address);
int osc_axi_SetAddressStartChD(uint32_t address);
int osc_axi_SetAddressEndChA(uint32_t address);
int osc_axi_SetAddressEndChB(uint32_t address);
int osc_axi_SetAddressEndChC(uint32_t address);
int osc_axi_SetAddressEndChD(uint32_t address);

int osc_axi_GetAddressStartChA(uint32_t* address);
int osc_axi_GetAddressStartChB(uint32_t* address);
int osc_axi_GetAddressStartChC(uint32_t* address);
int osc_axi_GetAddressStartChD(uint32_t* address);
int osc_axi_GetAddressEndChA(uint32_t* address);
int osc_axi_GetAddressEndChB(uint32_t* address);
int osc_axi_GetAddressEndChC(uint32_t* address);
int osc_axi_GetAddressEndChD(uint32_t* address);

int osc_axi_GetBufferFillStateChA(bool* state);
int osc_axi_GetBufferFillStateChB(bool* state);
int osc_axi_GetBufferFillStateChC(bool* state);
int osc_axi_GetBufferFillStateChD(bool* state);

int osc_axi_GetWritePointerChA(uint32_t* pos);
int osc_axi_GetWritePointerChB(uint32_t* pos);
int osc_axi_GetWritePointerChC(uint32_t* pos);
int osc_axi_GetWritePointerChD(uint32_t* pos);
int osc_axi_GetWritePointerAtTrigChA(uint32_t* pos);
int osc_axi_GetWritePointerAtTrigChB(uint32_t* pos);
int osc_axi_GetWritePointerAtTrigChC(uint32_t* pos);
int osc_axi_GetWritePointerAtTrigChD(uint32_t* pos);
int osc_axi_SetTriggerDelayChA(uint32_t decimated_data_num);
int osc_axi_SetTriggerDelayChB(uint32_t decimated_data_num);
int osc_axi_SetTriggerDelayChC(uint32_t decimated_data_num);
int osc_axi_SetTriggerDelayChD(uint32_t decimated_data_num);
int osc_axi_GetTriggerDelayChA(uint32_t* decimated_data_num);
int osc_axi_GetTriggerDelayChB(uint32_t* decimated_data_num);
int osc_axi_GetTriggerDelayChC(uint32_t* decimated_data_num);
int osc_axi_GetTriggerDelayChD(uint32_t* decimated_data_num);

const uint16_t* osc_axi_GetDataBufferCh(rp_channel_t channel);

#endif /* SRC_OSCILLOSCOPE_H_ */
