/**
 * @brief Red Pitaya FPGA Interface for the RadioBox sub-module.
 *
 * @author Ulrich Habel (DF4IAH) <espero7757@gmx.net>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef __FPGA_RB_H
#define __FPGA_RB_H

#include <stdint.h>

#include "main.h"


/** @defgroup fpga_rb_h FPGA RadioBox sub-module access
 * @{
 */

/** @brief RadioBox starting address of FPGA registers. */
#define FPGA_RB_BASE_ADDR       0x40600000

/** @brief RadioBox memory map size of FPGA registers. */
#define FPGA_RB_BASE_SIZE       0x10000

/** @brief FPGA register offset addresses of the RadioBox sub-system base address.
 */
enum {
    FPGA_RB_CTRL                = 0x00000,
    FPGA_RB_STATUS              = 0x00004,
    FPGA_RB_ICR                 = 0x00008,
    FPGA_RB_ISR                 = 0x0000C,
    FPGA_RB_DMA_CTRL            = 0x00010,
    FPGA_RB_LED_CTRL            = 0x0001C,
    FPGA_RB_OSC1_INC_LO         = 0x00020,
    FPGA_RB_OSC1_INC_HI         = 0x00024,
    FPGA_RB_OSC1_OFS_LO         = 0x00028,
    FPGA_RB_OSC1_OFS_HI         = 0x0002C,
    FPGA_RB_OSC1_MIX_GAIN       = 0x00030,
    FPGA_RB_OSC1_MIX_OFS_LO     = 0x00038,
    FPGA_RB_OSC1_MIX_OFS_HI     = 0x0003C,
    FPGA_RB_OSC2_INC_LO         = 0x00040,
    FPGA_RB_OSC2_INC_HI         = 0x00044,
    FPGA_RB_OSC2_OFS_LO         = 0x00048,
    FPGA_RB_OSC2_OFS_HI         = 0x0004C,
    FPGA_RB_OSC2_MIX_GAIN       = 0x00050,
    FPGA_RB_OSC2_MIX_OFS_LO     = 0x00058,
    FPGA_RB_OSC2_MIX_OFS_HI     = 0x0005C,
    FPGA_RB_MUXIN_SRC           = 0x00060,
    FPGA_RB_MUXIN_GAIN          = 0x00064
} FPGA_RB_REG_ENUMS;

/** @brief FPGA registry structure for the RadioBox sub-module.
 *
 * This structure is the direct image of the physical FPGA memory for the RadioBox sub-module.
 * It assures direct read / write FPGA access when it is mapped to the appropriate memory address
 * through the /dev/mem device.
 */
typedef struct fpga_rb_reg_mem_s {

    /** @brief  R/W RB_CTRL - Control register (addr: 0x40600000)
     *
     * bit h00: ENABLE - '1' enables the RadioBox sub-module. DDS-Oscillators, multipliers, LED handling are turned on. The DAC and LEDs are connected to this sub-module when enabled.
     *
     * bit h01: RESET OSC1 - '1' resets the OSC1 (carrier oscillator) to its initial state like the accumulating phase register.
     *
     * bit h02: RESET OSC2 - '1' resets the OSC2 (modulation oscillator) to its initial state like the accumulating phase register.
     *
     * bit h03: n/a
     *
     * bit h04: OSC1 RESYNC - '1' stops incrementing the accumulating phase register. That holds the oscillator just there, where it is. With '0' the OSC1 resumes operation.
     *
     * bit h05: OSC1 INC SRC STREAM - '1' places input MUXer for OSC1 DDS increment input to the first streamed input pipe. '0' places MUXer to registers "OSC1 INC HI" and "OSC1 INC LO".
     *
     * bit h06: OSC1 OFS SRC STREAM - '1' places input MUXer for OSC1 DDS offset input to the first streamed input pipe. '0' places MUXer to registers "OSC1 OFS HI" and "OSC1 OFS LO".
     *
     * bit h07: OSC1 GAIN SRC STREAM - '1' places input MUXer for OSC1 amplitude multiplier input to the first streamed input pipe. '0' places MUXer to register "OSC1 MIX GAIN".
     *
     * bit h0B..h08: n/a
     *
     * bit h0C: OSC2 RESYNC - '1' stops incrementing the accumulating phase register. That holds the oscillator just there, where it is. With '0' the OSC1 resumes operation.
     *
     * bit h0D: OSC2 INC SRC STREAM - '1' places input MUXer for OSC2 DDS increment input to the second streamed input pipe. '0' places MUXer to registers "OSC2 INC HI" and "OSC2 INC LO".
     *
     * bit h0E: OSC2 OFS SRC STREAM - '1' places input MUXer for OSC2 DDS offset input to the second streamed input pipe. '0' places MUXer to registers "OSC2 OFS HI" and "OSC2 OFS LO".
     *
     * bit h0F: n/a
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t ctrl;


    /** @brief  R/O RB_STATUS - Status register (addr: 0x40600004)
     *
     * bit h00: STAT_CLK_EN - '1' clock of the RadioBox sub-system is enabled (power up sub-module).
     *
     * bit h01: STAT_RESET - '1' reset of the RadioBox sub-system is active (clears phase accumulators).
     *
     * bit h02: STAT_LEDS_EN - '1' RadioBox LEDs state is shown at the diodes, any other output register is discarded.
     *
     * bit h03: n/a
     *
     * bit h04: STAT_OSC1_ZERO - '1' Oscillator-1 output equals zero. This state is based on the output of the DDS oscillator itself.
     *
     * bit h05: STAT_OSC1_VALID - '1' Oscillator-1 output is valid. After turning this sub-module active it needs some clocks going into valid state.
     *
     * bit h06: STAT_OSC1_MIX_VALID - '1' Oscillator-1 multiplier output is valid. After turning this sub-module active it needs some clocks going into valid state.
     *
     * bit h07: n/a
     *
     * bit h08: STAT_OSC2_ZERO - '1' Oscillator-2 output equals zero. This state is based on the output of the DDS oscillator itself.
     *
     * bit h09: STAT_OSC2_VALID - '1' Oscillator-2 output is valid. After turning this sub-module active it needs some clocks going into valid state.
     *
     * bit h0A: STAT_OSC2_MIX_VALID - '1' Oscillator-2 multiplier output is valid. After turning this sub-module active it needs some clocks going into valid state.
     *
     * bit h0B: n/a
     *
     * bit h17..h0C: n/a
     *
     * bit h18: STAT_LED0_ON - '1' RadioBox LED0 driver that is shown at the diodes, when STAT_LEDS_EN is '1'.
     *
     * bit h19: STAT_LED1_ON - '1' RadioBox LED1 driver that is shown at the diodes, when STAT_LEDS_EN is '1'.
     *
     * bit h1A: STAT_LED2_ON - '1' RadioBox LED2 driver that is shown at the diodes, when STAT_LEDS_EN is '1'.
     *
     * bit h1B: STAT_LED3_ON - '1' RadioBox LED3 driver that is shown at the diodes, when STAT_LEDS_EN is '1'.
     *
     * bit h1C: STAT_LED4_ON - '1' RadioBox LED4 driver that is shown at the diodes, when STAT_LEDS_EN is '1'.
     *
     * bit h1D: STAT_LED5_ON - '1' RadioBox LED5 driver that is shown at the diodes, when STAT_LEDS_EN is '1'.
     *
     * bit h1E: STAT_LED6_ON - '1' RadioBox LED6 driver that is shown at the diodes, when STAT_LEDS_EN is '1'.
     *
     * bit h1F: STAT_LED7_ON - '1' RadioBox LED7 driver that is shown at the diodes, when STAT_LEDS_EN is '1'.
     *
     */
    uint32_t status;


    /** @brief  R/W RB_ICR - Interrupt control register (addr: 0x40600008)
     *
     * n/a
     *
     */
    uint32_t icr;


    /** @brief  R/O RB_ISR Interrupt status register (addr: 0x4060000C)
     *
     * n/a
     *
     */
    uint32_t isr;


    /** @brief  R/W RB_DMA_CTRL - Interrupt status register (addr: 0x40600010)
     *
     * n/a
     *
     */
    uint32_t dma_ctrl;


    /** @brief  Placeholder for addr: 0x40600014
     *
     * n/a
     *
     */
    uint32_t reserved_14;


    /** @brief  Placeholder for addr: 0x40600018
     *
     * n/a
     *
     */
    uint32_t reserved_18;


    /** @brief  R/W RB_LED_CTRL - Interrupt status register (addr: 0x4060001C)
     *
     * bit h03..h00: LED magnitude input selector
     *
     *   value = h00  RadioBox does not touch LED state of other sub-module(s).
     *
     *   value = h01  All LEDs are driven off.
     *
     *   value = h02  LEDs show magnitude function with selected input port MIX1 output.
     *
     *   value = h03  LEDs show magnitude function with selected input port OSC1 output.
     *
     *   value = h04  LEDs show magnitude function with selected input port MIX2 output.
     *
     *   value = h05  LEDs show magnitude function with selected input port OSC2 output.
     *
     *   value = h06  LEDs show magnitude function with selected input port ADC input.
     *
     *   value = h07  LEDs show magnitude function with selected input port Mic MIX output.
     *
     *   value = h08..h0F  n/a
     *
     * bit h1F..h04: n/a
     *
     */
    uint32_t led_ctrl;


    /** @brief  R/W RB_OSC1_INC_LO - Oscillator-1 phase increment register, bits 31..0 (addr: 0x40600020)
     *
     * bit h1F..h00: LSB of Oscillator-1 phase increment register.
     *
     */
    uint32_t osc1_inc_lo;


    /** @brief  R/W RB_OSC1_INC_HI - Oscillator-1 phase increment register, bits 47..32 (addr: 0x40600024)
     *
     * bit h0F..h00: MSB of Oscillator-1 phase increment register.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t osc1_inc_hi;


    /** @brief  R/W RB_OSC1_OFS_LO - Oscillator-1 phase offset register, bits 31..0 (addr: 0x40600028)
     *
     * bit h1F..h00: LSB of Oscillator-1 phase offset register.
     *
     */
    uint32_t osc1_ofs_lo;


    /** @brief  R/W RB_OSC1_OFS_HI - Oscillator-1 phase offset register, bits 47..32 (addr: 0x4060002C)
     *
     * bit h0F..h00: MSB of Oscillator-1 phase offset register.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t osc1_ofs_hi;


    /** @brief  R/W RB_OSC1_MIX_GAIN - Oscillator-1 output mixer amplitude register, bits 31..0 (addr: 0x40600030)
     *
     * bit h1F..h00: Oscillator-1 output mixer amplitude.
     *
     */
    uint32_t osc1_mix_gain;


    /** @brief  Placeholder for addr: 0x40600034
     *
     * n/a
     *
     */
    uint32_t reserved_34;


    /** @brief  R/W RB_OSC1_MIX_OFS_LO - Oscillator-1 output mixer offset register, bits 31..0 (addr: 0x40600038)
     *
     * bit h1F..h00: LSB of Oscillator-1 mixer offset register.
     *
     */
    uint32_t osc1_mix_ofs_lo;


    /** @brief  R/W RB_OSC1_MIX_OFS_HI - Oscillator-1 output mixer offset register, bits 47..32 (addr: 0x4060003C)
     *
     * bit h0F..h00: MSB of Oscillator-1 mixer offset register.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t osc1_mix_ofs_hi;


    /** @brief  R/W RB_OSC2_INC_LO - Oscillator-2 phase increment register, bits 31..0 (addr: 0x40600040)
     *
     * bit h1F..h00: LSB of Oscillator-2 phase increment register.
     *
     */
    uint32_t osc2_inc_lo;


    /** @brief  R/W RB_OSC2_INC_HI - Oscillator-2 phase increment register, bits 47..32 (addr: 0x40600044)
     *
     * bit h0F..h00: MSB of Oscillator-2 phase increment register.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t osc2_inc_hi;


    /** @brief  R/W RB_OSC2_OFS_LO - Oscillator-2 phase offset register, bits 31..0 (addr: 0x40600048)
     *
     * bit h1F..h00: LSB of Oscillator-2 phase offset register.
     *
     */
    uint32_t osc2_ofs_lo;


    /** @brief  R/W RB_OSC2_OFS_HI - Oscillator-2 phase offset register, bits 47..32 (addr: 0x4060004C)
     *
     * bit h0F..h00: MSB of Oscillator-1 phase offset register.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t osc2_ofs_hi;


    /** @brief  R/W RB_OSC2_MIX_GAIN - Oscillator-2 output mixer amplitude register, bits 31..0 (addr: 0x40600050)
     *
     * bit h1F..h00: Oscillator-2 output mixer amplitude.
     *
     */
    uint32_t osc2_mix_gain;


    /** @brief  Placeholder for addr: 0x40600054
     *
     * n/a
     *
     */
    uint32_t reserved_54;


    /** @brief  R/W RB_OSC2_MIX_OFS_LO - Oscillator-2 output mixer offset register, bits 31..0 (addr: 0x40600058)
     *
     * bit h1F..h00: LSB of Oscillator-2 mixer offset register.
     *
     */
    uint32_t osc2_mix_ofs_lo;


    /** @brief  R/W RB_OSC2_MIX_OFS_HI - Oscillator-2 output mixer offset register, bits 47..32 (addr: 0x4060005C)
     *
     * bit h0F..h00: MSB of Oscillator-2 mixer offset register.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t osc2_mix_ofs_hi;


    /** @brief  R/W RB_MUXIN_SRC - analog MUX input selector (addr: 0x40600060)
     *
     * bit h05..h00: source ID (extended from the XADC source ID)
     *
     *   value = h00  no external signal used, OSC2 used instead.
     *
     *   value = h03  Vp_Vn,     mapped to: vin[4].
     *
     *   value = h10  XADC CH#0, mapped to: AI1.
     *
     *   value = h11  XADC CH#1, mapped to: AI0.
     *
     *   value = h18  XADC CH#8, mapped to: AI2.
     *
     *   value = h19  XADC CH#9, mapped to: AI3.
     *
     *   value = h20  ADC0,      mapped to: RF Input 1.
     *
     *   value = h21  ADC1,      mapped to: RF Input 2.
     *
     */
    uint32_t muxin_src;

    /** @brief  R/W RB_MUXIN_GAIN - gain for analog MUX input amplifier (addr: 0x40600064)
     *
     * bit h1F..h00: gain for MUXIN output amplifier.
     *
     */
    uint32_t muxin_gain;

} fpga_rb_reg_mem_t;



/* function declarations, detailed descriptions is in apparent implementation file  */


// RadioBox FPGA accessors

/**
 * @brief Initialize interface to RadioBox FPGA sub-module
 *
 * Set-up for FPGA access to the RadioBox sub-module.
 *
 * @retval  0 Success
 * @retval -1 Failure, error message is printed on standard error device
 *
 */
int fpga_rb_init(void);

/**
 * @brief Finalize and release allocated resources of the RadioBox sub-module
 *
 * @retval 0 Success, never fails
 */
int fpga_rb_exit(void);

/**
 * @brief Enables or disables RadioBox FPGA sub-module
 *
 * @param[in] enable  nonzero enables the RadioBox sub-module, zero disables it.
 *
 */
void fpga_rb_enable(int enable);

/**
 * @brief Resets RadioBox FPGA sub-module
 *
 */
void fpga_rb_reset(void);


/**
 * @brief Updates all modified data attributes to the RadioBox FPGA sub-module
 *
 * Being called out of the worker context.
 *
 * @param[inout]  p      List of parameters to be scanned for marked entries, removes MARKER.
 *
 * @retval        0      Success
 * @retval        -1     Failure, parameter list or RB accessor not valid
 */
int fpga_rb_update_all_params(rb_app_params_t* p);


/**
 * @brief Calculates and programs the FPGA RB ctrl and oscillator registers
 *
 * @param[in]  rb_run     RadioBox application  0: disabled, else: enabled.
 * @param[in]  modsrc     0==(none), 1==RF Input 1, 2==RF Input 2, 4==EXP AI0, 5==EXP AI1, 6==EXP AI2, 7==EXP AI3, 15==OSC2
 * @param[in]  modtyp     when modsrc == OSC2: 0==AM, 1==FM, 2==PM - else ignored.
 * @param[in]  led_ctrl   RB LED controller setting to be used.
 * @param[in]  osc1_qrg   Frequency for OSC1 in Hz.
 * @param[in]  osc2_qrg   Frequency for OSC2 in Hz.
 * @param[in]  osc1_amp   Vpp of OSC1 mixer output in mV.
 * @param[in]  osc2_mag   Magnitude of OSC2 mixer output. AM: 0-100%, FM: 0-1000000 Hz deviation, PM: 0-360°.
 * @param[in]  muxin_gain Slider value between 0 and 100 for the MUXIN range slider. 50 means amplification of 1:1, 0 and 100 full scale, logarithmic.
 */
void fpga_rb_set_ctrl(int rb_run, int modsrc, int modtyp, int led_ctrl, double osc1_qrg, double osc2_qrg, double osc1_amp, double osc2_mag, double muxin_gain);

/**
 * @brief Calculates and programs the FPGA OSC1 for AM and PM
 *
 * @param[in]  osc1_qrg   Frequency for OSC1 in Hz.
 */
//void fpga_rb_set_osc1_mod_none_am_pm(double osc1_qrg);
void fpga_rb_set_osc1_mod_none_am_pm(double osc1_qrg);

/**
 * @brief Calculates and programs the FPGA OSC1 mixer for AM
 *
 * @param[in]  osc1_amp   Vpp amplitude in mV.
 */
void fpga_rb_set_osc1_mixer_mod_none_fm_pm(double osc1_amp);

/**
 * @brief Calculates and programs the FPGA OSC2 for AM and PM
 *
 * @param[in]  osc2_qrg   Frequency for OSC2 in Hz.
 */
void fpga_rb_set_osc2_mod_am_fm_pm(double osc2_qrg);

/**
 * @brief Calculates and programs the FPGA OSC2 mixer for AM
 *
 * @param[in]  osc1_amp   Vpp amplitude in mV.
 * @param[in]  osc2_mag   Magnitude percentage 0 - 100.
 */
void fpga_rb_set_osc2_mixer_mod_am(double osc1_amp, double osc2_mag);

/**
 * @brief Calculates and programs the FPGA OSC2 mixer for FM
 *
 * @param[in]  osc1_qrg   Frequency for OSC1 in Hz.
 * @param[in]  osc2_mag   Deviation in Hz.
 */
void fpga_rb_set_osc2_mixer_mod_fm(double osc1_qrg, double osc2_mag);

/**
 * @brief Calculates and programs the FPGA OSC2 mixer for PM
 *
 * @param[in]  osc1_qrg   Base frequency in Hz.
 * @param[in]  osc2_mag   Deviation in deg.
 */
void fpga_rb_set_osc2_mixer_mod_pm(double osc1_qrg, double osc2_mag);

/**
 * @brief Calculates and programs the FPGA MUXIN gain setting
 *
 * @param[in]  muxin_gain Slider value between 0 and 100 for the MUXIN range slider. 50 means amplification of 1:1, 0 and 100 full scale, logarithmic.
 */
void fpga_rb_set_muxin_gain(double muxin_gain);


#if 0
uint32_t fpga_rb_read_register(unsigned int rb_reg_ofs);
int fpga_rb_write_register(unsigned int rb_reg_ofs, uint32_t value);
#endif

/** @} */


#endif /* __FPGA_RB_H */
