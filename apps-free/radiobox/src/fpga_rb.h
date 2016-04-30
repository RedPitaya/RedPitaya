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

#include "calib.h"

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
    FPGA_RW_RB_CTRL                                                                            = 0x00000,  // h000: RB control register
    FPGA_RD_RB_STATUS                                                                          = 0x00004,  // h004: RB status register
    FPGA_RW_RB_ICR                                                                             = 0x00008,  // h008: RB interrupt control register
    FPGA_RD_RB_ISR                                                                             = 0x0000C,  // h00C: RB interrupt status register

    FPGA_RW_RB_DMA_CTRL                                                                        = 0x00010,  // h010: RB DMA control register
    FPGA_RW_RB_PWR_CTRL                                                                        = 0x00014,  // h014: RB power savings control register             RX_MOD:     (Bit  7: 0), TX_MOD:     (Bit 15: 8)
    FPGA_RW_RB_CON_SRC_PNT                                                                     = 0x00018,  // h018: RB_LED, RB_RFOUT1 and RB_RFOUT2 connection matrix
	FPGA_RW_RB_LINE_IN_SRC_CON_PNT                                                             = 0x0001C,  // h01C: LINE-IN11 L/R, LINE-IN2 L/R connection matrix

    /* TX section */
    FPGA_RW_RB_TX_CAR_OSC_INC_LO                                                               = 0x00020,  // h020: RB TX_CAR_OSC increment register              LSB:        (Bit 31: 0)
    FPGA_RW_RB_TX_CAR_OSC_INC_HI                                                               = 0x00024,  // h024: RB TX_CAR_OSC increment register              MSB: 16'b0, (Bit 47:32)
    FPGA_RW_RB_TX_CAR_OSC_OFS_LO                                                               = 0x00028,  // h028: RB TX_CAR_OSC offset register                 LSB:        (Bit 31: 0)
    FPGA_RW_RB_TX_CAR_OSC_OFS_HI                                                               = 0x0002C,  // h02C: RB TX_CAR_OSC offset register                 MSB: 16'b0, (Bit 47:32)

    FPGA_RW_RB_TX_CAR_OSC_INC_SCNR_LO                                                          = 0x00030,  // h030: RB TX_CAR_OSC_SCNR increment register         LSB:        (Bit 31: 0)
    FPGA_RW_RB_TX_CAR_OSC_INC_SCNR_HI                                                          = 0x00034,  // h034: RB TX_CAR_OSC_SCNR increment register         MSB: 16'b0, (Bit 47:32)
    FPGA_RW_RB_TX_RF_AMP_GAIN                                                                  = 0x00038,  // h038: RB TX_RF_AMP gain:            SIGNED 16 bit
    FPGA_RW_RB_TX_RF_AMP_OFS                                                                   = 0x0003C,  // h03C: RB TX_RF_AMP offset:          SIGNED 16 bit

    FPGA_RW_RB_TX_MOD_OSC_INC_LO                                                               = 0x00040,  // h040: RB TX_MOD_OSC increment register              LSB:        (Bit 31: 0)
    FPGA_RW_RB_TX_MOD_OSC_INC_HI                                                               = 0x00044,  // h044: RB TX_MOD_OSC increment register              MSB: 16'b0, (Bit 47:32)
    FPGA_RW_RB_TX_MOD_OSC_OFS_LO                                                               = 0x00048,  // h048: RB TX_MOD_OSC offset register                 LSB:        (Bit 31: 0)
    FPGA_RW_RB_TX_MOD_OSC_OFS_HI                                                               = 0x0004C,  // h04C: RB TX_MOD_OSC offset register                 MSB: 16'b0, (Bit 47:32)

    FPGA_RW_RB_TX_MOD_QMIX_GAIN                                                                = 0x00050,  // h050: RB TX_MOD_OSC mixer gain:   UNSIGNED 16 bit
    //FPGA_RD_RB_TX_RSVD_H054,
    FPGA_RW_RB_TX_MOD_QMIX_OFS_LO                                                              = 0x00058,  // h058: RB TX_MOD_OSC mixer offset:   SIGNED 48 bit   LSB:        (Bit 31: 0)
    FPGA_RW_RB_TX_MOD_QMIX_OFS_HI                                                              = 0x0005C,  // h05C: RB TX_MOD_OSC mixer offset:   SIGNED 48 bit   MSB: 16'b0, (Bit 47:32)

    FPGA_RW_RB_TX_MUXIN_SRC                                                                    = 0x00060,  // h060: RB analog TX MUX input selector:
                                                                                                           //       d0 =(none),   d3 =VpVn,
                                                                                                           //       d16=EXT-CH0,  d24=EXT-CH8,
                                                                                                           //       d17=EXT-CH1,  d25=EXT-CH9,
                                                                                                           //       d32=adc_i[0], d33=adc_i[1]
    FPGA_RW_RB_TX_MUXIN_GAIN                                                                   = 0x00064,  // h064: RB analog TX MUX gain:      UNSIGNED 16 bit
    FPGA_RW_RB_TX_MUXIN_OFS                                                                    = 0x00068,  // h068: RB analog TX MUX gain:        SIGNED 16 bit
    //FPGA_RD_RB_TX_RSVD_H06C,

    /* RX section */
    FPGA_RW_RB_RX_CAR_CALC_WEAVER_INC_LO                                                       = 0x00100,  // h100: weaver increment phase correction register    LSB:        (Bit 31: 0)
    FPGA_RW_RB_RX_CAR_CALC_WEAVER_INC_HI                                                       = 0x00104,  // h104: weaver increment phase correction register    MSB: 16'b0, (Bit 47:32)
    //FPGA_RD_RB_RX_RSVD_H108,
    //FPGA_RD_RB_RX_RSVD_H10C,

    FPGA_RD_RB_RX_CAR_AFC_INC_LO                                                               = 0x00110,  // h110: RB RX_CAR_AFC increment register              LSB:        (Bit 31: 0)
    FPGA_RD_RB_RX_CAR_AFC_INC_HI                                                               = 0x00114,  // h114: RB RX_CAR_AFC increment register              MSB: 16'b0, (Bit 47:32)
    FPGA_RD_RB_RX_CAR_SUM_INC_LO                                                               = 0x00118,  // h118: RB RX_CAR_SUM increment register              LSB:        (Bit 31: 0)
    FPGA_RD_RB_RX_CAR_SUM_INC_HI                                                               = 0x0011C,  // h11C: RB RX_CAR_SUM increment register              MSB: 16'b0, (Bit 47:32)

    FPGA_RW_RB_RX_CAR_OSC_INC_LO                                                               = 0x00120,  // h120: RB RX_CAR_OSC increment register              LSB:        (Bit 31: 0)
    FPGA_RW_RB_RX_CAR_OSC_INC_HI                                                               = 0x00124,  // h124: RB RX_CAR_OSC increment register              MSB: 16'b0, (Bit 47:32)
    FPGA_RW_RB_RX_CAR_OSC_OFS_LO                                                               = 0x00128,  // h128: RB RX_CAR_OSC offset register                 LSB:        (Bit 31: 0)
    FPGA_RW_RB_RX_CAR_OSC_OFS_HI                                                               = 0x0012C,  // h12C: RB RX_CAR_OSC offset register                 MSB: 16'b0, (Bit 47:32)

    FPGA_RW_RB_RX_CAR_OSC_INC_SCNR_LO                                                          = 0x00130,  // h130: RB RX_CAR_OSC_SCNR increment register         LSB:        (Bit 31: 0)
    FPGA_RW_RB_RX_CAR_OSC_INC_SCNR_HI                                                          = 0x00134,  // h134: RB RX_CAR_OSC_SCNR increment register         MSB: 16'b0, (Bit 47:32)
    //FPGA_RD_RB_RX_RSVD_H138,
    //FPGA_RD_RB_RX_RSVD_H13C,

    FPGA_RW_RB_RX_MOD_OSC_INC_LO                                                               = 0x00140,  // h140: RB RX_MOD_OSC increment register              LSB:        (Bit 31: 0)
    FPGA_RW_RB_RX_MOD_OSC_INC_HI                                                               = 0x00144,  // h144: RB RX_MOD_OSC increment register              MSB: 16'b0, (Bit 47:32)
    FPGA_RW_RB_RX_MOD_OSC_OFS_LO                                                               = 0x00148,  // h148: RB RX_MOD_OSC offset register                 LSB:        (Bit 31: 0)
    FPGA_RW_RB_RX_MOD_OSC_OFS_HI                                                               = 0x0014C,  // h14C: RB RX_MOD_OSC offset register                 MSB: 16'b0, (Bit 47:32)

    //FPGA_RD_RB_RX_RSVD_H150,
    //FPGA_RD_RB_RX_RSVD_H154,
    //FPGA_RD_RB_RX_RSVD_H158,
    //FPGA_RD_RB_RX_RSVD_H15C,

    FPGA_RW_RB_RX_MUXIN_SRC                                                                    = 0x00160,  // h160: RB audio signal RX MUXIN input selector:
                                                                                                           //       d0 =(none),   d3 =VpVn,
                                                                                                           //       d16=EXT-CH0,  d24=EXT-CH8,
                                                                                                           //       d17=EXT-CH1,  d25=EXT-CH9,
                                                                                                           //       d32=adc_i[0], d33=adc_i[1]
    FPGA_RW_RB_RX_MUXIN_GAIN                                                                   = 0x00164,  // h164: RB audio signal RX MUXIN:   UNSIGNED 16 bit
    FPGA_RW_RB_RX_MUXIN_OFS                                                                    = 0x00168,  // h168: RB analog RX MUX gain:        SIGNED 16 bit
    FPGA_RD_RB_RX_SIGNAL_STRENGTH                                                              = 0x0016C,  // h16C: RB RX signal strength:      UNSIGNED 32 bit

    FPGA_RD_RB_RX_AFC_CORDIC_MAG                                                               = 0x00170,  // h170: RB RX_AFC_CORDIC magnitude value:                 UNSIGNED 32 bit
    FPGA_RD_RB_RX_AFC_CORDIC_PHS                                                               = 0x00174,  // h174: RB_RX_AFC_CORDIC phase value:                       SIGNED 32 bit
    FPGA_RD_RB_RX_AFC_CORDIC_PHS_PREV                                                          = 0x00178,  // h178: RB_RX_AFC_CORDIC previous 200kHz clock phase value: SIGNED 32 bit
    FPGA_RD_RB_RX_AFC_CORDIC_PHS_DIFF                                                          = 0x0017C,  // h17C: RB_RX_AFC_CORDIC difference phase value:            SIGNED 32 bit

    FPGA_RW_RB_RX_SSB_AM_GAIN                                                                  = 0x00180,  // h180: RB RX_SSB_AM gain:          UNSIGNED 16 bit        16'b0, (Bit 15: 0)
    FPGA_RW_RB_RX_AMENV_GAIN                                                                   = 0x00184,  // h184: RB RX_MOD AM-ENV mixer gain:UNSIGNED 16 bit
    FPGA_RW_RB_RX_FM_GAIN                                                                      = 0x00188,  // h188: RB RX_FM gain:              UNSIGNED 16 bit        16'b0, (Bit 15: 0)
    FPGA_RW_RB_RX_PM_GAIN                                                                      = 0x0018C,  // h18C: RB RX_PM gain:              UNSIGNED 16 bit        16'b0, (Bit 15: 0)

    /* RFOUTx AMP section */
    FPGA_RW_RB_RFOUT1_AMP_GAIN                                                                 = 0x00190,  // h190: RB RFOUT1 AMP output gain:    SIGNED 16 bit
    FPGA_RW_RB_RFOUT1_AMP_OFS                                                                  = 0x00194,  // h194: RB RFOUT1 AMP output offset:  SIGNED 16 bit
    FPGA_RW_RB_RFOUT2_AMP_GAIN                                                                 = 0x00198,  // h198: RB RFOUT2 AMP output gain:    SIGNED 16 bit
    FPGA_RW_RB_RFOUT2_AMP_OFS                                                                  = 0x0019C,  // h19C: RB RFOUT2 AMP output offset:  SIGNED 16 bit

    /* READOUT SIGNALS section */
    FPGA_RD_RB_READOUT_RFIN1                                                                   = 0x001A0,  // h1A0: RB RFIN1  current data:       SIGNED 16 bit
    FPGA_RD_RB_READOUT_RFIN2                                                                   = 0x001A4,  // h1A4: RB RFIN2  current data:       SIGNED 16 bit
    FPGA_RD_RB_READOUT_RFOUT1                                                                  = 0x001A8,  // h1A8: RB RFOUT1 current data:       SIGNED 16 bit
    FPGA_RD_RB_READOUT_RFOUT2                                                                  = 0x001AC   // h1AC: RB RFOUT2 current data:       SIGNED 16 bit

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
     * bit h15: RX_CAR_OSC INC SRC STREAM - '1' places input MUXer for RX_CAR_OSC DDS increment input to the second streamed input pipe. '0' places MUXer to registers "RX_CAR_OSC INC HI" and "RX_CAR_OSC INC LO".
     *
     * bit h16: RX_CAR_OSC OFS SRC STREAM - '1' places input MUXer for RX_CAR_OSC DDS offset input to the second streamed input pipe. '0' places MUXer to registers "RX_CAR_OSC OFS HI" and "RX_CAR_OSC OFS LO".
     *
     * bit h1B..h17: n/a
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
     * bit h13..h12: n/a
     *
     * bit h14: STAT_RX_AFC_HIGH_SIG - '1' RX_AFC_HIGH_SIG the signal strength is high enough to let the AFC correct the frequency.
     *
     * bit h15: n/a
     *
     * bit h16: STAT_OVERDRIVE_TX - '1' any overdrive signal in the TX path is signaled.
     *
     * bit h17: STAT_OVERDRIVE_RX - '1' any overdrive signal in the RX path is signaled.
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


    /** @brief  R/W RB_PWR_CTRL - power savings control register (addr: 0x40600014)
     *
     * bit h07..h00: RX modulation variant
     *   value = h00  no power savings, all clocks of the transceiver are turned on
     *   value = h01  complete transmitter is turned off
     *   value = h02  components of the SSB-USB transmitter are turned on
     *   value = h03  components of the SSB-LSB transmitter are turned on
     *   value = h04  components of the AM transmitter are turned on
     *   value = h07  components of the FM transmitter are turned on
     *   value = h08  components of the PM transmitter are turned on
     *
     * bit h0F..h08: TX modulation variant
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
                                     RB_LEDs, RFOUT1 and RFOUT2 (addr: 0x40600018)
     *
     * bit h07..h00: LEDs magnitude scope (logarithmic)  source position
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
     *   value = h20  RB_SRC_CON_PNT_NUM_RX_MUXIN_MIX input.
     *   value = h21  RB_SRC_CON_PNT_NUM_RX_MUXIN_MIX output.
     *   value = h22  RX_CAR_OSC_I output.
     *   value = h23  RX_CAR_OSC_Q output.
     *   value = h24  RX_CAR_QMIX_I output.
     *   value = h25  RX_CAR_QMIX_Q output.
     *   value = h26  RX_CAR_5M_I output.
     *   value = h27  RX_CAR_5M_Q output.
     *   value = h28  RX_CAR_200K_I output.
     *   value = h29  RX_CAR_200K_Q output.
     *
     *   value = h30  RX_MOD_8K_I output.
     *   value = h31  RX_MOD_8K_Q output.
     *   value = h32  RX_MOD_VB1_I output.
     *   value = h33  RX_MOD_VB1_Q output.
     *   value = h34  RX_MOD_OSC_I output.
     *   value = h35  RX_MOD_OSC_Q output.
     *   value = h36  RX_MOD_HLD_I output.
     *   value = h37  RX_MOD_HLD_Q output.
     *   value = h38  RX_MOD_QMIX_I output.
     *   value = h39  RX_MOD_QMIX_Q output.
     *   value = h3A  RX_MOD_VB2_I output.
     *   value = h3B  RX_MOD_VB2_Q output.
     *   value = h3C  RX_MOD_48K_I output.
     *   value = h3D  RX_MOD_48K_Q output.
     *   value = h3E  RX_MOD_SSB_AM_OUT output.
     *
     *   value = h40  RX_AFC_FIR1_I output.
     *   value = h41  RX_AFC_FIR1_Q output.
     *   value = h42  RX_AFC_CORDIC_MAG carrier magnitude value.
     *   value = h43  RX_AFC_CORDIC_PHS carrier phase value.
     *   value = h44  RX_AFC_CORDIC_PHS_PREV carrier phase value.
     *   value = h45  RX_AFC_CORDIC_PHS_DIFF carrier phase value.
     *   value = h46  RX_AFC_INC_REG increment deviation value.
     *   value = h47  RX_SUM_INC_REG increment value.
     *   value = h48  RX_MOD_FM output.
     *   value = h49  RX_MOD_PM output.
     *   value = h4A  RX_MOD_AMENV_OUT output.
     *
     *   value = h50  RX_AUDIO output.
     *
     *   value = hF8  TX_CAR_OSC_INC frequency value.
     *   value = hF9  RX_CAR_OSC_INC frequency value.
     *   value = hFD  AC97 diagnostic LEDs.
     *   value = hFE  current status of the overdrive signals.
     *   value = hFF  current test vector @see red_pitaya_radiobox.sv for details.
     *
     * bit h0F..h08: n/a
     *
     * bit h17..h10: RFOUT1 source connection point   and
     * bit h1F..h18: RFOUT2 source connection point   the following connection point applies:
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
     *   value = h20  RB_SRC_CON_PNT_NUM_RX_MUXIN_MIX input.
     *   value = h21  RB_SRC_CON_PNT_NUM_RX_MUXIN_MIX output.
     *   value = h22  RX_CAR_OSC_I output.
     *   value = h23  RX_CAR_OSC_Q output.
     *   value = h24  RX_CAR_QMIX_I output.
     *   value = h25  RX_CAR_QMIX_Q output.
     *   value = h26  RX_CAR_5M_I output.
     *   value = h27  RX_CAR_5M_Q output.
     *   value = h28  RX_CAR_200K_I output.
     *   value = h29  RX_CAR_200K_Q output.
     *
     *   value = h30  RX_MOD_8K_I output.
     *   value = h31  RX_MOD_8K_Q output.
     *   value = h32  RX_MOD_VB1_I output.
     *   value = h33  RX_MOD_VB1_Q output.
     *   value = h34  RX_MOD_OSC_I output.
     *   value = h35  RX_MOD_OSC_Q output.
     *   value = h36  RX_MOD_HLD_I output.
     *   value = h37  RX_MOD_HLD_Q output.
     *   value = h38  RX_MOD_QMIX_I output.
     *   value = h39  RX_MOD_QMIX_Q output.
     *   value = h3A  RX_MOD_VB2_I output.
     *   value = h3B  RX_MOD_VB2_Q output.
     *   value = h3C  RX_MOD_48K_I output.
     *   value = h3D  RX_MOD_48K_Q output.
     *   value = h3E  RX_MOD_SSB_AM_OUT output.
     *
     *   value = h40  RX_AFC_FIR1_I output.
     *   value = h41  RX_AFC_FIR1_Q output.
     *   value = h42  RX_AFC_CORDIC_MAG carrier magnitude value.
     *   value = h43  RX_AFC_CORDIC_PHS carrier phase value.
     *   value = h44  RX_AFC_CORDIC_PHS_PREV carrier phase value.
     *   value = h45  RX_AFC_CORDIC_PHS_DIFF carrier phase value.
     *   value = h46  RX_AFC_INC_REG increment deviation value.
     *   value = h47  RX_SUM_INC_REG increment value.
     *   value = h48  RX_MOD_FM output.
     *   value = h49  RX_MOD_PM output.
     *   value = h4A  RX_MOD_AMENV_OUT output.
     *
     *   value = h50  RX_AUDIO output.
     *
     *   value = hF8  TX_CAR_OSC_INC frequency value.
     *   value = hF9  RX_CAR_OSC_INC frequency value.
     *   value = hFF  current test vector @see red_pitaya_radiobox.sv for details.
     *
     */
    uint32_t src_con_pnt;

     /** @brief  R/W RB_CON_SRC_PNT2 - output connection matrix for
                                       LINE-IN1 L/R, LINE-IN2 L/R (addr: 0x4060001C)
       *
       * bit h07..h00: Linux ALSA LINE-IN 1 Left
       * bit h0F..h08: Linux ALSA LINE-IN 1 Right
       * bit h17..h10: Linux ALSA LINE-IN 2 Left
       * bit h1F..h18: Linux ALSA LINE-IN 2 Right
       *
       * For all fields the same connection value applies
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
       *   value = h20  RB_SRC_CON_PNT_NUM_RX_MUXIN_MIX input.
       *   value = h21  RB_SRC_CON_PNT_NUM_RX_MUXIN_MIX output.
       *   value = h22  RX_CAR_OSC_I output.
       *   value = h23  RX_CAR_OSC_Q output.
       *   value = h24  RX_CAR_QMIX_I output.
       *   value = h25  RX_CAR_QMIX_Q output.
       *   value = h26  RX_CAR_5M_I output.
       *   value = h27  RX_CAR_5M_Q output.
       *   value = h28  RX_CAR_200K_I output.
       *   value = h29  RX_CAR_200K_Q output.
       *
       *   value = h30  RX_MOD_8K_I output.
       *   value = h31  RX_MOD_8K_Q output.
       *   value = h32  RX_MOD_VB1_I output.
       *   value = h33  RX_MOD_VB1_Q output.
       *   value = h34  RX_MOD_OSC_I output.
       *   value = h35  RX_MOD_OSC_Q output.
       *   value = h36  RX_MOD_HLD_I output.
       *   value = h37  RX_MOD_HLD_Q output.
       *   value = h38  RX_MOD_QMIX_I output.
       *   value = h39  RX_MOD_QMIX_Q output.
       *   value = h3A  RX_MOD_VB2_I output.
       *   value = h3B  RX_MOD_VB2_Q output.
       *   value = h3C  RX_MOD_48K_I output.
       *   value = h3D  RX_MOD_48K_Q output.
       *   value = h3E  RX_MOD_SSB_AM_OUT output.
       *
       *   value = h40  RX_AFC_FIR1_I output.
       *   value = h41  RX_AFC_FIR1_Q output.
       *   value = h42  RX_AFC_CORDIC_MAG carrier magnitude value.
       *   value = h43  RX_AFC_CORDIC_PHS carrier phase value.
       *   value = h44  RX_AFC_CORDIC_PHS_PREV carrier phase value.
       *   value = h45  RX_AFC_CORDIC_PHS_DIFF carrier phase value.
       *   value = h46  RX_AFC_INC_REG increment deviation value.
       *   value = h47  RX_SUM_INC_REG increment value.
       *   value = h48  RX_MOD_FM output.
       *   value = h49  RX_MOD_PM output.
       *   value = h4A  RX_MOD_AMENV_OUT output.
       *
       *   value = h50  RX_AUDIO output.
       *
       *   value = hF8  TX_CAR_OSC_INC frequency value.
       *   value = hF9  RX_CAR_OSC_INC frequency value.
       *   value = hFF  current test vector @see red_pitaya_radiobox.sv for details.
       *
       * bit h1F..h18: RFOUT2 source connection point
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
       *   value = h20  RB_SRC_CON_PNT_NUM_RX_MUXIN_MIX input.
       *   value = h21  RB_SRC_CON_PNT_NUM_RX_MUXIN_MIX output.
       *   value = h22  RX_CAR_OSC_I output.
       *   value = h23  RX_CAR_OSC_Q output.
       *   value = h24  RX_CAR_QMIX_I output.
       *   value = h25  RX_CAR_QMIX_Q output.
       *   value = h26  RX_CAR_5M_I output.
       *   value = h27  RX_CAR_5M_Q output.
       *   value = h28  RX_CAR_200K_I output.
       *   value = h29  RX_CAR_200K_Q output.
       *
       *   value = h30  RX_MOD_8K_I output.
       *   value = h31  RX_MOD_8K_Q output.
       *   value = h32  RX_MOD_VB1_I output.
       *   value = h33  RX_MOD_VB1_Q output.
       *   value = h34  RX_MOD_OSC_I output.
       *   value = h35  RX_MOD_OSC_Q output.
       *   value = h36  RX_MOD_HLD_I output.
       *   value = h37  RX_MOD_HLD_Q output.
       *   value = h38  RX_MOD_QMIX_I output.
       *   value = h39  RX_MOD_QMIX_Q output.
       *   value = h3A  RX_MOD_VB2_I output.
       *   value = h3B  RX_MOD_VB2_Q output.
       *   value = h3C  RX_MOD_48K_I output.
       *   value = h3D  RX_MOD_48K_Q output.
       *   value = h3E  RX_MOD_SSB_AM_OUT output.
       *
       *   value = h40  RX_AFC_FIR1_I output.
       *   value = h41  RX_AFC_FIR1_Q output.
       *   value = h42  RX_AFC_CORDIC_MAG carrier magnitude value.
       *   value = h43  RX_AFC_CORDIC_PHS carrier phase value.
       *   value = h44  RX_AFC_CORDIC_PHS_PREV carrier phase value.
       *   value = h45  RX_AFC_CORDIC_PHS_DIFF carrier phase value.
       *   value = h46  RX_AFC_INC_REG increment deviation value.
       *   value = h47  RX_SUM_INC_REG increment value.
       *   value = h48  RX_MOD_FM output.
       *   value = h49  RX_MOD_PM output.
       *   value = h4A  RX_MOD_AMENV_OUT output.
       *
       *   value = h50  RX_AUDIO output.
       *
       *   value = hF8  TX_CAR_OSC_INC frequency value.
       *   value = hF9  RX_CAR_OSC_INC frequency value.
       *   value = hFF  current test vector @see red_pitaya_radiobox.sv for details.
       *
       */
      uint32_t src_con_pnt2;


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


    /** @brief  R/W RB_TX_CAR_OSC_INC_SCNR_LO - TX_CAR_OSC scanner increment register, bits 31..0 (addr: 0x40600030)
     *
     * bit h1F..h00: LSB of TX_CAR_OSC_INC_SCNR register.
     *
     */
    uint32_t tx_car_osc_inc_scnr_lo;

    /** @brief  R/W RB_TX_CAR_OSC_INC_SCNR_HI - TX_CAR_OSC scanner increment register, bits 47..32 (addr: 0x40600034)
     *
     * bit h0F..h00: MSB of TX_CAR_OSC_INC_SCNR register.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t tx_car_osc_inc_scnr_hi;


    /** @brief  R/W RB_TX_AMP_RF_GAIN - TX_AMP_RF gain register, bits 15..0 (addr: 0x40600038)
     *
     * bit h0F..h00: SIGNED 16 bit - TX Amplifier RF gain setting.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t tx_amp_rf_gain;

    /** @brief  R/W RB_TX_AMP_RF_OFS - TX AMP RF offset register, bits 15..0 (addr: 0x4060003C)
     *
     * bit h0F..h00: SIGNED 16 bit - TX Amplifier RF offset value.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t tx_amp_rf_ofs;


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
     * bit h0F..h00: UNSIGNED 16 bit - TX_MOD_QMIX amplitude setting.
     *
     * bit h1F..h10: n/a
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
     *   value = h03  Vp_Vn,      mapped to: vin[4].
     *   value = h10  XADC CH#0,  mapped to: AI1.
     *   value = h11  XADC CH#1,  mapped to: AI0.
     *   value = h18  XADC CH#8,  mapped to: AI2.
     *   value = h19  XADC CH#9,  mapped to: AI3.
     *   value = h20  ADC0,       mapped to: RF Input 1.
     *   value = h21  ADC1,       mapped to: RF Input 2.
     *   value = h30  LINE1_IN L, mapped to: Linux ALSA LINE-IN 1 Left.
     *   value = h31  LINE1_IN R, mapped to: Linux ALSA LINE-IN 1 Right.
     *
     * bit h1F..h06: n/a
     *
     */
    uint32_t tx_muxin_src;

    /** @brief  R/W RB_TX_MUXIN_GAIN - gain for analog TX MUX input amplifier (addr: 0x40600064)
     *
     * bit h0F..h00: UNSIGNED 16 bit - gain for TX MUXIN output amplifier.
     *
     * bit h12..h10: input booster left shift value from d0 .. d7 gives amplification of: 1x .. 128x.
     *
     * bit h1F..h13: n/a
     *
     */
    uint32_t tx_muxin_gain;

    /** @brief  R/W RB_TX_MUXIN_OFS - offset value for analog TX MUX input amplifier (addr: 0x40600068)
     *
     * bit h0F..h00:   SIGNED 16 bit - gain for TX MUXIN output amplifier.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t tx_muxin_ofs;

    /** @brief  Placeholder for addr: 0x4060006C
     *
     * n/a
     *
     */
    uint32_t reserved_06C;


    /** @brief  Placeholder for addr: 0x40600070 .. 0x406000FC
     *
     * n/a
     *
     */
    uint32_t reserved_070To0fc[((0x0fc - 0x070) >> 2) + 1];


    /* RX section */

    /** @brief  R/W RB_RX_CAR_CALC_WEAVER_INC_LO - RX_CAR_AFC weaver correction increment register, bits 31..0 (addr: 0x40600100)
     *
     * bit h1F..h00: LSB of RX_CAR_AFC phase increment register.
     *
     */
    uint32_t rx_car_calc_weaver_inc_lo;

    /** @brief  R/W RB_RX_CAR_CALC_WEAVER_INC_HI - RX_CAR_AFC weaver correction increment register, bits 47..32 (addr: 0x40600104)
     *
     * bit h0F..h00: MSB of RX_CAR_AFC phase increment register.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t rx_car_calc_weaver_inc_hi;


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
    uint32_t reserved_10c;


    /** @brief  R/O RB_RX_CAR_AFC_INC_LO - RX_CAR_AFC phase increment register, bits 31..0 (addr: 0x40600110)
     *
     * bit h1F..h00: LSB of RX_CAR_AFC phase increment register.
     *
     */
    uint32_t rx_car_afc_inc_lo;

    /** @brief  R/O RB_RX_CAR_AFC_INC_HI - RX_CAR_AFC phase increment register, bits 47..32 (addr: 0x40600114)
     *
     * bit h0F..h00: MSB of RX_CAR_AFC phase increment register.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t rx_car_afc_inc_hi;


    /** @brief  R/O RB_RX_CAR_SUM_INC_LO - RX_CAR_SUM phase increment register, bits 31..0 (addr: 0x40600118)
     *
     * bit h1F..h00: LSB of RX_CAR_SUM phase increment register.
     *
     */
    uint32_t rx_car_sum_inc_lo;

    /** @brief  R/O RB_RX_CAR_SUM_INC_HI - RX_CAR_SUM phase increment register, bits 47..32 (addr: 0x4060011C)
     *
     * bit h0F..h00: MSB of RX_CAR_SUM phase increment register.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t rx_car_sum_inc_hi;


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


    /** @brief  R/W RB_RX_CAR_OSC_INC_SCNR_LO increment register, bits 31..0 (addr: 0x40600130)
     *
     * bit h1F..h00: LSB of RX_CAR_OSC_INC_SCNR register.
     *
     */
    uint32_t rx_car_osc_inc_scnr_lo;

    /** @brief  R/W RB_RX_CAR_OSC_INC_SCNR_HI increment register, bits 47..32 (addr: 0x40600134)
     *
     * bit h0F..h00: MSB of RX_CAR_OSC_INC_SCNR register.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t rx_car_osc_inc_scnr_hi;


    /** @brief  Placeholder for addr: 0x40600138
     *
     * n/a
     *
     */
    uint32_t reserved_138;

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
    uint32_t reserved_15c;


    /** @brief  R/W RB_RX_MUX_SRC -  bits 31..0 (addr: 0x40600160)
     *
     * bit h05..h00: source ID (extended from the XADC source ID)
     *   value = h00  no external signal used, TX_MOD_OSC used instead.
     *   value = h03  Vp_Vn,      mapped to: vin[4].
     *   value = h10  XADC CH#0,  mapped to: AI1.
     *   value = h11  XADC CH#1,  mapped to: AI0.
     *   value = h18  XADC CH#8,  mapped to: AI2.
     *   value = h19  XADC CH#9,  mapped to: AI3.
     *   value = h20  ADC0,       mapped to: RF Input 1.
     *   value = h21  ADC1,       mapped to: RF Input 2.
     *   value = h30  LINE1_IN L, mapped to: Linux ALSA LINE-IN 1 Left.
     *   value = h31  LINE1_IN R, mapped to: Linux ALSA LINE-IN 1 Right.
     *
     * bit h1F..h06: n/a
     *
     */
    uint32_t rx_muxin_src;

    /** @brief  R/W RB_RX_MUX_GAIN -  bits 15..0 (addr: 0x40600164)
     *
     * bit h0F..h00: UNSIGNED 16 bit - gain for RX MUXIN input amplifier.
     *
     * bit h12..h10: input booster left shift value from d0 .. d7 gives amplification of: 1x .. 128x.
     *
     * bit h1F..h13: n/a
     *
     */
    uint32_t rx_muxin_gain;

    /** @brief  R/W RB_RX_MUX_OFS -  bits 15..0 (addr: 0x40600168)
     *
     * bit h0F..h00:   SIGNED 16 bit - offset value for RX MUXIN input amplifier.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t rx_muxin_ofs;

    /** @brief  R/O RB_RX_SIGNAL_STRENGTH - RX_AFC_CORDIC magnitude mean value 1/25 sec, bits 31..0 (addr: 0x4060016C)
     *
     * bit h1F..h00: UNSIGNED 32 bit - RX_AFC_CORDIC magnitude mean value register.
     *
     */
    uint32_t rx_signal_strength;


    /** @brief  R/O RB_RX_AFC_CORDIC_MAG - RX_AFC_CORDIC magnitude register, bits 15..0 (addr: 0x40600170)
     *
     * bit h0F..h00:   SIGNED 16 bit - RX_AFC_CORDIC magnitude register.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t rx_afc_cordic_mag;

    /** @brief  R/O RB_RX_AFC_CORDIC_phs - RX_AFC_CORDIC phase register, bits 15..0 (addr: 0x40600174)
     *
     * bit h0F..h00:   SIGNED 16 bit - RX_AFC_CORDIC phase register.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t rx_afc_cordic_phs;

    /** @brief  R/O RB_RX_AFC_CORDIC_phs_prev - RX_AFC_CORDIC previous phase register, bits 15..0 (addr: 0x40600178)
     *
     * bit h0F..h00:   SIGNED 16 bit - RX_AFC_CORDIC previous phase register - the 200 kHz clock before.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t rx_afc_cordic_phs_prev;

    /** @brief  R/O RB_RX_AFC_CORDIC_phs_diff - RX_AFC_CORDIC difference phase register, bits 15..0 (addr: 0x4060017C)
     *
     * bit h0F..h00:   SIGNED 16 bit - RX_AFC_CORDIC phase difference register - the difference of phase within 200 kHz clocks.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t rx_afc_cordic_phs_diff;


    /** @brief  R/W RB_RX_MOD_SSB_AM_GAIN -  bits 15..0 (addr: 0x40600180)
     *
     * bit h0F..h00: UNSIGNED 16 bit - gain value for the SSB/AM demodulator output.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t rx_mod_ssb_am_gain;

    /** @brief  R/W RB_RX_MOD_AMENV_GAIN -  bits 15..0 (addr: 0x40600184)
     *
     * bit h0F..h00: UNSIGNED 16 bit - gain value for the AM-Envelope demodulator output.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t rx_mod_amenv_gain;

    /** @brief  R/W RB_RX_MOD_FM_GAIN -  bits 15..0 (addr: 0x40600188)
     *
     * bit h0F..h00: UNSIGNED 16 bit - gain value for the FM demodulator output.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t rx_mod_fm_gain;

    /** @brief  R/W RB_RX_MOD_PM_GAIN -  bits 15..0 (addr: 0x4060018C)
     *
     * bit h0F..h00: UNSIGNED 16 bit - gain value for the PM demodulator output.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t rx_mod_pm_gain;


    /** @brief  R/W RB_RFOUT1_GAIN - RFOUT1 amplitude register, bits 15..0 (addr: 0x40600190)
     *
     * bit h0F..h00:   SIGNED 16 bit - RFOUT1 amplitude setting - 8 bit integer . 8 bit fraction value.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t rfout1_gain;

    /** @brief  R/W RB_RFOUT1_OFS - RFOUT1 offset register, bits 15..0 (addr: 0x40600194)
     *
     * bit h0F..h00:   SIGNED 16 bit - RFOUT1 offset value - 16 bit DAC value offset.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t rfout1_ofs;

    /** @brief  R/W RB_RFOUT2_GAIN - RFOUT2 amplitude register, bits 15..0 (addr: 0x40600198)
     *
     * bit h0F..h00:   SIGNED 16 bit - RFOUT2 amplitude setting - 8 bit integer . 8 bit fraction value.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t rfout2_gain;

    /** @brief  R/W RB_RFOUT2_OFS - RFOUT2 offset register, bits 15..0 (addr: 0x4060019C)
     *
     * bit h0F..h00:   SIGNED 16 bit - RFOUT2 offset value - 16 bit DAC value offset.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t rfout2_ofs;


    /** @brief  R/O RB_READOUT_RFIN1 - readout of current RFIN1, bits 15..0 (addr: 0x406001A0)
     *
     * bit h0F..h00:   SIGNED 16 bit - RFIN1 current data.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t readout_rfin1;

    /** @brief  R/O RB_READOUT_RFIN2 -  readout of current RFIN2, bits 15..0 (addr: 0x406001A4)
     *
     * bit h0F..h00:   SIGNED 16 bit - RFIN2 current data.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t readout_rfin2;

    /** @brief  R/O RB_READOUT_RFOUT1 - readout of current RFOUT1, bits 15..0 (addr: 0x406001A8)
     *
     * bit h0F..h00:   SIGNED 16 bit - RFOUT1 current data.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t readout_rfout1;

    /** @brief  R/O RB_READOUT_RFOUT2 -  readout of current RFOUT2, bits 15..0 (addr: 0x406001AC)
     *
     * bit h0F..h00:   SIGNED 16 bit - RFOUT2 current data.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t readout_rfout2;

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
 * @param[inout]  pb     List of base parameters with complete set of data entries.
 * @param[in]     p_pn   List of parameters to be scanned for marked entries, removes MARKER.
 *
 * @retval        0      Success
 * @retval        -1     Failure, parameter list or RB accessor not valid
 * @retval        -2     Failure, parameter list or RB accessor not valid
 */
int fpga_rb_update_all_params(rb_app_params_t* pb, rb_app_params_t** p_pn);

/**
 * @brief Read back automatic register values from the RadioBox FPGA sub-module
 *
 * Being called out of the worker context.
 *
 * @param[inout]  pb     List of base parameters with complete set of data entries.
 * @param[in]     p_pn   List of returned FPGA parameters added to that list.
 *
 * @retval        1      Success, new data available
 * @retval        0      Success, no change of data
 * @retval        -1     Failure, parameter list or RB accessor not valid
 * @retval        -2     Failure, parameter list or RB accessor not valid
 */
int fpga_rb_get_fpga_params(rb_app_params_t* pb, rb_app_params_t** p_pn);


/**
 * @brief Calculates and programs the FPGA RB ctrl and oscillator registers
 *
 * @param[in]  rb_run           RadioBox application  0: disabled, else: enabled.
 * @param[in]  tx_modsrc        0==(none), 1==RF Input 1, 2==RF Input 2, 4==EXP AI0, 5==EXP AI1, 6==EXP AI2, 7==EXP AI3, 15==TX_MOD_OSC
 * @param[in]  tx_modtyp        2==USB, 3==LSB, 4==AM, 7==FM, 8==PM - else ignored.
 * @param[in]  rx_modtyp        2==USB, 3==LSB, 4==AMenv, 5==AMsync_USB, 6==AMsync_LSB, 7==FM, 8==PM - else ignored.
 * @param[in]  src_con_pnt      RB LED controller, RF Output 1 and RF Output 2 setting to be used.
 * @param[in]  src_con2_pnt     AC97 LineIn Left and AC97 LineIn Right setting to be used.
 * @param[in]  rx_muxin_src     0==Off, 1==RF Input 1, 2==RF Input 2.
 * @param[in]  tx_car_osc_qrg   Frequency for TX_CAR_OSC in Hz.
 * @param[in]  rx_car_osc_qrg   Frequency for RX_CAR_OSC in Hz.
 * @param[in]  tx_mod_osc_qrg   Frequency for TX_MOD_OSC in Hz.
 * @param[in]  tx_muxin_gain    Slider value between 0% (value==0) and 100% (value==80) for the MUXIN range slider, 80 means amplification of 1:1. 80..100 logarithmic amplification.
 * @param[in]  rx_muxin_gain    Slider value between 0% and 100%.
 * @param[in]  tx_qrg_sel       Frequency QRG controller induces into TX frequency: 1==on, 0==off.
 * @param[in]  rx_qrg_sel       Frequency QRG controller induces into RX frequency: 1==on, 0==off.
 * @param[in]  tx_amp_rf_gain   Vpp of TX_AMP_RF output in mV.
 * @param[in]  tx_mod_osc_mag   Magnitude of TX_MOD_OSC mixer output. AM: 0-100%, FM: 0-1000000 Hz deviation, PM: 0-360°.
 * @param[in]  term_rfout1      Termination of RFOut1: 0==neutral, 1==50 ohms, 2==open ended.
 * @param[in]  term_rfout2      Termination of RFOut2: 0==neutral, 1==50 ohms, 2==open ended.
 * @param[in]  qrg_inc          Frequency QRG range controller increment value, 0%-100%.
 */
void fpga_rb_set_ctrl(int rb_run, int tx_modsrc, int tx_modtyp, int rx_modtyp, int src_con_pnt, int src_con2_pnt, int rx_muxin_src,
        double tx_car_osc_qrg, double rx_car_osc_qrg,
        double tx_mod_osc_qrg, int tx_muxin_gain, int rx_muxin_gain, int tx_qrg_sel, int rx_qrg_sel,
        int tx_amp_rf_gain, int tx_mod_osc_mag, int term_rfout1, int term_rfout2, int qrg_inc);

/**
 * @brief Reads FPGA RadioBox automatic registers
 *
 * @param[in]  tx_modtyp               2==USB, 3==LSB, 4==AM, 7==FM, 8==PM - else ignored.
 * @param[in]  rx_modtyp               2==USB, 3==LSB, 4==AMenv, 5==AMsync_USB, 6==AMsync_LSB, 7==FM, 8==PM - else ignored.
 * @param[out] loc_RD_tx_car_osc_qrg   Current frequency read-out of the TX_CAR_OSC oscillator.
 * @param[out] loc_RD_rx_car_osc_qrg   Current frequency read-out of the RX_CAR_OSC oscillator.
 * @param[out] loc_RD_ovrdrv           Current overdrive flags.
 */
void fpga_rb_get_ctrl(int tx_modtyp, int rx_modtyp,
        double* loc_RD_tx_car_osc_qrg, double* loc_RD_rx_car_osc_qrg, uint16_t* loc_RD_ovrdrv);


/**
 * @brief Power savings control - switches transmitter part to the selected modulation variant
 *
 * @param[in]  tx_modtyp   TX modulation variant.
 */
void fpga_rb_set_tx_modtyp(int tx_modtyp);

/**
 * @brief Calculates and programs the FPGA TX_CAR_OSC for CW, SSB, AM and PM
 *
 * @param[in]  tx_car_osc_qrg   Frequency for TX_CAR_OSC in Hz.
 */
void fpga_rb_set_tx_car_osc_qrg__4mod_cw_ssb_am_pm(double tx_car_osc_qrg);

/**
 * @brief Pulls current FPGA TX_CAR_OSC frequency setting
 *
 * @retval     double           Current frequency of TX_CAR_OSC in Hz.
 */
double fpga_rb_get_tx_car_osc_qrg();

/**
 * @brief Calculates and programs the FPGA TX_CAR_OSC sweep increment for CW, SSB, AM and PM
 *
 * @param[in]  tx_car_osc_qrg_inc   Range controller represents frequency increment, 0%-100%.
 */
void fpga_rb_set_tx_car_osc_qrg_inc__4mod_cw_ssb_am_pm(int tx_car_osc_qrg_inc);

/**
 * @brief Pulls current FPGA TX_CAR_OSC_SCNR_INC frequency setting
 *
 * @retval     double           Current scanner frequency increment in Hz/s of TX_CAR_OSC in Hz.
 */
double fpga_rb_get_tx_car_osc_qrg_inc();

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
 * @param[in]  tx_muxin_gain   Slider value between 0% and 100% for the MUXIN range slider. 80% means amplification of 1:1, over 80% the logarithmic booster is enabled.
 * @param[in]  tx_muxin_ofs    ADC offset value, signed 16 bit.
 */
void fpga_rb_set_tx_muxin_gain(int tx_muxin_gain, int tx_muxin_ofs);


/**
 * @brief Power savings control - switches receiver part to the selected modulation variant
 *
 * @param[in]  rx_modtyp   RX modulation variant.
 */
void fpga_rb_set_rx_modtyp(int rx_modtyp);

/**
 * @brief Calculates and programs the FPGA RX_MUXIN gain setting
 *
 * @param[in]  rx_muxin_gain   Slider value between 0% and 100% for the MUXIN range slider. 80% means amplification of 1:1, over 80% the logarithmic booster is enabled.
 * @param[in]  rx_muxin_ofs    ADC offset value, signed 16 bit.
 */
void fpga_rb_set_rx_muxin_gain(int rx_muxin_gain, int rx_muxin_ofs);

/**
 * @brief Calculates and programs the FPGA RX_CAR_OSC for SSB, AM and FM
 *
 * @param[in]  rx_car_osc_qrg   Frequency for RX_CAR_OSC in Hz.
 */
void fpga_rb_set_rx_car_osc_qrg__4mod_ssb_am_fm_pm(double rx_car_osc_qrg);

/**
 * @brief Pulls current FPGA RX_CAR_OSC frequency setting
 *
 * @retval     double           Current frequency of RX_CAR_OSC in Hz.
 */
double fpga_rb_get_rx_car_osc_qrg();

/**
 * @brief Calculates and programs the FPGA RX_CAR_OSC sweep increment for SSB, AM and FM
 *
 * @param[in]  rx_car_osc_qrg_inc   Range controller represents frequency increment, 0%-100%.
 */
void fpga_rb_set_rx_car_osc_qrg_inc__4mod_ssb_am_fm_pm(int rx_car_osc_qrg_inc);

/**
 * @brief Pulls current FPGA RX_CAR_OSC_SCNR_INC frequency setting
 *
 * @retval     double           Current scanner frequency increment in Hz/s of RX_CAR_OSC in Hz.
 */
double fpga_rb_get_rx_car_osc_qrg_inc();

/**
 * @brief Calculates and programs the FPGA RX_MOD_OSC for SSB and AM
 *
 * @param[in]  rx_mod_osc_qrg   Frequency for RX_MOD_OSC in Hz.
 */
void fpga_rb_set_rx_mod_osc_qrg__4mod_ssbweaver_am(double rx_mod_osc_qrg);

/**
 * @brief Calculates and programs the FPGA RX_AFC_WEAVER register
 *
 * @param[in]  rx_weaver_qrg   Weaver frequency in Hz to correct AFC frequency offset.
 */
void fpga_rb_set_rx_calc_afc_weaver__4mod_am_fm_pm(double rx_weaver_qrg);

/**
 * @brief Calculates and programs the FPGA RX_MOD_SSB_AM mixer
 *
 * @param[in]  rx_mod_ssb_am_gain  gain between 0% .. 100%.
 */
void fpga_rb_set_rx_mod_ssb_am_gain__4mod_ssb_am(double rx_mod_ssb_am_gain);

/**
 * @brief Calculates and programs the FPGA RX_MOD_AMENV mixer
 *
 * @param[in]  rx_mod_amenv_gain  gain between 0% .. 100%.
 */
void fpga_rb_set_rx_mod_amenv_gain__4mod_amenv(double rx_mod_amenv_gain);

/**
 * @brief Calculates and programs the FPGA RX_MOD_FM mixer
 *
 * @param[in]  rx_mod_fm_gain  gain between 0% .. 100%.
 */
void fpga_rb_set_rx_mod_fm_gain__4mod_fm(double rx_mod_fm_gain);

/**
 * @brief Calculates and programs the FPGA RX_MOD_PM mixer
 *
 * @param[in]  rx_mod_pm_gain  gain between 0% .. 100%.
 */
void fpga_rb_set_rx_mod_pm_gain__4mod_pm(double rx_mod_pm_gain);

#if 0
/**
 * @brief Calculates and programs the FPGA RX_AUDIO_OUT mixer
 *
 * @param[in]  rx_audio_out_gain  gain between 0% .. 100%.
 * @param[in]  rx_audio_out_ofs   Vpp amplitude in mV.
 */
void fpga_rb_set_rx_audio_out_gain_ofs__4mod_all(double rx_audio_out_gain, double rx_audio_out_ofs);
#endif

/**
 * @brief Calculates and programs the FPGA RFOUT1 gain correction amplifier
 *
 * @param[in]  rfout1_gain  signed factor value: 8 bit integer . 8 bit fractional part - value = (-127.999 .. +127.999) .
 * @param[in]  rfout1_ofs   DAC offset value.
 */
void fpga_rb_set_rfout1_gain_ofs(double rfout1_gain, uint16_t rfout1_ofs);

/**
 * @brief Calculates and programs the FPGA RFOUT2 gain correction amplifier
 *
 * @param[in]  rfout2_gain  signed factor value: 8 bit integer . 8 bit fractional part - value = (-127.999 .. +127.999) .
 * @param[in]  rfout2_ofs   DAC offset value.
 */
void fpga_rb_set_rfout2_gain_ofs(double rfout2_gain, uint16_t rfout2_ofs);


/**
 * @brief Calculates and programs the FPGA RFOUT2 gain correction amplifier
 *
 * @retval     int              Current overdrive flags of the FPGA signals.
 */
uint16_t fpga_rb_get_ovrdrv();


/**
 * @brief Prepare settings of the FPGA to measure the DC offset voltage of the RX path
 *
 */
void prepare_rx_measurement();

/**
 * @brief Finish the measurement and reset all RX registers to their defaulting values
 *
 */
void finish_rx_measurement();

/**
 * @brief Run a single measure step and return the state of minimizing the DC offset
 *
 * @param[in] adc_offset_val  ADC offset value to be tried and measured.
 * @param[in] reduction       shift right value to reduce the input signal magnitude
 * @retval                    signal strength as unsigned 32 bit value: lower is better.
 */
uint32_t test_rx_measurement(int16_t adc_offset_val, int reduction);

/**
 * @brief Minimizing the noise by ADC offset value compensation
 *
 */
int16_t rp_minimize_noise();

/**
 * @brief Measuring offset voltages to update entities of calib_params
 *
 * @param[inout]  calib_params  set of calibration data to be updated with the help of this measuring process.
 */
void rp_measure_calib_params(rp_calib_params_t* calib_params);


#if 0
uint32_t fpga_rb_read_register(unsigned int rb_reg_ofs);
int fpga_rb_write_register(unsigned int rb_reg_ofs, uint32_t value);
#endif

/** @} */


#endif /* __FPGA_RB_H */
