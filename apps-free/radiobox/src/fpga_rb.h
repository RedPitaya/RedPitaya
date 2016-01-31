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
    /* OMNI section */
    FPGA_RB_CTRL                   = 0x00000,                                                              // h000: RB control register
    FPGA_RB_STATUS                 = 0x00004,                                                              // h004: EB status register
    FPGA_RB_ICR                    = 0x00008,                                                              // h008: RB interrupt control register
    FPGA_RB_ISR                    = 0x0000C,                                                              // h00C: RB interrupt status register
    FPGA_RB_DMA_CTRL               = 0x00010,                                                              // h010: RB DMA control register
    //FPGA_RD_RB_RSVD_H014,
    //FPGA_RD_RB_RSVD_H018,
    FPGA_RB_CON_SRC_PNT            = 0x0001C,                                                              // h01C: RB_LED, RB_RFOUT1 and RB_RFOUT2 connection matrix

    /* TX section */
    FPGA_RB_TX_CAR_OSC_INC_LO      = 0x00020,                                                              // h020: RB TX_CAR_OSC increment register              LSB:        (Bit 31: 0)
    FPGA_RB_TX_CAR_OSC_INC_HI      = 0x00024,                                                              // h024: RB TX_CAR_OSC increment register              MSB: 16'b0, (Bit 47:32)
    FPGA_RB_TX_CAR_OSC_OFS_LO      = 0x00028,                                                              // h028: RB TX_CAR_OSC offset register                 LSB:        (Bit 31: 0)
    FPGA_RB_TX_CAR_OSC_OFS_HI      = 0x0002C,                                                              // h02C: RB TX_CAR_OSC offset register                 MSB: 16'b0, (Bit 47:32)
    FPGA_RB_TX_CAR_OSC_MIX_GAIN    = 0x00030,                                                              // h030: RB TX_CAR_OSC mixer gain:     SIGNED 16 bit
    //FPGA_RD_RB_TX_RSVD_H034,
    FPGA_RB_TX_CAR_OSC_MIX_OFS     = 0x00038,                                                              // h038: RB TX_CAR_OSC mixer offset:   SIGNED 17 bit
    //FPGA_RD_RB_TX_RSVD_H03C,
    FPGA_RB_TX_MOD_OSC_INC_LO      = 0x00040,                                                              // h040: RB TX_MOD_OSC increment register              LSB:        (Bit 31: 0)
    FPGA_RB_TX_MOD_OSC_INC_HI      = 0x00044,                                                              // h044: RB TX_MOD_OSC increment register              MSB: 16'b0, (Bit 47:32)
    FPGA_RB_TX_MOD_OSC_OFS_LO      = 0x00048,                                                              // h048: RB TX_MOD_OSC offset register                 LSB:        (Bit 31: 0)
    FPGA_RB_TX_MOD_OSC_OFS_HI      = 0x0004C,                                                              // h04C: RB TX_MOD_OSC offset register                 MSB: 16'b0, (Bit 47:32)
    FPGA_RB_TX_MOD_OSC_MIX_GAIN    = 0x00050,                                                              // h050: RB TX_MOD_OSC mixer gain:     SIGNED 16 bit
    //FPGA_RD_RB_TX_RSVD_H054,
    FPGA_RB_TX_MOD_OSC_MIX_OFS_LO  = 0x00058,                                                              // h058: RB TX_MOD_OSC mixer offset:   SIGNED 48 bit   LSB:        (Bit 31: 0)
    FPGA_RB_TX_MOD_OSC_MIX_OFS_HI  = 0x0005C,                                                              // h05C: RB TX_MOD_OSC mixer offset:   SIGNED 48 bit   MSB: 16'b0, (Bit 47:32)
    FPGA_RB_TX_MUXIN_SRC           = 0x00060,                                                              // h060: RB analog TX MUX input selector:  ...  @see below
    FPGA_RB_TX_MUXIN_GAIN          = 0x00064,                                                              // h064: RB analog TX MUX gain for input amplifier

    /* RX section */
    //FPGA_RD_RB_RX_RSVD_H100,
    //FPGA_RD_RB_RX_RSVD_H104,
    //FPGA_RD_RB_RX_RSVD_H108,
    //FPGA_RD_RB_RX_RSVD_H10C,
    //FPGA_RD_RB_RX_RSVD_H110,
    //FPGA_RD_RB_RX_RSVD_H114,
    //FPGA_RD_RB_RX_RSVD_H118,
    //FPGA_RD_RB_RX_RSVD_H11C,
    FPGA_RW_RB_RX_CAR_OSC_INC_LO   = 0x00120,                                                              // h120: RB RX_CAR_OSC increment register              LSB:        (Bit 31: 0)
    FPGA_RW_RB_RX_CAR_OSC_INC_HI   = 0x00124,                                                              // h124: RB RX_CAR_OSC increment register              MSB: 16'b0, (Bit 47:32)
    FPGA_RW_RB_RX_CAR_OSC_OFS_LO   = 0x00128,                                                              // h128: RB RX_CAR_OSC offset register                 LSB:        (Bit 31: 0)
    FPGA_RW_RB_RX_CAR_OSC_OFS_HI   = 0x0012C,                                                              // h12C: RB RX_CAR_OSC offset register                 MSB: 16'b0, (Bit 47:32)
    FPGA_RW_RB_RX_MOD_ADD_GAIN     = 0x00130,                                                              // h130: RB RX_MOD_OSC mixer gain:     SIGNED 16 bit
    //FPGA_RD_RB_RX_RSVD_H134,
    FPGA_RW_RB_RX_MOD_ADD_OFS      = 0x00138,                                                              // h138: RB RX_MOD_OSC mixer offset:   SIGNED 17 bit
    //FPGA_RD_RB_RX_RSVD_H13C,
    FPGA_RW_RB_RX_MOD_OSC_INC_LO   = 0x00140,                                                              // h140: RB RX_MOD_OSC increment register              LSB:        (Bit 31: 0)
    FPGA_RW_RB_RX_MOD_OSC_INC_HI   = 0x00144,                                                              // h144: RB RX_MOD_OSC increment register              MSB: 16'b0, (Bit 47:32)
    FPGA_RW_RB_RX_MOD_OSC_OFS_LO   = 0x00148,                                                              // h148: RB RX_MOD_OSC offset register                 LSB:        (Bit 31: 0)
    FPGA_RW_RB_RX_MOD_OSC_OFS_HI   = 0x0014C,                                                              // h14C: RB RX_MOD_OSC offset register                 MSB: 16'b0, (Bit 47:32)

    FPGA_RB_RX_MUXIN_SRC           = 0x00160,                                                              // h160: RB analog RX MUX input selector
    FPGA_RB_RX_MUXIN_GAIN          = 0x00164                                                               // h164: RB analog RX MUX gain for input amplifier
} FPGA_RB_REG_ENUMS;

/** @brief FPGA registry structure for the RadioBox sub-module.
 *
 * This structure is the direct image of the physical FPGA memory for the RadioBox sub-module.
 * It assures direct read / write FPGA access when it is mapped to the appropriate memory address
 * through the /dev/mem device.
 */
typedef struct fpga_rb_reg_mem_s {

    /* OMNI section */

    /** @brief  R/W RB_CTRL - Control register (addr: 0x40600000)
     *
     * bit h00: ENABLE - '1' enables the RadioBox sub-module. DDS-Oscillators, multipliers, LED handling are turned on. The DAC and LEDs are connected to this sub-module when enabled.
     *
     * bit h01: RESET TX_CAR_OSC - '1' resets the TX_CAR_OSC (carrier oscillator) to its initial state like the accumulating phase register.
     *
     * bit h02: RESET TX_MOD_OSC - '1' resets the TX_MOD_OSC (modulation oscillator) to its initial state like the accumulating phase register.
     *
     * bit h03: n/a
     *
     * bit h04: TX_CAR_OSC RESYNC - '1' stops incrementing the accumulating phase register. That holds the oscillator just there, where it is. With '0' the TX_CAR_OSC resumes operation.
     *
     * bit h05: TX_CAR_OSC INC SRC STREAM - '1' places input MUXer for TX_CAR_OSC DDS increment input to the first streamed input pipe. '0' places MUXer to registers "TX_CAR_OSC INC HI" and "TX_CAR_OSC INC LO".
     *
     * bit h06: TX_CAR_OSC OFS SRC STREAM - '1' places input MUXer for TX_CAR_OSC DDS offset input to the first streamed input pipe. '0' places MUXer to registers "TX_CAR_OSC OFS HI" and "TX_CAR_OSC OFS LO".
     *
     * bit h0B..h07: n/a
     *
     * bit h0C: TX_MOD_OSC RESYNC - '1' stops incrementing the accumulating phase register. That holds the oscillator just there, where it is. With '0' the TX_CAR_OSC resumes operation.
     *
     * bit h0D: TX_MOD_OSC INC SRC STREAM - '1' places input MUXer for TX_MOD_OSC DDS increment input to the second streamed input pipe. '0' places MUXer to registers "TX_MOD_OSC INC HI" and "TX_MOD_OSC INC LO".
     *
     * bit h0E: TX_MOD_OSC OFS SRC STREAM - '1' places input MUXer for TX_MOD_OSC DDS offset input to the second streamed input pipe. '0' places MUXer to registers "TX_MOD_OSC OFS HI" and "TX_MOD_OSC OFS LO".
     *
     * bit h10..h0F: n/a
     *
     * bit h11: RESET RX_CAR_OSC - '1' resets the RX_CAR_OSC (carrier oscillator) to its initial state like the accumulating phase register.
     *
     * bit h12: RESET RX_MOD_OSC - '1' resets the RX_MOD_OSC (SSB weaver beat / modulation oscillator) to its initial state like the accumulating phase register.
     *
     * bit h13: n/a
     *
     * bit h14: RX_CAR_OSC RESYNC - '1' stops incrementing the accumulating phase register. That holds the oscillator just there, where it is. With '0' the RX_CAR_OSC resumes operation.
     *
     * bit h1B..h15: n/a
     *
     * bit h1C: RX_MOD_OSC RESYNC - '1' stops incrementing the accumulating phase register. That holds the oscillator just there, where it is. With '0' the RX_MOD_OSC resumes operation.
     *
     * bit h1F..h1D: n/a
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
     * bit h04: STAT_TX_CAR_OSC_ZERO - '1' TX_CAR_OSC output equals zero. This state is based on the output of the DDS oscillator itself.
     *
     * bit h05: STAT_TX_CAR_OSC_VALID - '1' TX_CAR_OSC output is valid. After turning this sub-module active it needs some clocks going into valid state.
     *
     * bit h06: n/a
     *
     * bit h07: n/a
     *
     * bit h08: STAT_TX_MOD_OSC_ZERO - '1' TX_MOD_OSC output equals zero. This state is based on the output of the DDS oscillator itself.
     *
     * bit h09: STAT_TX_MOD_OSC_VALID - '1' TX_MOD_OSC output is valid. After turning this sub-module active it needs some clocks going into valid state.
     *
     * bit h0A: n/a
     *
     * bit h0B: n/a
     *
     * bit h0C: STAT_RX_CAR_OSC_ZERO - '1' RX_CAR_OSC output equals zero. This state is based on the output of the DDS oscillator itself.
     *
     * bit h0D: STAT_RX_CAR_OSC_VALID - '1' RX_CAR_OSC output is valid. After turning this sub-module active it needs some clocks going into valid state.
     *
     * bit h0E: n/a
     *
     * bit h0F: n/a
     *
     * bit h10: STAT_RX_MOD_OSC_ZERO - '1' RX_MOD_OSC output equals zero. This state is based on the output of the DDS oscillator itself.
     *
     * bit h11: STAT_RX_MOD_OSC_VALID - '1' RX_MOD_OSC output is valid. After turning this sub-module active it needs some clocks going into valid state.
     *
     * bit h17..h12: n/a
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
    uint32_t reserved_014;


    /** @brief  R/W RB_PWR_CTRL - power savings control register (addr: 0x40600018)
     *
     * bit h07..h00: TX modulation variant
     *   value = h00  no power savings, all clocks of the transceiver are turned on
     *   value = h01  complete transmitter is turned off
     *   value = h02  components of the SSB-USB transmitter are turned on
     *   value = h03  components of the SSB-LSB transmitter are turned on
     *   value = h04  components of the AM transmitter are turned on
     *   value = h07  components of the FM transmitter are turned on
     *   value = h08  components of the PM transmitter are turned on
     *
     * bit h0F..h08: RX modulation variant
     *   value = h00  no power savings, all clocks of the receiver are turned on
     *   value = h01  complete receiver is turned off
     *   value = h02  components of the SSB-USB receiver are turned on
     *   value = h03  components of the SSB-LSB receiver are turned on
     *   value = h04  components of the AM receiver are turned on
     *   value = h05  components of the AM syncro mode USB receiver are turned on
     *   value = h06  components of the AM syncro mode LSB receiver are turned on
     *   value = h07  components of the FM receiver are turned on
     *   value = h08  components of the PM receiver are turned on
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t pwr_ctrl;


    /** @brief  R/W RB_CON_SRC_PNT - output connection matrix for
                                     RB_LEDs, RFOUT1 and RFOUT2 (addr: 0x4060001C)
     *
     * bit h05..h00: LEDs magnitude scope (logarithmic)  source position
     *   value = h00  RadioBox does not touch LED state of other sub-module(s).
     *   value = h01  All LEDs are driven off.
     *
     *   value = h04  TX_MUXIN_MIX in.
     *   value = h05  TX_MOD_ADC in.
     *   value = h06  TX_MOD_ADC out.
     *
     *   value = h08  TX_MOD_OSC_I output.
     *   value = h09  TX_MOD_OSC_Q output.
     *   value = h0A  TX_MOD_QMIX_I output at stage 1.
     *   value = h0B  TX_MOD_QMIX_Q output at stage 1.
     *   value = h0C  TX_MOD_QMIX_I output at stage 2.
     *   value = h0D  TX_MOD_QMIX_Q output at stage 2.
     *   value = h0E  TX_MOD_QMIX_I output at stage 3.
     *   value = h0F  TX_MOD_QMIX_Q output at stage 3.
     *
     *   value = h10  TX_MOD_CIC I output.
     *   value = h11  TX_MOD_CIC Q output.
     *   value = h12  TX_MOD_FIR I output.
     *   value = h13  TX_MOD_FIR Q output.
     *   value = h14  TX_CAR_CIC_41M664 I output 41.664 MHz output.
     *   value = h15  TX_CAR_CIC_41M664 Q output 41.664 MHz output.
     *
     *   value = h18  TX_CAR_OSC_I output.
     *   value = h19  TX_CAR_OSC_Q output.
     *   value = h1A  TX_CAR_QMIX_I output.
     *   value = h1B  TX_CAR_QMIX_Q output.
     *
     *   value = h1C  TX_AMP_RF output.
     *
     *   value = h20  RX_CAR_OSC_I output.
     *   value = h21  RX_CAR_OSC_Q output.
     *   value = h22  RX_CAR_QMIX_I output.
     *   value = h23  RX_CAR_QMIX_Q output.
     *   value = h24  RX_CAR_CIC1_I output.
     *   value = h25  RX_CAR_CIC1_Q output.
     *   value = h26  RX_CAR_CIC2_I output.
     *   value = h27  RX_CAR_CIC2_Q output.
     *
     *   value = h28  RX_MOD_FIR1_I output.
     *   value = h29  RX_MOD_FIR1_Q output.
     *   value = h2A  RX_MOD_OSC_I output.
     *   value = h2B  RX_MOD_OSC_Q output.
     *   value = h2C  RX_MOD_HLD_I output.
     *   value = h2D  RX_MOD_HLD_Q output.
     *   value = h2E  RX_MOD_QMIX_I output.
     *   value = h2F  RX_MOD_QMIX_Q output.
     *   value = h30  RX_MOD_FIR2_I output.
     *   value = h31  RX_MOD_FIR2_Q output.
     *   value = h32  RX_MOD_CIC2_I output.
     *   value = h33  RX_MOD_CIC2_Q output.
     *
     *   value = h38  RX_MOD_ADD output.
     *
     *   value = h3A  RX_AFC_FIR1_I output.
     *   value = h3B  RX_AFC_FIR1_Q output.
     *   value = h3C  RX_AFC_CORDIC_MAG carrier magnitude value.
     *   value = h3D  RX_AFC_CORDIC_PHS carrier phase value.
     *
     *   value = h3F  LEDs show current test vector @see red_pitaya_radiobox.sv for details.
     *
     * bit h0F..h06: n/a
     *
     * bit h15..h10: RFOUT1 source connection point
     *   value = h00  silence.
     *   value = h01  silence.
     *
     *   value = h04  TX_MUXIN_MIX in.
     *   value = h05  TX_MOD_ADC in.
     *   value = h06  TX_MOD_ADC out.
     *
     *   value = h08  TX_MOD_OSC_I output.
     *   value = h09  TX_MOD_OSC_Q output.
     *   value = h0A  TX_MOD_QMIX_I output at stage 1.
     *   value = h0B  TX_MOD_QMIX_Q output at stage 1.
     *   value = h0C  TX_MOD_QMIX_I output at stage 2.
     *   value = h0D  TX_MOD_QMIX_Q output at stage 2.
     *   value = h0E  TX_MOD_QMIX_I output at stage 3.
     *   value = h0F  TX_MOD_QMIX_Q output at stage 3.
     *
     *   value = h10  TX_MOD_CIC I output.
     *   value = h11  TX_MOD_CIC Q output.
     *   value = h12  TX_MOD_FIR I output.
     *   value = h13  TX_MOD_FIR Q output.
     *   value = h14  TX_CAR_CIC_41M664 I output 41.664 MHz output.
     *   value = h15  TX_CAR_CIC_41M664 Q output 41.664 MHz output.
     *
     *   value = h18  TX_CAR_OSC_I output.
     *   value = h19  TX_CAR_OSC_Q output.
     *   value = h1A  TX_CAR_QMIX_I output.
     *   value = h1B  TX_CAR_QMIX_Q output.
     *
     *   value = h1C  TX_AMP_RF output.
     *
     *   value = h20  RX_CAR_OSC_I output.
     *   value = h21  RX_CAR_OSC_Q output.
     *   value = h22  RX_CAR_QMIX_I output.
     *   value = h23  RX_CAR_QMIX_Q output.
     *   value = h24  RX_CAR_CIC1_I output.
     *   value = h25  RX_CAR_CIC1_Q output.
     *   value = h26  RX_CAR_CIC2_I output.
     *   value = h27  RX_CAR_CIC2_Q output.
     *
     *   value = h28  RX_MOD_FIR1_I output.
     *   value = h29  RX_MOD_FIR1_Q output.
     *   value = h2A  RX_MOD_OSC_I output.
     *   value = h2B  RX_MOD_OSC_Q output.
     *   value = h2C  RX_MOD_HLD_I output.
     *   value = h2D  RX_MOD_HLD_Q output.
     *   value = h2E  RX_MOD_QMIX_I output.
     *   value = h2F  RX_MOD_QMIX_Q output.
     *   value = h30  RX_MOD_FIR2_I output.
     *   value = h31  RX_MOD_FIR2_Q output.
     *   value = h32  RX_MOD_CIC2_I output.
     *   value = h33  RX_MOD_CIC2_Q output.
     *
     *   value = h38  RX_MOD_ADD output.
     *
     *   value = h3A  RX_AFC_FIR1_I output.
     *   value = h3B  RX_AFC_FIR1_Q output.
     *   value = h3C  RX_AFC_CORDIC_MAG carrier magnitude value.
     *   value = h3D  RX_AFC_CORDIC_PHS carrier phase value.
     *
     *   value = h3F  LEDs show current test vector @see red_pitaya_radiobox.sv for details.
     *
     * bit h17..h16: n/a
     *
     * bit h1D..h18: RFOUT2 source connection point
     *   value = h00  silence.
     *   value = h01  silence.
     *
     *   value = h04  TX_MUXIN_MIX in.
     *   value = h05  TX_MOD_ADC in.
     *   value = h06  TX_MOD_ADC out.
     *
     *   value = h08  TX_MOD_OSC_I output.
     *   value = h09  TX_MOD_OSC_Q output.
     *   value = h0A  TX_MOD_QMIX_I output at stage 1.
     *   value = h0B  TX_MOD_QMIX_Q output at stage 1.
     *   value = h0C  TX_MOD_QMIX_I output at stage 2.
     *   value = h0D  TX_MOD_QMIX_Q output at stage 2.
     *   value = h0E  TX_MOD_QMIX_I output at stage 3.
     *   value = h0F  TX_MOD_QMIX_Q output at stage 3.
     *
     *   value = h10  TX_MOD_CIC I output.
     *   value = h11  TX_MOD_CIC Q output.
     *   value = h12  TX_MOD_FIR I output.
     *   value = h13  TX_MOD_FIR Q output.
     *   value = h14  TX_CAR_CIC_41M664 I output 41.664 MHz output.
     *   value = h15  TX_CAR_CIC_41M664 Q output 41.664 MHz output.
     *
     *   value = h18  TX_CAR_OSC_I output.
     *   value = h19  TX_CAR_OSC_Q output.
     *   value = h1A  TX_CAR_QMIX_I output.
     *   value = h1B  TX_CAR_QMIX_Q output.
     *
     *   value = h1C  TX_AMP_RF output.
     *
     *   value = h20  RX_CAR_OSC_I output.
     *   value = h21  RX_CAR_OSC_Q output.
     *   value = h22  RX_CAR_QMIX_I output.
     *   value = h23  RX_CAR_QMIX_Q output.
     *   value = h24  RX_CAR_CIC1_I output.
     *   value = h25  RX_CAR_CIC1_Q output.
     *   value = h26  RX_CAR_CIC2_I output.
     *   value = h27  RX_CAR_CIC2_Q output.
     *
     *   value = h28  RX_MOD_FIR1_I output.
     *   value = h29  RX_MOD_FIR1_Q output.
     *   value = h2A  RX_MOD_OSC_I output.
     *   value = h2B  RX_MOD_OSC_Q output.
     *   value = h2C  RX_MOD_HLD_I output.
     *   value = h2D  RX_MOD_HLD_Q output.
     *   value = h2E  RX_MOD_QMIX_I output.
     *   value = h2F  RX_MOD_QMIX_Q output.
     *   value = h30  RX_MOD_FIR2_I output.
     *   value = h31  RX_MOD_FIR2_Q output.
     *   value = h32  RX_MOD_CIC2_I output.
     *   value = h33  RX_MOD_CIC2_Q output.
     *
     *   value = h38  RX_MOD_ADD output.
     *
     *   value = h3A  RX_AFC_FIR1_I output.
     *   value = h3B  RX_AFC_FIR1_Q output.
     *   value = h3C  RX_AFC_CORDIC_MAG carrier magnitude value.
     *   value = h3D  RX_AFC_CORDIC_PHS carrier phase value.
     *
     *   value = h3F  LEDs show current test vector @see red_pitaya_radiobox.sv for details.
     *
     * bit h1F..h1E: n/a
     *
     */
    uint32_t src_con_pnt;


    /* TX section */

    /** @brief  R/W RB_TX_CAR_OSC_INC_LO - TX_CAR_OSC phase increment register, bits 31..0 (addr: 0x40600020)
     *
     * bit h1F..h00: LSB of TX_CAR_OSC phase increment register.
     *
     */
    uint32_t tx_car_osc_inc_lo;

    /** @brief  R/W RB_TX_CAR_OSC_INC_HI - TX_CAR_OSC phase increment register, bits 47..32 (addr: 0x40600024)
     *
     * bit h0F..h00: MSB of TX_CAR_OSC phase increment register.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t tx_car_osc_inc_hi;


    /** @brief  R/W RB_TX_CAR_OSC_OFS_LO - TX_CAR_OSC phase offset register, bits 31..0 (addr: 0x40600028)
     *
     * bit h1F..h00: LSB of TX_CAR_OSC phase offset register.
     *
     */
    uint32_t tx_car_osc_ofs_lo;

    /** @brief  R/W RB_TX_CAR_OSC_OFS_HI - TX_CAR_OSC phase offset register, bits 47..32 (addr: 0x4060002C)
     *
     * bit h0F..h00: MSB of TX_CAR_OSC phase offset register.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t tx_car_osc_ofs_hi;


    /** @brief  R/W RB_TX_AMP_RF_GAIN - TX_AMP_RF gain register, bits 15..0 (addr: 0x40600030)
     *
     * bit h0F..h00: SIGNED 16 bit - TX Amplifier RF gain setting.
     *
     */
    uint32_t tx_amp_rf_gain;


    /** @brief  Placeholder for addr: 0x40600034
     *
     * n/a
     *
     */
    uint32_t reserved_034;


    /** @brief  R/W RB_TX_AMP_RF_OFS - TX AMP RF offset register, bits 15..0 (addr: 0x40600038)
     *
     * bit h0F..h00: SIGNED 16 bit - TX Amplifier RF offset value.
     *
     */
    uint32_t tx_amp_rf_ofs;


    /** @brief  Placeholder for addr: 0x4060003C
     *
     * n/a
     *
     */
    uint32_t reserved_03c;


    /** @brief  R/W RB_TX_MOD_OSC_INC_LO - TX_MOD_OSC phase increment register, bits 31..0 (addr: 0x40600040)
     *
     * bit h1F..h00: LSB of TX_MOD_OSC phase increment register.
     *
     */
    uint32_t tx_mod_osc_inc_lo;

    /** @brief  R/W RB_TX_MOD_OSC_INC_HI - TX_MOD_OSC phase increment register, bits 47..32 (addr: 0x40600044)
     *
     * bit h0F..h00: MSB of TX_MOD_OSC phase increment register.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t tx_mod_osc_inc_hi;


    /** @brief  R/W RB_TX_MOD_OSC_OFS_LO - TX_MOD_OSC phase offset register, bits 31..0 (addr: 0x40600048)
     *
     * bit h1F..h00: LSB of TX_MOD_OSC phase offset register.
     *
     */
    uint32_t tx_mod_osc_ofs_lo;

    /** @brief  R/W RB_TX_MOD_OSC_OFS_HI - TX_MOD_OSC phase offset register, bits 47..32 (addr: 0x4060004C)
     *
     * bit h0F..h00: MSB of TX_MOD_OSC phase offset register.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t tx_mod_osc_ofs_hi;


    /** @brief  R/W RB_TX_MOD_QMIX_GAIN - TX_MOD_QMIX amplitude register, bits 15..0 (addr: 0x40600050)
     *
     * bit h0F..h00: SIGNED 16 bit - TX_MOD_QMIX amplitude setting.
     *
     */
    uint32_t tx_mod_qmix_gain;


    /** @brief  Placeholder for addr: 0x40600054
     *
     * n/a
     *
     */
    uint32_t reserved_054;


    /** @brief  R/W RB_TX_MOD_QMIX_OFS_LO - TX_MOD_QMIX offset register, bits 31..0 (addr: 0x40600058)
     *
     * bit h1F..h00: LSB of TX_MOD_QMIX offset value.
     *
     */
    uint32_t tx_mod_qmix_ofs_lo;

    /** @brief  R/W RB_TX_MOD_QMIX_OFS_HI - TX_MOD_QMIX offset register, bits 47..32 (addr: 0x4060005C)
     *
     * bit h0F..h00: MSB of TX_MOD_QMIX offset value.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t tx_mod_qmix_ofs_hi;


    /** @brief  R/W RB_TX_MUXIN_SRC - analog MUX input selector (addr: 0x40600060)
     *
     * bit h05..h00: source ID (extended from the XADC source ID)
     *   value = h00  no external signal used, TX_MOD_OSC used instead.
     *   value = h03  Vp_Vn,     mapped to: vin[4].
     *   value = h10  XADC CH#0, mapped to: AI1.
     *   value = h11  XADC CH#1, mapped to: AI0.
     *   value = h18  XADC CH#8, mapped to: AI2.
     *   value = h19  XADC CH#9, mapped to: AI3.
     *   value = h20  ADC0,      mapped to: RF Input 1.
     *   value = h21  ADC1,      mapped to: RF Input 2.
     *
     */
    uint32_t tx_muxin_src;

    /** @brief  R/W RB_TX_MUXIN_GAIN - gain for analog TX MUX input amplifier (addr: 0x40600064)
     *
     * bit h0F..h00: SIGNED 16 bit - gain for TX MUXIN output amplifier.
     *
     * bit h12..h10: input booster left shift value from d0 .. d7 gives amplification of: 1x .. 128x.
     *
     */
    uint32_t tx_muxin_gain;


    /** @brief  Placeholder for addr: 0x40600068 .. 0x406000FC
     *
     * n/a
     *
     */
    uint32_t reserved_068To0fc[((0x0fc - 0x068) >> 2) + 1];



    /* RX section */

    /** @brief  Placeholder for addr: 0x40600100
     *
     * n/a
     *
     */
    uint32_t reserved_100;


    /** @brief  Placeholder for addr: 0x40600104
     *
     * n/a
     *
     */
    uint32_t reserved_104;


    /** @brief  Placeholder for addr: 0x40600108
     *
     * n/a
     *
     */
    uint32_t reserved_108;


    /** @brief  Placeholder for addr: 0x4060010C
     *
     * n/a
     *
     */
    uint32_t reserved_10C;


    /** @brief  Placeholder for addr: 0x40600110
     *
     * n/a
     *
     */
    uint32_t reserved_110;


    /** @brief  Placeholder for addr: 0x40600114
     *
     * n/a
     *
     */
    uint32_t reserved_114;


    /** @brief  Placeholder for addr: 0x40600118
     *
     * n/a
     *
     */
    uint32_t reserved_118;


    /** @brief  Placeholder for addr: 0x4060011C
     *
     * n/a
     *
     */
    uint32_t reserved_11C;


    /** @brief  R/W RB_RX_CAR_OSC_INC_LO - RX_CAR_OSC phase increment register, bits 31..0 (addr: 0x40600120)
     *
     * bit h1F..h00: LSB of RX_CAR_OSC phase increment register.
     *
     */
    uint32_t rx_car_osc_inc_lo;

    /** @brief  R/W RB_RX_CAR_OSC_INC_HI - RX_CAR_OSC phase increment register, bits 47..32 (addr: 0x40600124)
     *
     * bit h0F..h00: MSB of RX_CAR_OSC phase increment register.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t rx_car_osc_inc_hi;


    /** @brief  R/W RB_RX_CAR_OSC_OFS_LO - RX_CAR_OSC phase offset register, bits 31..0 (addr: 0x40600128)
     *
     * bit h1F..h00: LSB of RX_CAR_OSC phase offset register.
     *
     */
    uint32_t rx_car_osc_ofs_lo;

    /** @brief  R/W RB_RX_CAR_OSC_OFS_HI - RX_CAR_OSC phase offset register, bits 47..32 (addr: 0x4060012C)
     *
     * bit h0F..h00: MSB of RX_CAR_OSC phase offset register.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t rx_car_osc_ofs_hi;


    /** @brief  R/W RB_RX_AMP_RF_GAIN - RX_AMP_RF gain register, bits 15..0 (addr: 0x40600130)
      *
      * bit h0F..h00: SIGNED 16 bit - RX_MOD_ADD gain setting.
      *
      */
     uint32_t rx_mod_add_gain;


     /** @brief  Placeholder for addr: 0x40600134
      *
      * n/a
      *
      */
     uint32_t reserved_134;


     /** @brief  R/W RB_RX_MOD_ADD_OFS - RX_MOD_ADD offset register, bits 15..0 (addr: 0x40600138)
      *
      * bit h0F..h00: SIGNED 16 bit - RX_MOD_ADD offset value.
      *
      */
     uint32_t rx_mod_add_ofs;


     /** @brief  Placeholder for addr: 0x4060013C
      *
      * n/a
      *
      */
     uint32_t reserved_13c;


     /** @brief  R/W RB_RX_MOD_OSC_INC_LO - RX_MOD_OSC phase increment register, bits 31..0 (addr: 0x40600140)
      *
      * bit h1F..h00: LSB of RX_MOD_OSC phase increment register.
      *
      */
     uint32_t rx_mod_osc_inc_lo;

     /** @brief  R/W RB_RX_MOD_OSC_INC_HI - RX_MOD_OSC phase increment register, bits 47..32 (addr: 0x40600144)
      *
      * bit h0F..h00: MSB of RX_MOD_OSC phase increment register.
      *
      * bit h1F..h10: n/a
      *
      */
     uint32_t rx_mod_osc_inc_hi;


     /** @brief  R/W RB_RX_MOD_OSC_OFS_LO - RX_MOD_OSC phase offset register, bits 31..0 (addr: 0x40600148)
      *
      * bit h1F..h00: LSB of RX_MOD_OSC phase offset register.
      *
      */
     uint32_t rx_mod_osc_ofs_lo;

     /** @brief  R/W RB_RX_MOD_OSC_OFS_HI - RX_MOD_OSC phase offset register, bits 47..32 (addr: 0x4060014C)
      *
      * bit h0F..h00: MSB of RX_MOD_OSC phase offset register.
      *
      * bit h1F..h10: n/a
      *
      */
     uint32_t rx_mod_osc_ofs_hi;


     /** @brief  Placeholder for addr: 0x40600150
      *
      * n/a
      *
      */
     uint32_t reserved_150;


     /** @brief  Placeholder for addr: 0x40600154
      *
      * n/a
      *
      */
     uint32_t reserved_154;


     /** @brief  Placeholder for addr: 0x40600158
      *
      * n/a
      *
      */
     uint32_t reserved_158;


     /** @brief  Placeholder for addr: 0x4060015C
      *
      * n/a
      *
      */
     uint32_t reserved_15C;


     /** @brief  R/W RB_RX_MUX_SRC -  bits 31..0 (addr: 0x40600160)
      *
      * bit h05..h00: value.
      *   value = h00  off.
      *   value = h01  RF Input 1.
      *   value = h02  RF Input 2.
      *   value > h02  (undefined)
      *
      * bit h1F..h06: n/a
      *
      */
     uint32_t rx_muxin_src;

     /** @brief  R/W RB_RX_MUX_GAIN -  bits 15..0 (addr: 0x40600164)
      *
      * bit h0F..h00: gain value for the MUX input.
      *
      * bit h1F..h10: n/a
      *
      */
     uint32_t rx_muxin_gain;


     /** @brief  Placeholder for addr: 0x40600168
      *
      * n/a
      *
      */
     uint32_t reserved_168;


     /** @brief  Placeholder for addr: 0x4060016C
      *
      * n/a
      *
      */
     uint32_t reserved_16C;


     /** @brief  R/W RB_RX_AFC_CORDIC_MAG - RX_AFC_CORDIC magnitude register, bits 15..0 (addr: 0x40600170)
      *
      * bit h0F..h00: RX_AFC_CORDIC magnitude register.
      *
      * bit h1F..h10: n/a
      *
      */
     uint32_t rx_afc_cordic_mag;

     /** @brief  R/W RB_RX_AFC_CORDIC_phs - RX_AFC_CORDIC phase register, bits 15..0 (addr: 0x40600174)
      *
      * bit h0F..h00: RX_AFC_CORDIC phase register.
      *
      * bit h1F..h10: n/a
      *
      */
     uint32_t rx_afc_cordic_phs;

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
 * @param[in]  rb_run           RadioBox application  0: disabled, else: enabled.
 * @param[in]  tx_modsrc        0==(none), 1==RF Input 1, 2==RF Input 2, 4==EXP AI0, 5==EXP AI1, 6==EXP AI2, 7==EXP AI3, 15==TX_MOD_OSC
 * @param[in]  tx_modtyp        0==USB, 1==LSB, 2==AM, 3==FM, 4==PM - else ignored.
 * @param[in]  rx_modtyp        0==USB, 1==LSB, 2==AM, 3==FM, 4==PM - else ignored.
 * @param[in]  src_con_pnt      RB LED controller, RF Output 1 and RF Output 2 setting to be used.
 * @param[in]  rx_muxin_src     0==Off, 1==RF Input 1, 2==RF Input 2.
 * @param[in]  tx_car_osc_qrg   Frequency for TX_CAR_OSC in Hz.
 * @param[in]  tx_mod_osc_qrg   Frequency for TX_MOD_OSC in Hz.
 * @param[in]  tx_amp_rf_gain   Vpp of TX_AMP_RF output in mV.
 * @param[in]  tx_mod_osc_mag   Magnitude of TX_MOD_OSC mixer output. AM: 0-100%, FM: 0-1000000 Hz deviation, PM: 0-360°.
 * @param[in]  tx_muxin_gain    Slider value between 0% (value==0) and 100% (value==80) for the MUXIN range slider, 80 means amplification of 1:1. 80..100 logarithmic amplification.
 * @param[in]  rx_muxin_gain    Slider value between 0% and 100%.
 * @param[in]  rx_car_osc_qrg   Frequency for RX_CAR_OSC in Hz.
 */
void fpga_rb_set_ctrl(int rb_run, int tx_modsrc, int tx_modtyp, int rx_modtyp,
        int src_con_pnt, int rx_muxin_src,
        double tx_car_osc_qrg, double tx_mod_osc_qrg,
        double tx_amp_rf_gain, double tx_mod_osc_mag,
        double tx_muxin_gain, double rx_muxin_gain,
        double rx_car_osc_qrg);


/**
 * @brief Power savings control - switches transmitter part to the selected modulation variant
 *
 * @param[in]  tx_modtyp   TX modulation variant.
 */
void fpga_rb_set_tx_modtyp(int tx_modtyp);

/**
 * @brief Power savings control - switches receiver part to the selected modulation variant
 *
 * @param[in]  rx_modtyp   RX modulation variant.
 */
void fpga_rb_set_rx_modtyp(int rx_modtyp);


/**
 * @brief Calculates and programs the FPGA TX_CAR_OSC for CW, SSB, AM and PM
 *
 * @param[in]  tx_car_osc_qrg   Frequency for TX_CAR_OSC in Hz.
 */
//void fpga_rb_set_tx_car_osc_mod_none_am_pm(double tx_car_osc_qrg);
void fpga_rb_set_tx_car_osc_qrg__4mod_cw_ssb_am_pm(double tx_car_osc_qrg);

/**
 * @brief Calculates and programs the FPGA TX_CAR_OSC mixer for AM
 *
 * @param[in]  tx_amp_rf_gain  Vpp amplitude in mV.
 * @param[in]  tx_amp_rf_ofs   Vpp amplitude in mV.
 */
void fpga_rb_set_tx_amp_rf_gain_ofs__4mod_all(double tx_amp_rf_gain, double tx_amp_rf_ofs);

/**
 * @brief Calculates and programs the FPGA TX_MOD_OSC for AM, FM and PM
 *
 * @param[in]  tx_mod_osc_qrg   Frequency for TX_MOD_OSC in Hz.
 */
void fpga_rb_set_tx_mod_osc_qrg__4mod_ssbweaver_am_fm_pm(double tx_mod_osc_qrg);

/**
 * @brief Calculates and programs the FPGA TX_MOD_OSC mixer for CW, SSB and AM
 *
 * @param[in]  tx_mod_osc_grade   Magnitude grade 0% .. 100%.
 * @param[in]  isOffset           1=based on 100% carrier minus modulation grade, 0=no carrier.
 */
void fpga_rb_set_tx_mod_qmix_gain_ofs__4mod_cw_ssbweaver_am(double tx_mod_qmix_grade, int isOffset);

/**
 * @brief Calculates and programs the FPGA TX_MOD_OSC mixer for FM
 *
 * @param[in]  tx_car_osc_qrg   Frequency for TX_CAR_OSC in Hz.
 * @param[in]  tx_mod_osc_mag   Deviation in Hz.
 */
void fpga_rb_set_tx_mod_qmix_gain_ofs__4mod_fm(double tx_car_osc_qrg, double tx_mod_osc_mag);

/**
 * @brief Calculates and programs the FPGA TX_MOD_OSC mixer for PM
 *
 * @param[in]  tx_car_osc_qrg   Base frequency in Hz.
 * @param[in]  tx_mod_osc_mag   Deviation in deg.
 */
void fpga_rb_set_tx_mod_qmix_gain_ofs__4mod_pm(double tx_car_osc_qrg, double tx_mod_osc_mag);

/**
 * @brief Calculates and programs the FPGA TX_MUXIN gain setting
 *
 * @param[in]  tx_muxin_gain Slider value between 0 and 100 for the MUXIN range slider. 50 means amplification of 1:1, 0 and 100 full scale, logarithmic.
 */
void fpga_rb_set_tx_muxin_gain(double tx_muxin_gain);

/**
 * @brief Calculates and programs the FPGA RX_CAR_OSC for SSB, AM and FM
 *
 * @param[in]  rx_car_osc_qrg   Frequency for RX_CAR_OSC in Hz.
 */
void fpga_rb_set_rx_car_osc_qrg__4mod_ssb_am_fm_pm(double rx_car_osc_qrg);

/**
 * @brief Calculates and programs the FPGA RX_MOD_OSC for SSB and AM
 *
 * @param[in]  rx_mod_osc_qrg   Frequency for RX_MOD_OSC in Hz.
 */
void fpga_rb_set_rx_mod_osc_qrg__4mod_ssbweaver_am(double rx_mod_osc_qrg);

/**
 * @brief Calculates and programs the FPGA RX_MOD_ADD mixer
 *
 * @param[in]  tx_amp_rf_gain  gain between 0% .. 100%.
 * @param[in]  tx_amp_rf_ofs   Vpp amplitude in mV.
 */
void fpga_rb_set_rx_mod_add_gain_ofs__4mod_all(double rx_mod_add_gain, double rx_mod_add_ofs);


#if 0
uint32_t fpga_rb_read_register(unsigned int rb_reg_ofs);
int fpga_rb_write_register(unsigned int rb_reg_ofs, uint32_t value);
#endif

/** @} */


#endif /* __FPGA_RB_H */
