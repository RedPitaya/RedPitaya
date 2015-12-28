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
    FPGA_RB_CON_SRC_PNT         = 0x0001C,
    FPGA_RB_CAR_OSC_INC_LO      = 0x00020,
    FPGA_RB_CAR_OSC_INC_HI      = 0x00024,
    FPGA_RB_CAR_OSC_OFS_LO      = 0x00028,
    FPGA_RB_CAR_OSC_OFS_HI      = 0x0002C,
    FPGA_RB_CAR_OSC_MIX_GAIN    = 0x00030,
    FPGA_RB_CAR_OSC_MIX_OFS_LO  = 0x00038,
    FPGA_RB_CAR_OSC_MIX_OFS_HI  = 0x0003C,
    FPGA_RB_MOD_OSC_INC_LO      = 0x00040,
    FPGA_RB_MOD_OSC_INC_HI      = 0x00044,
    FPGA_RB_MOD_OSC_OFS_LO      = 0x00048,
    FPGA_RB_MOD_OSC_OFS_HI      = 0x0004C,
    FPGA_RB_MOD_OSC_MIX_GAIN    = 0x00050,
    FPGA_RB_MOD_OSC_MIX_OFS_LO  = 0x00058,
    FPGA_RB_MOD_OSC_MIX_OFS_HI  = 0x0005C,
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
     * bit h01: RESET CAR_OSC - '1' resets the CAR_OSC (carrier oscillator) to its initial state like the accumulating phase register.
     *
     * bit h02: RESET MOD_OSC - '1' resets the MOD_OSC (modulation oscillator) to its initial state like the accumulating phase register.
     *
     * bit h03: n/a
     *
     * bit h04: CAR_OSC RESYNC - '1' stops incrementing the accumulating phase register. That holds the oscillator just there, where it is. With '0' the CAR_OSC resumes operation.
     *
     * bit h05: CAR_OSC INC SRC STREAM - '1' places input MUXer for CAR_OSC DDS increment input to the first streamed input pipe. '0' places MUXer to registers "CAR_OSC INC HI" and "CAR_OSC INC LO".
     *
     * bit h06: CAR_OSC OFS SRC STREAM - '1' places input MUXer for CAR_OSC DDS offset input to the first streamed input pipe. '0' places MUXer to registers "CAR_OSC OFS HI" and "CAR_OSC OFS LO".
     *
     * bit h0B..h07: n/a
     *
     * bit h0C: MOD_OSC RESYNC - '1' stops incrementing the accumulating phase register. That holds the oscillator just there, where it is. With '0' the CAR_OSC resumes operation.
     *
     * bit h0D: MOD_OSC INC SRC STREAM - '1' places input MUXer for MOD_OSC DDS increment input to the second streamed input pipe. '0' places MUXer to registers "MOD_OSC INC HI" and "MOD_OSC INC LO".
     *
     * bit h0E: MOD_OSC OFS SRC STREAM - '1' places input MUXer for MOD_OSC DDS offset input to the second streamed input pipe. '0' places MUXer to registers "MOD_OSC OFS HI" and "MOD_OSC OFS LO".
     *
     * bit h13..h0F: n/a
     *
     * bit h14: AMP_RF_Q_EN - '1' enables the CAR_QMIX Q path for the SSB modulation
     *
     * bit h1F..h15: n/a
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
     * bit h04: STAT_CAR_OSC_ZERO - '1' CAR_OSC output equals zero. This state is based on the output of the DDS oscillator itself.
     *
     * bit h05: STAT_CAR_OSC_VALID - '1' CAR_OSC output is valid. After turning this sub-module active it needs some clocks going into valid state.
     *
     * bit h06: n/a
     *
     * bit h07: n/a
     *
     * bit h08: STAT_MOD_OSC_ZERO - '1' MOD_OSC output equals zero. This state is based on the output of the DDS oscillator itself.
     *
     * bit h09: STAT_MOD_OSC_VALID - '1' MOD_OSC output is valid. After turning this sub-module active it needs some clocks going into valid state.
     *
     * bit h0A: n/a
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


    /** @brief  R/W RB_CON_SRC_PNT - output connection matrix for
                                     RB_LEDs, RFOUT1 and RFOUT2 (addr: 0x4060001C)
     *
     * bit h05..h00: LEDs magnitude scope (logarithmic)  source position
     *   value = h00  RadioBox does not touch LED state of other sub-module(s).
     *   value = h01  All LEDs are driven off.
     *   value = h04  MUXIN_MIX in.
     *   value = h05  MOD_ADC in.
     *   value = h06  MOD_ADC out.
     *   value = h08  MOD_QMIX_I output at stage 1.
     *   value = h09  MOD_QMIX_Q output at stage 1.
     *   value = h0A  MOD_QMIX_I output at stage 2.
     *   value = h0B  MOD_QMIX_Q output at stage 2.
     *   value = h0C  MOD_QMIX_I output at stage 3.
     *   value = h0D  MOD_QMIX_Q output at stage 3.
     *   value = h10  low pass MOD_CIC I output.
     *   value = h11  low pass MOD_CIC Q output.
     *   value = h12  signal forming MOD_FIR I output.
     *   value = h13  signal forming MOD_FIR Q output.
     *   value = h14  interpolator CAR_CIC I output 41.664 MHz output.
     *   value = h15  interpolator CAR_CIC Q output 41.664 MHz output.
     *   value = h18  CAR_QMIX_I output.
     *   value = h19  CAR_QMIX_Q output.
     *   value = h1C  AMP_RF output.
     *   value = h3F  LEDs show current test vector @see red_pitaya_radiobox.sv for details.
     *
     * bit h0F..h06: n/a
     *
     * bit h15..h10: RFOUT1 source connection point
     *   value = h00  silence.
     *   value = h01  silence.
     *   value = h04  MUXIN_MIX in.
     *   value = h05  MOD_ADC in.
     *   value = h06  MOD_ADC out.
     *   value = h08  MOD_QMIX_I output at stage 1.
     *   value = h09  MOD_QMIX_Q output at stage 1.
     *   value = h0A  MOD_QMIX_I output at stage 2.
     *   value = h0B  MOD_QMIX_Q output at stage 2.
     *   value = h0C  MOD_QMIX_I output at stage 3.
     *   value = h0D  MOD_QMIX_Q output at stage 3.
     *   value = h10  low pass MOD_CIC I output.
     *   value = h11  low pass MOD_CIC Q output.
     *   value = h12  signal forming MOD_FIR I output.
     *   value = h13  signal forming MOD_FIR Q output.
     *   value = h14  interpolator CAR_CIC I output 41.664 MHz output.
     *   value = h15  interpolator CAR_CIC Q output 41.664 MHz output.
     *   value = h18  CAR_QMIX_I output.
     *   value = h19  CAR_QMIX_Q output.
     *   value = h1C  AMP_RF output.
     *   value = h3F  test vector as signal @see red_pitaya_radiobox.sv for details.
     *
     * bit h17..h16: n/a
     *
     * bit h1D..h18: RFOUT2 source connection point
     *   value = h00  silence.
     *   value = h01  silence.
     *   value = h04  MUXIN_MIX in.
     *   value = h05  MOD_ADC in.
     *   value = h06  MOD_ADC out.
     *   value = h08  MOD_QMIX_I output at stage 1.
     *   value = h09  MOD_QMIX_Q output at stage 1.
     *   value = h0A  MOD_QMIX_I output at stage 2.
     *   value = h0B  MOD_QMIX_Q output at stage 2.
     *   value = h0C  MOD_QMIX_I output at stage 3.
     *   value = h0D  MOD_QMIX_Q output at stage 3.
     *   value = h10  low pass MOD_CIC I output.
     *   value = h11  low pass MOD_CIC Q output.
     *   value = h12  signal forming MOD_FIR I output.
     *   value = h13  signal forming MOD_FIR Q output.
     *   value = h14  interpolator CAR_CIC I output 41.664 MHz output.
     *   value = h15  interpolator CAR_CIC Q output 41.664 MHz output.
     *   value = h18  CAR_QMIX_I output.
     *   value = h19  CAR_QMIX_Q output.
     *   value = h1C  AMP_RF output.
     *   value = h3F  test vector as signal @see red_pitaya_radiobox.sv for details.
     *
     * bit h1F..h1E: n/a
     *
     */
    uint32_t src_con_pnt;


    /** @brief  R/W RB_CAR_OSC_INC_LO - CAR_OSC phase increment register, bits 31..0 (addr: 0x40600020)
     *
     * bit h1F..h00: LSB of CAR_OSC phase increment register.
     *
     */
    uint32_t car_osc_inc_lo;

    /** @brief  R/W RB_CAR_OSC_INC_HI - CAR_OSC phase increment register, bits 47..32 (addr: 0x40600024)
     *
     * bit h0F..h00: MSB of CAR_OSC phase increment register.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t car_osc_inc_hi;


    /** @brief  R/W RB_CAR_OSC_OFS_LO - CAR_OSC phase offset register, bits 31..0 (addr: 0x40600028)
     *
     * bit h1F..h00: LSB of CAR_OSC phase offset register.
     *
     */
    uint32_t car_osc_ofs_lo;

    /** @brief  R/W RB_CAR_OSC_OFS_HI - CAR_OSC phase offset register, bits 47..32 (addr: 0x4060002C)
     *
     * bit h0F..h00: MSB of CAR_OSC phase offset register.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t car_osc_ofs_hi;


    /** @brief  R/W RB_AMP_RF_GAIN - AMP RF gain register, bits 15..0 (addr: 0x40600030)
     *
     * bit h0F..h00: SIGNED 16 bit - Amplifier RF gain setting.
     *
     */
    uint32_t amp_rf_gain;


    /** @brief  Placeholder for addr: 0x40600034
     *
     * n/a
     *
     */
    uint32_t reserved_34;


    /** @brief  R/W RB_AMP_RF_OFS - AMP RF offset register, bits 15..0 (addr: 0x40600038)
     *
     * bit h0F..h00: SIGNED 16 bit - Amplifier RF offset value.
     *
     */
    uint32_t amp_rf_ofs;


    /** @brief  Placeholder for addr: 0x4060003C
     *
     * n/a
     *
     */
    uint32_t reserved_3c;


    /** @brief  R/W RB_MOD_OSC_INC_LO - MOD_OSC phase increment register, bits 31..0 (addr: 0x40600040)
     *
     * bit h1F..h00: LSB of MOD_OSC phase increment register.
     *
     */
    uint32_t mod_osc_inc_lo;

    /** @brief  R/W RB_MOD_OSC_INC_HI - MOD_OSC phase increment register, bits 47..32 (addr: 0x40600044)
     *
     * bit h0F..h00: MSB of MOD_OSC phase increment register.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t mod_osc_inc_hi;


    /** @brief  R/W RB_MOD_OSC_OFS_LO - MOD_OSC phase offset register, bits 31..0 (addr: 0x40600048)
     *
     * bit h1F..h00: LSB of MOD_OSC phase offset register.
     *
     */
    uint32_t mod_osc_ofs_lo;

    /** @brief  R/W RB_MOD_OSC_OFS_HI - MOD_OSC phase offset register, bits 47..32 (addr: 0x4060004C)
     *
     * bit h0F..h00: MSB of MOD_OSC phase offset register.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t mod_osc_ofs_hi;


    /** @brief  R/W RB_MOD_QMIX_GAIN - MOD_QMIX amplitude register, bits 15..0 (addr: 0x40600050)
     *
     * bit h0F..h00: SIGNED 16 bit - MOD_QMIX amplitude setting.
     *
     */
    uint32_t mod_qmix_gain;


    /** @brief  Placeholder for addr: 0x40600054
     *
     * n/a
     *
     */
    uint32_t reserved_54;


    /** @brief  R/W RB_MOD_QMIX_OFS_LO - MOD_QMIX offset register, bits 31..0 (addr: 0x40600058)
     *
     * bit h1F..h00: LSB of MOD_QMIX offset value.
     *
     */
    uint32_t mod_qmix_ofs_lo;

    /** @brief  R/W RB_MOD_QMIX_OFS_HI - MOD_QMIX offset register, bits 47..32 (addr: 0x4060005C)
     *
     * bit h0F..h00: MSB of MOD_QMIX offset value.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t mod_qmix_ofs_hi;


    /** @brief  R/W RB_MUXIN_SRC - analog MUX input selector (addr: 0x40600060)
     *
     * bit h05..h00: source ID (extended from the XADC source ID)
     *   value = h00  no external signal used, MOD_OSC used instead.
     *   value = h03  Vp_Vn,     mapped to: vin[4].
     *   value = h10  XADC CH#0, mapped to: AI1.
     *   value = h11  XADC CH#1, mapped to: AI0.
     *   value = h18  XADC CH#8, mapped to: AI2.
     *   value = h19  XADC CH#9, mapped to: AI3.
     *   value = h20  ADC0,      mapped to: RF Input 1.
     *   value = h21  ADC1,      mapped to: RF Input 2.
     *
     */
    uint32_t muxin_src;

    /** @brief  R/W RB_MUXIN_GAIN - gain for analog MUX input amplifier (addr: 0x40600064)
     *
     * bit h0F..h00: SIGNED 16 bit - gain for MUXIN output amplifier.
     *
     * bit h12..h10: input booster left shift value from d0 .. d7 gives amplification of: 1x .. 128x.
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
 * @param[in]  rb_run        RadioBox application  0: disabled, else: enabled.
 * @param[in]  modsrc        0==(none), 1==RF Input 1, 2==RF Input 2, 4==EXP AI0, 5==EXP AI1, 6==EXP AI2, 7==EXP AI3, 15==MOD_OSC
 * @param[in]  modtyp        when modsrc == MOD_OSC: 0==USB, 1==LSB, 2==AM, 3==FM, 4==PM - else ignored.
 * @param[in]  src_con_pnt   RB LED controller setting to be used.
 * @param[in]  car_osc_qrg   Frequency for CAR_OSC in Hz.
 * @param[in]  mod_osc_qrg   Frequency for MOD_OSC in Hz.
 * @param[in]  amp_rf_gain   Vpp of AMP_RF output in mV.
 * @param[in]  mod_osc_mag   Magnitude of MOD_OSC mixer output. AM: 0-100%, FM: 0-1000000 Hz deviation, PM: 0-360°.
 * @param[in]  muxin_gain    Slider value between 0 and 100 for the MUXIN range slider. 50 means amplification of 1:1, 0 and 100 full scale, logarithmic.
 */
void fpga_rb_set_ctrl(int rb_run, int modsrc, int modtyp, int src_con_pnt, double car_osc_qrg, double mod_osc_qrg, double amp_rf_gain, double mod_osc_mag, double muxin_gain);

/**
 * @brief Calculates and programs the FPGA CAR_OSC for AM and PM
 *
 * @param[in]  car_osc_qrg   Frequency for CAR_OSC in Hz.
 */
//void fpga_rb_set_car_osc_mod_none_am_pm(double car_osc_qrg);
void fpga_rb_set_car_osc_qrg__4mod_cw_ssb_am_pm(double car_osc_qrg);

/**
 * @brief Calculates and programs the FPGA CAR_OSC mixer for AM
 *
 * @param[in]  amp_rf_gain  Vpp amplitude in mV.
 * @param[in]  amp_rf_ofs   Vpp amplitude in mV.
 */
void fpga_rb_set_amp_rf_gain_ofs__4mod_all(double amp_rf_gain, double amp_rf_ofs);

/**
 * @brief Calculates and programs the FPGA MOD_OSC for AM and PM
 *
 * @param[in]  mod_osc_qrg   Frequency for MOD_OSC in Hz.
 */
void fpga_rb_set_mod_osc_qrg__4mod_ssbweaver_am_fm_pm(double mod_osc_qrg);

/**
 * @brief Calculates and programs the FPGA MOD_OSC mixer for AM
 *
 * @param[in]  mod_osc_grade   Magnitude grade 0% .. 100%.
 * @param[in]  isOffset        1=based on 100% carrier minus modulation grade, 0=no carrier.
 */
void fpga_rb_set_mod_qmix_gain_ofs__4mod_cw_ssbweaver_am(double mod_qmix_grade, int isOffset);

/**
 * @brief Calculates and programs the FPGA MOD_OSC mixer for FM
 *
 * @param[in]  car_osc_qrg   Frequency for CAR_OSC in Hz.
 * @param[in]  mod_osc_mag   Deviation in Hz.
 */
void fpga_rb_set_mod_qmix_gain_ofs__4mod_fm(double car_osc_qrg, double mod_osc_mag);

/**
 * @brief Calculates and programs the FPGA MOD_OSC mixer for PM
 *
 * @param[in]  car_osc_qrg   Base frequency in Hz.
 * @param[in]  mod_osc_mag   Deviation in deg.
 */
void fpga_rb_set_mod_qmix_gain_ofs__4mod_pm(double car_osc_qrg, double mod_osc_mag);

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
