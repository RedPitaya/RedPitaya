/**
 * $Id: red_pitaya_radiobox.v 001 2015-09-11 18:10:00Z DF4IAH $
 *
 * @brief Red Pitaya RadioBox application, used to expand RedPitaya for
 * radio ham operators. Transmitter as well as receiver components are
 * included like modulators/demodulators, filters, (R)FFT transformations
 * and that like.
 *
 * @Author Ulrich Habel, DF4IAH
 *
 * (c) Ulrich Habel / GitHub.com open source  http://df4iah.github.io/RedPitaya_RadioBox/
 *
 * This part of code is written in Verilog hardware description language (HDL).
 * Please visit http://en.wikipedia.org/wiki/Verilog
 * for more details on the language used herein.
 */

/**
 * GENERAL DESCRIPTION:
 *
 * TODO: explanations.
 *
 * TODO: graphics - exmaple by red_pitaya_scope.v
 *
 * TODO: detailed information
 * 
 */


`timescale 1ns / 1ps

module red_pitaya_radiobox #(
  // parameter RSZ = 14  // RAM size 2^RSZ
)(
   // ADC clock & reset
   input                 clk_adc_125mhz  ,      // ADC based clock, 125 MHz
   input                 adc_rstn_i      ,      // ADC reset - active low

   // activation
   output                rb_activated    ,      // RB sub-module is activated

   // LEDs
   output reg            rb_leds_en      ,      // RB LEDs are enabled and overwrites HK sub-module
   output reg   [  7: 0] rb_leds_data    ,      // RB LEDs data

   // ADC data
   input        [ 13: 0] adc_i[1:0]      ,      // ADC data { CHB, CHA }

   // DAC data
   output reg   [ 15: 0] rb_out_ch [1:0] ,      // RadioBox output signals

   // System bus - slave
   input        [ 31: 0] sys_addr        ,      // bus saddress
   input        [ 31: 0] sys_wdata       ,      // bus write data
   input        [  3: 0] sys_sel         ,      // bus write byte select
   input                 sys_wen         ,      // bus write enable
   input                 sys_ren         ,      // bus read enable
   output reg   [ 31: 0] sys_rdata       ,      // bus read data
   output reg            sys_err         ,      // bus error indicator
   output reg            sys_ack         ,      // bus acknowledge signal

   // AXI streaming master from XADC
   input              xadc_axis_aclk     ,      // AXI-streaming from the XADC, clock from the AXI-S FIFO
   input   [ 16-1: 0] xadc_axis_tdata    ,      // AXI-streaming from the XADC, data
   input   [  5-1: 0] xadc_axis_tid      ,      // AXI-streaming from the XADC, analog data source channel for this data
                                                // TID=0x10:VAUXp0_VAUXn0 & TID=0x18:VAUXp8_VAUXn8, TID=0x11:VAUXp1_VAUXn1 & TID=0x19:VAUXp9_VAUXn9, TID=0x03:Vp_Vn
   output reg         xadc_axis_tready   ,      // AXI-streaming from the XADC, slave indicating ready for data
   input              xadc_axis_tvalid          // AXI-streaming from the XADC, data transfer valid
);


//---------------------------------------------------------------------------------
//  Registers accessed by the system bus

enum {
    /* OMNI section */
    REG_RW_RB_CTRL                        =  0, // h000: RB control register
    REG_RD_RB_STATUS,                           // h004: EB status register
    REG_RW_RB_ICR,                              // h008: RB interrupt control register
    REG_RD_RB_ISR,                              // h00C: RB interrupt status register
    REG_RW_RB_DMA_CTRL,                         // h010: RB DMA control register
    //REG_RD_RB_RSVD_H014,
    REG_RW_RB_PWR_CTRL,                         // h018: RB power savings control register             RX_MOD:     (Bit  7: 0)
                                                //                                                     TX_MOD:     (Bit 15: 8)
    REG_RW_RB_RFOUTx_LED_SRC_CON_PNT,           // h01C: RB_LED, RB_RFOUT1 and RB_RFOUT2 connection matrix


    /* TX section */
    REG_RW_RB_TX_CAR_OSC_INC_LO,                // h020: RB TX_CAR_OSC increment register              LSB:        (Bit 31: 0)
    REG_RW_RB_TX_CAR_OSC_INC_HI,                // h024: RB TX_CAR_OSC increment register              MSB: 16'b0, (Bit 47:32)
    REG_RW_RB_TX_CAR_OSC_OFS_LO,                // h028: RB TX_CAR_OSC offset register                 LSB:        (Bit 31: 0)
    REG_RW_RB_TX_CAR_OSC_OFS_HI,                // h02C: RB TX_CAR_OSC offset register                 MSB: 16'b0, (Bit 47:32)

    REG_RW_RB_TX_RF_AMP_GAIN,                   // h030: RB TX_CAR_OSC mixer gain:     SIGNED 16 bit
    //REG_RD_RB_TX_RSVD_H034,
    REG_RW_RB_TX_RF_AMP_OFS,                    // h038: RB TX_CAR_OSC mixer offset:   SIGNED 17 bit
    //REG_RD_RB_TX_RSVD_H03C,

    REG_RW_RB_TX_MOD_OSC_INC_LO,                // h040: RB TX_MOD_OSC increment register              LSB:        (Bit 31: 0)
    REG_RW_RB_TX_MOD_OSC_INC_HI,                // h044: RB TX_MOD_OSC increment register              MSB: 16'b0, (Bit 47:32)
    REG_RW_RB_TX_MOD_OSC_OFS_LO,                // h048: RB TX_MOD_OSC offset register                 LSB:        (Bit 31: 0)
    REG_RW_RB_TX_MOD_OSC_OFS_HI,                // h04C: RB TX_MOD_OSC offset register                 MSB: 16'b0, (Bit 47:32)

    REG_RW_RB_TX_MOD_QMIX_GAIN,                 // h050: RB TX_MOD_OSC mixer gain:     SIGNED 16 bit
    //REG_RD_RB_TX_RSVD_H054,
    REG_RW_RB_TX_MOD_QMIX_OFS_LO,               // h058: RB TX_MOD_OSC mixer offset:   SIGNED 48 bit   LSB:        (Bit 31: 0)
    REG_RW_RB_TX_MOD_QMIX_OFS_HI,               // h05C: RB TX_MOD_OSC mixer offset:   SIGNED 48 bit   MSB: 16'b0, (Bit 47:32)

    REG_RW_RB_TX_MUXIN_SRC,                     // h060: RB analog TX MUX input selector:
                                                //      d0 =(none),   d3 =VpVn,
                                                //      d16=EXT-CH0,  d24=EXT-CH8,
                                                //      d17=EXT-CH1,  d25=EXT-CH9,
                                                //      d32=adc_i[0], d33=adc_i[1]
    REG_RW_RB_TX_MUXIN_GAIN,                    // h064: RB analog TX MUX gain:      UNSIGNED 16 bit


    /* RX section */
    REG_RW_RB_RX_CAR_CALC_WEAVER_INC_LO,        // h100: weaver increment phase correction register    LSB:        (Bit 31: 0)
    REG_RW_RB_RX_CAR_CALC_WEAVER_INC_HI,        // h104: weaver increment phase correction register    MSB: 16'b0, (Bit 47:32)
    //REG_RD_RB_TX_RSVD_H108,
    //REG_RD_RB_TX_RSVD_H10C,

    REG_RD_RB_RX_CAR_AFC_INC_LO,                // h110: RB RX_CAR_AFC increment register              LSB:        (Bit 31: 0)
    REG_RD_RB_RX_CAR_AFC_INC_HI,                // h114: RB RX_CAR_AFC increment register              MSB: 16'b0, (Bit 47:32)

    REG_RD_RB_RX_CAR_SUM_INC_LO,                // h118: RB RX_CAR_SUM increment register              LSB:        (Bit 31: 0)
    REG_RD_RB_RX_CAR_SUM_INC_HI,                // h11C: RB RX_CAR_SUM increment register              MSB: 16'b0, (Bit 47:32)

    REG_RW_RB_RX_CAR_OSC_INC_LO,                // h120: RB RX_CAR_OSC increment register              LSB:        (Bit 31: 0)
    REG_RW_RB_RX_CAR_OSC_INC_HI,                // h124: RB RX_CAR_OSC increment register              MSB: 16'b0, (Bit 47:32)
    REG_RW_RB_RX_CAR_OSC_OFS_LO,                // h128: RB RX_CAR_OSC offset register                 LSB:        (Bit 31: 0)
    REG_RW_RB_RX_CAR_OSC_OFS_HI,                // h12C: RB RX_CAR_OSC offset register                 MSB: 16'b0, (Bit 47:32)

    //REG_RD_RB_RX_RSVD_H130,
    //REG_RD_RB_RX_RSVD_H134,
    //REG_RD_RB_RX_RSVD_H138,
    //REG_RD_RB_RX_RSVD_H13C,

    REG_RW_RB_RX_MOD_OSC_INC_LO,                // h140: RB RX_MOD_OSC increment register              LSB:        (Bit 31: 0)
    REG_RW_RB_RX_MOD_OSC_INC_HI,                // h144: RB RX_MOD_OSC increment register              MSB: 16'b0, (Bit 47:32)
    REG_RW_RB_RX_MOD_OSC_OFS_LO,                // h148: RB RX_MOD_OSC offset register                 LSB:        (Bit 31: 0)
    REG_RW_RB_RX_MOD_OSC_OFS_HI,                // h14C: RB RX_MOD_OSC offset register                 MSB: 16'b0, (Bit 47:32)

    //REG_RD_RB_RX_RSVD_H150,
    //REG_RD_RB_RX_RSVD_H154,
    //REG_RD_RB_RX_RSVD_H158,
    //REG_RD_RB_RX_RSVD_H15C,

    REG_RW_RB_RX_MUXIN_SRC,                     // h160: RB audio signal RX MUXIN input selector:
                                                //      d0 =(none),   d3 =VpVn,
                                                //      d16=EXT-CH0,  d24=EXT-CH8,
                                                //      d17=EXT-CH1,  d25=EXT-CH9,
                                                //      d32=adc_i[0], d33=adc_i[1]
    REG_RW_RB_RX_MUXIN_GAIN,                    // h164: RB audio signal RX MUXIN    UNSIGNED 16 bit
    //REG_RD_RB_RX_RSVD_H168,
    //REG_RD_RB_RX_RSVD_H16C,

    REG_RD_RB_RX_AFC_CORDIC_MAG,                // h170: RB RX_AFC_CORDIC magnitude value
    REG_RD_RB_RX_AFC_CORDIC_PHS,                // h174: RB_RX_AFC_CORDIC phase value
    REG_RD_RB_RX_AFC_CORDIC_PHS_PREV,           // h178: RB_RX_AFC_CORDIC previous 8kHz clock phase value
    REG_RD_RB_RX_AFC_CORDIC_PHS_DIFF,           // h17C: RB_RX_AFC_CORDIC difference phase value

    REG_RW_RB_RX_SSB_AM_GAIN,                   // h180: RB RX_MOD_OSC mixer gain:     UNSIGNED 16 bit
    REG_RW_RB_RX_FM_GAIN,                       // h184: RB RX_MOD_OSC mixer gain:     UNSIGNED 16 bit
    REG_RW_RB_RX_PM_GAIN,                       // h188: RB RX_MOD_OSC mixer gain:     UNSIGNED 16 bit
    //REG_RD_RB_RX_RSVD_H18C,

    REG_RW_RB_RX_AUDIO_OUT_GAIN,                // h190: RB RX_AUDIO_OUT mixer gain:   UNSIGNED 16 bit
    //REG_RD_RB_RX_RSVD_H194,
    REG_RW_RB_RX_AUDIO_OUT_OFS,                 // h198: RB RX_AUDIO_OUT mixer offset:   SIGNED 16 bit
    //REG_RD_RB_RX_RSVD_H19C,

    REG_RB_COUNT
} REG_RB_ENUMS;

reg  [31: 0]    regs    [REG_RB_COUNT];         // registers to be accessed by the system bus

enum {
    RB_CTRL_ENABLE                        =  0, // enabling the RadioBox sub-module
    RB_CTRL_TX_CAR_OSC_RESET,                   // reset TX_CAR_OSC, does not touch clock enable
    RB_CTRL_TX_MOD_OSC_RESET,                   // reset TX_MOD_OSC, does not touch clock enable
    RB_CTRL_RSVD_D03,

    RB_CTRL_TX_CAR_OSC_RESYNC,                  // TX_CAR_OSC restart with phase register = 0
    RB_CTRL_TX_CAR_OSC_INC_SRC_STREAM,          // TX_CAR_OSC incrementing: use stream instead of OSC register setting
    RB_CTRL_TX_CAR_OSC_OFS_SRC_STREAM,          // TX_CAR_OSC offset: use stream instead of OSC register setting
    RB_CTRL_RSVD_D07,

    RB_CTRL_RSVD_D08,
    RB_CTRL_RSVD_D09,
    RB_CTRL_RSVD_D10,
    RB_CTRL_RSVD_D11,

    RB_CTRL_TX_MOD_OSC_RESYNC,                  // TX_MOD_OSC restart with phase register = 0
    RB_CTRL_TX_MOD_OSC_INC_SRC_STREAM,          // TX_MOD_OSC incrementing: use stream instead of OSC register setting
    RB_CTRL_TX_MOD_OSC_OFS_SRC_STREAM,          // TX_MOD_OSC offset: use stream instead of OSC register setting
    RB_CTRL_RSVD_D15,

    RB_CTRL_RSVD_D16,
    RB_CTRL_RX_CAR_OSC_RESET,                   // reset RX_CAR_OSC, does not touch clock enable
    RB_CTRL_RX_MOD_OSC_RESET,                   // reset RX_MOD_OSC, does not touch clock enable
    RB_CTRL_RSVD_D19,

    RB_CTRL_RX_CAR_OSC_RESYNC,                  // RX_CAR_OSC restart with phase register = 0
    RB_CTRL_RX_CAR_OSC_INC_SRC_STREAM,          // RX_CAR_OSC incrementing: use SUM stream instead of OSC register setting
    RB_CTRL_RX_CAR_OSC_OFS_SRC_STREAM,          // RX_CAR_OSC offset: use SUM stream instead of OSC register setting
    RB_CTRL_RSVD_D23,

    RB_CTRL_RSVD_D24,
    RB_CTRL_RSVD_D25,
    RB_CTRL_RSVD_D26,
    RB_CTRL_RSVD_D27,

    RB_CTRL_RX_MOD_OSC_RESYNC,                  // RX_MOD_OSC restart with phase register = 0
    RB_CTRL_RSVD_D29,
    RB_CTRL_RSVD_D30,
    RB_CTRL_RSVD_D31
} RB_CTRL_BITS_ENUM;

enum {
    RB_PWR_CTRL_TX_MOD_OFF                =  0, // RB_PWR_CTRL TX modulation: complete transmitter is turned off
    RB_PWR_CTRL_TX_MOD_USB                =  2, // RB_PWR_CTRL TX modulation: components of the SSB-USB transmitter are turned on
    RB_PWR_CTRL_TX_MOD_LSB,                     // RB_PWR_CTRL TX modulation: components of the SSB-LSB transmitter are turned on
    RB_PWR_CTRL_TX_MOD_AM,                      // RB_PWR_CTRL TX modulation: components of the AM transmitter are turned on
    RB_PWR_CTRL_TX_MOD_FM                 =  7, // RB_PWR_CTRL TX modulation: components of the FM transmitter are turned on
    RB_PWR_CTRL_TX_MOD_PM                       // RB_PWR_CTRL TX modulation: components of the PM transmitter are turned on
} RB_PWR_CTRL_TX_MOD_BITS_ENUM;

enum {
    RB_PWR_CTRL_RX_MOD_OFF                =  0, // RB_PWR_CTRL RX modulation: complete receiver is turned off
    RB_PWR_CTRL_RX_MOD_USB                =  2, // RB_PWR_CTRL RX modulation: components of the SSB-USB receiver are turned on
    RB_PWR_CTRL_RX_MOD_LSB,                     // RB_PWR_CTRL RX modulation: components of the SSB-LSB receiver are turned on
    RB_PWR_CTRL_RX_MOD_AM_ENV,                  // RB_PWR_CTRL RX modulation: components of the AM receiver are turned on
    RB_PWR_CTRL_RX_MOD_AM_SYNC_USB,             // RB_PWR_CTRL RX modulation: components of the AM syncro mode USB receiver are turned on
    RB_PWR_CTRL_RX_MOD_AM_SYNC_LSB,             // RB_PWR_CTRL RX modulation: components of the AM syncro mode LSB receiver are turned on
    RB_PWR_CTRL_RX_MOD_FM,                      // RB_PWR_CTRL RX modulation: components of the FM receiver are turned on
    RB_PWR_CTRL_RX_MOD_PM                       // RB_PWR_CTRL RX modulation: components of the PM receiver are turned on
} RB_PWR_CTRL_RX_MOD_BITS_ENUM;

enum {
    RB_STAT_CLK_EN                        =  0, // RB clock enable
    RB_STAT_RESET,                              // RB reset
    RB_STAT_LEDS_EN,                            // RB LEDs enabled
    RB_STAT_RSVD_D03,

    RB_STAT_TX_CAR_OSC_ZERO,                    // TX_CAR_OSC output is zero
    RB_STAT_TX_CAR_OSC_VALID,                   // TX_CAR_OSC output valid
    RB_STAT_RSVD_D06,
    RB_STAT_RSVD_D07,

    RB_STAT_TX_MOD_OSC_ZERO,                    // TX_MOD_OSC output is zero
    RB_STAT_TX_MOD_OSC_VALID,                   // TX_MOD_OSC output valid
    RB_STAT_RSVD_D10,
    RB_STAT_RSVD_D11,

    RB_STAT_RX_CAR_OSC_ZERO,                    // RX_CAR_OSC output is zero
    RB_STAT_RX_CAR_OSC_VALID,                   // RX_CAR_OSC output valid
    RB_STAT_RSVD_D14,
    RB_STAT_RSVD_D15,

    RB_STAT_RX_MOD_OSC_ZERO,                    // RX_MOD_OSC output is zero
    RB_STAT_RX_MOD_OSC_VALID,                   // RX_MOD_OSC output valid
    RB_STAT_RSVD_D18,
    RB_STAT_RSVD_D19,

    RB_STAT_RX_AFC_HIGH_SIG,                    // RX_AFC the receiving AM, FM, PM signal is strong enough to let the AFC do the frequency correction
    RB_STAT_RSVD_D21,
    RB_STAT_RSVD_D22,
    RB_STAT_RSVD_D23,

    RB_STAT_LED0_ON,                            // LED0 on
    RB_STAT_LED1_ON,                            // LED1 on
    RB_STAT_LED2_ON,                            // LED2 on
    RB_STAT_LED3_ON,                            // LED3 on
    RB_STAT_LED4_ON,                            // LED4 on
    RB_STAT_LED5_ON,                            // LED5 on
    RB_STAT_LED6_ON,                            // LED6 on
    RB_STAT_LED7_ON                             // LED7 on
} RB_STAT_BITS_ENUM;

enum {
    RB_SRC_CON_PNT_NUM_DISABLED                     =  0, // LEDs not driven by RB,   RFOUTx silence
    RB_SRC_CON_PNT_NUM_OFF,                               // all LEDs driven but off, RFOUTx silence

    RB_SRC_CON_PNT_NUM_TX_MUXIN_MIX_IN              =  4, // TX ADC selector input
    RB_SRC_CON_PNT_NUM_TX_MOD_ADC_IN,                     // TX modulation amplifier input
    RB_SRC_CON_PNT_NUM_TX_MOD_ADC_OUT,                    // TX modulation amplifier output

    RB_SRC_CON_PNT_NUM_TX_MOD_OSC_I_OUT             =  8, // TX_MOD_OSC I (cos) output
    RB_SRC_CON_PNT_NUM_TX_MOD_OSC_Q_OUT,                  // TX_MOD_OSC Q (sin) output
    RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_I_S1_OUT,              // TX_MOD_QMIX I output at stage 1
    RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_Q_S1_OUT,              // TX_MOD_QMIX Q output at stage 1
    RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_I_S2_OUT,              // TX_MOD_QMIX I output at stage 2
    RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_Q_S2_OUT,              // TX_MOD_QMIX Q output at stage 2
    RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_I_S3_OUT,              // TX_MOD_QMIX I output at stage 3
    RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_Q_S3_OUT,              // TX_MOD_QMIX Q output at stage 3

    RB_SRC_CON_PNT_NUM_TX_MOD_CIC_I_OUT             = 16, // TX_MOD_CIC I output
    RB_SRC_CON_PNT_NUM_TX_MOD_CIC_Q_OUT,                  // TX_MOD_CIC Q output
    RB_SRC_CON_PNT_NUM_TX_MOD_FIR_I_OUT,                  // TX_MOD_FIR I output
    RB_SRC_CON_PNT_NUM_TX_MOD_FIR_Q_OUT,                  // TX_MOD_FIR Q output
    RB_SRC_CON_PNT_NUM_TX_CAR_CIC_41M664_I_OUT,           // TX_CAR_CIC I stage 1 - 41.664 MHz output
    RB_SRC_CON_PNT_NUM_TX_CAR_CIC_41M664_Q_OUT,           // TX_CAR_CIC Q stage 1 - 41.664 MHz output

    RB_SRC_CON_PNT_NUM_TX_CAR_OSC_I_OUT             = 24, // TX_CAR_OSC I (cos) output
    RB_SRC_CON_PNT_NUM_TX_CAR_OSC_Q_OUT,                  // TX_CAR_OSC Q (sin) output
    RB_SRC_CON_PNT_NUM_TX_CAR_QMIX_I_OUT,                 // TX_CAR_QMIX I output
    RB_SRC_CON_PNT_NUM_TX_CAR_QMIX_Q_OUT,                 // TX_CAR_QMIX Q output

    RB_SRC_CON_PNT_NUM_TX_RF_AMP_OUT                = 28, // TX_RF_AMP output

    RB_SRC_CON_PNT_NUM_RX_CAR_OSC_I_OUT             = 32, // RX_CAR_OSC I output
    RB_SRC_CON_PNT_NUM_RX_CAR_OSC_Q_OUT,                  // RX_CAR_OSC Q output
    RB_SRC_CON_PNT_NUM_RX_CAR_QMIX_I_OUT,                 // RX_CAR_QMIX I output
    RB_SRC_CON_PNT_NUM_RX_CAR_QMIX_Q_OUT,                 // RX_CAR_QMIX Q output
    RB_SRC_CON_PNT_NUM_RX_CAR_5M_I_OUT,                   // RX_CAR_CIC1 I output
    RB_SRC_CON_PNT_NUM_RX_CAR_5M_Q_OUT,                   // RX_CAR_CIC1 Q output
    RB_SRC_CON_PNT_NUM_RX_CAR_200K_I_OUT,                 // RX_CAR_CIC2 I output
    RB_SRC_CON_PNT_NUM_RX_CAR_200K_Q_OUT,                 // RX_CAR_CIC2 Q output

    RB_SRC_CON_PNT_NUM_RX_MOD_8K_I_OUT              = 48, // RX_MOD_CIC1 I output
    RB_SRC_CON_PNT_NUM_RX_MOD_8K_Q_OUT,                   // RX_MOD_CIC1 Q output
    RB_SRC_CON_PNT_NUM_RX_MOD_VB1_I_OUT,                  // RX_MOD_FIR2 I output (weaver speech band, single sized)
    RB_SRC_CON_PNT_NUM_RX_MOD_VB1_Q_OUT,                  // RX_MOD_FIR2 Q output (weaver speech band, single sized)
    RB_SRC_CON_PNT_NUM_RX_MOD_OSC_I_OUT,                  // RX_MOD_OSC I output
    RB_SRC_CON_PNT_NUM_RX_MOD_OSC_Q_OUT,                  // RX_MOD_OSC Q output
    RB_SRC_CON_PNT_NUM_RX_MOD_HLD_I_OUT,                  // RX_MOD_HLD I output
    RB_SRC_CON_PNT_NUM_RX_MOD_HLD_Q_OUT,                  // RX_MOD_HLD Q output
    RB_SRC_CON_PNT_NUM_RX_MOD_QMIX_I_OUT,                 // RX_MOD_QMIX I output
    RB_SRC_CON_PNT_NUM_RX_MOD_QMIX_Q_OUT,                 // RX_MOD_QMIX Q output
    RB_SRC_CON_PNT_NUM_RX_MOD_VB2_I_OUT,                  // RX_MOD_FIR3 I output (weaver speech band, double sized)
    RB_SRC_CON_PNT_NUM_RX_MOD_VB2_Q_OUT,                  // RX_MOD_FIR3 Q output (weaver speech band, double sized)
    RB_SRC_CON_PNT_NUM_RX_MOD_48K_I_OUT,                  // RX_MOD_CIC4 I output
    RB_SRC_CON_PNT_NUM_RX_MOD_48K_Q_OUT,                  // RX_MOD_CIC4 Q output
    RB_SRC_CON_PNT_NUM_RX_MOD_SSB_AM_OUT,                 // RX_MOD_SSB_AM SSB or AM-sync output

    RB_SRC_CON_PNT_NUM_RX_AFC_FIR_I_OUT             = 64, // RX_AFC_FIR I output
    RB_SRC_CON_PNT_NUM_RX_AFC_FIR_Q_OUT,                  // RX_AFC_FIR Q output
    RB_SRC_CON_PNT_NUM_RX_AFC_CORDIC_MAG,                 // RX_AFC_CORDIC magnitude output
    RB_SRC_CON_PNT_NUM_RX_AFC_CORDIC_PHS,                 // RX_AFC_CORDIC phase output
    RB_SRC_CON_PNT_NUM_RX_AFC_CORDIC_PHS_PREV,            // RX_AFC_CORDIC previous 8kHz clock phase output
    RB_SRC_CON_PNT_NUM_RX_AFC_CORDIC_PHS_DIFF,            // RX_AFC_CORDIC phase difference output
    RB_SRC_CON_PNT_NUM_RX_AFC_INC_REG,                    // RX_AFC increment correction
    RB_SRC_CON_PNT_NUM_RX_SUM_INC_REG,                    // RX_SUM increment correction
    RB_SRC_CON_PNT_NUM_RX_MOD_FM_OUT,                     // RX_MOD FM output
    RB_SRC_CON_PNT_NUM_RX_MOD_PM_OUT,                     // RX_MOD PM output
    RB_SRC_CON_PNT_NUM_RX_MOD_AMENV_OUT,                  // RX_MOD AM-envelope output

    RB_SRC_CON_PNT_NUM_RX_AUDIO_OUT                 = 80, // RX_AUDIO output

    RB_SRC_CON_PNT_NUM_TEST_VECTOR_OUT              = 255 // Current test vector, look at assignments within this file

} RB_SRC_CON_PNT_ENUM;                                    // 256 entries = 2^8 --> 8 bit field

enum {
    RB_XADC_MAPPING_EXT_CH0                         =  0, // CH0 and CH8 are sampled simultaneously, mapped to: vinp_i[0]/vinn_i[0]
    RB_XADC_MAPPING_EXT_CH8,                              // CH0 and CH8 are sampled simultaneously, mapped to: vinp_i[1]/vinn_i[1]
    RB_XADC_MAPPING_EXT_CH1,                              // CH1 and CH9 are sampled simultaneously, mapped to: vinp_i[2]/vinn_i[2]
    RB_XADC_MAPPING_EXT_CH9,                              // CH1 and CH9 are sampled simultaneously, mapped to: vinp_i[3]/vinn_i[3]
    RB_XADC_MAPPING_VpVn,                                 // The dedicated Vp/Vn input mapped to: vinp_i[4]/vinn_i[4]
    RB_XADC_MAPPING__COUNT
} RB_XADC_MAPPING_ENUM;


// === OMNI section ===

//---------------------------------------------------------------------------------
// Short hand names

wire                   rb_enable              = regs[REG_RW_RB_CTRL][RB_CTRL_ENABLE];
wire                   tx_car_osc_inc_mux     = regs[REG_RW_RB_CTRL][RB_CTRL_TX_CAR_OSC_INC_SRC_STREAM];
wire                   tx_car_osc_ofs_mux     = regs[REG_RW_RB_CTRL][RB_CTRL_TX_CAR_OSC_OFS_SRC_STREAM];
wire                   tx_mod_osc_inc_mux     = regs[REG_RW_RB_CTRL][RB_CTRL_TX_MOD_OSC_INC_SRC_STREAM];
wire                   tx_mod_osc_ofs_mux     = regs[REG_RW_RB_CTRL][RB_CTRL_TX_MOD_OSC_OFS_SRC_STREAM];
wire                   rx_car_osc_inc_mux     = regs[REG_RW_RB_CTRL][RB_CTRL_RX_CAR_OSC_INC_SRC_STREAM];
wire                   rx_car_osc_ofs_mux     = regs[REG_RW_RB_CTRL][RB_CTRL_RX_CAR_OSC_OFS_SRC_STREAM];

wire                   tx_car_osc_reset       = regs[REG_RW_RB_CTRL][RB_CTRL_TX_CAR_OSC_RESET];
wire                   tx_car_osc_resync      = regs[REG_RW_RB_CTRL][RB_CTRL_TX_CAR_OSC_RESYNC];
wire                   tx_mod_osc_reset       = regs[REG_RW_RB_CTRL][RB_CTRL_TX_MOD_OSC_RESET];
wire                   tx_mod_osc_resync      = regs[REG_RW_RB_CTRL][RB_CTRL_TX_MOD_OSC_RESYNC];
wire                   rx_car_osc_reset       = regs[REG_RW_RB_CTRL][RB_CTRL_RX_CAR_OSC_RESET];
wire                   rx_car_osc_resync      = regs[REG_RW_RB_CTRL][RB_CTRL_RX_CAR_OSC_RESYNC];
wire                   rx_mod_osc_reset       = regs[REG_RW_RB_CTRL][RB_CTRL_RX_MOD_OSC_RESET];
wire                   rx_mod_osc_resync      = regs[REG_RW_RB_CTRL][RB_CTRL_RX_MOD_OSC_RESYNC];

wire unsigned [  7: 0] rb_pwr_rx_modvar       = regs[REG_RW_RB_PWR_CTRL][ 7:0];
wire unsigned [  7: 0] rb_pwr_tx_modvar       = regs[REG_RW_RB_PWR_CTRL][15:8];

wire unsigned [  7: 0] led_src_con_pnt        = regs[REG_RW_RB_RFOUTx_LED_SRC_CON_PNT][ 7: 0];
wire unsigned [  7: 0] rfout1_src_con_pnt     = regs[REG_RW_RB_RFOUTx_LED_SRC_CON_PNT][23:16];
wire unsigned [  7: 0] rfout2_src_con_pnt     = regs[REG_RW_RB_RFOUTx_LED_SRC_CON_PNT][31:24];

wire unsigned [  5: 0] tx_muxin_src           = regs[REG_RW_RB_TX_MUXIN_SRC][5:0];
wire unsigned [  5: 0] rx_muxin_src           = regs[REG_RW_RB_RX_MUXIN_SRC][5:0];

wire unsigned [ 15: 0] tx_muxin_mix_gain      = regs[REG_RW_RB_TX_MUXIN_GAIN][15: 0];
wire unsigned [  2: 0] tx_muxin_mix_log2      = regs[REG_RW_RB_TX_MUXIN_GAIN][18:16];
wire unsigned [ 15: 0] rx_muxin_mix_gain      = regs[REG_RW_RB_RX_MUXIN_GAIN][15: 0];
wire unsigned [  2: 0] rx_muxin_mix_log2      = regs[REG_RW_RB_RX_MUXIN_GAIN][18:16];

wire   signed [ 15: 0] tx_rf_amp_gain         = regs[REG_RW_RB_TX_RF_AMP_GAIN][15:0];
wire   signed [ 15: 0] tx_rf_amp_ofs          = regs[REG_RW_RB_TX_RF_AMP_OFS][15:0];

wire   signed [ 15: 0] tx_mod_qmix_gain       = regs[REG_RW_RB_TX_MOD_QMIX_GAIN][15:0];
wire   signed [ 47: 0] tx_mod_qmix_ofs        = { regs[REG_RW_RB_TX_MOD_QMIX_OFS_HI][15:0], regs[REG_RW_RB_TX_MOD_QMIX_OFS_LO][31:0] };

wire unsigned [ 47: 0] tx_car_osc_inc         = { regs[REG_RW_RB_TX_CAR_OSC_INC_HI][15:0], regs[REG_RW_RB_TX_CAR_OSC_INC_LO][31:0] };
wire unsigned [ 47: 0] tx_car_osc_ofs         = { regs[REG_RW_RB_TX_CAR_OSC_OFS_HI][15:0], regs[REG_RW_RB_TX_CAR_OSC_OFS_LO][31:0] };

wire unsigned [ 47: 0] tx_mod_osc_inc         = { regs[REG_RW_RB_TX_MOD_OSC_INC_HI][15:0], regs[REG_RW_RB_TX_MOD_OSC_INC_LO][31:0] };
wire unsigned [ 47: 0] tx_mod_osc_ofs         = { regs[REG_RW_RB_TX_MOD_OSC_OFS_HI][15:0], regs[REG_RW_RB_TX_MOD_OSC_OFS_LO][31:0] };

wire unsigned [ 47: 0] rx_car_osc_inc         = { regs[REG_RW_RB_RX_CAR_OSC_INC_HI][15:0], regs[REG_RW_RB_RX_CAR_OSC_INC_LO][31:0] };
wire unsigned [ 47: 0] rx_car_osc_ofs         = { regs[REG_RW_RB_RX_CAR_OSC_OFS_HI][15:0], regs[REG_RW_RB_RX_CAR_OSC_OFS_LO][31:0] };

wire   signed [ 31: 0] rx_afc_cordic_phs      = regs[REG_RD_RB_RX_AFC_CORDIC_PHS];
wire   signed [ 31: 0] rx_afc_cordic_phs_prev = regs[REG_RD_RB_RX_AFC_CORDIC_PHS_PREV];
wire   signed [ 31: 0] rx_afc_cordic_phs_diff = regs[REG_RD_RB_RX_AFC_CORDIC_PHS_DIFF];

wire unsigned [ 47: 0] rx_afc_calc_weaver_inc = { regs[REG_RW_RB_RX_CAR_CALC_WEAVER_INC_HI][15:0], regs[REG_RW_RB_RX_CAR_CALC_WEAVER_INC_LO][31:0] };
wire unsigned [ 47: 0] rx_car_afc_inc         = { regs[REG_RD_RB_RX_CAR_AFC_INC_HI][15:0], regs[REG_RD_RB_RX_CAR_AFC_INC_LO][31:0] };
wire unsigned [ 47: 0] rx_car_sum_inc         = { regs[REG_RD_RB_RX_CAR_SUM_INC_HI][15:0], regs[REG_RD_RB_RX_CAR_SUM_INC_LO][31:0] };

wire unsigned [ 47: 0] rx_mod_osc_inc         = { regs[REG_RW_RB_RX_MOD_OSC_INC_HI][15:0], regs[REG_RW_RB_RX_MOD_OSC_INC_LO][31:0] };
wire   signed [ 47: 0] rx_mod_osc_ofs         = { regs[REG_RW_RB_RX_MOD_OSC_OFS_HI][15:0], regs[REG_RW_RB_RX_MOD_OSC_OFS_LO][31:0] };

wire unsigned [ 15: 0] rx_ssb_am_gain         = regs[REG_RW_RB_RX_SSB_AM_GAIN];
wire unsigned [ 15: 0] rx_fm_gain             = regs[REG_RW_RB_RX_FM_GAIN];
wire unsigned [ 15: 0] rx_pm_gain             = regs[REG_RW_RB_RX_PM_GAIN];
wire   signed [ 15: 0] rx_audio_out_gain      = regs[REG_RW_RB_RX_AUDIO_OUT_GAIN];
wire   signed [ 15: 0] rx_audio_out_ofs       = regs[REG_RW_RB_RX_AUDIO_OUT_OFS];


//---------------------------------------------------------------------------------
//  RadioBox sub-module activation

wire          rb_clk_en;
wire          rb_reset_n;
assign        rb_activated = rb_reset_n;

red_pitaya_rst_clken rb_rst_clken_master (
  // global signals
  .clk                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .global_rst_n            ( adc_rstn_i                  ),  // ADC global reset

  // input signals
  .enable_i                ( rb_enable                   ),

  // output signals
  .reset_n_o               ( rb_reset_n                  ),
  .clk_en_o                ( rb_clk_en                   )
);

reg           rb_pwr_tx_OSC_en            = 1'b0;
reg           rb_pwr_tx_I_en              = 1'b0;
reg           rb_pwr_tx_Q_en              = 1'b0;

always @(posedge clk_adc_125mhz)                                       // power savings control based on TX modulation variants
if (!adc_rstn_i || !rb_reset_n) begin
   rb_pwr_tx_OSC_en <= 1'b0;
   rb_pwr_tx_I_en   <= 1'b0;
   rb_pwr_tx_Q_en   <= 1'b0;
   end
else begin
   case (rb_pwr_tx_modvar)
   8'h02: begin                                                        // USB
      rb_pwr_tx_OSC_en <= 1'b1;
      rb_pwr_tx_I_en   <= 1'b1;
      rb_pwr_tx_Q_en   <= 1'b1;
      end
   8'h03: begin                                                        // LSB
      rb_pwr_tx_OSC_en <= 1'b1;
      rb_pwr_tx_I_en   <= 1'b1;
      rb_pwr_tx_Q_en   <= 1'b1;
      end
   8'h04: begin                                                        // AM
      rb_pwr_tx_OSC_en <= 1'b1;
      rb_pwr_tx_I_en   <= 1'b0;
      rb_pwr_tx_Q_en   <= 1'b0;
      end
   8'h07: begin                                                        // FM
      rb_pwr_tx_OSC_en <= 1'b1;
      rb_pwr_tx_I_en   <= 1'b0;
      rb_pwr_tx_Q_en   <= 1'b0;
      end
   8'h08: begin                                                        // PM
      rb_pwr_tx_OSC_en <= 1'b1;
      rb_pwr_tx_I_en   <= 1'b0;
      rb_pwr_tx_Q_en   <= 1'b0;
      end
   default: begin                                                      // OFF
      rb_pwr_tx_OSC_en <= 1'b0;
      rb_pwr_tx_I_en   <= 1'b0;
      rb_pwr_tx_Q_en   <= 1'b0;
      end
   endcase
   end

wire          rb_pwr_tx_OSC_rst_n;
wire          rb_pwr_tx_OSC_clken;

red_pitaya_rst_clken rb_rst_clken_tx_OSC (
  // global signals
  .clk                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .global_rst_n            ( adc_rstn_i                  ),  // ADC global reset

  // input signals
  .enable_i                ( rb_pwr_tx_OSC_en            ),

  // output signals
  .reset_n_o               ( rb_pwr_tx_OSC_rst_n         ),
  .clk_en_o                ( rb_pwr_tx_OSC_clken         )
);

wire          rb_pwr_tx_I_rst_n;
wire          rb_pwr_tx_I_clken;

red_pitaya_rst_clken rb_rst_clken_tx_I (
  // global signals
  .clk                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .global_rst_n            ( adc_rstn_i                  ),  // ADC global reset

  // input signals
  .enable_i                ( rb_pwr_tx_I_en              ),

  // output signals
  .reset_n_o               ( rb_pwr_tx_I_rst_n           ),
  .clk_en_o                ( rb_pwr_tx_I_clken           )
);

wire          rb_pwr_tx_Q_rst_n;
wire          rb_pwr_tx_Q_clken;

red_pitaya_rst_clken rb_rst_clken_tx_Q (
  // global signals
  .clk                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .global_rst_n            ( adc_rstn_i                  ),  // ADC global reset

  // input signals
  .enable_i                ( rb_pwr_tx_Q_en              ),

  // output signals
  .reset_n_o               ( rb_pwr_tx_Q_rst_n           ),
  .clk_en_o                ( rb_pwr_tx_Q_clken           )
);

reg           rb_pwr_rx_CAR_en            = 1'b0;
reg           rb_pwr_rx_MOD_en            = 1'b0;
reg           rb_pwr_rx_AFC_en            = 1'b0;

always @(posedge clk_adc_125mhz)                                       // power savings control based on RX modulation variants
if (!adc_rstn_i || !rb_reset_n) begin
   rb_pwr_rx_CAR_en <= 1'b0;
   rb_pwr_rx_MOD_en <= 1'b0;
   rb_pwr_rx_AFC_en <= 1'b0;
   end
else begin
   case (rb_pwr_rx_modvar)
   8'h02: begin                                                        // USB
      rb_pwr_rx_CAR_en <= 1'b1;
      rb_pwr_rx_MOD_en <= 1'b1;
      rb_pwr_rx_AFC_en <= 1'b0;
      end
   8'h03: begin                                                        // LSB
      rb_pwr_rx_CAR_en <= 1'b1;
      rb_pwr_rx_MOD_en <= 1'b1;
      rb_pwr_rx_AFC_en <= 1'b0;
      end
   8'h04: begin                                                        // AM-ENV
      rb_pwr_rx_CAR_en <= 1'b1;
      rb_pwr_rx_MOD_en <= 1'b0;
      rb_pwr_rx_AFC_en <= 1'b1;
      end
   8'h05: begin                                                        // AM-SYNC-USB
      rb_pwr_rx_CAR_en <= 1'b1;
      rb_pwr_rx_MOD_en <= 1'b1;
      rb_pwr_rx_AFC_en <= 1'b1;
      end
   8'h06: begin                                                        // AM-SYNC-LSB
      rb_pwr_rx_CAR_en <= 1'b1;
      rb_pwr_rx_MOD_en <= 1'b1;
      rb_pwr_rx_AFC_en <= 1'b1;
      end
   8'h07: begin                                                        // FM
      rb_pwr_rx_CAR_en <= 1'b1;
      rb_pwr_rx_MOD_en <= 1'b0;
      rb_pwr_rx_AFC_en <= 1'b1;
      end
   8'h08: begin                                                        // PM
      rb_pwr_rx_CAR_en <= 1'b1;
      rb_pwr_rx_MOD_en <= 1'b0;
      rb_pwr_rx_AFC_en <= 1'b1;
      end
   default: begin                                                      // OFF
      rb_pwr_rx_CAR_en <= 1'b0;
      rb_pwr_rx_MOD_en <= 1'b0;
      rb_pwr_rx_AFC_en <= 1'b0;
      end
   endcase
   end

wire          rb_pwr_rx_CAR_rst_n;
wire          rb_pwr_rx_CAR_clken;

red_pitaya_rst_clken rb_rst_clken_rx_CAR (
  // global signals
  .clk                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .global_rst_n            ( adc_rstn_i                  ),  // ADC global reset

  // input signals
  .enable_i                ( rb_pwr_rx_CAR_en            ),

  // output signals
  .reset_n_o               ( rb_pwr_rx_CAR_rst_n         ),
  .clk_en_o                ( rb_pwr_rx_CAR_clken         )
);

wire          rb_pwr_rx_MOD_rst_n;
wire          rb_pwr_rx_MOD_clken;

red_pitaya_rst_clken rb_rst_clken_rx_MOD (
  // global signals
  .clk                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .global_rst_n            ( adc_rstn_i                  ),  // ADC global reset

  // input signals
  .enable_i                ( rb_pwr_rx_MOD_en            ),

  // output signals
  .reset_n_o               ( rb_pwr_rx_MOD_rst_n         ),
  .clk_en_o                ( rb_pwr_rx_MOD_clken         )
);

wire          rb_pwr_rx_AFC_rst_n;
wire          rb_pwr_rx_AFC_clken;

red_pitaya_rst_clken rb_rst_clken_rx_AFC (
  // global signals
  .clk                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .global_rst_n            ( adc_rstn_i                  ),  // ADC global reset

  // input signals
  .enable_i                ( rb_pwr_rx_AFC_en            ),

  // output signals
  .reset_n_o               ( rb_pwr_rx_AFC_rst_n         ),
  .clk_en_o                ( rb_pwr_rx_AFC_clken         )
);


//---------------------------------------------------------------------------------
//  Signal input matrix

// AXI streaming master from XADC

reg  [ 15: 0] rb_xadc[RB_XADC_MAPPING__COUNT - 1: 0];

always @(posedge xadc_axis_aclk) begin                                 // CLOCK_DOMAIN: FCLK_CLK0 (125 MHz) phase asynchron to clk_adc_125mhz
if (!adc_rstn_i) begin
   rb_xadc[RB_XADC_MAPPING_EXT_CH0] <= 16'b0;
   rb_xadc[RB_XADC_MAPPING_EXT_CH8] <= 16'b0;
   rb_xadc[RB_XADC_MAPPING_EXT_CH1] <= 16'b0;
   rb_xadc[RB_XADC_MAPPING_EXT_CH9] <= 16'b0;
   rb_xadc[RB_XADC_MAPPING_VpVn]    <= 16'b0;
   xadc_axis_tready <= 0;
   end

else begin
   xadc_axis_tready <= 1;                                              // no reason for signaling not to be ready
   if (xadc_axis_tvalid) begin
      casez (xadc_axis_tid)                                            // @see ug480_7Series_XADC.pdf for XADC channel mapping
      5'h10: begin                                                     // channel ID d16 for EXT-CH#0
         rb_xadc[RB_XADC_MAPPING_EXT_CH0]  <= xadc_axis_tdata;         // CH0 and CH8 are sampled simultaneously, mapped to: vinp_i[1]/vinn_i[1]
         end
      5'h18: begin                                                     // channel ID d24 for EXT-CH#8
         rb_xadc[RB_XADC_MAPPING_EXT_CH8]  <= xadc_axis_tdata;         // CH0 and CH8 are sampled simultaneously, mapped to: vinp_i[0]/vinn_i[0]
         end

      5'h11: begin                                                     // channel ID d17 for EXT-CH#1
         rb_xadc[RB_XADC_MAPPING_EXT_CH1]  <= xadc_axis_tdata;         // CH1 and CH9 are sampled simultaneously, mapped to: vinp_i[2]/vinn_i[2]
         end
      5'h19: begin                                                     // channel ID d25 for EXT-CH#9
         rb_xadc[RB_XADC_MAPPING_EXT_CH9]  <= xadc_axis_tdata;         // CH1 and CH9 are sampled simultaneously, mapped to: vinp_i[3]/vinn_i[3]
         end

      5'h03: begin                                                     // channel ID d3 for dedicated Vp/Vn input lines
         rb_xadc[RB_XADC_MAPPING_VpVn]     <= xadc_axis_tdata;         // The dedicated Vp/Vn input mapped to: vinp_i[4]/vinn_i[4]
         end

      default:   begin
         end
      endcase
      end
   end
end


parameter CLK_48KHZ_CTR_MAX = 2604;             // long run max value
parameter CLK_48KHZ_FRC_MAX = 5;

reg  [ 11: 0] clk_48khz_ctr  = 0;
reg  [  2: 0] clk_48khz_frc  = 0;
reg           clk_48khz      = 'b0;
reg           clk_8khz       = 'b0;

always @(posedge clk_adc_125mhz)                // assign clk_48khz, clk_8khz
if (!rb_clk_en) begin
   clk_48khz_ctr <= 'b0;
   clk_48khz_frc <= 'b0;
   clk_48khz <= 'b0;
   clk_8khz  <= 'b0;
   end
else
   if (clk_48khz_ctr == CLK_48KHZ_CTR_MAX) begin
      clk_48khz <= 1'b1;
      if (clk_48khz_frc == CLK_48KHZ_FRC_MAX) begin
         clk_48khz_frc <= 1'b0;
         clk_48khz_ctr <= 1'b0;                 // overflow of the frac part makes a long run
         clk_8khz <= 1'b1;
         end
      else begin
         clk_48khz_frc = clk_48khz_frc + 1;
         clk_48khz_ctr <= 12'b1;                // short run
         end
      end
   else begin
      clk_8khz  <= 1'b0;
      clk_48khz <= 1'b0;
      clk_48khz_ctr = clk_48khz_ctr + 1;
      end


parameter CLK_200KHZ_CTR_MAX = 624;

reg                    clk_200khz     = 1'b0;
reg  unsigned [ 11: 0] clk_200khz_ctr =  'b0;

always @(posedge clk_adc_125mhz)                // assign clk_200khz
if (!rb_clk_en) begin
   clk_200khz     <= 1'b0;
   clk_200khz_ctr <=  'b0;
   end
else
   if (clk_200khz_ctr == CLK_200KHZ_CTR_MAX) begin
      clk_200khz     <= 1'b1;
      clk_200khz_ctr <=  'b0;
      end
   else begin
      clk_200khz     <= 1'b0;
      clk_200khz_ctr <= clk_200khz_ctr + 1;
      end


// === Transmitter section ===
// === TX_MOD section ===

//---------------------------------------------------------------------------------
//  ADC modulation offset correction and gain

wire   signed [ 15: 0] tx_muxin_mix_in = (tx_muxin_src == 6'h20) ?  { ~adc_i[0], 2'b0 }              :
                                         (tx_muxin_src == 6'h21) ?  { ~adc_i[1], 2'b0 }              :
                                         (tx_muxin_src == 6'h18) ?  rb_xadc[RB_XADC_MAPPING_EXT_CH0] :  // swapped here due to pin connection warnings when swapping @ XADC <--> pins
                                         (tx_muxin_src == 6'h10) ?  rb_xadc[RB_XADC_MAPPING_EXT_CH8] :
                                         (tx_muxin_src == 6'h11) ?  rb_xadc[RB_XADC_MAPPING_EXT_CH1] :
                                         (tx_muxin_src == 6'h19) ?  rb_xadc[RB_XADC_MAPPING_EXT_CH9] :
                                         (tx_muxin_src == 6'h03) ?  rb_xadc[RB_XADC_MAPPING_VpVn]    :
                                         16'b0                                                       ;

wire   signed [ 15: 0] tx_mod_adc_in = (tx_muxin_mix_in << tx_muxin_mix_log2);  // unsigned value: input booster for
                                                                                // factor: 1x .. 2^3=7 shift postions=128x (16 mV --> full-scale)
wire   signed [ 15: 0] tx_mod_adc_ofs = 16'b0;                                  // TODO: FSM for calculating mean value to strip of the DC component
wire   signed [ 15: 0] tx_muxin_mix_gain_in = { 1'b0, tx_muxin_mix_gain[15:1] };
wire   signed [ 31: 0] tx_mod_adc_out;

rb_dsp48_AaDmB_A16_D16_B16_P32 i_rb_tx_mod_adc_dsp48 (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_pwr_tx_OSC_en            ),  // power down when needed to

  .A                       ( tx_mod_adc_in               ),  // MUX in signal:    SIGNED 16 bit
  .D                       ( tx_mod_adc_ofs              ),  // offset setting:   SIGNED 16 bit
  .B                       ( tx_muxin_mix_gain_in        ),  // gain setting:     SIGNED 16 bit

  .P                       ( tx_mod_adc_out              )   // PreAmp output     SIGSIG 32 bit
);


//---------------------------------------------------------------------------------
//  TX_MOD_OSC modulation oscillator and SSB weaver modulator

wire          tx_mod_osc_reset_n = rb_pwr_tx_OSC_rst_n & !tx_mod_osc_reset;

wire [ 47: 0] tx_mod_osc_inc_stream = 48'b0;  // TODO: ADC
wire [ 47: 0] tx_mod_osc_ofs_stream = 48'b0;  // TODO: ADC

wire [ 47: 0] tx_mod_osc_stream_inc = tx_mod_osc_inc_mux ?  tx_mod_osc_inc_stream :
                                                            tx_mod_osc_inc        ;
wire [ 47: 0] tx_mod_osc_stream_ofs = tx_mod_osc_ofs_mux ?  tx_mod_osc_ofs_stream :
                                                            tx_mod_osc_ofs        ;

wire          tx_mod_osc_axis_s_vld   = tx_mod_osc_reset_n;  // TODO: ADC
wire [103: 0] tx_mod_osc_axis_s_phase = { 7'b0, tx_mod_osc_resync, tx_mod_osc_stream_ofs, tx_mod_osc_stream_inc };

wire          tx_mod_osc_axis_m_vld;
wire [ 31: 0] tx_mod_osc_axis_m_data;

rb_dds_48_16_125 i_rb_tx_mod_osc (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_tx_OSC_clken         ),  // power down when needed to
  .aresetn                 ( tx_mod_osc_reset_n          ),  // reset of TX_MOD_OSC

  // AXI-Stream slave in port: streaming data for TX_MOD_OSC modulation
  .s_axis_phase_tvalid     ( tx_mod_osc_axis_s_vld       ),  // AXIS slave data valid
  .s_axis_phase_tdata      ( tx_mod_osc_axis_s_phase     ),  // AXIS slave data

  // AXI-Stream master out port: TX_MOD_OSC signal
  .m_axis_data_tvalid      ( tx_mod_osc_axis_m_vld       ),  // AXIS master TX_MOD_OSC data valid
  .m_axis_data_tdata       ( tx_mod_osc_axis_m_data      )   // AXIS master TX_MOD_OSC output: 2x SIGNED 16 bit
);

wire [ 15: 0] tx_mod_osc_cos = tx_mod_osc_axis_m_data[15: 0];
wire [ 15: 0] tx_mod_osc_sin = tx_mod_osc_axis_m_data[31:16];


//---------------------------------------------------------------------------------
//  TX_MOD_QMIX quadrature mixer for the base band

wire [ 15: 0] tx_mod_qmix_in      =  (tx_muxin_src == 6'h00) ?  16'h7fff : tx_mod_adc_out[30:15];  // when ADC source ID is zero take cos() from MOD_OSC only
wire [ 31: 0] tx_mod_qmix_i_s1_out;

rb_dsp48_AmB_A16_B16_P32 i_rb_tx_mod_qmix_I_s1_dsp48 (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_pwr_tx_OSC_clken         ),  // power down when needed to

  .A                       ( tx_mod_qmix_in              ),  // TX_MUX in signal:         SIGNED 16 bit
  .B                       ( tx_mod_osc_cos              ),  // TX_MOD_OSC cos:           SIGNED 16 bit

  .P                       ( tx_mod_qmix_i_s1_out        )   // TX_QMIX I output:         SIGSIG 32 bit
);

wire [ 31: 0] tx_mod_qmix_i_s2_out;

rb_dsp48_AmB_A16_B16_P32 i_rb_tx_mod_qmix_I_s2_dsp48 (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_pwr_tx_OSC_clken         ),  // power down when needed to

  .A                       ( tx_mod_qmix_i_s1_out[30:15] ),  // TX_MUX in signal:         SIGNED 16 bit
  .B                       ( tx_mod_qmix_gain            ),  // gain setting:             SIGNED 16 bit

  .P                       ( tx_mod_qmix_i_s2_out        )   // TX_QMIX I regulated:      SIGSIG 32 bit
);

wire [ 47: 0] tx_mod_qmix_i_s3_in = tx_car_osc_inc_mux ?  { {15{tx_mod_qmix_i_s2_out[30]}}, tx_mod_qmix_i_s2_out[29:0], 3'b0 } :  /* when FM is used, take 2^14 finer resolution */
                                                          { tx_mod_qmix_i_s2_out[30:0], 17'b0 }                                ;
wire [ 47: 0] tx_mod_qmix_i_s3_out;

rb_dsp48_CONaC_CON48_C48_P48 i_rb_tx_mod_qmix_I_s3_dsp48 (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_pwr_tx_OSC_clken         ),  // power down when needed to

  .C                       ( tx_mod_qmix_i_s3_in         ),  // modulation:               SIGNED 48 bit
  .CONCAT                  ( tx_mod_qmix_ofs             ),  // offset:                   SIGNED 48 bit

  .P                       ( tx_mod_qmix_i_s3_out        )   // TX_QMIX I for OSC:        SIGNED 48 bit
);

wire [ 31: 0] tx_mod_qmix_q_s1_out;

rb_dsp48_AmB_A16_B16_P32 i_rb_tx_mod_qmix_Q_s1_dsp48 (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_pwr_tx_Q_clken           ),  // power down when needed to

  .A                       ( tx_mod_qmix_in              ),  // MUX in signal:            SIGNED 16 bit
  .B                       ( tx_mod_osc_sin              ),  // MOD_OSC sin:              SIGNED 16 bit

  .P                       ( tx_mod_qmix_q_s1_out        )   // QMIX Q output:            SIGSIG 32 bit
);

wire [ 31: 0] tx_mod_qmix_q_s2_out;

rb_dsp48_AmB_A16_B16_P32 i_rb_tx_mod_qmix_Q_s2_dsp48 (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_pwr_tx_Q_clken           ),  // power down when needed to

  .A                       ( tx_mod_qmix_q_s1_out[30:15] ),  // MUX in signal:     SIGNED 16 bit
  .B                       ( tx_mod_qmix_gain            ),  // gain setting:             SIGNED 16 bit

  .P                       ( tx_mod_qmix_q_s2_out        )   // TX_QMIX Q regulated:      SIGSIG 32 bit
);

wire [ 47: 0] tx_mod_qmix_q_s3_in = { tx_mod_qmix_q_s2_out[30:0], 17'b0 };
wire [ 47: 0] tx_mod_qmix_q_s3_out;

rb_dsp48_CONaC_CON48_C48_P48 i_rb_tx_mod_qmix_Q_s3_dsp48 (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_pwr_tx_Q_clken           ),  // power down when needed to

  .C                       ( tx_mod_qmix_q_s3_in         ),  // modulation:               SIGNED 48 bit
  .CONCAT                  ( tx_mod_qmix_ofs             ),  // offset:                   SIGNED 48 bit

  .P                       ( tx_mod_qmix_q_s3_out        )   // TX_QMIX Q for OSC:        SIGNED 48 bit
);


//---------------------------------------------------------------------------------
//  TX_MOD_CIC sampling rate down convertion 48 kSPS to 8 kSPS

reg           tx_mod_cic_s_vld_i = 'b0;
wire          tx_mod_cic_s_rdy_i;

always @(posedge clk_adc_125mhz)                // assign tx_mod_cic_s_vld_i
begin
   if (!rb_pwr_tx_I_rst_n)
      tx_mod_cic_s_vld_i <= 'b0;
   else begin
      if (tx_mod_cic_s_vld_i && tx_mod_cic_s_rdy_i)
         tx_mod_cic_s_vld_i <= 'b0;             // falling back to non-active state

      if (clk_48khz)                            // trigger able to overwrite tx_mod_cic_s_vld_i
         tx_mod_cic_s_vld_i <= 'b1;             // entering active state
      end
end

reg           tx_mod_cic_s_vld_q = 'b0;
wire          tx_mod_cic_s_rdy_q;

always @(posedge clk_adc_125mhz)                // assign tx_mod_cic_s_vld_q
begin
   if (!rb_pwr_tx_Q_rst_n)
      tx_mod_cic_s_vld_q <= 'b0;
   else begin
      if (tx_mod_cic_s_vld_q && tx_mod_cic_s_rdy_q)
         tx_mod_cic_s_vld_q <= 'b0;             // falling back to non-active state

      if (clk_48khz)                            // trigger able to overwrite tx_mod_cic_s_vld_q
         tx_mod_cic_s_vld_q <= 'b1;             // entering active state
      end
end

wire [ 23: 0] tx_mod_cic_i_in = { 6'b0, tx_mod_qmix_i_s2_out[31:14] };  // AXIS word expansion
wire [ 23: 0] tx_mod_cic_i_out;
wire          tx_mod_cic_i_out_vld;
wire          tx_mod_cic_i_out_rdy;

rb_cic_48k_to_8k_18T18 i_rb_tx_mod_cic_I (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_tx_I_clken           ),  // power down when needed to
  .aresetn                 ( rb_pwr_tx_I_rst_n           ),

  .s_axis_data_tdata       ( tx_mod_cic_i_in             ),  // TX_QMIX I stage 2
  .s_axis_data_tvalid      ( tx_mod_cic_s_vld_i          ),
  .s_axis_data_tready      ( tx_mod_cic_s_rdy_i          ),

  .m_axis_data_tdata       ( tx_mod_cic_i_out            ),  // TX_MOD_CIC output I
  .m_axis_data_tvalid      ( tx_mod_cic_i_out_vld        ),
  .m_axis_data_tready      ( tx_mod_cic_i_out_rdy        ),
  .event_halted            (                             )
);

wire [ 23: 0] tx_mod_cic_q_in = { 6'b0, tx_mod_qmix_q_s2_out[31:14] };  // AXIS word expansion
wire [ 23: 0] tx_mod_cic_q_out;
wire          tx_mod_cic_q_out_vld;
wire          tx_mod_cic_q_out_rdy;

rb_cic_48k_to_8k_18T18 i_rb_tx_mod_cic_Q (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_tx_Q_clken           ),  // power down when needed to
  .aresetn                 ( rb_pwr_tx_Q_rst_n           ),

  .s_axis_data_tdata       ( tx_mod_cic_q_in             ),  // TX_QMIX Q stage 2
  .s_axis_data_tvalid      ( tx_mod_cic_s_vld_q          ),
  .s_axis_data_tready      ( tx_mod_cic_s_rdy_q          ),

  .m_axis_data_tdata       ( tx_mod_cic_q_out            ),  // TX_MOD_CIC output Q
  .m_axis_data_tvalid      ( tx_mod_cic_q_out_vld        ),
  .m_axis_data_tready      ( tx_mod_cic_q_out_rdy        ),
  .event_halted            (                             )
);


//---------------------------------------------------------------------------------
//  TX_MOD_FIR low pass filter for side-band selection
//
//  Coefficients built with Octave:
//  hn=fir2(62, [0 0.38 0.39 1], [1 1 0.000001 0.000001], 512, kaiser(63,4)); freqz(hn);

wire [ 23: 0] tx_mod_fir_i_in = { 7'b0, tx_mod_cic_i_out[16:0] };
wire [ 39: 0] tx_mod_fir_i_out;
wire          tx_mod_fir_i_out_vld;
wire          tx_mod_fir_i_out_rdy;

rb_fir_8k_to_8k_25c23_17i16_35o33 i_rb_tx_mod_fir_I (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_tx_I_clken           ),  // power down when needed to
  .aresetn                 ( rb_pwr_tx_I_rst_n           ),

  .s_axis_data_tdata       ( tx_mod_fir_i_in             ),  // TX_MOD_CIC output I - 8 kHz (17.16 bit width)
  .s_axis_data_tvalid      ( tx_mod_cic_i_out_vld        ),
  .s_axis_data_tready      ( tx_mod_cic_i_out_rdy        ),

  .m_axis_data_tdata       ( tx_mod_fir_i_out            ),  // TX_MOD_FIR output I - 8kHz (35.33 bit width)
  .m_axis_data_tvalid      ( tx_mod_fir_i_out_vld        ),
  .m_axis_data_tready      ( tx_mod_fir_i_out_rdy        )
);

wire [ 23: 0] tx_mod_fir_q_in = { 7'b0, tx_mod_cic_q_out[16:0] };
wire [ 39: 0] tx_mod_fir_q_out;
wire          tx_mod_fir_q_out_vld;
wire          tx_mod_fir_q_out_rdy;

rb_fir_8k_to_8k_25c23_17i16_35o33 i_rb_tx_mod_fir_Q (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_tx_Q_clken           ),  // power down when needed to
  .aresetn                 ( rb_pwr_tx_Q_rst_n           ),

  .s_axis_data_tdata       ( tx_mod_fir_q_in             ),  // TX_MOD_CIC output Q - 8 kHz (17.16 bit width)
  .s_axis_data_tvalid      ( tx_mod_cic_q_out_vld        ),
  .s_axis_data_tready      ( tx_mod_cic_q_out_rdy        ),

  .m_axis_data_tdata       ( tx_mod_fir_q_out            ),  // TX_MOD_FIR output Q - 8 kHz (35.33 bit width)
  .m_axis_data_tvalid      ( tx_mod_fir_q_out_vld        ),
  .m_axis_data_tready      ( tx_mod_fir_q_out_rdy        )
);


// === TX_CAR section ===

//---------------------------------------------------------------------------------
//  TX_CAR_CIC sampling rate up convertion 8 kSPS to 41.664 MSPS

wire [ 23: 0] tx_car_cic_41M664_i_in = { 6'b0, tx_mod_fir_i_out[32:15] };  // AXIS word expansion
wire [ 23: 0] tx_car_cic_41M664_i_out;
wire          tx_car_cic_41M664_i_out_vld;

rb_cic_8k_to_41M664_18T18 i_rb_tx_car_cic_I (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_tx_I_clken           ),  // power down when needed to
  .aresetn                 ( rb_pwr_tx_I_rst_n           ),

  .s_axis_data_tdata       ( tx_car_cic_41M664_i_in      ),  // TX_MOD_FIR I - 8 kHz
  .s_axis_data_tvalid      ( tx_mod_fir_i_out_vld        ),
  .s_axis_data_tready      ( tx_mod_fir_i_out_rdy        ),

  .m_axis_data_tdata       ( tx_car_cic_41M664_i_out     ),  // TX_CAR_CIC I stage 1 output - 41.664 MHz
  .m_axis_data_tvalid      ( tx_car_cic_41M664_i_out_vld )
);

wire [ 23: 0] tx_car_cic_41M664_q_in = { 6'b0, tx_mod_fir_q_out[32:15] };  // AXIS word expansion
wire [ 23: 0] tx_car_cic_41M664_q_out;
wire          tx_car_cic_41M664_q_out_vld;

rb_cic_8k_to_41M664_18T18 i_rb_tx_car_cic_Q (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_tx_Q_clken           ),  // power down when needed to
  .aresetn                 ( rb_pwr_tx_Q_rst_n           ),

  .s_axis_data_tdata       ( tx_car_cic_41M664_q_in      ),  // TX_MOD_FIR Q - 8 kHz
  .s_axis_data_tvalid      ( tx_mod_fir_q_out_vld        ),
  .s_axis_data_tready      ( tx_mod_fir_q_out_rdy        ),

  .m_axis_data_tdata       ( tx_car_cic_41M664_q_out     ),  // TX_CAR_CIC Q stage 1 output - 41.664 MHz
  .m_axis_data_tvalid      ( tx_car_cic_41M664_q_out_vld )
);


//---------------------------------------------------------------------------------
//  TX_CAR_REGS

reg  [ 15: 0] tx_car_regs_i_data = 'b0;

always @(posedge clk_adc_125mhz)
if (!rb_pwr_tx_I_rst_n)                            // register input I
   tx_car_regs_i_data <= 'b0;
else if (tx_car_cic_41M664_i_out_vld)
   tx_car_regs_i_data <= tx_car_cic_41M664_i_out[16:1];

reg  [ 15: 0] tx_car_regs_q_data = 'b0;

always @(posedge clk_adc_125mhz)
if (!rb_pwr_tx_Q_rst_n)                            // register input Q
   tx_car_regs_q_data <= 'b0;
else if (tx_car_cic_41M664_q_out_vld)
   tx_car_regs_q_data <= tx_car_cic_41M664_q_out[16:1];


//---------------------------------------------------------------------------------
//  TX_CAR_OSC carrier frequency oscillator  (CW, FM, PM modulated)
wire          tx_car_osc_reset_n      = rb_pwr_tx_OSC_rst_n & !tx_car_osc_reset;

wire [ 47: 0] tx_car_osc_stream_inc   = tx_car_osc_inc_mux ?  tx_mod_qmix_i_s3_out :
                                                              tx_car_osc_inc       ;
wire [ 47: 0] tx_car_osc_stream_ofs   = tx_car_osc_ofs_mux ?  tx_mod_qmix_i_s3_out :
                                                              tx_car_osc_ofs       ;
wire          tx_car_osc_axis_s_vld   = tx_car_osc_reset_n;
wire [103: 0] tx_car_osc_axis_s_phase = { 7'b0, tx_car_osc_resync, tx_car_osc_stream_ofs, tx_car_osc_stream_inc };

wire          tx_car_osc_axis_m_vld;
wire [ 31: 0] tx_car_osc_axis_m_data;

rb_dds_48_16_125 i_rb_tx_car_osc (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_tx_OSC_clken         ),  // power down when needed to
  .aresetn                 ( tx_car_osc_reset_n          ),  // reset of TX_CAR_OSC

  // simple-AXI slave in port: streaming data for TX_CAR_OSC modulation
  .s_axis_phase_tvalid     ( tx_car_osc_axis_s_vld       ),  // AXIS slave data valid
  .s_axis_phase_tdata      ( tx_car_osc_axis_s_phase     ),  // AXIS slave data

  // simple-AXI master out port: TX_CAR_OSC signal
  .m_axis_data_tvalid      ( tx_car_osc_axis_m_vld       ),  // AXIS master TX_CAR_OSC data valid
  .m_axis_data_tdata       ( tx_car_osc_axis_m_data      )   // AXIS master TX_CAR_OSC output: Q SIGNED 16 bit, I SIGNED 16 bit
);

wire [ 15: 0] tx_car_osc_cos             = tx_car_osc_axis_m_data[15: 0];
wire [ 15: 0] tx_car_osc_sin             = tx_car_osc_axis_m_data[31:16];


//---------------------------------------------------------------------------------
//  TX_CAR_QMIX quadrature mixer for the radio frequency

wire [ 15: 0] tx_car_qmix_i_in = (tx_car_osc_inc_mux || tx_car_osc_ofs_mux)                       ?  16'h7fff                    :
                                  rb_pwr_tx_I_rst_n                                               ?  tx_car_regs_i_data          :
                                                                                                     tx_mod_qmix_i_s3_out[47:32] ;  // TX_MOD_QMIX/TX_MOD_CIC uses full scale constant for CW, FM and PM modulations - SSB uses TX_CIC I instead
wire [ 31: 0] tx_car_qmix_i_out;

rb_dsp48_AmB_A16_B16_P32 i_rb_tx_car_qmix_I_dsp48 (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_pwr_tx_OSC_clken         ),  // power down when needed to

  .A                       ( tx_car_qmix_i_in            ),  // TX_MUX in signal:     SIGNED 16 bit
  .B                       ( tx_car_osc_cos              ),  // TX_CAR_OSC cos:       SIGNED 16 bit

  .P                       ( tx_car_qmix_i_out           )   // TX_CAR_QMIX I output: SIGSIG 32 bit
);

wire [ 15: 0] tx_car_qmix_q_in = (tx_car_osc_inc_mux || tx_car_osc_ofs_mux || !rb_pwr_tx_Q_rst_n) ?  16'h0000                    :
                                                                                                     tx_car_regs_q_data          ;  // TX_MOD_QMIX/TX_MOD_CIC Q path keep quiet when Q is disabled - SSB uses TX_CIC Q instead
wire [ 31: 0] tx_car_qmix_q_out;

rb_dsp48_AmB_A16_B16_P32 i_rb_tx_car_qmix_Q_dsp48 (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_pwr_tx_OSC_clken         ),  // power down when needed to

  .A                       ( tx_car_qmix_q_in            ),  // TX_MUX in signal:     SIGNED 16 bit
  .B                       ( tx_car_osc_sin              ),  // TX_CAR_OSC sin:       SIGNED 16 bit

  .P                       ( tx_car_qmix_q_out           )   // TX_CAR_QMIX Q output: SIGSIG 32 bit
);


//---------------------------------------------------------------------------------
//  TX_RF_AMP amplifier for the radio frequency output (CW, AM modulated)

wire [ 16: 0] tx_amp_rf_i_var =                      { tx_car_qmix_i_out[30], tx_car_qmix_i_out[30:15] } ;  // halfed and sign corrected 17 bit extension
wire [ 16: 0] tx_amp_rf_q_var = rb_pwr_tx_Q_rst_n ?  { tx_car_qmix_q_out[30], tx_car_qmix_q_out[30:15] } :
                                                       17'b0                                             ;  // halfed and sign corrected 17 bit extension
wire [ 16: 0] tx_amp_rf_gain  = { tx_rf_amp_gain,  1'b0 };  // signed register value
wire [ 34: 0] tx_amp_rf_ofs   = { tx_rf_amp_ofs,  19'b0 };  // signed register value

wire [ 35: 0] tx_amp_rf_out;

rb_dsp48_AaDmBaC_A17_D17_B17_C35_P36 i_rb_tx_amp_rf_dsp48 (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_pwr_tx_OSC_clken         ),  // power down when needed to
  .SCLR                    ( !rb_pwr_tx_OSC_rst_n        ),  // put output to neutral when activated

  .A                       ( tx_amp_rf_i_var             ),  // TX_QMIX_RF I      SIGNED 17 bit
  .D                       ( tx_amp_rf_q_var             ),  // TX_QMIX_RF Q      SIGNED 17 bit
  .B                       ( tx_amp_rf_gain              ),  // TX_RF_AMP gain    SIGNED 17 bit
  .C                       ( tx_amp_rf_ofs               ),  // TX_RF_AMP ofs     SIGSIG 35 bit

  .P                       ( tx_amp_rf_out               )   // TX_AMP RF output  SIGSIG 36 bit
);


// === Receiver section ===
// === RX_CAR section ===

//---------------------------------------------------------------------------------
//  RX_MUXIN amplifier

wire   signed [ 15: 0] rx_muxin_sig = (rx_muxin_src == 6'h20) ?  { ~adc_i[0], 2'b0 }              :
                                      (rx_muxin_src == 6'h21) ?  { ~adc_i[1], 2'b0 }              :
                                      (rx_muxin_src == 6'h18) ?  rb_xadc[RB_XADC_MAPPING_EXT_CH0] :  // swapped here due to pin connection warnings when swapping @ XADC <--> pins
                                      (rx_muxin_src == 6'h10) ?  rb_xadc[RB_XADC_MAPPING_EXT_CH8] :
                                      (rx_muxin_src == 6'h11) ?  rb_xadc[RB_XADC_MAPPING_EXT_CH1] :
                                      (rx_muxin_src == 6'h19) ?  rb_xadc[RB_XADC_MAPPING_EXT_CH9] :
                                      (rx_muxin_src == 6'h03) ?  rb_xadc[RB_XADC_MAPPING_VpVn]    :
                                      16'b0                                                       ;

wire   signed [ 15: 0] rx_muxin_mix_in = (rx_muxin_sig << rx_muxin_mix_log2);  // unsigned value: input booster for
                                                                               // factor: 1x .. 2^3=7 shift postions=128x (16 mV --> full-scale)
wire   signed [ 15: 0] rx_muxin_mix_gain_in = { 1'b0, rx_muxin_mix_gain[15:1] };
wire   signed [ 31: 0] rx_muxin_mix_out;

rb_dsp48_AmB_A16_B16_P32 i_rb_rx_muxin_amplifier (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_pwr_rx_CAR_clken         ),  // power down when needed to

  .A                       ( rx_muxin_mix_in             ),  // input signal            SIGNED 16 bit
  .B                       ( rx_muxin_mix_gain_in        ),  // RX amplifier gain       SIGNED 16 bit

  .P                       ( rx_muxin_mix_out            )   // RX level adj. input     SIGSIG 32 bit
);

wire [ 15: 0] rx_muxin_out = rx_muxin_mix_out[27:12];


//---------------------------------------------------------------------------------
//  RX_CAR_OSC carrier oscillator

wire          rx_car_osc_reset_n      = rb_pwr_rx_CAR_rst_n & !rx_car_osc_reset;
wire [ 47: 0] rx_car_osc_stream_inc   = rx_car_osc_inc_mux ?  rx_car_sum_inc :
                                                              rx_car_osc_inc ;
wire unsigned [ 47: 0] rx_car_osc_stream_ofs   = rx_car_osc_ofs;

wire          rx_car_osc_axis_s_vld   = rb_pwr_rx_CAR_rst_n;
wire [103: 0] rx_car_osc_axis_s_phase = { 7'b0, rx_car_osc_resync, rx_car_osc_stream_ofs, rx_car_osc_stream_inc };
wire          rx_car_osc_axis_m_vld;
wire [ 31: 0] rx_car_osc_axis_m_data;

rb_dds_48_16_125 i_rb_rx_car_osc (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_rx_CAR_clken         ),  // power down when needed to
  .aresetn                 ( rx_car_osc_reset_n          ),  // reset of RX_CAR_OSC

  // simple-AXI slave in port: streaming data for RX_CAR_OSC modulation
  .s_axis_phase_tvalid     ( rx_car_osc_axis_s_vld       ),  // AXIS slave data valid
  .s_axis_phase_tdata      ( rx_car_osc_axis_s_phase     ),  // AXIS slave data

  // simple-AXI master out port: RX_CAR_OSC signal
  .m_axis_data_tvalid      ( rx_car_osc_axis_m_vld       ),  // AXIS master TX_CAR_OSC data valid
  .m_axis_data_tdata       ( rx_car_osc_axis_m_data      )   // AXIS master TX_CAR_OSC output: Q SIGNED 16 bit, I SIGNED 16 bit
);

wire [ 15: 0] rx_car_osc_cos = rx_car_osc_axis_m_data[15: 0];
wire [ 15: 0] rx_car_osc_sin = rx_car_osc_axis_m_data[31:16];


//---------------------------------------------------------------------------------
//  RX_CAR_QMIX quadrature mixer for the radio frequency

wire [ 31: 0] rx_car_qmix_i_out;

rb_dsp48_AmB_A16_B16_P32 i_rb_rx_car_qmix_I_dsp48 (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_pwr_rx_CAR_clken         ),  // power down when needed to

  .A                       ( rx_muxin_out                ),  // RX_MUX in signal:       SIGNED 16 bit
  .B                       ( rx_car_osc_cos              ),  // RX_CAR_OSC cos:         SIGNED 16 bit

  .P                       ( rx_car_qmix_i_out           )   // RX_CAR_QMIX I output:   SIGSIG 32 bit
);

wire [ 31: 0] rx_car_qmix_q_out;

rb_dsp48_AmB_A16_B16_P32 i_rb_rx_car_qmix_Q_dsp48 (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_pwr_rx_CAR_clken         ),  // power down when needed to

  .A                       ( rx_muxin_out                ),  // RX_MUX in signal:       SIGNED 16 bit
  .B                       ( rx_car_osc_sin              ),  // RX_CAR_OSC sin:         SIGNED 16 bit

  .P                       ( rx_car_qmix_q_out           )   // RX_CAR_QMIX Q output:   SIGSIG 32 bit
);


//---------------------------------------------------------------------------------
//  RX_CAR_CIC1 sampling rate down convertion 125 MSPS to 5 MSPS

wire [ 23: 0] rx_car_cic1_i_in = { 6'b0, rx_car_qmix_i_out[31:14] };  // AXIS word expansion
wire [ 23: 0] rx_car_cic1_i_out;
wire          rx_car_cic1_i_out_vld;
wire          rx_car_cic1_i_out_rdy;

rb_cic_125M_to_5M_18T18 i_rb_rx_car_cic1_I (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_rx_CAR_clken         ),  // power down when needed to
  .aresetn                 ( rb_pwr_rx_CAR_rst_n         ),

  .s_axis_data_tdata       ( rx_car_cic1_i_in            ),
  .s_axis_data_tvalid      ( 1'b1                        ),
  .s_axis_data_tready      (                             ),

  .m_axis_data_tdata       ( rx_car_cic1_i_out           ),  // RX_CAR_CIC1 output I
  .m_axis_data_tvalid      ( rx_car_cic1_i_out_vld       ),
  .m_axis_data_tready      ( rx_car_cic1_i_out_rdy       ),
  .event_halted            (                             )
);

wire [ 23: 0] rx_car_cic1_q_in = { 6'b0, rx_car_qmix_q_out[31:14] };  // AXIS word expansion
wire [ 23: 0] rx_car_cic1_q_out;
wire          rx_car_cic1_q_out_vld;
wire          rx_car_cic1_q_out_rdy;

rb_cic_125M_to_5M_18T18 i_rb_rx_car_cic1_Q (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_rx_CAR_clken         ),  // power down when needed to
  .aresetn                 ( rb_pwr_rx_CAR_rst_n         ),

  .s_axis_data_tdata       ( rx_car_cic1_q_in            ),
  .s_axis_data_tvalid      ( 1'b1                        ),
  .s_axis_data_tready      (                             ),

  .m_axis_data_tdata       ( rx_car_cic1_q_out           ),  // RX_CAR_CIC1 output Q
  .m_axis_data_tvalid      ( rx_car_cic1_q_out_vld       ),
  .m_axis_data_tready      ( rx_car_cic1_q_out_rdy       ),
  .event_halted            (                             )
);


//---------------------------------------------------------------------------------
//  RX_CAR_CIC2 sampling rate down convertion 5 MSPS to 200 kSPS

wire [ 23: 0] rx_car_cic2_i_in = { 6'b0, rx_car_cic1_i_out[17:0] };  // AXIS word expansion
wire [ 23: 0] rx_car_cic2_i_out;
wire          rx_car_cic2_i_out_vld;

rb_cic_5M_to_200k_18T18 i_rb_rx_car_cic2_I (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_rx_CAR_clken         ),  // power down when needed to
  .aresetn                 ( rb_pwr_rx_CAR_rst_n         ),

  .s_axis_data_tdata       ( rx_car_cic2_i_in            ),
  .s_axis_data_tvalid      ( rx_car_cic1_i_out_vld       ),
  .s_axis_data_tready      ( rx_car_cic1_i_out_rdy       ),

  .m_axis_data_tdata       ( rx_car_cic2_i_out           ),  // RX_CAR_CIC2 output I
  .m_axis_data_tvalid      ( rx_car_cic2_i_out_vld       )
);

wire [ 23: 0] rx_car_cic2_q_in = { 6'b0, rx_car_cic1_q_out[17:0] };  // AXIS word expansion
wire [ 23: 0] rx_car_cic2_q_out;
wire          rx_car_cic2_q_out_vld;

rb_cic_5M_to_200k_18T18 i_rb_rx_car_cic2_Q (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_rx_CAR_clken         ),  // power down when needed to
  .aresetn                 ( rb_pwr_rx_CAR_rst_n         ),

  .s_axis_data_tdata       ( rx_car_cic2_q_in            ),
  .s_axis_data_tvalid      ( rx_car_cic1_q_out_vld       ),
  .s_axis_data_tready      ( rx_car_cic1_q_out_rdy       ),

  .m_axis_data_tdata       ( rx_car_cic2_q_out           ),  // RX_CAR_CIC2 output Q
  .m_axis_data_tvalid      ( rx_car_cic2_q_out_vld       )
);


//---------------------------------------------------------------------------------
//  RX_CAR_REGS2 @ 200 kSPS

reg  [ 17: 0] rx_car_regs2_i_data    =  'b0;
reg           rx_car_regs2_i_mod_new = 1'b0;
reg           rx_car_regs2_i_afc_new = 1'b0;

always @(posedge clk_adc_125mhz)
if (!rb_pwr_rx_CAR_rst_n) begin                    // register input I
   rx_car_regs2_i_data    <=  'b0;
   rx_car_regs2_i_mod_new <= 1'b0;
   rx_car_regs2_i_afc_new <= 1'b0;
   end
else if (rx_car_cic2_i_out_vld) begin
   rx_car_regs2_i_data    <= rx_car_cic2_i_out[17:0];
   rx_car_regs2_i_mod_new <= 1'b1;
   rx_car_regs2_i_afc_new <= 1'b1;
   end
else begin
   if (rx_car_regs2_i_mod_vld)
      rx_car_regs2_i_mod_new <= 1'b0;
   if (rx_car_regs2_i_afc_vld)
      rx_car_regs2_i_afc_new <= 1'b0;
   end

reg  [ 17: 0] rx_car_regs2_q_data    =  'b0;
reg           rx_car_regs2_q_mod_new = 1'b0;
reg           rx_car_regs2_q_afc_new = 1'b0;

always @(posedge clk_adc_125mhz)
if (!rb_pwr_rx_CAR_rst_n) begin                    // register input Q
   rx_car_regs2_q_data    <=  'b0;
   rx_car_regs2_q_mod_new <= 1'b0;
   rx_car_regs2_q_afc_new <= 1'b0;
   end
else if (rx_car_cic2_q_out_vld) begin
   rx_car_regs2_q_data    <= rx_car_cic2_q_out[17:0];
   rx_car_regs2_q_mod_new <= 1'b1;
   rx_car_regs2_q_afc_new <= 1'b1;
   end
else begin
   if (rx_car_regs2_q_mod_vld)
      rx_car_regs2_q_mod_new <= 1'b0;
   if (rx_car_regs2_q_afc_vld)
      rx_car_regs2_q_afc_new <= 1'b0;
   end

reg           rx_car_regs2_i_mod_vld = 1'b0;
wire          rx_car_regs2_i_mod_rdy;

always @(posedge clk_adc_125mhz)
if (!rb_pwr_rx_MOD_rst_n)                          // register output I for MOD
   rx_car_regs2_i_mod_vld <= 1'b0;
else
   if (rx_car_regs2_i_mod_new && rx_car_regs2_q_mod_new)
      rx_car_regs2_i_mod_vld <= 1'b1;
   else if (rx_car_regs2_i_mod_vld && rx_car_regs2_i_mod_rdy)
      rx_car_regs2_i_mod_vld <= 1'b0;

reg           rx_car_regs2_q_mod_vld = 1'b0;
wire          rx_car_regs2_q_mod_rdy;

always @(posedge clk_adc_125mhz)
if (!rb_pwr_rx_MOD_rst_n)                          // register output Q for MOD
   rx_car_regs2_q_mod_vld <= 1'b0;
else
   if (rx_car_regs2_i_mod_new && rx_car_regs2_q_mod_new)
      rx_car_regs2_q_mod_vld <= 1'b1;
   else if (rx_car_regs2_q_mod_vld && rx_car_regs2_q_mod_rdy)
      rx_car_regs2_q_mod_vld <= 1'b0;

reg           rx_car_regs2_i_afc_vld = 1'b0;
wire          rx_car_regs2_i_afc_rdy;

always @(posedge clk_adc_125mhz)
if (!rb_pwr_rx_AFC_rst_n)                          // register output I for AFC
   rx_car_regs2_i_afc_vld <= 1'b0;
else
   if (rx_car_regs2_i_afc_new && rx_car_regs2_q_afc_new)
      rx_car_regs2_i_afc_vld <= 1'b1;
   else if (rx_car_regs2_i_afc_vld && rx_car_regs2_i_afc_rdy)
      rx_car_regs2_i_afc_vld <= 1'b0;

reg           rx_car_regs2_q_afc_vld = 1'b0;
wire          rx_car_regs2_q_afc_rdy;

always @(posedge clk_adc_125mhz)
if (!rb_pwr_rx_AFC_rst_n)                          // register output Q for AFC
   rx_car_regs2_q_afc_vld <= 1'b0;
else
   if (rx_car_regs2_i_afc_new && rx_car_regs2_q_afc_new)
      rx_car_regs2_q_afc_vld <= 1'b1;
   else if (rx_car_regs2_q_afc_vld && rx_car_regs2_q_afc_rdy)
      rx_car_regs2_q_afc_vld <= 1'b0;


// === RX_MOD section ===

//---------------------------------------------------------------------------------
//  RX_MOD_CIC1 sampling rate down convertion 200 kSPS to 8 kSPS

wire unsigned [ 23: 0] rx_mod_cic1_i_in      = { 6'b0, rx_car_regs2_i_data[16:0], 1'b0 };  // AXIS word expansion
wire unsigned [ 23: 0] rx_mod_cic1_i_out;
wire                   rx_mod_cic1_i_out_vld;

rb_cic_200k_to_8k_18T18 i_rb_rx_mod_cic1_I (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_rx_MOD_clken         ),  // power down when needed to
  .aresetn                 ( rb_pwr_rx_MOD_rst_n         ),

  .s_axis_data_tdata       ( rx_mod_cic1_i_in            ),
  .s_axis_data_tvalid      ( rx_car_regs2_i_mod_vld      ),
  .s_axis_data_tready      ( rx_car_regs2_i_mod_rdy      ),

  .m_axis_data_tdata       ( rx_mod_cic1_i_out           ),  // RX_MOD_CIC1 output I
  .m_axis_data_tvalid      ( rx_mod_cic1_i_out_vld       )
);

wire unsigned [ 23: 0] rx_mod_cic1_q_in      = { 6'b0, rx_car_regs2_q_data[16:0], 1'b0 };  // AXIS word expansion
wire unsigned [ 23: 0] rx_mod_cic1_q_out;
wire                   rx_mod_cic1_q_out_vld;

rb_cic_200k_to_8k_18T18 i_rb_rx_mod_cic1_Q (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_rx_MOD_clken         ),  // power down when needed to
  .aresetn                 ( rb_pwr_rx_MOD_rst_n         ),

  .s_axis_data_tdata       ( rx_mod_cic1_q_in            ),
  .s_axis_data_tvalid      ( rx_car_regs2_q_mod_vld      ),
  .s_axis_data_tready      ( rx_car_regs2_q_mod_rdy      ),

  .m_axis_data_tdata       ( rx_mod_cic1_q_out           ),  // RX_MOD_CIC1 output Q
  .m_axis_data_tvalid      ( rx_mod_cic1_q_out_vld       )
);


//---------------------------------------------------------------------------------
//  RX_MOD_REGS1

reg  [ 17: 0] rx_mod_regs1_i_data =  'b0;
reg           rx_mod_regs1_i_new  = 1'b0;

always @(posedge clk_adc_125mhz) begin
if (!rb_pwr_rx_MOD_rst_n) begin                    // register input I
   rx_mod_regs1_i_data <=  'b0;
   rx_mod_regs1_i_new  <= 1'b0;
   end
else if (rx_mod_cic1_i_out_vld) begin
   rx_mod_regs1_i_data <= rx_mod_cic1_i_out[17:0];
   rx_mod_regs1_i_new  <= 1'b1;
   end
if (rx_mod_regs1_i_out_vld)
   rx_mod_regs1_i_new <= 1'b0;
end

reg  [ 17: 0] rx_mod_regs1_q_data =  'b0;
reg           rx_mod_regs1_q_new  = 1'b0;

always @(posedge clk_adc_125mhz) begin
if (!rb_pwr_rx_MOD_rst_n) begin                    // register input Q
   rx_mod_regs1_q_data <=  'b0;
   rx_mod_regs1_q_new  <= 1'b0;
   end
else if (rx_mod_cic1_q_out_vld) begin
   rx_mod_regs1_q_data <= rx_mod_cic1_q_out[17:0];
   rx_mod_regs1_q_new  <= 1'b1;
   end
if (rx_mod_regs1_q_out_vld)
   rx_mod_regs1_q_new <= 1'b0;
end

reg           rx_mod_regs1_i_out_vld = 1'b0;
wire          rx_mod_regs1_i_out_rdy;

always @(posedge clk_adc_125mhz)
if (!rb_pwr_rx_MOD_rst_n)                          // register output I
   rx_mod_regs1_i_out_vld <= 1'b0;
else
   if (rx_mod_regs1_i_new)
      rx_mod_regs1_i_out_vld <= 1'b1;
   else if (rx_mod_regs1_i_out_vld && rx_mod_regs1_i_out_rdy)
      rx_mod_regs1_i_out_vld <= 1'b0;

reg           rx_mod_regs1_q_out_vld = 1'b0;
wire          rx_mod_regs1_q_out_rdy;

always @(posedge clk_adc_125mhz)
if (!rb_pwr_rx_MOD_rst_n)                          // register output Q
   rx_mod_regs1_q_out_vld <= 1'b0;
else
   if (rx_mod_regs1_q_new)
      rx_mod_regs1_q_out_vld <= 1'b1;
   else if (rx_mod_regs1_q_out_vld && rx_mod_regs1_q_out_rdy)
      rx_mod_regs1_q_out_vld <= 1'b0;


//---------------------------------------------------------------------------------
//  RX_MOD_FIR2 low pass filter for side-band selection
//
//  Coefficients built with Octave:
//  hn=fir2(126, [0 1400/8000 1425/8000 1], [1 1 0.000001 0.000001], 4096, kaiser(127, 10)); freqz(hn);

wire [ 23: 0] rx_mod_fir2_i_in = { 7'b0, rx_mod_regs1_i_data[16:0] };  // AXIS word expansion
wire [ 39: 0] rx_mod_fir2_i_out;
wire          rx_mod_fir2_i_out_vld;

rb_fir1_8k_to_8k_25c_17i16_35o32 i_rb_rx_mod_fir2_I (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_rx_MOD_clken         ),  // power down when needed to
  .aresetn                 ( rb_pwr_rx_MOD_rst_n         ),

  .s_axis_data_tdata       ( rx_mod_fir2_i_in            ),
  .s_axis_data_tvalid      ( rx_mod_regs1_i_out_vld      ),
  .s_axis_data_tready      ( rx_mod_regs1_i_out_rdy      ),

  .m_axis_data_tdata       ( rx_mod_fir2_i_out           ),  // RX_MOD_FIR1 output I - 8kHz (35.33 bit width)
  .m_axis_data_tvalid      ( rx_mod_fir2_i_out_vld       )
);

wire [ 23: 0] rx_mod_fir2_q_in = { 7'b0, rx_mod_regs1_q_data[16:0] };  // AXIS word expansion
wire [ 39: 0] rx_mod_fir2_q_out;
wire          rx_mod_fir2_q_out_vld;

rb_fir1_8k_to_8k_25c_17i16_35o32 i_rb_rx_mod_fir2_Q (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_rx_MOD_clken         ),  // power down when needed to
  .aresetn                 ( rb_pwr_rx_MOD_rst_n         ),

  .s_axis_data_tdata       ( rx_mod_fir2_q_in            ),
  .s_axis_data_tvalid      ( rx_mod_regs1_q_out_vld      ),
  .s_axis_data_tready      ( rx_mod_regs1_q_out_rdy      ),

  .m_axis_data_tdata       ( rx_mod_fir2_q_out           ),  // RX_MOD_FIR1 output Q - 8 kHz (35.33 bit width)
  .m_axis_data_tvalid      ( rx_mod_fir2_q_out_vld       )
);


//---------------------------------------------------------------------------------
//  RX_MOD_REGS2

reg  [ 15: 0] rx_mod_regs2_i_data =  'b0;
reg           rx_mod_regs2_i_new  = 1'b0;

always @(posedge clk_adc_125mhz)
if (!rb_pwr_rx_MOD_rst_n) begin                    // register input I
   rx_mod_regs2_i_data <=  'b0;
   rx_mod_regs2_i_new  <= 1'b0;
   end
else if (rx_mod_fir2_i_out_vld) begin
   rx_mod_regs2_i_data <= rb_pwr_rx_AFC_en ?  rx_mod_fir2_i_out[31:16] :
                                              rx_mod_fir2_i_out[33:18] ;  // drive by factor 4 when modulation is AM
   rx_mod_regs2_i_new  <= 1'b1;
   end
else if (rx_mod_regs2_i_out_vld)
   rx_mod_regs2_i_new  <= 1'b0;

reg  [ 15: 0] rx_mod_regs2_q_data =  'b0;
reg           rx_mod_regs2_q_new  = 1'b0;

always @(posedge clk_adc_125mhz)
if (!rb_pwr_rx_MOD_rst_n) begin                    // register input Q
   rx_mod_regs2_q_data <=  'b0;
   rx_mod_regs2_q_new  <= 1'b0;
   end
else if (rx_mod_fir2_q_out_vld) begin
   rx_mod_regs2_q_data <= rb_pwr_rx_AFC_en ?  rx_mod_fir2_q_out[31:16] :
                                              rx_mod_fir2_q_out[33:18] ;  // drive by factor 4 when modulation is AM
   rx_mod_regs2_q_new  <= 1'b1;
   end
else if (rx_mod_regs2_q_out_vld)
   rx_mod_regs2_q_new  <= 1'b0;

reg           rx_mod_regs2_i_out_vld = 1'b0;
wire          rx_mod_regs2_i_out_rdy;

always @(posedge clk_adc_125mhz)
if (!rb_pwr_rx_MOD_rst_n)                          // register output I
   rx_mod_regs2_i_out_vld <= 1'b0;
else
   if (rx_mod_regs2_i_new)
      rx_mod_regs2_i_out_vld <= 1'b1;
   else if (rx_mod_regs2_i_out_vld && rx_mod_regs2_i_out_rdy)
      rx_mod_regs2_i_out_vld <= 1'b0;

reg           rx_mod_regs2_q_out_vld = 1'b0;
wire          rx_mod_regs2_q_out_rdy;

always @(posedge clk_adc_125mhz)
if (!rb_pwr_rx_MOD_rst_n)                          // register output Q
   rx_mod_regs2_q_out_vld <= 1'b0;
else
   if (rx_mod_regs2_q_new)
      rx_mod_regs2_q_out_vld <= 1'b1;
   else if (rx_mod_regs2_q_out_vld && rx_mod_regs2_q_out_rdy)
      rx_mod_regs2_q_out_vld <= 1'b0;


//---------------------------------------------------------------------------------
//  RX_MOD_OSC modulation oscillator and SSB weaver modulator

wire          rx_mod_osc_reset_n = rb_pwr_rx_MOD_rst_n & !rx_mod_osc_reset;

wire [ 47: 0] rx_mod_osc_stream_inc = rx_mod_osc_inc;
wire [ 47: 0] rx_mod_osc_stream_ofs = rx_mod_osc_ofs;

wire          rx_mod_osc_axis_s_vld   = rx_mod_osc_reset_n;
wire [103: 0] rx_mod_osc_axis_s_phase = { 7'b0, rx_mod_osc_resync, rx_mod_osc_stream_ofs, rx_mod_osc_stream_inc };

wire          rx_mod_osc_axis_m_vld;
wire [ 31: 0] rx_mod_osc_axis_m_data;

rb_dds_48_16_125 i_rb_rx_mod_osc (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_rx_MOD_clken         ),  // power down when needed to
  .aresetn                 ( rx_mod_osc_reset_n          ),  // reset of RX_MOD_OSC

  // AXI-Stream slave in port: streaming data for RX_MOD_OSC modulation
  .s_axis_phase_tvalid     ( rx_mod_osc_axis_s_vld       ),  // AXIS slave data valid
  .s_axis_phase_tdata      ( rx_mod_osc_axis_s_phase     ),  // AXIS slave data

  // AXI-Stream master out port: RX_MOD_OSC signal
  .m_axis_data_tvalid      ( rx_mod_osc_axis_m_vld       ),  // AXIS master RX_MOD_OSC data valid
  .m_axis_data_tdata       ( rx_mod_osc_axis_m_data      )   // AXIS master RX_MOD_OSC output: 2x SIGNED 16 bit
);

wire [ 15: 0] rx_mod_osc_cos = rx_mod_osc_axis_m_data[15: 0];
wire [ 15: 0] rx_mod_osc_sin = rx_mod_osc_axis_m_data[31:16];


//---------------------------------------------------------------------------------
//  RX_MOD_HLD sample & hold for the RX_MOD_OSC

reg  [ 15: 0] rx_mod_hld_i_data = 'b0;
reg  [ 15: 0] rx_mod_hld_q_data = 'b0;

always @(posedge clk_adc_125mhz)
if (!rb_pwr_rx_MOD_rst_n) begin
   rx_mod_hld_i_data <= 'b0;
   rx_mod_hld_q_data <= 'b0;
   end
else if (clk_8khz) begin
   rx_mod_hld_i_data <= rx_mod_osc_cos;
   rx_mod_hld_q_data <= rx_mod_osc_sin;
   end


//---------------------------------------------------------------------------------
//  RX_MOD_QMIX quadrature mixer for the base band

wire [ 31: 0] rx_mod_qmix_i_out;

rb_dsp48_AmB_A16_B16_P32 i_rb_rx_mod_qmix_I_dsp48 (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_pwr_rx_MOD_clken         ),  // power down when needed to

  .A                       ( rx_mod_regs2_i_data         ),  // RX_MOD_FIR2 in I sig.:  SIGNED 16 bit
  .B                       ( rx_mod_hld_i_data           ),  // RX_MOD_OSC cos:         SIGNED 16 bit

  .P                       ( rx_mod_qmix_i_out           )   // RX_MOD_QMIX I output:   SIGSIG 32 bit
);

wire [ 31: 0] rx_mod_qmix_q_out;

rb_dsp48_AmB_A16_B16_P32 i_rb_rx_mod_qmix_Q_dsp48 (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_pwr_rx_MOD_clken         ),  // power down when needed to

  .A                       ( rx_mod_regs2_q_data         ),  // RX_MOD_FIR2 in Q sig.:  SIGNED 16 bit
  .B                       ( rx_mod_hld_q_data           ),  // RX_MOD_OSC sin:         SIGNED 16 bit

  .P                       ( rx_mod_qmix_q_out           )   // RX_MOD_QMIX I output:   SIGSIG 32 bit
);


//---------------------------------------------------------------------------------
//  RX_MOD_FIR3 low pass filter for weaver upper/lower side-band selection
//
//  Coefficients built with Octave:
//  hn=fir2(126, [0/4000 900/4000 1700/4000 2500/4000 3300/4000  3350/4000 1], [1 1.5 2.25 3.375 5  0.000001 0.000001], 4096, kaiser(127, 4.5)); freqz(hn);

wire [ 23: 0] rx_mod_fir3_i_in = { 7'b0, rx_mod_qmix_i_out[30:14] };  // AXIS word expansion
wire [ 39: 0] rx_mod_fir3_i_out;
wire          rx_mod_fir3_i_out_vld;

rb_fir2_8k_to_8k_25c_17i16_35o32 i_rb_rx_mod_fir3_I (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_rx_MOD_clken         ),  // power down when needed to
  .aresetn                 ( rb_pwr_rx_MOD_rst_n         ),

  .s_axis_data_tdata       ( rx_mod_fir3_i_in            ),
  .s_axis_data_tvalid      ( rx_mod_regs2_i_out_vld      ),
  .s_axis_data_tready      ( rx_mod_regs2_i_out_rdy      ),

  .m_axis_data_tdata       ( rx_mod_fir3_i_out           ),  // RX_MOD_FIR3 output I - 8kHz (35.33 bit width)
  .m_axis_data_tvalid      ( rx_mod_fir3_i_out_vld       )
);

wire [ 23: 0] rx_mod_fir3_q_in = { 7'b0, rx_mod_qmix_q_out[30:14] };  // AXIS word expansion
wire [ 39: 0] rx_mod_fir3_q_out;
wire          rx_mod_fir3_q_out_vld;

rb_fir2_8k_to_8k_25c_17i16_35o32 i_rb_rx_mod_fir3_Q (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_rx_MOD_clken         ),  // power down when needed to
  .aresetn                 ( rb_pwr_rx_MOD_rst_n         ),

  .s_axis_data_tdata       ( rx_mod_fir3_q_in            ),
  .s_axis_data_tvalid      ( rx_mod_regs2_q_out_vld      ),
  .s_axis_data_tready      ( rx_mod_regs2_q_out_rdy      ),

  .m_axis_data_tdata       ( rx_mod_fir3_q_out           ),  // RX_MOD_FIR3 output Q - 8 kHz (35.33 bit width)
  .m_axis_data_tvalid      ( rx_mod_fir3_q_out_vld       )
);


//---------------------------------------------------------------------------------
//  RX_MOD_REGS3

reg  [ 17: 0] rx_mod_regs3_i_data =  'b0;
reg           rx_mod_regs3_i_new  = 1'b0;
wire          rx_mod_regs3_i_out_rdy;

always @(posedge clk_adc_125mhz)
if (!rb_pwr_rx_MOD_rst_n) begin                    // register input I
   rx_mod_regs3_i_data <=  'b0;
   rx_mod_regs3_i_new  <= 1'b0;
   end
else if (rx_mod_fir3_i_out_vld) begin
   rx_mod_regs3_i_data <= rx_mod_fir3_i_out[32:15];
   rx_mod_regs3_i_new  <= 1'b1;
   end
else if (rx_mod_regs3_i_out_vld)
   rx_mod_regs3_i_new  <= 1'b0;

reg  [ 17: 0] rx_mod_regs3_q_data =  'b0;
reg           rx_mod_regs3_q_new  = 1'b0;
wire          rx_mod_regs3_q_out_rdy;

always @(posedge clk_adc_125mhz)
if (!rb_pwr_rx_MOD_rst_n) begin                    // register input Q
   rx_mod_regs3_q_data <=  'b0;
   rx_mod_regs3_q_new  <= 1'b0;
   end
else if (rx_mod_fir3_q_out_vld) begin
   rx_mod_regs3_q_data <= rx_mod_fir3_q_out[32:15];
   rx_mod_regs3_q_new  <= 1'b1;
   end
else if (rx_mod_regs3_q_out_vld)
   rx_mod_regs3_q_new  <= 1'b0;

reg           rx_mod_regs3_i_out_vld = 1'b0;

always @(posedge clk_adc_125mhz)
if (!rb_pwr_rx_MOD_rst_n)                          // register output I
   rx_mod_regs3_i_out_vld <= 1'b0;
else
   if (rx_mod_regs3_i_new)
      rx_mod_regs3_i_out_vld <= 1'b1;
   else if (rx_mod_regs3_i_out_vld && rx_mod_regs3_i_out_rdy)
      rx_mod_regs3_i_out_vld <= 1'b0;

reg           rx_mod_regs3_q_out_vld = 1'b0;

always @(posedge clk_adc_125mhz)
if (!rb_pwr_rx_MOD_rst_n)                          // register output Q
   rx_mod_regs3_q_out_vld <= 1'b0;
else
   if (rx_mod_regs3_q_new)
      rx_mod_regs3_q_out_vld <= 1'b1;
   else if (rx_mod_regs3_q_out_vld && rx_mod_regs3_q_out_rdy)
      rx_mod_regs3_q_out_vld <= 1'b0;


//---------------------------------------------------------------------------------
//  RX_MOD_CIC4 sampling rate up convertion 8 kSPS to 48 kSPS

reg                    rx_mod_cic4_chk_rst_n = 1'b0;
wire unsigned [ 23: 0] rx_mod_cic4_i_in      = { 6'b0, rx_mod_regs3_i_data[16:0], 1'b0 };  // AXIS word expansion
wire unsigned [ 23: 0] rx_mod_cic4_i_out;
wire                   rx_mod_cic4_i_out_vld;

rb_cic_8k_to_48k_18T18 i_rb_rx_mod_cic4_I (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_rx_MOD_clken         ),  // power down when needed to
  .aresetn                 ( rx_mod_cic4_chk_rst_n       ),

  .s_axis_data_tdata       ( rx_mod_cic4_i_in            ),
  .s_axis_data_tvalid      ( rx_mod_regs3_i_out_vld      ),
  .s_axis_data_tready      ( rx_mod_regs3_i_out_rdy      ),

  .m_axis_data_tdata       ( rx_mod_cic4_i_out           ),  // RX_MOD_CIC4 output I
  .m_axis_data_tvalid      ( rx_mod_cic4_i_out_vld       )
);

wire unsigned [ 23: 0] rx_mod_cic4_q_in      = { 6'b0, rx_mod_regs3_q_data[16:0], 1'b0 };  // AXIS word expansion
wire unsigned [ 23: 0] rx_mod_cic4_q_out;
wire                   rx_mod_cic4_q_vld;

rb_cic_8k_to_48k_18T18 i_rb_rx_mod_cic4_Q (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_rx_MOD_clken         ),  // power down when needed to
  .aresetn                 ( rx_mod_cic4_chk_rst_n       ),

  .s_axis_data_tdata       ( rx_mod_cic4_q_in            ),
  .s_axis_data_tvalid      ( rx_mod_regs3_q_out_vld      ),
  .s_axis_data_tready      ( rx_mod_regs3_q_out_rdy      ),

  .m_axis_data_tdata       ( rx_mod_cic4_q_out           ),  // RX_MOD_CIC4 output Q
  .m_axis_data_tvalid      ( rx_mod_cic4_q_out_vld       )
);

reg    signed [ 17: 0] rx_mod_cic4_chk_i_data_cur = 'b0;
reg    signed [ 17: 0] rx_mod_cic4_chk_i_data_lst = 'b0;
reg    signed [ 17: 0] rx_mod_cic4_chk_q_data_cur = 'b0;
reg    signed [ 17: 0] rx_mod_cic4_chk_q_data_lst = 'b0;

always @(posedge clk_adc_125mhz)                             // This FSM corrects a bug within the CIC when interpolation is active. Sometimes the CIC enters into a violated state. This FSM detects that and resets the CIC.
if (!rb_pwr_rx_MOD_rst_n) begin                              // register input I
   rx_mod_cic4_chk_i_data_cur <= 'b0;
   rx_mod_cic4_chk_i_data_lst <= 'b0;
   rx_mod_cic4_chk_q_data_cur <= 'b0;
   rx_mod_cic4_chk_q_data_lst <= 'b0;
   end
else begin
   if (rx_mod_cic4_i_out_vld) begin
      rx_mod_cic4_chk_i_data_lst <= rx_mod_cic4_chk_i_data_cur;
      rx_mod_cic4_chk_i_data_cur <= rx_mod_cic4_i_out;
      end
   if (rx_mod_cic4_q_out_vld) begin
      rx_mod_cic4_chk_q_data_lst <= rx_mod_cic4_chk_q_data_cur;
      rx_mod_cic4_chk_q_data_cur <= rx_mod_cic4_q_out;
      end
   end

wire   signed [ 17: 0] rx_mod_cic4_chk_i_data_lst_diff1 = rx_mod_cic4_chk_i_data_cur - rx_mod_cic4_chk_i_data_lst;
wire   signed [ 17: 0] rx_mod_cic4_chk_i_data_lst_diff2 = rx_mod_cic4_chk_i_data_lst - rx_mod_cic4_chk_i_data_cur;
wire   signed [ 17: 0] rx_mod_cic4_chk_q_data_lst_diff1 = rx_mod_cic4_chk_q_data_cur - rx_mod_cic4_chk_q_data_lst;
wire   signed [ 17: 0] rx_mod_cic4_chk_q_data_lst_diff2 = rx_mod_cic4_chk_q_data_lst - rx_mod_cic4_chk_q_data_cur;
reg  unsigned [  2: 0] rx_mod_cic4_chk_rst_cnt         =  'b0;

always @(posedge clk_adc_125mhz)
if (!rb_pwr_rx_MOD_rst_n) begin                              // register input I
   rx_mod_cic4_chk_rst_n   <= 1'b0;
   rx_mod_cic4_chk_rst_cnt <=  'b0;
   end
else if (clk_48khz)
   if (!rx_mod_cic4_chk_rst_cnt)
      if (((rx_mod_cic4_chk_i_data_lst_diff1[17:16] != 2'b0) && (rx_mod_cic4_chk_i_data_lst_diff2[17:16] != 2'b0)) ||
          ((rx_mod_cic4_chk_q_data_lst_diff1[17:16] != 2'b0) && (rx_mod_cic4_chk_q_data_lst_diff2[17:16] != 2'b0))) begin
         // CIC entered distored state - start reset procedure
         rx_mod_cic4_chk_rst_n   <= 1'b0;
         rx_mod_cic4_chk_rst_cnt <= 3'b111;
         end
      else
         rx_mod_cic4_chk_rst_n   <= 1'b1;                    // after power-on release of reset
   else begin
      if (rx_mod_cic4_chk_rst_cnt == 3'b100)
         rx_mod_cic4_chk_rst_n   <= 1'b1;                    // release of reset from distorted state
      rx_mod_cic4_chk_rst_cnt <= rx_mod_cic4_chk_rst_cnt - 1;
      end


//---------------------------------------------------------------------------------
//  RX_MOD_SSB_ADD reconstruction of the modulation

wire [ 16: 0] rx_mod_ssb_am_i_var = rx_mod_cic4_i_out[16:0];
wire [ 16: 0] rx_mod_ssb_am_q_var = rx_mod_cic4_q_out[16:0];
wire [ 16: 0] rx_mod_ssb_am_g_var = { 1'b0, rx_ssb_am_gain };  // expand unsigned 16 bit to signed 17 bit

wire [ 35: 0] rx_mod_ssb_mix_out;

rb_dsp48_AaDmBaC_A17_D17_B17_C35_P36 i_rb_rx_mod_ssb_am_dsp48 (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_pwr_rx_MOD_clken         ),  // power down when needed to
  .SCLR                    ( !rb_pwr_rx_MOD_rst_n        ),  // put output to neutral when activated

  .A                       ( rx_mod_ssb_am_i_var         ),  // RX_MOD_CIC4 I     SIGNED 17 bit
  .D                       ( rx_mod_ssb_am_q_var         ),  // RX_MOD_CIC4 Q     SIGNED 17 bit
  .B                       ( rx_mod_ssb_am_g_var         ),  // RX_MOD SSB gain   SIGNED 17 bit
  .C                       ( 35'b0                       ),  // no offset value specified

  .P                       ( rx_mod_ssb_mix_out          )   // RX_MOD SSB output SIGSIG 36 bit
);

wire [ 15: 0] rx_mod_ssb_am_out = rx_mod_ssb_mix_out[31:16];


// === RX_AFC section ===

//---------------------------------------------------------------------------------
//  RX_AFC_FIR low pass filter for carrier detection - @ 200 kSPS
//
//  Coefficients built with Octave:
//  Set 1: SSB / AM - band pass for 1700 Hz with abt. 440 Hz @ -40 dB band width
//  hn=fir2(253, [0 1660/1000000 1680/1000000 1700/1000000 1720/1000000 1740/1000000 1], [0.000000001 0.00001 0.001 114 0.001 0.00001 0.000000001], 65536, kaiser(254, 0.01)); freqz(hn);
//
//  Set 2: FM / PM - low pass with abt. 2.56 kHz @ -6 dB
//  hn=fir1(254, 50000/200000, 'low', 'barthannwin'); freqz(hn);

wire                   rx_afc_fir_is_set2   = ((rb_pwr_rx_modvar == RB_PWR_CTRL_RX_MOD_FM)      ||
                                               (rb_pwr_rx_modvar == RB_PWR_CTRL_RX_MOD_PM)      ||
                                               (rb_pwr_rx_modvar == RB_PWR_CTRL_RX_MOD_AM_ENV)) ?  1'b1 : 1'b0;
wire unsigned [  7: 0] rx_afc_fir_cfg_in    = { 7'b0, rx_afc_fir_is_set2 };         // AXIS word expansion
wire   signed [ 23: 0] rx_afc_fir_i_in      = { 7'b0, rx_car_regs2_i_data[17:1] };  // AXIS word expansion
wire unsigned [ 39: 0] rx_afc_fir_i_out;
wire                   rx_afc_fir_i_out_vld;

rb_fir3_200k_to_200k_24c_17i16_35o i_rb_rx_afc_fir_I (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_rx_AFC_clken         ),  // power down when needed to
  .aresetn                 ( rb_pwr_rx_AFC_rst_n         ),

  .s_axis_config_tdata     ( rx_afc_fir_cfg_in           ),  // 0: filter set 1 (SSB/AM), 1: filter set 2 (FM/PM)
  .s_axis_config_tvalid    ( 1'b1                        ),
  .s_axis_config_tready    (                             ),

  .s_axis_data_tdata       ( rx_afc_fir_i_in             ),
  .s_axis_data_tvalid      ( rx_car_regs2_i_afc_vld      ),
  .s_axis_data_tready      ( rx_car_regs2_i_afc_rdy      ),

  .m_axis_data_tdata       ( rx_afc_fir_i_out            ),  // RX_AFC_FIR output I - 8kHz (35.33 bit width)
  .m_axis_data_tvalid      ( rx_afc_fir_i_out_vld        )
);

wire   signed [ 23: 0] rx_afc_fir_q_in      = { 7'b0, rx_car_regs2_q_data[17:1] };  // AXIS word expansion
wire unsigned [ 39: 0] rx_afc_fir_q_out;
wire                   rx_afc_fir_q_out_vld;

rb_fir3_200k_to_200k_24c_17i16_35o i_rb_rx_afc_fir_Q (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_rx_AFC_clken         ),  // power down when needed to
  .aresetn                 ( rb_pwr_rx_AFC_rst_n         ),

  .s_axis_config_tdata     ( rx_afc_fir_cfg_in           ),  // 0: filter set 1 (SSB/AM), 1: filter set 2 (FM/PM)
  .s_axis_config_tvalid    ( 1'b1                        ),
  .s_axis_config_tready    (                             ),

  .s_axis_data_tdata       ( rx_afc_fir_q_in             ),
  .s_axis_data_tvalid      ( rx_car_regs2_q_afc_vld      ),
  .s_axis_data_tready      ( rx_car_regs2_q_afc_rdy      ),

  .m_axis_data_tdata       ( rx_afc_fir_q_out            ),  // RX_AFC_FIR output Q - 8 kHz (35.33 bit width)
  .m_axis_data_tvalid      ( rx_afc_fir_q_out_vld        )
);


//---------------------------------------------------------------------------------
//  RX_AFC_CORDIC_FSM1

reg  [ 31: 0] rx_afc_fir_i_reg  =  'b0;
reg           rx_afc_fsm1_i_new = 1'b0;

always @(posedge clk_adc_125mhz)                // input register I
if (!rb_pwr_rx_AFC_rst_n) begin
   rx_afc_fir_i_reg  <=  'b0;
   rx_afc_fsm1_i_new <= 1'b0;
   end
else if (rx_afc_fir_i_out_vld) begin
   rx_afc_fir_i_reg  <= rx_afc_fir_i_out[31:0];
   rx_afc_fsm1_i_new <= 1'b1;
   end
else if (rx_afc_fsm1_ctr)
   rx_afc_fsm1_i_new <= 1'b0;

reg  [ 31: 0] rx_afc_fir_q_reg  =  'b0;
reg           rx_afc_fsm1_q_new = 1'b0;

always @(posedge clk_adc_125mhz)                // input register Q
if (!rb_pwr_rx_AFC_rst_n) begin
   rx_afc_fir_q_reg  <=  'b0;
   rx_afc_fsm1_q_new <= 1'b0;
   end
else if (rx_afc_fir_q_out_vld) begin
   rx_afc_fir_q_reg <= rx_afc_fir_q_out[31:0];
   rx_afc_fsm1_q_new <= 1'b1;
   end
else if (rx_afc_fsm1_ctr)
   rx_afc_fsm1_q_new <= 1'b0;

reg  [  1: 0] rx_afc_fsm1_ctr = 2'b0;
reg           rx_afc_cordic_cart_vld = 1'b0;
wire          rx_afc_cordic_cart_rdy;

always @(posedge clk_adc_125mhz)
if (!rb_pwr_rx_AFC_rst_n) begin
   rx_afc_cordic_cart_vld <= 1'b0;
   rx_afc_fsm1_ctr <= 2'b0;
   end
else if (!rx_afc_fsm1_ctr && (rx_afc_fsm1_i_new && rx_afc_fsm1_q_new && clk_200khz))  // new data is available
   rx_afc_fsm1_ctr <= 2'd3;
else if (rx_afc_fsm1_ctr) begin
   rx_afc_cordic_cart_vld <= 1'b1;
   if (rx_afc_cordic_cart_rdy)                 // data output handshake
      rx_afc_fsm1_ctr <= rx_afc_fsm1_ctr - 1;
   end
else
   rx_afc_cordic_cart_vld <= 1'b0;


//---------------------------------------------------------------------------------
//  RX_AFC_CORDIC_DLY

reg           rx_afc_cordic_dly_pulse = 1'b0;
reg  [  7: 0] rx_afc_cordic_dly_ctr   =  'b0;

always @(posedge clk_adc_125mhz)
if (!rb_pwr_rx_AFC_rst_n) begin
   rx_afc_cordic_dly_pulse <= 1'b0;
   rx_afc_cordic_dly_ctr   <=  'b0;
   end
else if (!rx_afc_cordic_dly_ctr && rx_afc_cordic_cart_vld && rx_afc_cordic_cart_rdy)
   rx_afc_cordic_dly_ctr <= 8'd185;
else if (rx_afc_cordic_dly_ctr) begin
   if (rx_afc_cordic_dly_ctr == 8'd1)
      rx_afc_cordic_dly_pulse <= 1'b1;
   rx_afc_cordic_dly_ctr <= rx_afc_cordic_dly_ctr - 1;
   end
else
   rx_afc_cordic_dly_pulse <= 1'b0;


//---------------------------------------------------------------------------------
//  RX_AFC_CORDIC
//
//  MAGNITUDE:  - <= MAGNITUDE <= y
//  MAGNITUDE data format: sign + 1 bit integers . fractions ==> sign = rx_afc_cordic_polar_out_mag[31], integer = rx_afc_cordic_polar_out_mag[30], fractions = rx_afc_cordic_polar_out_mag[29:0]
//
//  CORDIC:     -1 <= PHASE <= 1
//  PHASE data format: sign + 2 bit integers . fractions ==> sign = rx_afc_cordic_polar_out_phs[31], integer = rx_afc_cordic_polar_out_phs[30:29], fractions = rx_afc_cordic_polar_out_phs[28:0]

wire unsigned [ 63: 0] rx_afc_cordic_cart_in = { ~rx_afc_fir_q_reg, rx_afc_fir_i_reg };

wire unsigned [ 63: 0] rx_afc_cordic_polar_out;
wire                   rx_afc_cordic_polar_out_vld;

rb_cordic_T_WS_O_SR_32T32_CR_B i_rb_rx_afc_cordic (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_rx_AFC_clken         ),  // power down when needed to
  .aresetn                 ( rb_pwr_rx_AFC_rst_n         ),

  .s_axis_cartesian_tdata  ( rx_afc_cordic_cart_in       ),
  .s_axis_cartesian_tvalid ( rx_afc_cordic_cart_vld      ),
  .s_axis_cartesian_tready ( rx_afc_cordic_cart_rdy      ),

  .m_axis_dout_tdata       ( rx_afc_cordic_polar_out     ),
  .m_axis_dout_tvalid      ( rx_afc_cordic_polar_out_vld )
);

wire unsigned [ 31: 0] rx_afc_cordic_polar_out_mag =   rx_afc_cordic_polar_out[31:0];
wire   signed [ 31: 0] rx_afc_cordic_polar_out_phs = { rx_afc_cordic_polar_out[63], rx_afc_cordic_polar_out[60:32], 2'b0 };  // -0.999 .. +0.999 represents -180° .. +180°


always @(posedge clk_adc_125mhz)
if (!rb_pwr_rx_AFC_rst_n) begin
   regs[REG_RD_RB_RX_AFC_CORDIC_PHS_PREV] <= 32'h00000000;
   regs[REG_RD_RB_RX_AFC_CORDIC_MAG]      <= 32'h00000000;
   regs[REG_RD_RB_RX_AFC_CORDIC_PHS]      <= 32'h00000000;
   end
else if (rx_afc_cordic_dly_pulse) begin
   regs[REG_RD_RB_RX_AFC_CORDIC_PHS_PREV] <= regs[REG_RD_RB_RX_AFC_CORDIC_PHS];
   regs[REG_RD_RB_RX_AFC_CORDIC_MAG]      <= rx_afc_cordic_polar_out_mag;
   regs[REG_RD_RB_RX_AFC_CORDIC_PHS]      <= rx_afc_cordic_polar_out_phs;
   end


//---------------------------------------------------------------------------------
//  RX_AFC_CALC_PHASE_SUB

wire   signed [ 47: 0] rx_afc_cordic_phs_in      = { rx_afc_cordic_phs,      16'b0 };
wire   signed [ 47: 0] rx_afc_cordic_phs_prev_in = { rx_afc_cordic_phs_prev, 16'b0 };

wire   signed [ 47: 0] rx_afc_calc_phs_wvr_diff;

rb_addsub_48M48 i_rb_rx_afc_calc_phase (
  // global signals
  .CLK                  ( clk_adc_125mhz            ),  // global 125 MHz clock
  .CE                   ( rb_pwr_rx_AFC_clken       ),  // power down when needed to

  .ADD                  ( 1'b0                      ),  // SUBTRACT
  .A                    ( rx_afc_cordic_phs_in      ),  // current CORDIC phase
  .B                    ( rx_afc_cordic_phs_prev_in ),  // previous CORDIC phase
  .S                    ( rx_afc_calc_phs_wvr_diff  )   // phase difference between 8kHz samples
);

wire   signed [ 47: 0] rx_afc_calc_phs_out;

rb_addsub_48M48 i_rb_rx_afc_calc_weaver (
  // global signals
  .CLK                  ( clk_adc_125mhz            ),  // global 125 MHz clock
  .CE                   ( rb_pwr_rx_AFC_clken       ),  // power down when needed to

  .ADD                  ( 1'b0                      ),  // SUBTRACT
  .A                    ( rx_afc_calc_phs_wvr_diff  ),  // phase difference with weaver osc. frequency offset included
  .B                    ( rx_afc_calc_weaver_inc    ),  // weaver frequency correction increment value for 8 kHz = 125µs time span
  .S                    ( rx_afc_calc_phs_out       )   // corrected phase value
);

always @(posedge clk_adc_125mhz)
if (!rb_pwr_rx_AFC_rst_n)
   regs[REG_RD_RB_RX_AFC_CORDIC_PHS_DIFF] <= 32'b0;
else
   regs[REG_RD_RB_RX_AFC_CORDIC_PHS_DIFF] <= rx_afc_calc_phs_out[47:16];


//---------------------------------------------------------------------------------
//  RX_AFC_CALC_DIV

wire    signed [ 31: 0] rx_afc_div_phs_in      = rx_afc_cordic_phs_diff;
wire                    rx_afc_div_phs_rdy;
reg                     rx_afc_div_phs_vld     = 1'b0;

always @(posedge clk_adc_125mhz)
if (!rb_pwr_rx_AFC_rst_n)
   rx_afc_div_phs_vld <= 1'b0;
else if (rx_afc_cordic_dly_pulse)
   rx_afc_div_phs_vld <= 1'b1;
else if (rx_afc_div_phs_vld && rx_afc_div_phs_rdy)
   rx_afc_div_phs_vld <= 1'b0;

wire    signed [ 15: 0] rx_afc_div_cnst_in     = { 3'b0, 13'd625 };  // AXIS word expansion
wire    signed [ 47: 0] rx_afc_div_afc_out;
wire                    rx_afc_div_afc_out_vld;

rb_div_32Div13R13 i_rb_rx_afc_calc_div (
  // global signals
  .aclk                   ( clk_adc_125mhz         ),  // global 125 MHz clock
  .aclken                 ( rb_pwr_rx_AFC_clken    ),  // power down when needed to
  .aresetn                ( rb_pwr_rx_AFC_rst_n    ),

  .s_axis_dividend_tdata  ( rx_afc_div_phs_in      ),
  .s_axis_dividend_tvalid ( rx_afc_div_phs_vld     ),
  .s_axis_dividend_tready ( rx_afc_div_phs_rdy     ),

  .s_axis_divisor_tdata   ( rx_afc_div_cnst_in     ),
  .s_axis_divisor_tvalid  ( 1'b1                   ),
  .s_axis_divisor_tready  (                        ),

  .m_axis_dout_tdata      ( rx_afc_div_afc_out     ),  // RX_AFC_CALC AFC output
  .m_axis_dout_tvalid     ( rx_afc_div_afc_out_vld )
);

reg   signed [ 47: 0] rx_afc_div_afc_quot = 'b0;

always @(posedge clk_adc_125mhz)
if (!rb_pwr_rx_AFC_rst_n)
   rx_afc_div_afc_quot <= 48'b0;
else if (rx_afc_div_afc_out_vld)
   rx_afc_div_afc_quot <= { {16{rx_afc_div_afc_out[47]}}, rx_afc_div_afc_out[47:16] };  // quotient's integer part, sign expanded to 48 bits

reg            rx_afc_div_afc_out_vld_last =  1'b0;
reg            rx_afc_div_afc_pulse        =  1'b0;

always @(posedge clk_adc_125mhz)
if (!rb_pwr_rx_AFC_rst_n) begin
  rx_afc_div_afc_out_vld_last <= 1'b0;
  rx_afc_div_afc_pulse <= 1'b0;
  end
else begin
  if (rx_afc_div_afc_out_vld && !rx_afc_div_afc_out_vld_last)
     rx_afc_div_afc_pulse <= 1'b1;
  else
     rx_afc_div_afc_pulse <= 1'b0;

  rx_afc_div_afc_out_vld_last <= rx_afc_div_afc_out_vld;
  end


//---------------------------------------------------------------------------------
//  RX_AFC_CALC_AFCREG

wire unsigned [ 47: 0] rx_afc_reg_inc_out;

rb_addsub_48M48 i_rb_rx_afc_calc_add_inc (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_pwr_rx_AFC_clken         ),  // power down when needed to

  .ADD                     ( 1'b1                        ),  // ADD
  .A                       ( rx_car_afc_inc              ),  // current AFC increment value
  .B                       ( rx_afc_div_afc_quot         ),  // AFC correction part to the current AFC increment value
  .S                       ( rx_afc_reg_inc_out          )   // next AFC increment value
);

reg                    rx_afc_addsub_afc_pulse = 1'b0;

always @(posedge clk_adc_125mhz)
if (!rb_pwr_rx_AFC_rst_n)
   rx_afc_addsub_afc_pulse <= 1'b0;
else
   rx_afc_addsub_afc_pulse <= rx_afc_div_afc_pulse;  // delayed by one clock

reg                    rx_afc_high_sig = 1'b0;

always @(posedge clk_adc_125mhz)
if (!rb_pwr_rx_AFC_rst_n) begin
   { regs[REG_RD_RB_RX_CAR_AFC_INC_HI][15:0], regs[REG_RD_RB_RX_CAR_AFC_INC_LO][31:0] } <= 48'b0;
   rx_afc_high_sig <= 1'b0;
   end
else if (rx_afc_addsub_afc_pulse)
   if (regs[REG_RD_RB_RX_AFC_CORDIC_MAG][30:28] != 3'b000) begin
      if (rx_afc_high_sig)                      // correct frequency only when phase monitoring is established
         { regs[REG_RD_RB_RX_CAR_AFC_INC_HI][15:0], regs[REG_RD_RB_RX_CAR_AFC_INC_LO][31:0] } <= rx_afc_reg_inc_out;
      rx_afc_high_sig <= 1'b1;
      end
   else begin                                   // low signal, reset to center frequency
      { regs[REG_RD_RB_RX_CAR_AFC_INC_HI][15:0], regs[REG_RD_RB_RX_CAR_AFC_INC_LO][31:0] } <= 48'b0;
      rx_afc_high_sig <= 1'b0;
      end


//---------------------------------------------------------------------------------
//  RX_AFC_CALC_SUMREG

wire unsigned [ 47: 0] rx_car_sum_out_inc;

rb_addsub_48M48 i_rb_rx_afc_sum_add_inc (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_pwr_rx_AFC_clken         ),  // power down when needed to

  .ADD                     ( 1'b1                        ),  // ADD
  .A                       ( rx_car_osc_inc              ),  // carrier frequency:      UNSIGNED 48 bit
  .B                       ( rx_car_afc_inc              ),  // increment value:        UNSIGNED 48 bit
  .S                       ( rx_car_sum_out_inc          )   // RX_CAR_SUM INC:         UNSIGNED 48 bit
);

always @(posedge clk_adc_125mhz)
if (!rb_pwr_rx_AFC_rst_n)
   { regs[REG_RD_RB_RX_CAR_SUM_INC_HI][15:0], regs[REG_RD_RB_RX_CAR_SUM_INC_LO][31:0] } <= 48'b0;
else
   { regs[REG_RD_RB_RX_CAR_SUM_INC_HI][15:0], regs[REG_RD_RB_RX_CAR_SUM_INC_LO][31:0] } <= rx_car_sum_out_inc;


//---------------------------------------------------------------------------------
//  RX_MOD FM ouput - strategy: rx_afc_cordic_phs_diff = delta-f = FM modulation

wire   signed [ 15: 0] rx_mod_fm_mix_in = rx_afc_cordic_phs_diff[28:13];
wire   signed [ 15: 0] rx_fm_gain_in = { 1'b0, rx_fm_gain };
wire   signed [ 31: 0] rx_mod_fm_mix_out;

rb_dsp48_AmB_A16_B16_P32 i_rb_rx_mod_fm_mixer (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_pwr_rx_AFC_clken         ),  // power down when needed to

  .A                       ( rx_mod_fm_mix_in            ),  // FM modulation signal    SIGNED 16 bit
  .B                       ( rx_fm_gain_in               ),  // FM mixer gain           SIGNED 16 bit

  .P                       ( rx_mod_fm_mix_out           )   // FM demodulated output   SIGSIG 32 bit
);

wire signed [ 15: 0] rx_mod_fm_out = rx_mod_fm_mix_out[30:15];


//---------------------------------------------------------------------------------
//  RX_MOD PM ouput - strategy: lossy integrator

reg    signed [ 47: 0] rx_mod_pm_accu         = 'b0;                                                     // integration: FM --> PM

wire   signed [ 47: 0] rx_mod_fm_in           = { {3{rx_mod_fm_out[15]}}, rx_mod_fm_out[15:0], 29'b0 };  // sign extension
wire   signed [ 47: 0] rx_mod_pm_s1_out;

wire   signed [ 47: 0] rx_mod_pm_accu_release = ~{ {14{rx_mod_pm_accu[47]}} , rx_mod_pm_accu[47:14] };   // sign extension and negation, balance to zero within 400 ms
wire   signed [ 47: 0] rx_mod_pm_s2_out;

wire   signed [ 15: 0] rx_pm_gain_in          = { 1'b0, rx_pm_gain[15:1] };

rb_dsp48_CONaC_CON48_C48_P48 i_rb_rx_mod_pm_accu_s1 (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_reset_n                  ),  // power down when needed to

  .CONCAT                  ( rx_mod_pm_accu              ),  // accumulator             SIGNED 48 bit
  .C                       ( rx_mod_fm_in                ),  // FM modulation signal    SIGNED 48 bit

  .P                       ( rx_mod_pm_s1_out            )   // step 1                  SIGNED 48 bit
);

rb_dsp48_CONaC_CON48_C48_P48 i_rb_rx_mod_pm_accu_s2 (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_reset_n                  ),  // power down when needed to

  .CONCAT                  ( rx_mod_pm_s1_out            ),  // accumulator             SIGNED 48 bit
  .C                       ( rx_mod_pm_accu_release      ),  // release                 SIGNED 48 bit

  .P                       ( rx_mod_pm_s2_out            )   // step 2                  SIGNED 48 bit
);

always @(posedge clk_adc_125mhz)
if (!rb_pwr_rx_AFC_rst_n)
   rx_mod_pm_accu <= 'b0;
else if (clk_200khz)
   rx_mod_pm_accu <= rx_mod_pm_s2_out;

wire   signed [ 31: 0] rx_mod_pm_mix_out;

rb_dsp48_AmB_A16_B16_P32 i_rb_rx_mod_pm_mixer (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_pwr_rx_AFC_clken         ),  // power down when needed to

  .A                       ( rx_mod_pm_accu[47:32]       ),  // PM modulation signal    SIGNED 16 bit
  .B                       ( rx_pm_gain_in               ),  // PM mixer gain           SIGNED 16 bit

  .P                       ( rx_mod_pm_mix_out           )   // PM demodulated output   SIGSIG 32 bit
);

wire   signed [ 15: 0] rx_mod_pm_out = rx_mod_pm_mix_out[30:15];


//---------------------------------------------------------------------------------
//  RX_MOD_AMENV envelope ouput - strategy: difference to lossy integrator

reg    signed [ 47: 0] rx_mod_amenv_accu         = 'b0;      // mean value of magnitue = mean value of AM envelope

wire   signed [ 47: 0] rx_mod_amenv_s1_sig_in    = { 4'b0, rx_afc_cordic_polar_out_mag[31:0], 12'b0 };   // magnitude is unsigned
wire   signed [ 47: 0] rx_mod_amenv_s1_out;

wire   signed [ 47: 0] rx_mod_amenv_accu_release = ~{ {2{rx_mod_amenv_accu[47]}} , rx_mod_amenv_accu[47:2] };  // sign extension, balance to zero within 10 ms
wire   signed [ 47: 0] rx_mod_amenv_s2_out;

wire   signed [ 47: 0] rx_mod_amenv_s3_mean_in   = ~rx_mod_amenv_s2_out;
wire   signed [ 47: 0] rx_mod_amenv_s3_sig_in    = { rx_afc_cordic_polar_out_mag[31:0], 16'b0 };
wire   signed [ 47: 0] rx_mod_amenv_diff_out;

rb_dsp48_CONaC_CON48_C48_P48 i_rb_rx_mod_amenv_accu_s1 (     // mean value - signal integrator
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_reset_n                  ),  // power down when needed to

  .CONCAT                  ( rx_mod_amenv_accu           ),  // accumulator             SIGNED 48 bit
  .C                       ( rx_mod_amenv_s1_sig_in      ),  // CORDIC mag signal       SIGNED 48 bit

  .P                       ( rx_mod_amenv_s1_out         )   // accu step 1             SIGNED 48 bit
);

rb_dsp48_CONaC_CON48_C48_P48 i_rb_rx_mod_amenv_accu_s2 (     // mean value - release unit
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_reset_n                  ),  // power down when needed to

  .CONCAT                  ( rx_mod_amenv_s1_out         ),  // accu step 1             SIGNED 48 bit
  .C                       ( rx_mod_amenv_accu_release   ),  // release                 SIGNED 48 bit

  .P                       ( rx_mod_amenv_s2_out         )   // accu step 2             SIGNED 48 bit
);

rb_dsp48_CONaC_CON48_C48_P48 i_rb_rx_mod_amenv_accu_mod (    // AM envelope demodulation
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_reset_n                  ),  // power down when needed to

  .CONCAT                  ( rx_mod_amenv_s3_mean_in     ),  // negative accu step      SIGNED 48 bit
  .C                       ( rx_mod_amenv_s3_sig_in      ),  // current magnitude       SIGNED 48 bit

  .P                       ( rx_mod_amenv_diff_out       )   // AM envelope             SIGNED 48 bit
);

always @(posedge clk_adc_125mhz)
if (!rb_pwr_rx_AFC_rst_n)
   rx_mod_amenv_accu <= 'b0;
else if (clk_200khz)
   rx_mod_amenv_accu <= rx_mod_amenv_s2_out;

wire   signed [ 15: 0] rx_mod_amenv_out = rx_mod_amenv_diff_out[31:16];


// === RX_AUDIO section ===

//---------------------------------------------------------------------------------
//  RX_AUDIO_OUT audio output mixer

wire   signed [ 15: 0] rx_audio_out_var_in = (rb_pwr_rx_modvar == RB_PWR_CTRL_RX_MOD_USB)         ||
                                             (rb_pwr_rx_modvar == RB_PWR_CTRL_RX_MOD_LSB)         ||
                                             (rb_pwr_rx_modvar == RB_PWR_CTRL_RX_MOD_AM_SYNC_USB) ||
                                             (rb_pwr_rx_modvar == RB_PWR_CTRL_RX_MOD_AM_SYNC_LSB) ?  rx_mod_ssb_am_out :
                                             (rb_pwr_rx_modvar == RB_PWR_CTRL_RX_MOD_FM)          ?  rx_mod_fm_out     :
                                             (rb_pwr_rx_modvar == RB_PWR_CTRL_RX_MOD_PM)          ?  rx_mod_pm_out     :
                                             (rb_pwr_rx_modvar == RB_PWR_CTRL_RX_MOD_AM_ENV)      ?  rx_mod_amenv_out  :
                                                                                                     16'b0             ;
wire   signed [ 31: 0] rx_audio_out_ofs_in = { rx_audio_out_ofs, 16'b0 };
wire   signed [ 31: 0] rx_audio_mix_out;

rb_dsp48_AmBaC_A16_B16_C32_P32 i_rb_rx_audio_out_add (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_clk_en                   ),  // power down when needed to

  .A                       ( rx_audio_out_var_in         ),  // selected demodulator:     SIGNED 16 bit
  .B                       ( rx_audio_out_gain           ),  // gain value:               SIGNED 16 bit
  .C                       ( rx_audio_out_ofs_in         ),  // offset value              SIGNED 32 bit

  .P                       ( rx_audio_mix_out            )   // RX_CAR_SUM INC:           SIGNED 32 bit
);

wire [ 15: 0] rx_audio_out = rx_audio_mix_out[29:14];


// === Connection Matrix section ===

//---------------------------------------------------------------------------------
//  LEDs Magnitude indicator

function bit [7:0] fct_mag (input bit [15:0] val);
   automatic bit [7:0] leds = 8'b0;             // exact zero indicator

   if (!val[15]) begin                          // positive value
      if (val[14: 0] == 15'b0)
         leds = 8'b00000000;
      else if (val[14: 9] == 6'b0)
         leds = 8'b00010000;
      else if (val[14:11] == 4'b0)
         leds = 8'b00110000;
      else if (val[14:13] == 2'b0)
         leds = 8'b01110000;
      else
         leds = 8'b11110000;
      end

   else begin                                   // negative value
      if      (val[14: 9] == 6'b111111)
         leds = 8'b00001000;
      else if (val[14:11] == 4'b1111)
         leds = 8'b00001100;
      else if (val[14:13] == 2'b11)
         leds = 8'b00001110;
      else
         leds = 8'b00001111;
      end

   fct_mag = leds;
endfunction: fct_mag

reg  [ 19: 0] led_ctr  = 20'b0;

always @(posedge clk_adc_125mhz)
if (!adc_rstn_i || !rb_reset_n) begin
   rb_leds_en    <=  1'b0;
   rb_leds_data  <=  8'b0;
   led_ctr       <= 20'b0;
   end

else if (led_src_con_pnt && rb_reset_n) begin
   rb_leds_en <=  1'b1;                         // LEDs magnitude indicator active
   case (led_src_con_pnt)

   RB_SRC_CON_PNT_NUM_OFF: begin
      rb_leds_data <=  8'b0;                    // turn all LEDs off
      end

   RB_SRC_CON_PNT_NUM_TX_MUXIN_MIX_IN: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(tx_muxin_mix_in[15:0]);  // updating about 120 Hz for reducing EMI
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_ADC_IN: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(tx_mod_adc_in[15:0]);
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_ADC_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(tx_mod_adc_out[30:15]);
      end

   RB_SRC_CON_PNT_NUM_TX_MOD_OSC_I_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(tx_mod_osc_cos[15:0]);
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_OSC_Q_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(tx_mod_osc_sin[15:0]);
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_I_S1_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(tx_mod_qmix_i_s1_out[30:15]);
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_Q_S1_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(tx_mod_qmix_q_s1_out[30:15]);
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_I_S2_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(tx_mod_qmix_i_s2_out[30:15]);
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_Q_S2_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(tx_mod_qmix_q_s2_out[30:15]);
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_I_S3_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(tx_mod_qmix_i_s3_out[47:32]);
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_Q_S3_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(tx_mod_qmix_q_s3_out[47:32]);
      end

   RB_SRC_CON_PNT_NUM_TX_MOD_CIC_I_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(tx_mod_cic_i_out[16:1]);
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_CIC_Q_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(tx_mod_cic_q_out[16:1]);
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_FIR_I_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(tx_mod_fir_i_out[32:17]);
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_FIR_Q_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(tx_mod_fir_q_out[32:17]);
      end
   RB_SRC_CON_PNT_NUM_TX_CAR_CIC_41M664_I_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(tx_car_regs_i_data);
      end
   RB_SRC_CON_PNT_NUM_TX_CAR_CIC_41M664_Q_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(tx_car_regs_q_data);
      end

   RB_SRC_CON_PNT_NUM_TX_CAR_OSC_I_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(tx_car_osc_cos[15:0]);
      end
   RB_SRC_CON_PNT_NUM_TX_CAR_OSC_Q_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(tx_car_osc_sin[15:0]);
      end
   RB_SRC_CON_PNT_NUM_TX_CAR_QMIX_I_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(tx_car_qmix_i_out[30:15]);
      end
   RB_SRC_CON_PNT_NUM_TX_CAR_QMIX_Q_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(tx_car_qmix_q_out[30:15]);
      end

   RB_SRC_CON_PNT_NUM_TX_RF_AMP_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(tx_amp_rf_out[31:16]);
      end

   RB_SRC_CON_PNT_NUM_RX_CAR_OSC_I_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(rx_car_osc_cos[15:0]);
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_OSC_Q_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(rx_car_osc_sin[15:0]);
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_QMIX_I_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(rx_car_qmix_i_out[30:15]);
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_QMIX_Q_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(rx_car_qmix_q_out[30:15]);
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_5M_I_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(rx_car_cic1_i_out[16:1]);
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_5M_Q_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(rx_car_cic1_q_out[16:1]);
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_200K_I_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(rx_car_regs2_i_data[16:1]);
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_200K_Q_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(rx_car_regs2_q_data[16:1]);
      end

   RB_SRC_CON_PNT_NUM_RX_MOD_8K_I_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(rx_mod_regs1_i_data[16:1]);
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_8K_Q_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(rx_mod_regs1_q_data[16:1]);
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_VB1_I_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(rx_mod_regs2_i_data[15:0]);
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_VB1_Q_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(rx_mod_regs2_q_data[15:0]);
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_OSC_I_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(rx_mod_osc_cos[15:0]);
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_OSC_Q_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(rx_mod_osc_sin[15:0]);
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_HLD_I_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(rx_mod_hld_i_data[15:0]);
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_HLD_Q_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(rx_mod_hld_q_data[15:0]);
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_QMIX_I_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(rx_mod_qmix_i_out[30:15]);
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_QMIX_Q_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(rx_mod_qmix_q_out[30:15]);
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_VB2_I_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(rx_mod_regs3_i_data[16:1]);
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_VB2_Q_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(rx_mod_regs3_q_data[16:1]);
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_48K_I_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(rx_mod_cic4_i_out[16:1]);
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_48K_Q_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(rx_mod_cic4_q_out[16:1]);
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_SSB_AM_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(rx_mod_ssb_am_out);
      end

   RB_SRC_CON_PNT_NUM_RX_AFC_FIR_I_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(rx_afc_fir_i_out[31:16]);
      end
   RB_SRC_CON_PNT_NUM_RX_AFC_FIR_Q_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(rx_afc_fir_q_out[31:16]);
      end

   RB_SRC_CON_PNT_NUM_RX_AFC_CORDIC_MAG: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(regs[REG_RD_RB_RX_AFC_CORDIC_MAG][30:15] - 16'h8000);
      end
   RB_SRC_CON_PNT_NUM_RX_AFC_CORDIC_PHS: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(regs[REG_RD_RB_RX_AFC_CORDIC_PHS][31:16]);
      end
   RB_SRC_CON_PNT_NUM_RX_AFC_CORDIC_PHS_PREV: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(regs[REG_RD_RB_RX_AFC_CORDIC_PHS_PREV][31:16]);
      end
   RB_SRC_CON_PNT_NUM_RX_AFC_CORDIC_PHS_DIFF: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(regs[REG_RD_RB_RX_AFC_CORDIC_PHS_DIFF][31:16]);
      end

   RB_SRC_CON_PNT_NUM_RX_AFC_INC_REG: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag({ regs[REG_RD_RB_RX_CAR_AFC_INC_HI][2:0], regs[REG_RD_RB_RX_CAR_AFC_INC_LO][31:19] });
      end

   RB_SRC_CON_PNT_NUM_RX_SUM_INC_REG: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag({ regs[REG_RD_RB_RX_CAR_SUM_INC_HI][2:0], regs[REG_RD_RB_RX_CAR_SUM_INC_LO][31:19] });
      end

   RB_SRC_CON_PNT_NUM_RX_MOD_FM_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(rx_mod_fm_out);
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_PM_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(rx_mod_pm_out);
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_AMENV_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(rx_mod_amenv_out);
      end

   RB_SRC_CON_PNT_NUM_RX_AUDIO_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(rx_audio_out);
      end

   RB_SRC_CON_PNT_NUM_TEST_VECTOR_OUT: begin
      if (!led_ctr)
         //                LED7                    LED6                    LED5                    LED4                    LED3                    LED2                    LED1                    LED0
         rb_leds_data <= { tx_car_osc_ofs_mux,     tx_car_osc_inc_mux,     rb_pwr_rx_AFC_rst_n,    rb_pwr_rx_MOD_rst_n,    rb_pwr_rx_CAR_rst_n,    rb_pwr_tx_Q_rst_n,      rb_pwr_tx_I_rst_n,      rb_pwr_tx_OSC_rst_n  };
      end

   default: begin
      rb_leds_data <= led_ctr[19:12];
      end

   endcase
   led_ctr <= led_ctr + 1;
   end

else begin                                      // RB_SRC_CON_PNT_NUM_DISABLED
   rb_leds_en   <=  1'b0;
   rb_leds_data <=  8'b0;
   led_ctr      <= 20'b0;
   end


//---------------------------------------------------------------------------------
//  RB RFOUT1 signal assignment

always @(posedge clk_adc_125mhz)
if (!adc_rstn_i || !rb_reset_n)
   rb_out_ch[0] <= 16'b0;

else if (rfout1_src_con_pnt && rb_reset_n)
   case (rfout1_src_con_pnt)

   RB_SRC_CON_PNT_NUM_TX_MUXIN_MIX_IN: begin
      rb_out_ch[0] <= tx_muxin_mix_in[15:0];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_ADC_IN: begin
      rb_out_ch[0] <= tx_mod_adc_in[15:0];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_ADC_OUT: begin
      rb_out_ch[0] <= tx_mod_adc_out[30:15];
      end

   RB_SRC_CON_PNT_NUM_TX_MOD_OSC_I_OUT: begin
      rb_out_ch[0] <= tx_mod_osc_cos[15:0];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_OSC_Q_OUT: begin
      rb_out_ch[0] <= tx_mod_osc_sin[15:0];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_I_S1_OUT: begin
      rb_out_ch[0] <= tx_mod_qmix_i_s1_out[30:15];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_Q_S1_OUT: begin
      rb_out_ch[0] <= tx_mod_qmix_q_s1_out[30:15];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_I_S2_OUT: begin
      rb_out_ch[0] <= tx_mod_qmix_i_s2_out[30:15];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_Q_S2_OUT: begin
      rb_out_ch[0] <= tx_mod_qmix_q_s2_out[30:15];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_I_S3_OUT: begin
      rb_out_ch[0] <= tx_mod_qmix_i_s3_out[47:32];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_Q_S3_OUT: begin
      rb_out_ch[0] <= tx_mod_qmix_q_s3_out[47:32];
      end

   RB_SRC_CON_PNT_NUM_TX_MOD_CIC_I_OUT: begin
      if (tx_mod_cic_i_out_vld)
         rb_out_ch[0] <= tx_mod_cic_i_out[16:1];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_CIC_Q_OUT: begin
      if (tx_mod_cic_q_out_vld)
         rb_out_ch[0] <= tx_mod_cic_q_out[16:1];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_FIR_I_OUT: begin
      if (tx_mod_fir_i_out_vld)
         rb_out_ch[0] <= tx_mod_fir_i_out[32:17];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_FIR_Q_OUT: begin
      if (tx_mod_fir_q_out_vld)
         rb_out_ch[0] <= tx_mod_fir_q_out[32:17];
      end
   RB_SRC_CON_PNT_NUM_TX_CAR_CIC_41M664_I_OUT: begin
      rb_out_ch[0] <= tx_car_regs_i_data;
      end
   RB_SRC_CON_PNT_NUM_TX_CAR_CIC_41M664_Q_OUT: begin
      rb_out_ch[0] <= tx_car_regs_q_data;
      end

   RB_SRC_CON_PNT_NUM_TX_CAR_OSC_I_OUT: begin
      rb_out_ch[0] <= tx_car_osc_cos[15:0];
      end
   RB_SRC_CON_PNT_NUM_TX_CAR_OSC_Q_OUT: begin
      rb_out_ch[0] <= tx_car_osc_sin[15:0];
      end
   RB_SRC_CON_PNT_NUM_TX_CAR_QMIX_I_OUT: begin
      rb_out_ch[0] <= tx_car_qmix_i_out[30:15];
      end
   RB_SRC_CON_PNT_NUM_TX_CAR_QMIX_Q_OUT: begin
      rb_out_ch[0] <= tx_car_qmix_q_out[30:15];
      end

   RB_SRC_CON_PNT_NUM_TX_RF_AMP_OUT: begin
      rb_out_ch[0] <= tx_amp_rf_out[31:16];
      end

   RB_SRC_CON_PNT_NUM_RX_CAR_OSC_I_OUT: begin
      rb_out_ch[0] <= rx_car_osc_cos[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_OSC_Q_OUT: begin
      rb_out_ch[0] <= rx_car_osc_sin[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_QMIX_I_OUT: begin
      rb_out_ch[0] <= rx_car_qmix_i_out[30:15];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_QMIX_Q_OUT: begin
      rb_out_ch[0] <= rx_car_qmix_q_out[30:15];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_5M_I_OUT: begin
      if (rx_car_cic1_i_out_vld)
         rb_out_ch[0] <= rx_car_cic1_i_out[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_5M_Q_OUT: begin
      if (rx_car_cic1_q_out_vld)
         rb_out_ch[0] <= rx_car_cic1_q_out[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_200K_I_OUT: begin
      rb_out_ch[0] <= rx_car_regs2_i_data[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_200K_Q_OUT: begin
      rb_out_ch[0] <= rx_car_regs2_q_data[16:1];
      end

   RB_SRC_CON_PNT_NUM_RX_MOD_8K_I_OUT: begin
      rb_out_ch[0] <= rx_mod_regs1_i_data[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_8K_Q_OUT: begin
      rb_out_ch[0] <= rx_mod_regs1_q_data[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_VB1_I_OUT: begin
      rb_out_ch[0] <= rx_mod_regs2_i_data[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_VB1_Q_OUT: begin
      rb_out_ch[0] <= rx_mod_regs2_q_data[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_OSC_I_OUT: begin
      rb_out_ch[0] <= rx_mod_osc_cos[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_OSC_Q_OUT: begin
      rb_out_ch[0] <= rx_mod_osc_sin[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_HLD_I_OUT: begin
      rb_out_ch[0] <= rx_mod_hld_i_data[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_HLD_Q_OUT: begin
      rb_out_ch[0] <= rx_mod_hld_q_data[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_QMIX_I_OUT: begin
      rb_out_ch[0] <= rx_mod_qmix_i_out[30:15];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_QMIX_Q_OUT: begin
      rb_out_ch[0] <= rx_mod_qmix_q_out[30:15];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_VB2_I_OUT: begin
      rb_out_ch[0] <= rx_mod_regs3_i_data[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_VB2_Q_OUT: begin
      rb_out_ch[0] <= rx_mod_regs3_q_data[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_48K_I_OUT: begin
      if (rx_mod_cic4_i_out_vld)
         rb_out_ch[0] <= rx_mod_cic4_i_out[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_48K_Q_OUT: begin
      if (rx_mod_cic4_q_out_vld)
         rb_out_ch[0] <= rx_mod_cic4_q_out[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_SSB_AM_OUT: begin
      rb_out_ch[0] <= rx_mod_ssb_am_out;
      end

   RB_SRC_CON_PNT_NUM_RX_AFC_FIR_I_OUT: begin
      if (rx_afc_fir_i_out_vld)
         rb_out_ch[0] <= rx_afc_fir_i_out[31:16];
      end
   RB_SRC_CON_PNT_NUM_RX_AFC_FIR_Q_OUT: begin
      if (rx_afc_fir_q_out_vld)
         rb_out_ch[0] <= rx_afc_fir_q_out[31:16];
      end

   RB_SRC_CON_PNT_NUM_RX_AFC_CORDIC_MAG: begin
      rb_out_ch[0] <= (regs[REG_RD_RB_RX_AFC_CORDIC_MAG][30:15] - 16'h8000);
      end
   RB_SRC_CON_PNT_NUM_RX_AFC_CORDIC_PHS: begin
      rb_out_ch[0] <= regs[REG_RD_RB_RX_AFC_CORDIC_PHS][31:16];
      end
   RB_SRC_CON_PNT_NUM_RX_AFC_CORDIC_PHS_PREV: begin
      rb_out_ch[0] <= regs[REG_RD_RB_RX_AFC_CORDIC_PHS_PREV][31:16];
      end
   RB_SRC_CON_PNT_NUM_RX_AFC_CORDIC_PHS_DIFF: begin
      rb_out_ch[0] <= regs[REG_RD_RB_RX_AFC_CORDIC_PHS_DIFF][31:16];
      end

   RB_SRC_CON_PNT_NUM_RX_AFC_INC_REG: begin
      rb_out_ch[0] <= { regs[REG_RD_RB_RX_CAR_AFC_INC_HI][2:0], regs[REG_RD_RB_RX_CAR_AFC_INC_LO][31:19] };
      end

   RB_SRC_CON_PNT_NUM_RX_SUM_INC_REG: begin
      rb_out_ch[0] <= { regs[REG_RD_RB_RX_CAR_SUM_INC_HI][2:0], regs[REG_RD_RB_RX_CAR_SUM_INC_LO][31:19] };
      end

   RB_SRC_CON_PNT_NUM_RX_MOD_FM_OUT: begin
      rb_out_ch[0] <= rx_mod_fm_out;
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_PM_OUT: begin
      rb_out_ch[0] <= rx_mod_pm_out;
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_AMENV_OUT: begin
      rb_out_ch[0] <= rx_mod_amenv_out;
      end

   RB_SRC_CON_PNT_NUM_RX_AUDIO_OUT: begin
      rb_out_ch[0] <= rx_audio_out;
      end

   RB_SRC_CON_PNT_NUM_TEST_VECTOR_OUT: begin
      // rb_out_ch[0] <= { 1'b0, clk_200khz, 14'b0 };
      rb_out_ch[0] <= rx_mod_amenv_s1_out[47:32];
      end

   default: begin
      rb_out_ch[0] <= led_ctr[19:4];            // error-ramp
      end

   endcase

else                                            // RB_SRC_CON_PNT_NUM_DISABLED
   rb_out_ch[0]     <= 16'b0;                   // silence


//---------------------------------------------------------------------------------
//  RB RFOUT2 signal assignment

always @(posedge clk_adc_125mhz)
if (!adc_rstn_i || !rb_reset_n) begin
   rb_out_ch[1] <= 16'b0;
   end

else if (rfout2_src_con_pnt && rb_reset_n)
   case (rfout2_src_con_pnt)

   RB_SRC_CON_PNT_NUM_TX_MUXIN_MIX_IN: begin
      rb_out_ch[1] <= tx_muxin_mix_in[15:0];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_ADC_IN: begin
      rb_out_ch[1] <= tx_mod_adc_in[15:0];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_ADC_OUT: begin
      rb_out_ch[1] <= tx_mod_adc_out[30:15];
      end

   RB_SRC_CON_PNT_NUM_TX_MOD_OSC_I_OUT: begin
      rb_out_ch[1] <= tx_mod_osc_cos[15:0];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_OSC_Q_OUT: begin
      rb_out_ch[1] <= tx_mod_osc_sin[15:0];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_I_S1_OUT: begin
      rb_out_ch[1] <= tx_mod_qmix_i_s1_out[30:15];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_Q_S1_OUT: begin
      rb_out_ch[1] <= tx_mod_qmix_q_s1_out[30:15];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_I_S2_OUT: begin
      rb_out_ch[1] <= tx_mod_qmix_i_s2_out[30:15];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_Q_S2_OUT: begin
      rb_out_ch[1] <= tx_mod_qmix_q_s2_out[30:15];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_I_S3_OUT: begin
      rb_out_ch[1] <= tx_mod_qmix_i_s3_out[47:32];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_Q_S3_OUT: begin
      rb_out_ch[1] <= tx_mod_qmix_q_s3_out[47:32];
      end

   RB_SRC_CON_PNT_NUM_TX_MOD_CIC_I_OUT: begin
      if (tx_mod_cic_i_out_vld)
         rb_out_ch[1] <= tx_mod_cic_i_out[16:1];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_CIC_Q_OUT: begin
      if (tx_mod_cic_q_out_vld)
         rb_out_ch[1] <= tx_mod_cic_q_out[16:1];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_FIR_I_OUT: begin
      if (tx_mod_fir_i_out_vld)
         rb_out_ch[1] <= tx_mod_fir_i_out[32:17];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_FIR_Q_OUT: begin
      if (tx_mod_fir_q_out_vld)
         rb_out_ch[1] <= tx_mod_fir_q_out[32:17];
      end
   RB_SRC_CON_PNT_NUM_TX_CAR_CIC_41M664_I_OUT: begin
      rb_out_ch[1] <= tx_car_regs_i_data;
      end
   RB_SRC_CON_PNT_NUM_TX_CAR_CIC_41M664_Q_OUT: begin
      rb_out_ch[1] <= tx_car_regs_q_data;
      end

   RB_SRC_CON_PNT_NUM_TX_CAR_OSC_I_OUT: begin
      rb_out_ch[1] <= tx_car_osc_cos[15:0];
      end
   RB_SRC_CON_PNT_NUM_TX_CAR_OSC_Q_OUT: begin
      rb_out_ch[1] <= tx_car_osc_sin[15:0];
      end
   RB_SRC_CON_PNT_NUM_TX_CAR_QMIX_I_OUT: begin
      rb_out_ch[1] <= tx_car_qmix_i_out[30:15];
      end
   RB_SRC_CON_PNT_NUM_TX_CAR_QMIX_Q_OUT: begin
      rb_out_ch[1] <= tx_car_qmix_q_out[30:15];
      end

   RB_SRC_CON_PNT_NUM_TX_RF_AMP_OUT: begin
      rb_out_ch[1] <= tx_amp_rf_out[31:16];
      end

   RB_SRC_CON_PNT_NUM_RX_CAR_OSC_I_OUT: begin
      rb_out_ch[1] <= rx_car_osc_cos[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_OSC_Q_OUT: begin
      rb_out_ch[1] <= rx_car_osc_sin[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_QMIX_I_OUT: begin
      rb_out_ch[1] <= rx_car_qmix_i_out[30:15];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_QMIX_Q_OUT: begin
      rb_out_ch[1] <= rx_car_qmix_q_out[30:15];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_5M_I_OUT: begin
      if (rx_car_cic1_i_out_vld)
         rb_out_ch[1] <= rx_car_cic1_i_out[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_5M_Q_OUT: begin
      if (rx_car_cic1_q_out_vld)
         rb_out_ch[1] <= rx_car_cic1_q_out[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_200K_I_OUT: begin
      rb_out_ch[1] <= rx_car_regs2_i_data[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_200K_Q_OUT: begin
      rb_out_ch[1] <= rx_car_regs2_q_data[16:1];
      end

   RB_SRC_CON_PNT_NUM_RX_MOD_8K_I_OUT: begin
      rb_out_ch[1] <= rx_mod_regs1_i_data[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_8K_Q_OUT: begin
      rb_out_ch[1] <= rx_mod_regs1_q_data[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_VB1_I_OUT: begin
      rb_out_ch[1] <= rx_mod_regs2_i_data[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_VB1_Q_OUT: begin
      rb_out_ch[1] <= rx_mod_regs2_q_data[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_OSC_I_OUT: begin
      rb_out_ch[1] <= rx_mod_osc_cos[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_OSC_Q_OUT: begin
      rb_out_ch[1] <= rx_mod_osc_sin[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_HLD_I_OUT: begin
      rb_out_ch[1] <= rx_mod_hld_i_data[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_HLD_Q_OUT: begin
      rb_out_ch[1] <= rx_mod_hld_q_data[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_QMIX_I_OUT: begin
      rb_out_ch[1] <= rx_mod_qmix_i_out[30:15];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_QMIX_Q_OUT: begin
      rb_out_ch[1] <= rx_mod_qmix_q_out[30:15];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_VB2_I_OUT: begin
      rb_out_ch[1] <= rx_mod_regs3_i_data[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_VB2_Q_OUT: begin
      rb_out_ch[1] <= rx_mod_regs3_q_data[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_48K_I_OUT: begin
      if (rx_mod_cic4_i_out_vld)
         rb_out_ch[1] <= rx_mod_cic4_i_out[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_48K_Q_OUT: begin
      if (rx_mod_cic4_q_out_vld)
         rb_out_ch[1] <= rx_mod_cic4_q_out[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_SSB_AM_OUT: begin
      rb_out_ch[1] <= rx_mod_ssb_am_out;
      end

   RB_SRC_CON_PNT_NUM_RX_AFC_FIR_I_OUT: begin
      if (rx_afc_fir_i_out_vld)
         rb_out_ch[1] <= rx_afc_fir_i_out[31:16];
      end
   RB_SRC_CON_PNT_NUM_RX_AFC_FIR_Q_OUT: begin
      if (rx_afc_fir_q_out_vld)
         rb_out_ch[1] <= rx_afc_fir_q_out[31:16];
      end

   RB_SRC_CON_PNT_NUM_RX_AFC_CORDIC_MAG: begin
      rb_out_ch[1] <= (regs[REG_RD_RB_RX_AFC_CORDIC_MAG][30:15] - 16'h8000);
      end
   RB_SRC_CON_PNT_NUM_RX_AFC_CORDIC_PHS: begin
      rb_out_ch[1] <= regs[REG_RD_RB_RX_AFC_CORDIC_PHS][31:16];
      end
   RB_SRC_CON_PNT_NUM_RX_AFC_CORDIC_PHS_PREV: begin
      rb_out_ch[1] <= regs[REG_RD_RB_RX_AFC_CORDIC_PHS_PREV][31:16];
      end
   RB_SRC_CON_PNT_NUM_RX_AFC_CORDIC_PHS_DIFF: begin
      rb_out_ch[1] <= regs[REG_RD_RB_RX_AFC_CORDIC_PHS_DIFF][31:16];
      end

   RB_SRC_CON_PNT_NUM_RX_AFC_INC_REG: begin
      rb_out_ch[1] <= { regs[REG_RD_RB_RX_CAR_AFC_INC_HI][2:0], regs[REG_RD_RB_RX_CAR_AFC_INC_LO][31:19] };
      end

   RB_SRC_CON_PNT_NUM_RX_SUM_INC_REG: begin
      rb_out_ch[1] <= { regs[REG_RD_RB_RX_CAR_SUM_INC_HI][2:0], regs[REG_RD_RB_RX_CAR_SUM_INC_LO][31:19] };
      end

   RB_SRC_CON_PNT_NUM_RX_MOD_FM_OUT: begin
      rb_out_ch[1] <= rx_mod_fm_out;
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_PM_OUT: begin
      rb_out_ch[1] <= rx_mod_pm_out;
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_AMENV_OUT: begin
      rb_out_ch[1] <= rx_mod_amenv_out;
      end

   RB_SRC_CON_PNT_NUM_RX_AUDIO_OUT: begin
      rb_out_ch[1] <= rx_audio_out;
      end

   RB_SRC_CON_PNT_NUM_TEST_VECTOR_OUT: begin
      // rb_out_ch[1] <= { 1'b0, rx_mod_cic4_chk_rst_cnt, 12'b0 };
      // rb_out_ch[1] <= { 1'b0, rx_mod_cic1_chk_rst, 14'b0 };
      rb_out_ch[1] <= rx_mod_amenv_s2_out[47:32];
      end

   default: begin
      rb_out_ch[1] <= led_ctr[19:4];            // error-ramp
      end

   endcase

else                                            // RB_SRC_CON_PNT_NUM_DISABLED
   rb_out_ch[1] <= 16'b0;                       // silence


// === Bus handling ===

//---------------------------------------------------------------------------------
//  Status register

always @(posedge clk_adc_125mhz)
if (!adc_rstn_i)
   regs[REG_RD_RB_STATUS] <= 32'b0;
else begin
   regs[REG_RD_RB_STATUS][RB_STAT_CLK_EN]                    <= rb_clk_en;
   regs[REG_RD_RB_STATUS][RB_STAT_RESET]                     <= rb_reset_n;
   regs[REG_RD_RB_STATUS][RB_STAT_LEDS_EN]                   <= rb_leds_en;

   regs[REG_RD_RB_STATUS][RB_STAT_TX_CAR_OSC_ZERO]           <= !tx_car_osc_sin;  // when phase is 0 deg
   regs[REG_RD_RB_STATUS][RB_STAT_TX_CAR_OSC_VALID]          <=  tx_car_osc_axis_m_vld;

   regs[REG_RD_RB_STATUS][RB_STAT_TX_MOD_OSC_ZERO]           <= !tx_mod_osc_sin;  // when phase is 0 deg
   regs[REG_RD_RB_STATUS][RB_STAT_TX_MOD_OSC_VALID]          <=  tx_mod_osc_axis_m_vld;

   regs[REG_RD_RB_STATUS][RB_STAT_RX_CAR_OSC_ZERO]           <= !rx_car_osc_sin;  // when phase is 0 deg
   regs[REG_RD_RB_STATUS][RB_STAT_RX_CAR_OSC_VALID]          <=  rx_car_osc_axis_m_vld;

   regs[REG_RD_RB_STATUS][RB_STAT_RX_MOD_OSC_ZERO]           <= !rx_mod_osc_sin;  // when phase is 0 deg
   regs[REG_RD_RB_STATUS][RB_STAT_RX_MOD_OSC_VALID]          <=  rx_mod_osc_axis_m_vld;

   regs[REG_RD_RB_STATUS][RB_STAT_RX_AFC_HIGH_SIG]           <=  rx_afc_high_sig;

   regs[REG_RD_RB_STATUS][RB_STAT_LED7_ON : RB_STAT_LED0_ON] <= rb_leds_data;
   end


//---------------------------------------------------------------------------------
//  System bus connection

// write access to the registers
always @(posedge clk_adc_125mhz)
if (!adc_rstn_i) begin
   regs[REG_RW_RB_CTRL]                      <= 32'h00000000;
   regs[REG_RW_RB_ICR]                       <= 32'h00000000;
   regs[REG_RD_RB_ISR]                       <= 32'h00000000;
   regs[REG_RW_RB_DMA_CTRL]                  <= 32'h00000000;
   regs[REG_RW_RB_PWR_CTRL]                  <= 32'h00000000;
   regs[REG_RW_RB_RFOUTx_LED_SRC_CON_PNT]    <= 32'h00000000;
   regs[REG_RW_RB_TX_CAR_OSC_INC_LO]         <= 32'h00000000;
   regs[REG_RW_RB_TX_CAR_OSC_INC_HI]         <= 32'h00000000;
   regs[REG_RW_RB_TX_CAR_OSC_OFS_LO]         <= 32'h00000000;
   regs[REG_RW_RB_TX_CAR_OSC_OFS_HI]         <= 32'h00000000;
   regs[REG_RW_RB_TX_RF_AMP_GAIN]            <= 32'h00000000;
   regs[REG_RW_RB_TX_RF_AMP_OFS]             <= 32'h00000000;
   regs[REG_RW_RB_TX_MOD_OSC_INC_LO]         <= 32'h00000000;
   regs[REG_RW_RB_TX_MOD_OSC_INC_HI]         <= 32'h00000000;
   regs[REG_RW_RB_TX_MOD_OSC_OFS_LO]         <= 32'h00000000;
   regs[REG_RW_RB_TX_MOD_OSC_OFS_HI]         <= 32'h00000000;
   regs[REG_RW_RB_TX_MOD_QMIX_GAIN]          <= 32'h00000000;
   regs[REG_RW_RB_TX_MOD_QMIX_OFS_LO]        <= 32'h00000000;
   regs[REG_RW_RB_TX_MOD_QMIX_OFS_HI]        <= 32'h00000000;
   regs[REG_RW_RB_TX_MUXIN_SRC]              <= 32'h00000000;
   regs[REG_RW_RB_TX_MUXIN_GAIN]             <= 32'h00000000;
   regs[REG_RW_RB_RX_CAR_CALC_WEAVER_INC_LO] <= 32'h00000000;
   regs[REG_RW_RB_RX_CAR_CALC_WEAVER_INC_HI] <= 32'h00000000;
   regs[REG_RW_RB_RX_CAR_OSC_INC_LO]         <= 32'h00000000;
   regs[REG_RW_RB_RX_CAR_OSC_INC_HI]         <= 32'h00000000;
   regs[REG_RW_RB_RX_CAR_OSC_OFS_LO]         <= 32'h00000000;
   regs[REG_RW_RB_RX_CAR_OSC_OFS_HI]         <= 32'h00000000;
   regs[REG_RW_RB_RX_MOD_OSC_INC_LO]         <= 32'h00000000;
   regs[REG_RW_RB_RX_MOD_OSC_INC_HI]         <= 32'h00000000;
   regs[REG_RW_RB_RX_MOD_OSC_OFS_LO]         <= 32'h00000000;
   regs[REG_RW_RB_RX_MOD_OSC_OFS_HI]         <= 32'h00000000;
   regs[REG_RW_RB_RX_MUXIN_SRC]              <= 32'h00000000;
   regs[REG_RW_RB_RX_MUXIN_GAIN]             <= 32'h00000000;
   regs[REG_RW_RB_RX_SSB_AM_GAIN]            <= 32'h00000000;
   regs[REG_RW_RB_RX_FM_GAIN]                <= 32'h00000000;
   regs[REG_RW_RB_RX_PM_GAIN]                <= 32'h00000000;
   regs[REG_RW_RB_RX_AUDIO_OUT_GAIN]         <= 32'h00000000;
   regs[REG_RW_RB_RX_AUDIO_OUT_OFS]          <= 32'h00000000;
   end

else begin
   if (sys_wen) begin
      casez (sys_addr[19:0])

      /* control */
      20'h00000: begin
         regs[REG_RW_RB_CTRL]                     <= sys_wdata[31:0];
         end
      20'h00008: begin
         regs[REG_RW_RB_ICR]                      <= sys_wdata[31:0];
         end
      20'h00010: begin
         regs[REG_RW_RB_DMA_CTRL]                 <= sys_wdata[31:0];
         end
      20'h00018: begin
         regs[REG_RW_RB_PWR_CTRL]                 <= { 16'b0, sys_wdata[15:0] };
         end
      20'h0001C: begin
         regs[REG_RW_RB_RFOUTx_LED_SRC_CON_PNT]   <= sys_wdata[31:0] & 32'hFFFF00FF;
         end

      /* TX_CAR_OSC */
      20'h00020: begin
         regs[REG_RW_RB_TX_CAR_OSC_INC_LO]        <= sys_wdata[31:0];
         end
      20'h00024: begin
         regs[REG_RW_RB_TX_CAR_OSC_INC_HI]        <= { 16'b0, sys_wdata[15:0] };
         end
      20'h00028: begin
         regs[REG_RW_RB_TX_CAR_OSC_OFS_LO]        <= sys_wdata[31:0];
         end
      20'h0002C: begin
         regs[REG_RW_RB_TX_CAR_OSC_OFS_HI]        <= { 16'b0, sys_wdata[15:0] };
         end
      20'h00030: begin
         regs[REG_RW_RB_TX_RF_AMP_GAIN]           <= sys_wdata[15:0];
         end
      20'h00038: begin
         regs[REG_RW_RB_TX_RF_AMP_OFS]            <= sys_wdata[15:0];
         end

      /* TX_MOD_OSC */
      20'h00040: begin
         regs[REG_RW_RB_TX_MOD_OSC_INC_LO]        <= sys_wdata[31:0];
         end
      20'h00044: begin
         regs[REG_RW_RB_TX_MOD_OSC_INC_HI]        <= { 16'b0, sys_wdata[15:0] };
         end
      20'h00048: begin
         regs[REG_RW_RB_TX_MOD_OSC_OFS_LO]        <= sys_wdata[31:0];
         end
      20'h0004C: begin
         regs[REG_RW_RB_TX_MOD_OSC_OFS_HI]        <= { 16'b0, sys_wdata[15:0] };
         end
      20'h00050: begin
         regs[REG_RW_RB_TX_MOD_QMIX_GAIN]         <= sys_wdata[31:0];
         end
      20'h00058: begin
         regs[REG_RW_RB_TX_MOD_QMIX_OFS_LO]       <= sys_wdata[31:0];
         end
      20'h0005C: begin
         regs[REG_RW_RB_TX_MOD_QMIX_OFS_HI]       <= { 16'b0, sys_wdata[15:0] };
         end

      /* Input TX_MUXIN */
      20'h00060: begin
         regs[REG_RW_RB_TX_MUXIN_SRC]             <= { regs[REG_RW_RB_TX_MUXIN_SRC][31:6], sys_wdata[5:0] };
         end
      20'h00064: begin
         regs[REG_RW_RB_TX_MUXIN_GAIN]            <= { 13'b0, sys_wdata[18:0] };
         end

      /* RX_CAR_CALC_WEAVER */
      20'h00100: begin
         regs[REG_RW_RB_RX_CAR_CALC_WEAVER_INC_LO] <= sys_wdata[31:0];
         end
      20'h00104: begin
         regs[REG_RW_RB_RX_CAR_CALC_WEAVER_INC_HI] <= { 16'b0, sys_wdata[15:0] };
         end

      /* RX_CAR_OSC */
      20'h00120: begin
         regs[REG_RW_RB_RX_CAR_OSC_INC_LO]        <= sys_wdata[31:0];
         end
      20'h00124: begin
         regs[REG_RW_RB_RX_CAR_OSC_INC_HI]        <= { 16'b0, sys_wdata[15:0] };
         end
      20'h00128: begin
         regs[REG_RW_RB_RX_CAR_OSC_OFS_LO]        <= sys_wdata[31:0];
         end
      20'h0012C: begin
         regs[REG_RW_RB_RX_CAR_OSC_OFS_HI]        <= { 16'b0, sys_wdata[15:0] };
         end

      /* RX_MOD_OSC */
      20'h00140: begin
         regs[REG_RW_RB_RX_MOD_OSC_INC_LO]        <= sys_wdata[31:0];
         end
      20'h00144: begin
         regs[REG_RW_RB_RX_MOD_OSC_INC_HI]        <= { 16'b0, sys_wdata[15:0] };
         end
      20'h00148: begin
         regs[REG_RW_RB_RX_MOD_OSC_OFS_LO]        <= sys_wdata[31:0];
         end
      20'h0014C: begin
         regs[REG_RW_RB_RX_MOD_OSC_OFS_HI]        <= { 16'b0, sys_wdata[15:0] };
         end

      /* RX_MUX */
      20'h00160: begin
         regs[REG_RW_RB_RX_MUXIN_SRC]             <= sys_wdata[31:0];
         end
      20'h00164: begin
         regs[REG_RW_RB_RX_MUXIN_GAIN]            <= { 13'b0, sys_wdata[18:0] };
         end

      /* RX_DEMOD_GAIN */
      20'h00180: begin
        regs[REG_RW_RB_RX_SSB_AM_GAIN]            <= { 16'b0, sys_wdata[15:0] };
        end
      20'h00184: begin
        regs[REG_RW_RB_RX_FM_GAIN]                <= { 16'b0, sys_wdata[15:0] };
        end
      20'h00188: begin
        regs[REG_RW_RB_RX_PM_GAIN]                <= { 16'b0, sys_wdata[15:0] };
        end

      /* RX_AUDIO_OUT */
      20'h00190: begin
        regs[REG_RW_RB_RX_AUDIO_OUT_GAIN]         <= { 16'b0, sys_wdata[15:0] };
        end
      20'h00198: begin
        regs[REG_RW_RB_RX_AUDIO_OUT_OFS]          <= { 16'b0, sys_wdata[15:0] };
        end

      default:   begin
         end

      endcase
      end
   end


wire sys_en;
assign sys_en = sys_wen | sys_ren;

// read access to the registers
always @(posedge clk_adc_125mhz)
if (!adc_rstn_i) begin
   sys_err      <= 1'b0;
   sys_ack      <= 1'b0;
   sys_rdata    <= 32'h00000000;
   end

else begin
   sys_err <= 1'b0;
   if (sys_ren) begin
      case (sys_addr[19:0])

      /* control */
      20'h00000: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_CTRL];
         end
      20'h00004: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RD_RB_STATUS];
         end
      20'h00008: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_ICR];
         end
      20'h0000C: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RD_RB_ISR];
         end
      20'h00010: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_DMA_CTRL];
         end
      20'h00018: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_PWR_CTRL];
         end
      20'h0001C: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_RFOUTx_LED_SRC_CON_PNT];
         end

      /* TX_CAR_OSC */
      20'h00020: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_TX_CAR_OSC_INC_LO];
         end
      20'h00024: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_TX_CAR_OSC_INC_HI];
         end
      20'h00028: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_TX_CAR_OSC_OFS_LO];
         end
      20'h0002C: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_TX_CAR_OSC_OFS_HI];
         end
      20'h00030: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_TX_RF_AMP_GAIN];
         end
      20'h00038: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_TX_RF_AMP_OFS];
         end

      /* TX_MOD_OSC */
      20'h00040: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_TX_MOD_OSC_INC_LO];
         end
      20'h00044: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_TX_MOD_OSC_INC_HI];
         end
      20'h00048: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_TX_MOD_OSC_OFS_LO];
         end
      20'h0004C: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_TX_MOD_OSC_OFS_HI];
         end
      20'h00050: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_TX_MOD_QMIX_GAIN];
         end
      20'h00058: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_TX_MOD_QMIX_OFS_LO];
         end
      20'h0005C: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_TX_MOD_QMIX_OFS_HI];
         end

      /* Input TX_MUX */
      20'h00060: begin
         sys_ack   <= sys_en;
         sys_rdata <= { 26'b0, regs[REG_RW_RB_TX_MUXIN_SRC][5:0] };
         end
      20'h00064: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_TX_MUXIN_GAIN];
         end

      /* RX_CAR_CALC_WEAVER */
      20'h00100: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_RX_CAR_CALC_WEAVER_INC_LO];
         end
      20'h00104: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_RX_CAR_CALC_WEAVER_INC_HI];
         end

      /* RX_CAR_AFC */
      20'h00110: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RD_RB_RX_CAR_AFC_INC_LO];
         end
      20'h00114: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RD_RB_RX_CAR_AFC_INC_HI];
         end

      /* RX_CAR_SUM */
      20'h00118: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RD_RB_RX_CAR_SUM_INC_LO];
         end
      20'h0011C: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RD_RB_RX_CAR_SUM_INC_HI];
         end

      /* RX_CAR_OSC */
      20'h00120: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_RX_CAR_OSC_INC_LO];
         end
      20'h00124: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_RX_CAR_OSC_INC_HI];
         end
      20'h00128: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_RX_CAR_OSC_OFS_LO];
         end
      20'h0012C: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_RX_CAR_OSC_OFS_HI];
         end

      /* RX_MOD_OSC */
      20'h00140: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_RX_MOD_OSC_INC_LO];
         end
      20'h00144: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_RX_MOD_OSC_INC_HI];
         end
      20'h00148: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_RX_MOD_OSC_OFS_LO];
         end
      20'h0014C: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_RX_MOD_OSC_OFS_HI];
         end

      /* RX_MUX */
      20'h00160: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_RX_MUXIN_SRC];
         end
      20'h00164: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_RX_MUXIN_GAIN];
         end

      /* RX_AFC_CORDIC */
      20'h00170: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RD_RB_RX_AFC_CORDIC_MAG];
         end
      20'h00174: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RD_RB_RX_AFC_CORDIC_PHS];
         end
      20'h00178: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RD_RB_RX_AFC_CORDIC_PHS_PREV];
         end
      20'h0017C: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RD_RB_RX_AFC_CORDIC_PHS_DIFF];
         end

      /* RX_DEMOD_GAIN */
      20'h00180: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_RX_SSB_AM_GAIN];
         end
      20'h00184: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_RX_FM_GAIN];
         end
      20'h00188: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_RX_PM_GAIN];
         end

      /* RX_AUDIO_OUT */
      20'h00190: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_RX_AUDIO_OUT_GAIN];
         end
      20'h00198: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_RX_AUDIO_OUT_OFS];
         end

      default:   begin
         sys_ack   <= sys_en;
         sys_rdata <= 32'h00000000;
         end

      endcase
      end

   else if (sys_wen) begin  // keep sys_ack assignment in this process
      sys_ack <= sys_en;
      end

   else begin
      sys_ack <= 1'b0;
      end
   end

endmodule
