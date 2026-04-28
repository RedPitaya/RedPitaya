/**
 * $Id: $
 *
 * @brief Red Pitaya library oscilloscope module interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
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
    uint8_t trig_source : 5;
    uint8_t trig_lock : 1;
    uint8_t : 2;
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
    uint8_t enable_16b_mode : 1;  // (R/W) Sets ADC mode to 16 Bit
    uint8_t : 6;
    void print() volatile {
        printRegBit(" - %-39s = 0x%08X (%d)\n", "average", average);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "enable_16b_mode", enable_16b_mode);
    };
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

typedef struct {
    uint32_t bypass_ch1 : 1;
    uint32_t bypass_ch2 : 1;
    void print() volatile {
        printRegBit(" - %-39s = 0x%08X (%d)\n", "bypass_ch1", bypass_ch1);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "bypass_ch2", bypass_ch2);
    };
} rec_filter_bypass_t;

typedef union {
    uint32_t reg_full;
    rec_filter_bypass_t reg;
} rec_filter_bypass_u_t;

/**
 * @brief IRQ Mask Register (0xAC) with union for bitfield or full value access
 */
typedef union {
    struct {
        uint32_t trigger_en : 1;
        uint32_t buffer_full_en : 1;
        uint32_t reserved : 30;
    } bits;
    uint32_t value;
    void print() volatile {
        printRegBit(" - %-39s = 0x%08X (%d)\n", "trigger_en", bits.trigger_en);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "buffer_full_en", bits.buffer_full_en);
    };
} acquisition_irq_mask_t;

/**
 * @brief IRQ Status/Clear Register (0xB0) with union for bitfield or full value access
 */
typedef union {
    struct {
        uint32_t trigger_pending : 1;
        uint32_t buffer_full_pending : 1;
        uint32_t reserved : 30;
    } bits;
    uint32_t value;
    void print() volatile {
        printRegBit(" - %-39s = 0x%08X (%d)\n", "trigger_pending", bits.trigger_pending);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "buffer_full_pending", bits.buffer_full_pending);
    };
} acquisition_irq_status_t;

/**
 * @brief Split IRQ Mask Register (0xB4)
 * @details Enable/disable per-channel interrupt sources
 *
 * Bits 0-3:   Trigger enable for channels 1-4
 * Bits 4-7:   Buffer full enable for channels 1-4
 */
typedef union {
    struct {
        uint32_t trig_ch1_en : 1;  // bit0
        uint32_t trig_ch2_en : 1;  // bit1
        uint32_t trig_ch3_en : 1;  // bit2
        uint32_t trig_ch4_en : 1;  // bit3
        uint32_t fill_ch1_en : 1;  // bit4
        uint32_t fill_ch2_en : 1;  // bit5
        uint32_t fill_ch3_en : 1;  // bit6
        uint32_t fill_ch4_en : 1;  // bit7
        uint32_t reserved : 24;
    } bits;
    uint32_t value;
    void print() volatile {
        printRegBit(" - %-39s = 0x%08X (%d)\n", "trig_ch1_en", bits.trig_ch1_en);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "trig_ch2_en", bits.trig_ch2_en);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "trig_ch3_en", bits.trig_ch3_en);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "trig_ch4_en", bits.trig_ch4_en);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "fill_ch1_en", bits.fill_ch1_en);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "fill_ch2_en", bits.fill_ch2_en);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "fill_ch3_en", bits.fill_ch3_en);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "fill_ch4_en", bits.fill_ch4_en);
    };
} split_irq_mask_t;

/**
 * @brief Split IRQ Status/Clear Register (0xB8)
 * @details Read: Get per-channel interrupt status
 *          Write: Clear pending interrupts (write 1 to clear)
 *
 * Bits 0-3:   Trigger pending for channels 1-4
 * Bits 4-7:   Buffer full pending for channels 1-4
 */
typedef union {
    struct {
        uint32_t trig_ch1_pending : 1;  // bit0
        uint32_t trig_ch2_pending : 1;  // bit1
        uint32_t trig_ch3_pending : 1;  // bit2
        uint32_t trig_ch4_pending : 1;  // bit3
        uint32_t fill_ch1_pending : 1;  // bit4
        uint32_t fill_ch2_pending : 1;  // bit5
        uint32_t fill_ch3_pending : 1;  // bit6
        uint32_t fill_ch4_pending : 1;  // bit7
        uint32_t reserved : 24;
    } bits;
    uint32_t value;
    void print() volatile {
        printRegBit(" - %-39s = 0x%08X (%d)\n", "trig_ch1_pending", bits.trig_ch1_pending);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "trig_ch2_pending", bits.trig_ch2_pending);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "trig_ch3_pending", bits.trig_ch3_pending);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "trig_ch4_pending", bits.trig_ch4_pending);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "fill_ch1_pending", bits.fill_ch1_pending);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "fill_ch2_pending", bits.fill_ch2_pending);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "fill_ch3_pending", bits.fill_ch3_pending);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "fill_ch4_pending", bits.fill_ch4_pending);
    };
} split_irq_status_t;

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

    /** @brief Offset 0x04 - Trigger Source Register
     *
     * Each channel occupies an 8-bit (1-byte) slot.
     * Common bits layout for each channel group:
     *   bits [4 : 0] - Trigger source value (see table below)
     *   bit  [5]     - Trigger lock state (1: Locked/Armed, 0: Waiting/Idle)
     *   bits [7 : 6] - Reserved
     *
     * For 125 / 250 / lite / ll variants (2-channel):
     *   bits [4 : 0]   - CH0 trigger source
     *   bit  [5]       - CH0 trigger lock state
     *   bits [12 : 8]  - CH1 trigger source
     *   bit  [13]      - CH1 trigger lock state
     *   bits [31 : 14] - Reserved
     *
     * For 125 4-Input variant (4-channel):
     *   bits [4 : 0]   - CH0 trigger source
     *   bit  [5]       - CH0 trigger lock state
     *   bits [12 : 8]  - CH1 trigger source
     *   bit  [13]      - CH1 trigger lock state
     *   bits [20 : 16] - CH2 trigger source
     *   bit  [21]      - CH2 trigger lock state
     *   bits [28 : 24] - CH3 trigger source
     *   bit  [29]      - CH3 trigger lock state
     *   bits [31 : 30] - Reserved
     *
     * Trigger source values (for each channel):
     *   0  - Disabled
     *   1  - Trigger immediately
     *   2  - Threshold positive edge (Self-trigger)
     *   3  - Threshold negative edge (Self-trigger)
     *   4  - Threshold positive edge (CH B for 2-ch, reserved for 4-ch)
     *   5  - Threshold negative edge (CH B for 2-ch, reserved for 4-ch)
     *   6  - External trigger positive edge (DIO0_P pin)
     *   7  - External trigger negative edge
     *   8  - AWG positive edge
     *   9  - AWG negative edge
     *   10 - Threshold positive edge (CH C for 4-ch)
     *   11 - Threshold negative edge (CH C for 4-ch)
     *   12 - Threshold positive edge (CH D for 4-ch)
     *   13 - Threshold negative edge (CH D for 4-ch)
     *   18 - Threshold any edge (Self-trigger)
     *   20 - Threshold any edge (CH B for 2-ch, reserved for 4-ch)
     *   22 - External trigger any edge
     *   24 - AWG any edge
     *   26 - Threshold any edge (CH C for 4-ch)
     *   28 - Threshold any edge (CH D for 4-ch)
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
    uint32_t trigger_lock_ctr;

    /**
     * Offset 0x98 - Reconstruction filter bypass
     * bit[0] - (R/W) Bypass ch1
     * bit[1] - (R/W) Bypass ch2
     * bit[31:2] - reserved
    */
    uint32_t filter_bypass;

    /**
     * @brief Offset 0x9С-0xA8 - Reserved area
    * @note 11 reserved registers (0x98 to 0xA8 inclusive)
    */
    uint32_t reserved_9C[4];  // 0x9C - 0xA8

    /**
     * @brief 0xAC - IRQ Mask
     * @details Enable interrupt sources
     * Values: 0x1 (trigger), 0x2 (buffer full), 0x3 (both)
     */
    uint32_t irq_mask;

    /**
     * @brief 0xB0 - IRQ Status/Clear
     * @details Read: get latched IRQ status (bits: 0=trigger, 1=buffer full)
     *          Write: clear pending IRQs (write 1 to corresponding bit)
     */
    uint32_t irq_status_clear;

    /**
     * @brief 0xB4 - Split IRQ Mask Register
     * @details Enable/disable per-channel interrupt sources for independent trigger mode.
     *          Used when indep_mode is enabled (each channel operates independently).
     *
     * Bit mapping:
     * - Bits 0-3:   Trigger event enable for channels 1-4
     * - Bits 4-7:   Buffer full / acquisition finished enable for channels 1-4
     * - Bits 8-31:  Reserved (write 0, read undefined)
     *
     * Values per bit:
     * - 0: Interrupt disabled
     * - 1: Interrupt enabled
     *
     * Common mask values:
     * - 0x01: Enable trigger only on channel 1
     * - 0x02: Enable trigger only on channel 2
     * - 0x04: Enable trigger only on channel 3
     * - 0x08: Enable trigger only on channel 4
     * - 0x10: Enable buffer full only on channel 1
     * - 0x20: Enable buffer full only on channel 2
     * - 0x30: Enable buffer full on channels 1 and 2
     * - 0x0F: Enable trigger on all channels
     * - 0xF0: Enable buffer full on all channels
     * - 0xFF: Enable all events on all channels
     *
     * @note This register is only effective when indep_mode = 1
     * @note For legacy combined mode, use irq_mask (0xAC) instead
     *
     * @example Enable trigger on channel 1 and buffer full on channel 2:
     *         monitor 0x401000B4 0x21  (0x20 | 0x01)
    **/
    uint32_t irq_split_mask;

    /**
     * @brief 0xB8 - Split IRQ Status / Clear Register
     * @details Read: Get latched per-channel interrupt status
     *          Write: Clear pending interrupts (write 1 to corresponding bit)
     *
     * Bit mapping (same as mask register):
     * - Bits 0-3:   Trigger event pending for channels 1-4
     * - Bits 4-7:   Buffer full pending for channels 1-4
     * - Bits 8-31:  Reserved (read as 0)
     *
     * Status values when reading:
     * - 0x00: No pending interrupts on any channel
     * - 0x01: Trigger pending on channel 1
     * - 0x02: Trigger pending on channel 2
     * - 0x04: Trigger pending on channel 3
     * - 0x08: Trigger pending on channel 4
     * - 0x10: Buffer full pending on channel 1
     * - 0x20: Buffer full pending on channel 2
     * - 0x30: Buffer full pending on channels 1 and 2
     * - 0x0F: Trigger pending on all channels
     * - 0xF0: Buffer full pending on all channels
     * - 0xFF: All events pending on all channels
     *
     * Clearing interrupts (write):
     * - Write 1 to a bit to clear the corresponding pending interrupt
     * - Multiple bits can be cleared simultaneously
     * - Write 0x0F to clear all trigger events
     * - Write 0xF0 to clear all buffer full events
     * - Write 0xFF to clear all events on all channels
     *
     * @note Interrupts are edge-triggered and latched until cleared
     * @note Writing 0 has no effect
     * @note After clearing, the bit returns to 0 if no new event occurred
     *
    **/
    uint32_t irq_split_status_clear;

    /** @brief Offset 0xBС to 0x10C */
    uint32_t reserved_BC[21];

    /** @brief Offset 0x110 - After trigger delay register
     *
     * After trigger delay register (offset 0x110)
     * bits [31: 0] - trigger delay
     * 32 bit number - how many decimated samples should be stored into a buffer.
     * (max 16k samples)
     */
    uint32_t trigger_delay_ch2;

    /** @brief Offset 0x114 - Data decimation register
     *
     * Data decimation register (offset 0x114):
     * bits [16: 0] - decimation factor, legal values:
     * 1, 8, 64, 1024, 8192 65536
     * If other values are written data is undefined
     * bits [31:17] - reserved
     */
    uint32_t data_dec_ch2;

    /** @brief Offset 0x118 - Current write pointer register
     *
     * Current write pointer register (offset 0x118), read only:
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
     * Pre Trigger counter (offset 0x12C)
     * bits [31: 0] - Pre Trigger counter
     * 32 bit number - how many decimated samples have been stored into a buffer
     * before trigger arrived.
     */
    uint32_t pre_trigger_counter_ch2;

    /** @brief Offset 0x130 - reserved
     */
    uint32_t reserved_130[52];

    /** @brief Calibration offset CH1 0x200
     *
     * Trigger write pointer register (offset 0x200):
     * bits [15: 0] - Offset R/W
     * bits [31:16] - reserved
     */
    uint32_t calib_offset_ch1;

    /** @brief Calibration gain CH1 0x204
     *
     * Trigger write pointer register (offset 0x204):
     * bits [15: 0] - Gain R/W
     * bits [31:16] - reserved
     */
    uint32_t calib_gain_ch1;

    /** @brief Calibration offset CH2 0x208
     *
     * Trigger write pointer register (offset 0x208):
     * bits [15: 0] - Offset R/W
     * bits [31:16] - reserved
     */
    uint32_t calib_offset_ch2;

    /** @brief Calibration gain CH2 0x20C
     *
     * Trigger write pointer register (offset 0x20C):
     * bits [15: 0] - Gain R/W
     * bits [31:16] - reserved
     */
    uint32_t calib_gain_ch2;

    /** @brief Offset 0x210 - Reserved
     */
    uint32_t reserved_210[4];

    /** @brief Offset 0x220 - Global Timestamp Counter LO
     *
     * Lower 32 bits of the global 64-bit timer.
     * IMPORTANT: Reading this register latches (snapshots) the full 64-bit
     * counter value into a temporary buffer. Always read LO first.
     */
    uint32_t timestamp_init_lo;

    /** @brief Offset 0x224 - Global Timestamp Counter HI
     *
     * Upper 32 bits of the global 64-bit timer.
     * Returns the upper 32 bits of the value latched when LO was read.
     */
    uint32_t timestamp_init_hi;

    /** @brief Offset 0x228 - CH1 Trigger Timestamp LO
     *
     * Lower 32 bits of the timestamp captured at the last CH1 trigger event.
     */
    uint32_t trig_timestamp_lo_ch1;

    /** @brief Offset 0x22C - CH1 Trigger Timestamp HI
     *
     * Upper 32 bits of the timestamp captured at the last CH1 trigger event.
     */
    uint32_t trig_timestamp_hi_ch1;

    /** @brief Offset 0x230 - CH2 Trigger Timestamp LO
     *
     * Lower 32 bits of the timestamp captured at the last CH2 trigger event.
     */
    uint32_t trig_timestamp_lo_ch2;

    /** @brief Offset 0x234 - CH2 Trigger Timestamp HI
     *
     * Upper 32 bits of the timestamp captured at the last CH2 trigger event.
     */
    uint32_t trig_timestamp_hi_ch2;

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

static const uint32_t CALIB_MASK = 0xFFFF;  // (16 bit)

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
int osc_Set16BitMode(rp_channel_t channel, bool enable);
int osc_Get16BitMode(rp_channel_t channel, bool* state);
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
int osc_SetEqFilterBypass(rp_channel_t channel, bool enable);
int osc_GetEqFilterBypass(rp_channel_t channel, bool* enable);
int osc_SetEqFiltersChA(uint32_t coef_aa, uint32_t coef_bb, uint32_t coef_kk, uint32_t coef_pp);
int osc_GetEqFiltersChA(uint32_t* coef_aa, uint32_t* coef_bb, uint32_t* coef_kk, uint32_t* coef_pp);
int osc_SetEqFiltersChB(uint32_t coef_aa, uint32_t coef_bb, uint32_t coef_kk, uint32_t coef_pp);
int osc_GetEqFiltersChB(uint32_t* coef_aa, uint32_t* coef_bb, uint32_t* coef_kk, uint32_t* coef_pp);
int osc_SetEqFiltersChC(uint32_t coef_aa, uint32_t coef_bb, uint32_t coef_kk, uint32_t coef_pp);
int osc_GetEqFiltersChC(uint32_t* coef_aa, uint32_t* coef_bb, uint32_t* coef_kk, uint32_t* coef_pp);
int osc_SetEqFiltersChD(uint32_t coef_aa, uint32_t coef_bb, uint32_t coef_kk, uint32_t coef_pp);
int osc_GetEqFiltersChD(uint32_t* coef_aa, uint32_t* coef_bb, uint32_t* coef_kk, uint32_t* coef_pp);

int osc_IntUnmask();
int osc_IntUnmaskCh(rp_channel_t channel);
int osc_ClearInt();
int osc_ClearInt(rp_channel_t channel);
int osc_IntTriggerRead(int timeout);
int osc_IntFullRead(int timeout);
int osc_IntTriggerReadCh(rp_channel_t channel, int timeout);
int osc_IntFullReadCh(rp_channel_t channel, int timeout);
int osc_IntClearTrigger();
int osc_IntClearBufferFull();
int osc_IntClearAll();
int osc_IntClearTriggerCh(rp_channel_t channel);
int osc_IntClearBufferFullCh(rp_channel_t channel);
int osc_IntClearAllCh(rp_channel_t channel);

int osc_SetCalibOffsetInFPGA(rp_channel_t channel, int32_t offset);
int osc_SetCalibGainInFPGA(rp_channel_t channel, double gain);
int osc_GetCalibOffsetInFPGA(rp_channel_t channel, int32_t* offset);
int osc_GetCalibGainInFPGA(rp_channel_t channel, double* gain);

int osc_SetExtTriggerDebouncer(uint32_t value);
int osc_GetExtTriggerDebouncer(uint32_t* value);

int osc_SetInitTimestamp(uint64_t value);
int osc_GetTimestamp(rp_channel_t channel, uint64_t* value);

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
