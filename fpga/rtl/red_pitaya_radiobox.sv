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
 * (c) Ulrich Habel / GitHub.com open source  https://github.com/DF4IAH/RedPitaya_RadioBox/
 *
 * This part of code is written in Verilog hardware description language (HDL).
 * Please visit http://en.wikipedia.org/wiki/Verilog
 * for more details on the language used herein.
 */

/**
 * GENERAL DESCRIPTION:
 *
 * This modules enables the Red Pitaya to behave like a Transceiver for voice transmission. At the time of writing these modulation variants are supported:
 *   transmitter:  SSB - USB upper-side-band modulation                      - realized with a weaver oscillator @ 1700 Hz
 *                 SSB - LSB lower-side-band modulation                      - realized with a weaver oscillator @ 1700 Hz
 *                 AM  - amplitude modulation                                - realized by modulating the carrier output amplifier
 *                 FM  - frequency modulation                                - realized by modulating the DDS phase increment value
 *                 PM  - phase modulation                                    - realized by modulating the DDS phase offset value
 *
 *   receiver:     SSB - USB upper-side-band demodulation                    - realized with a weaver oscillator @ 1700 Hz
 *                 SSB - LSB lower-side-band demodulation                    - realized with a weaver oscillator @ 1700 Hz
 *                 AM  - amplitude envelope demodulation                     - realized by the OORDIC magnitude information
 *                 AM  - synchronized carrier upper-side-band demodulation   - realized by a CORDIC assisted automatic frequency correction (AFC) and weaver USB demodulation
 *                 AM  - synchronized carrier lower-side-band demodulation   - realized by a CORDIC assisted automatic frequency correction (AFC) and weaver LSB demodulation
 *                 FM  - frequency demodulation                              - realized by a CORDIC assisted automatic frequency correction (AFC) frequency offset as demodulation information
 *                 PM  - phase demodulation                                  - realized by an integrator of the FM signal
 *
 *
 * TODO: graphics - exmaple by red_pitaya_scope.v
 *
 *
 * The idea of this concept is to realize a complete audio transceiver for all known analog voice transmissions by short wave radio stations as well as HAM radio operators. Due to the used digital
 * concept all frequencies could be kept at a very low value because the "amplifiers" and "mixers" are simply multiplicators which does not have any "DC" blockers within. Students and radio enthusiasts
 * are able to play and learn about the idea behind modulation and demodulation of radio frequencies (RF). Any new ideas to have any variants of "modulation" and "demodulation" are welcome.
 *
 * For any further connections to the Red Pitaya the Linux sound system could be connected as audio streams to enable SDR radio-software to access this RadioBox submodule. Want to connect digital
 * CODECS to this RadioBox submodule? Simply connect the audio system to and from it. By doinf this it would be easy to use fldigi or other digital software working with a baseband concept.
 *
 * Another realization would be to adapt a front plate on top of the Red Pitaya to have a display, controllers and anything you need to operate this Red Pitaya anywhere you like and without the help
 * of a browser.
 *
 * Just have fun and learn!
 */


`timescale 1ns / 1ps

module red_pitaya_radiobox #(
  // parameter RSZ = 14  // RAM size 2^RSZ
)(
   // ADC clock & reset
   input                 clk_adc_125mhz  ,      // ADC based clock, 125 MHz
   input                 adc_rstn_i      ,      // ADC reset - active low
   input        [  1: 0] ac97_clks_i     ,      // sound interface sample rates

   // activation
   output                rb_activated    ,      // RB sub-module is activated

   // LEDs
   output reg            rb_leds_en      ,      // RB LEDs are enabled and overwrites HK sub-module
   output reg   [  7: 0] rb_leds_data    ,      // RB LEDs data, LED0 is located at the connector, ... , LED7 is located near to the red / green / blue LEDs
   input        [  7: 0] ac97_leds_i     ,      // AC97 diagnostic LEDs

   // ADC data
   input        [ 13: 0] adc_i[1:0]      ,      // ADC data { CHB, CHA }

   // DAC data
   output reg   [ 15: 0] rb_out_ch [1:0] ,      // RadioBox output signals

   // ALSA
   input        [ 31: 0] rb_line_out_i   ,      // Linux sound system ALSA LINE-OUT stereo, 2x 16 bit
   output reg   [ 31: 0] rb_line_in_o    ,      // Linux sound system ALSA LINE-IN  stereo, 2x 16 bit
   input                 ac97_irq_play_i ,      // monitor IRQ line for playing stream
   input                 ac97_irq_rec_i  ,      // monitor IRQ line for recording stream

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
    REG_RW_RB_PWR_CTRL,                         // h014: RB power savings control register             RX_MOD:     (Bit  7: 0)
                                                //                                                     TX_MOD:     (Bit 15: 8)
    REG_RW_RB_RFOUTx_LED_SRC_CON_PNT,           // h018: RB_LED, RB_RFOUT1 and RB_RFOUT2 connection matrix
    REG_RW_RB_LINE_IN_SRC_CON_PNT,              // h01C: LINE-IN11 L/R, LINE-IN2 L/R connection matrix

    /* TX section */
    REG_RW_RB_TX_CAR_OSC_INC_LO,                // h020: RB TX_CAR_OSC increment register              LSB:        (Bit 31: 0)
    REG_RW_RB_TX_CAR_OSC_INC_HI,                // h024: RB TX_CAR_OSC increment register              MSB: 16'b0, (Bit 47:32)
    REG_RW_RB_TX_CAR_OSC_OFS_LO,                // h028: RB TX_CAR_OSC offset register                 LSB:        (Bit 31: 0)
    REG_RW_RB_TX_CAR_OSC_OFS_HI,                // h02C: RB TX_CAR_OSC offset register                 MSB: 16'b0, (Bit 47:32)

    REG_RW_RB_TX_CAR_OSC_INC_SCNR_LO,           // h030: RB TX_CAR_OSC_SCNR increment register         LSB:        (Bit 31: 0)
    REG_RW_RB_TX_CAR_OSC_INC_SCNR_HI,           // h034: RB TX_CAR_OSC_SCNR increment register         MSB: 16'b0, (Bit 47:32)
    REG_RW_RB_TX_RF_AMP_GAIN,                   // h038: RB TX_RF_AMP gain:            SIGNED 16 bit
    REG_RW_RB_TX_RF_AMP_OFS,                    // h03C: RB TX_RF_AMP offset:          SIGNED 16 bit

    REG_RW_RB_TX_MOD_OSC_INC_LO,                // h040: RB TX_MOD_OSC increment register              LSB:        (Bit 31: 0)
    REG_RW_RB_TX_MOD_OSC_INC_HI,                // h044: RB TX_MOD_OSC increment register              MSB: 16'b0, (Bit 47:32)
    REG_RW_RB_TX_MOD_OSC_OFS_LO,                // h048: RB TX_MOD_OSC offset register                 LSB:        (Bit 31: 0)
    REG_RW_RB_TX_MOD_OSC_OFS_HI,                // h04C: RB TX_MOD_OSC offset register                 MSB: 16'b0, (Bit 47:32)

    REG_RW_RB_TX_MOD_QMIX_GAIN,                 // h050: RB TX_MOD_OSC mixer gain:   UNSIGNED 16 bit
    //REG_RD_RB_TX_RSVD_H054,
    REG_RW_RB_TX_MOD_QMIX_OFS_LO,               // h058: RB TX_MOD_OSC mixer offset:   SIGNED 48 bit   LSB:        (Bit 31: 0)
    REG_RW_RB_TX_MOD_QMIX_OFS_HI,               // h05C: RB TX_MOD_OSC mixer offset:   SIGNED 48 bit   MSB: 16'b0, (Bit 47:32)

    REG_RW_RB_TX_MUXIN_SRC,                     // h060: RB analog TX MUX input selector:
                                                //      d0 =(none),   d3 =VpVn,
                                                //      d16=EXT-CH0,  d24=EXT-CH8,
                                                //      d17=EXT-CH1,  d25=EXT-CH9,
                                                //      d32=adc_i[0], d33=adc_i[1]
    REG_RW_RB_TX_MUXIN_GAIN,                    // h064: RB analog TX MUX gain:      UNSIGNED 16 bit
    REG_RW_RB_TX_MUXIN_OFS,                     // h068: RB analog TX MUX gain:        SIGNED 16 bit
    //REG_RD_RB_TX_RSVD_H06C,


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

    REG_RW_RB_RX_CAR_OSC_INC_SCNR_LO,           // h130: RB RX_CAR_OSC_SCNR increment register         LSB:        (Bit 31: 0)
    REG_RW_RB_RX_CAR_OSC_INC_SCNR_HI,           // h134: RB RX_CAR_OSC_SCNR increment register         MSB: 16'b0, (Bit 47:32)
    //REG_RD_RB_RX_RSVD_H138,
    //REG_RD_RB_RX_RSVD_H13C,

    REG_RW_RB_RX_MOD_OSC_INC_LO,                // h140: RB RX_MOD_OSC increment register              LSB:        (Bit 31: 0)
    REG_RW_RB_RX_MOD_OSC_INC_HI,                // h144: RB RX_MOD_OSC increment register              MSB: 16'b0, (Bit 47:32)
    REG_RW_RB_RX_MOD_OSC_OFS_LO,                // h148: RB RX_MOD_OSC offset register                 LSB:        (Bit 31: 0)
    REG_RW_RB_RX_MOD_OSC_OFS_HI,                // h14C: RB RX_MOD_OSC offset register                 MSB: 16'b0, (Bit 47:32)

    //REG_RD_RB_RX_RSVD_H150,
    //REG_RD_RB_RX_RSVD_H154,
    //REG_RD_RB_RX_RSVD_H158,
    REG_RW_RB_RX_EMENV_FILT_VARIANT,            // h15C: RB_RX_EMENV_FILT_VARIANT  wide, middle, narrow            (Bit  1: 0)

    REG_RW_RB_RX_MUXIN_SRC,                     // h160: RB audio signal RX MUXIN input selector:
                                                //      d0 =(none),   d3 =VpVn,
                                                //      d16=EXT-CH0,  d24=EXT-CH8,
                                                //      d17=EXT-CH1,  d25=EXT-CH9,
                                                //      d32=adc_i[0], d33=adc_i[1]
    REG_RW_RB_RX_MUXIN_GAIN,                    // h164: RB audio signal RX MUXIN    UNSIGNED 16 bit
    REG_RW_RB_RX_MUXIN_OFS,                     // h168: RB analog RX MUX gain:        SIGNED 16 bit
    REG_RD_RB_RX_SIGNAL_STRENGTH,               // h16C: RB RX signal strength:      UNSIGNED 32 bit

    REG_RD_RB_RX_AFC_CORDIC_MAG,                // h170: RB RX_AFC_CORDIC magnitude value,                 UNSIGNED 16 bit
    REG_RD_RB_RX_AFC_CORDIC_PHS,                // h174: RB_RX_AFC_CORDIC phase value,                       SIGNED 16 bit
    REG_RD_RB_RX_AFC_CORDIC_PHS_PREV,           // h178: RB_RX_AFC_CORDIC previous 8kHz clock phase value,   SIGNED 16 bit
    REG_RD_RB_RX_AFC_CORDIC_PHS_DIFF,           // h17C: RB_RX_AFC_CORDIC difference phase value,            SIGNED 16 bit

    REG_RW_RB_RX_SSB_AM_GAIN,                   // h180: RB RX_MOD SSB/AM-Sync gain:   UNSIGNED 16 bit
    REG_RW_RB_RX_AMENV_GAIN,                    // h184: RB RX_MOD AM-ENV mixer gain:  UNSIGNED 16 bit
    REG_RW_RB_RX_FM_GAIN,                       // h188: RB RX_MOD FM mixer gain:      UNSIGNED 16 bit
    REG_RW_RB_RX_PM_GAIN,                       // h18C: RB RX_MOD PM mixer gain:      UNSIGNED 16 bit

    /* RFOUTx AMP section */
    REG_RW_RB_RFOUT1_AMP_GAIN,                  // h190: RB RFOUT1 AMP output gain:      SIGNED 16 bit
    REG_RW_RB_RFOUT1_AMP_OFS,                   // h194: RB RFOUT1 AMP output offset:    SIGNED 16 bit
    REG_RW_RB_RFOUT2_AMP_GAIN,                  // h198: RB RFOUT2 AMP output gain:      SIGNED 16 bit
    REG_RW_RB_RFOUT2_AMP_OFS,                   // h19C: RB RFOUT2 AMP output offset:    SIGNED 16 bit

    REG_RD_RB_READOUT_RFIN1,                    // h1A0: RB RFIN1  current data:         SIGNED 16 bit
    REG_RD_RB_READOUT_RFIN2,                    // h1A4: RB RFIN2  current data:         SIGNED 16 bit
    REG_RD_RB_READOUT_RFOUT1,                   // h1A8: RB RFOUT1 current data:         SIGNED 16 bit
    REG_RD_RB_READOUT_RFOUT2,                   // h1AC: RB RFOUT2 current data:         SIGNED 16 bit

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

    RB_CTRL_ADC_AUTO_OFS,                       // ADC_AUTO_OFS enabling automatic A/D-Converter offset compensation
    RB_CTRL_RSVD_D25,
    RB_CTRL_RSVD_D26,
    RB_CTRL_RSVD_D27,

    RB_CTRL_RX_MOD_OSC_RESYNC,                  // RX_MOD_OSC restart with phase register = 0
    RB_CTRL_RSVD_D29,
    RB_CTRL_RSVD_D30,
    RB_CTRL_RSVD_D31
} RB_CTRL_BITS_ENUM;

enum {
    RB_PWR_CTRL_TX_MOD_ALL_ON             =  0, // RB_PWR_CTRL TX modulation: no power savings, all clocks of the receiver are turned on
    RB_PWR_CTRL_TX_MOD_OFF,                     // RB_PWR_CTRL TX modulation: complete transmitter is turned off
    RB_PWR_CTRL_TX_MOD_USB                =  2, // RB_PWR_CTRL TX modulation: components of the SSB-USB transmitter are turned on
    RB_PWR_CTRL_TX_MOD_LSB,                     // RB_PWR_CTRL TX modulation: components of the SSB-LSB transmitter are turned on
    RB_PWR_CTRL_TX_MOD_AM,                      // RB_PWR_CTRL TX modulation: components of the AM transmitter are turned on
    RB_PWR_CTRL_TX_MOD_FM                 =  7, // RB_PWR_CTRL TX modulation: components of the FM transmitter are turned on
    RB_PWR_CTRL_TX_MOD_PM                       // RB_PWR_CTRL TX modulation: components of the PM transmitter are turned on
} RB_PWR_CTRL_TX_MOD_BITS_ENUM;

enum {
    RB_PWR_CTRL_RX_MOD_ALL_ON             =  0, // RB_PWR_CTRL RX modulation: no power savings, all clocks of the transceiver are turned on
    RB_PWR_CTRL_RX_MOD_OFF,                     // RB_PWR_CTRL RX modulation: complete receiver is turned off
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
    RB_STAT_OVERDRIVE_TX,                       // any overdrive signal in the TX path is signaled
    RB_STAT_OVERDRIVE_RX,                       // any overdrive signal in the RX path is signaled

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

    RB_SRC_CON_PNT_NUM_RX_MUXIN_MIX_IN              = 32, // RX_MUXIN_MIX input
    RB_SRC_CON_PNT_NUM_RX_MUXIN_MIX_OUT,                  // RX_MUXIN_MIX output
    RB_SRC_CON_PNT_NUM_RX_CAR_OSC_I_OUT,                  // RX_CAR_OSC I output
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

    RB_SRC_CON_PNT_NUM_TX_CAR_OSC_INC               = 192,// current TX_CAR_OSC_INC value
    RB_SRC_CON_PNT_NUM_RX_CAR_OSC_INC,                    // current RX_CAR_OSC_INC value

    RB_SRC_CON_PNT_NUM_ADC_AUTO_OFS_RFIN1           = 208,// current LSB bits from offset register of RFIN1
    RB_SRC_CON_PNT_NUM_ADC_AUTO_OFS_RFIN2,                // current LSB bits from offset register of RFIN1
    RB_SRC_CON_PNT_NUM_ADC_AUTO_OFS_EXT_CH0,              // current LSB bits from offset register of XADC channel 0
    RB_SRC_CON_PNT_NUM_ADC_AUTO_OFS_EXT_CH8,              // current LSB bits from offset register of XADC channel 8
    RB_SRC_CON_PNT_NUM_ADC_AUTO_OFS_EXT_CH1,              // current LSB bits from offset register of XADC channel 1
    RB_SRC_CON_PNT_NUM_ADC_AUTO_OFS_EXT_CH9,              // current LSB bits from offset register of XADC channel 9
    RB_SRC_CON_PNT_NUM_ADC_AUTO_OFS_EXT_VpVn,             // current LSB bits from offset register of XADC channel VpVn

    RB_SRC_CON_PNT_NUM_TEST_AC97                    = 240,// AC97 diagnostic LEDs

    RB_SRC_CON_PNT_NUM_TEST_OVERDRIVE               = 248,// overdrive signals

    RB_SRC_CON_PNT_NUM_TEST_VECTOR_OUT              = 255 // Test vector, look at assignments within this file

} RB_SRC_CON_PNT_ENUM;                                    // 256 entries = 2^8 --> 8 bit field

enum {
    RB_XADC_MAPPING_EXT_CH0                         =  0, // CH0 and CH8 are sampled simultaneously, mapped to: vinp_i[0]/vinn_i[0]
    RB_XADC_MAPPING_EXT_CH8,                              // CH0 and CH8 are sampled simultaneously, mapped to: vinp_i[1]/vinn_i[1]
    RB_XADC_MAPPING_EXT_CH1,                              // CH1 and CH9 are sampled simultaneously, mapped to: vinp_i[2]/vinn_i[2]
    RB_XADC_MAPPING_EXT_CH9,                              // CH1 and CH9 are sampled simultaneously, mapped to: vinp_i[3]/vinn_i[3]
    RB_XADC_MAPPING_VpVn,                                 // The dedicated Vp/Vn input mapped to: vinp_i[4]/vinn_i[4]
    RB_XADC_MAPPING__COUNT
} RB_XADC_MAPPING_ENUM;

enum {
    RB_ADC_AUTO_OFS_RFIN1                           =  0, // RF SMA input 1
    RB_ADC_AUTO_OFS_RFIN2,                                // RF SMA input 2
    RB_ADC_AUTO_OFS_EXT_CH0,                              // XADC channel 0, synchron to channel 8
    RB_ADC_AUTO_OFS_EXT_CH8,                              // XADC channel 8, synchron to channel 0
    RB_ADC_AUTO_OFS_EXT_CH1,                              // XADC channel 1, synchron to channel 9
    RB_ADC_AUTO_OFS_EXT_CH9,                              // XADC channel 9, synchron to channel 1
    RB_ADC_AUTO_OFS_VpVn,                                 // The dedicated Vp/Vn input mapped to: vinp_i[4]/vinn_i[4]
    RB_ADC_AUTO_OFS__COUNT
} RB_ADC_OFFSET_MAPPING_ENUM;


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

wire                   adc_auto_ofs           = regs[REG_RW_RB_CTRL][RB_CTRL_ADC_AUTO_OFS];

wire unsigned [  7: 0] rb_pwr_rx_modvar       = regs[REG_RW_RB_PWR_CTRL][ 7:0];
wire unsigned [  7: 0] rb_pwr_tx_modvar       = regs[REG_RW_RB_PWR_CTRL][15:8];

wire unsigned [  7: 0] led_src_con_pnt        = regs[REG_RW_RB_RFOUTx_LED_SRC_CON_PNT][ 7: 0];
wire unsigned [  7: 0] rfout1_src_con_pnt     = regs[REG_RW_RB_RFOUTx_LED_SRC_CON_PNT][23:16];
wire unsigned [  7: 0] rfout2_src_con_pnt     = regs[REG_RW_RB_RFOUTx_LED_SRC_CON_PNT][31:24];
wire unsigned [  7: 0] line_in1l_src_con_pnt  = regs[REG_RW_RB_LINE_IN_SRC_CON_PNT][ 7: 0];
wire unsigned [  7: 0] line_in1r_src_con_pnt  = regs[REG_RW_RB_LINE_IN_SRC_CON_PNT][15: 8];
//wire unsigned [  7: 0] line_in2l_src_con_pnt  = regs[REG_RW_RB_LINE_IN_SRC_CON_PNT][23:16];
//wire unsigned [  7: 0] line_in2r_src_con_pnt  = regs[REG_RW_RB_LINE_IN_SRC_CON_PNT][31:24];

wire unsigned [  5: 0] tx_muxin_src           = regs[REG_RW_RB_TX_MUXIN_SRC][5:0];
wire unsigned [  5: 0] rx_muxin_src           = regs[REG_RW_RB_RX_MUXIN_SRC][5:0];

wire unsigned [ 15: 0] tx_muxin_mix_gain      = regs[REG_RW_RB_TX_MUXIN_GAIN][15: 0];
wire unsigned [  2: 0] tx_muxin_mix_log2      = regs[REG_RW_RB_TX_MUXIN_GAIN][18:16];
wire   signed [ 15: 0] tx_muxin_mix_ofs       = regs[REG_RW_RB_TX_MUXIN_OFS][15: 0];
wire unsigned [ 15: 0] rx_muxin_mix_gain      = regs[REG_RW_RB_RX_MUXIN_GAIN][15: 0];
wire unsigned [  2: 0] rx_muxin_mix_log2      = regs[REG_RW_RB_RX_MUXIN_GAIN][18:16];
wire   signed [ 15: 0] rx_muxin_mix_ofs       = regs[REG_RW_RB_RX_MUXIN_OFS][15: 0];

wire unsigned [ 15: 0] tx_rf_amp_gain         = regs[REG_RW_RB_TX_RF_AMP_GAIN][15:0];
wire   signed [ 15: 0] tx_rf_amp_ofs          = regs[REG_RW_RB_TX_RF_AMP_OFS][15:0];

wire unsigned [ 15: 0] tx_mod_qmix_gain       = regs[REG_RW_RB_TX_MOD_QMIX_GAIN][15:0];
wire   signed [ 47: 0] tx_mod_qmix_ofs        = { regs[REG_RW_RB_TX_MOD_QMIX_OFS_HI][15:0], regs[REG_RW_RB_TX_MOD_QMIX_OFS_LO][31:0] };

wire unsigned [ 47: 0] tx_car_osc_inc         = { regs[REG_RW_RB_TX_CAR_OSC_INC_HI][15:0], regs[REG_RW_RB_TX_CAR_OSC_INC_LO][31:0] };
wire   signed [ 47: 0] tx_car_osc_ofs         = { regs[REG_RW_RB_TX_CAR_OSC_OFS_HI][15:0], regs[REG_RW_RB_TX_CAR_OSC_OFS_LO][31:0] };
wire unsigned [ 47: 0] tx_car_osc_inc_scanner = { regs[REG_RW_RB_TX_CAR_OSC_INC_SCNR_HI][15:0], regs[REG_RW_RB_TX_CAR_OSC_INC_SCNR_LO][31:0] };

wire unsigned [ 47: 0] tx_mod_osc_inc         = { regs[REG_RW_RB_TX_MOD_OSC_INC_HI][15:0], regs[REG_RW_RB_TX_MOD_OSC_INC_LO][31:0] };
wire   signed [ 47: 0] tx_mod_osc_ofs         = { regs[REG_RW_RB_TX_MOD_OSC_OFS_HI][15:0], regs[REG_RW_RB_TX_MOD_OSC_OFS_LO][31:0] };

wire unsigned [ 47: 0] rx_car_osc_inc         = { regs[REG_RW_RB_RX_CAR_OSC_INC_HI][15:0], regs[REG_RW_RB_RX_CAR_OSC_INC_LO][31:0] };
wire   signed [ 47: 0] rx_car_osc_ofs         = { regs[REG_RW_RB_RX_CAR_OSC_OFS_HI][15:0], regs[REG_RW_RB_RX_CAR_OSC_OFS_LO][31:0] };
wire unsigned [ 47: 0] rx_car_osc_inc_scanner = { regs[REG_RW_RB_RX_CAR_OSC_INC_SCNR_HI][15:0], regs[REG_RW_RB_RX_CAR_OSC_INC_SCNR_LO][31:0] };

wire unsigned [  1: 0] rx_afc_amenv_filtvar   = regs[REG_RW_RB_RX_EMENV_FILT_VARIANT][1:0];

wire   signed [ 31: 0] rx_afc_cordic_phs      = regs[REG_RD_RB_RX_AFC_CORDIC_PHS];
wire   signed [ 31: 0] rx_afc_cordic_phs_prev = regs[REG_RD_RB_RX_AFC_CORDIC_PHS_PREV];
wire   signed [ 31: 0] rx_afc_cordic_phs_diff = regs[REG_RD_RB_RX_AFC_CORDIC_PHS_DIFF];

wire   signed [ 47: 0] rx_afc_calc_weaver_inc = { regs[REG_RW_RB_RX_CAR_CALC_WEAVER_INC_HI][15:0], regs[REG_RW_RB_RX_CAR_CALC_WEAVER_INC_LO][31:0] };
wire   signed [ 47: 0] rx_car_afc_inc         = { regs[REG_RD_RB_RX_CAR_AFC_INC_HI][15:0], regs[REG_RD_RB_RX_CAR_AFC_INC_LO][31:0] };
wire unsigned [ 47: 0] rx_car_sum_inc         = { regs[REG_RD_RB_RX_CAR_SUM_INC_HI][15:0], regs[REG_RD_RB_RX_CAR_SUM_INC_LO][31:0] };

wire unsigned [ 47: 0] rx_mod_osc_inc         = { regs[REG_RW_RB_RX_MOD_OSC_INC_HI][15:0], regs[REG_RW_RB_RX_MOD_OSC_INC_LO][31:0] };
wire   signed [ 47: 0] rx_mod_osc_ofs         = { regs[REG_RW_RB_RX_MOD_OSC_OFS_HI][15:0], regs[REG_RW_RB_RX_MOD_OSC_OFS_LO][31:0] };

wire unsigned [ 15: 0] rx_ssb_am_gain         = regs[REG_RW_RB_RX_SSB_AM_GAIN];
wire unsigned [ 15: 0] rx_amenv_gain          = regs[REG_RW_RB_RX_AMENV_GAIN];
wire unsigned [ 15: 0] rx_fm_gain             = regs[REG_RW_RB_RX_FM_GAIN];
wire unsigned [ 15: 0] rx_pm_gain             = regs[REG_RW_RB_RX_PM_GAIN];

wire unsigned [ 15: 0] rfout1_amp_gain        = regs[REG_RW_RB_RFOUT1_AMP_GAIN][15:0];
wire   signed [ 15: 0] rfout1_amp_ofs         = regs[REG_RW_RB_RFOUT1_AMP_OFS][15:0];
wire unsigned [ 15: 0] rfout2_amp_gain        = regs[REG_RW_RB_RFOUT2_AMP_GAIN][15:0];
wire   signed [ 15: 0] rfout2_amp_ofs         = regs[REG_RW_RB_RFOUT2_AMP_OFS][15:0];


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

   8'h01: begin                                                        // OFF
      rb_pwr_tx_OSC_en <= 1'b0;
      rb_pwr_tx_I_en   <= 1'b0;
      rb_pwr_tx_Q_en   <= 1'b0;
      end
   default: begin                                                      // ALL_ON (no power reduction selected)
      rb_pwr_tx_OSC_en <= 1'b1;
      rb_pwr_tx_I_en   <= 1'b1;
      rb_pwr_tx_Q_en   <= 1'b1;
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

   8'h01: begin                                                        // OFF
      rb_pwr_rx_CAR_en <= 1'b0;
      rb_pwr_rx_MOD_en <= 1'b0;
      rb_pwr_rx_AFC_en <= 1'b0;
      end
   default: begin                                                      // ALL_ON (no power reduction selected)
      rb_pwr_rx_CAR_en <= 1'b1;
      rb_pwr_rx_MOD_en <= 1'b1;
      rb_pwr_rx_AFC_en <= 1'b1;
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

reg  signed   [ 15: 0] rb_xadc[RB_XADC_MAPPING__COUNT - 1: 0];

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


// ADC AUTO OFS correction

wire signed   [ 15: 0] xadc_in = xadc_axis_tdata;
reg  signed   [ 15: 0] adc_enhanced[RB_ADC_AUTO_OFS__COUNT - 1 : 0] = '{RB_ADC_AUTO_OFS__COUNT{0}};
reg  signed   [ 15: 0] adc_offset[  RB_ADC_AUTO_OFS__COUNT - 1 : 0] = '{RB_ADC_AUTO_OFS__COUNT{0}};
reg  signed   [ 29: 0] adc_sumreg[  RB_ADC_AUTO_OFS__COUNT - 1 : 0] = '{RB_ADC_AUTO_OFS__COUNT{0}};

always @(posedge clk_adc_125mhz) begin                                 // assign adc_enhanced - corrected ADC values
if (!adc_rstn_i) begin
   adc_enhanced[RB_ADC_AUTO_OFS_RFIN1]   <= 'd0;
   adc_enhanced[RB_ADC_AUTO_OFS_RFIN2]   <= 'd0;
   adc_enhanced[RB_ADC_AUTO_OFS_EXT_CH0] <= 'd0;
   adc_enhanced[RB_ADC_AUTO_OFS_EXT_CH8] <= 'd0;
   adc_enhanced[RB_ADC_AUTO_OFS_EXT_CH1] <= 'd0;
   adc_enhanced[RB_ADC_AUTO_OFS_EXT_CH9] <= 'd0;
   adc_enhanced[RB_ADC_AUTO_OFS_VpVn]    <= 'd0;
   end
else if (adc_auto_ofs) begin
   adc_enhanced[RB_ADC_AUTO_OFS_RFIN1]   <= { ~adc_i[0][13:0], 2'b0 }        - adc_offset[RB_ADC_AUTO_OFS_RFIN1];
   adc_enhanced[RB_ADC_AUTO_OFS_RFIN2]   <= { ~adc_i[1][13:0], 2'b0 }        - adc_offset[RB_ADC_AUTO_OFS_RFIN2];
   adc_enhanced[RB_ADC_AUTO_OFS_EXT_CH0] <= rb_xadc[RB_XADC_MAPPING_EXT_CH0] - adc_offset[RB_ADC_AUTO_OFS_EXT_CH0];
   adc_enhanced[RB_ADC_AUTO_OFS_EXT_CH8] <= rb_xadc[RB_XADC_MAPPING_EXT_CH8] - adc_offset[RB_ADC_AUTO_OFS_EXT_CH8];
   adc_enhanced[RB_ADC_AUTO_OFS_EXT_CH1] <= rb_xadc[RB_XADC_MAPPING_EXT_CH1] - adc_offset[RB_ADC_AUTO_OFS_EXT_CH1];
   adc_enhanced[RB_ADC_AUTO_OFS_EXT_CH9] <= rb_xadc[RB_XADC_MAPPING_EXT_CH9] - adc_offset[RB_ADC_AUTO_OFS_EXT_CH9];
   adc_enhanced[RB_ADC_AUTO_OFS_VpVn]    <= rb_xadc[RB_XADC_MAPPING_VpVn]    - adc_offset[RB_ADC_AUTO_OFS_VpVn];
   end
else begin
   adc_enhanced[RB_ADC_AUTO_OFS_RFIN1]   <= { ~adc_i[0][13:0], 2'b0 };
   adc_enhanced[RB_ADC_AUTO_OFS_RFIN2]   <= { ~adc_i[1][13:0], 2'b0 };
   adc_enhanced[RB_ADC_AUTO_OFS_EXT_CH0] <= rb_xadc[RB_XADC_MAPPING_EXT_CH0];
   adc_enhanced[RB_ADC_AUTO_OFS_EXT_CH8] <= rb_xadc[RB_XADC_MAPPING_EXT_CH8];
   adc_enhanced[RB_ADC_AUTO_OFS_EXT_CH1] <= rb_xadc[RB_XADC_MAPPING_EXT_CH1];
   adc_enhanced[RB_ADC_AUTO_OFS_EXT_CH9] <= rb_xadc[RB_XADC_MAPPING_EXT_CH9];
   adc_enhanced[RB_ADC_AUTO_OFS_VpVn]    <= rb_xadc[RB_XADC_MAPPING_VpVn];
   end
end

always @(posedge clk_adc_125mhz) begin                                 // assign adc_sumreg, adc_offset - adjust ADC offset values
if (!adc_rstn_i || !adc_auto_ofs) begin
   adc_sumreg[RB_ADC_AUTO_OFS_RFIN1] <= 'd0;
   adc_offset[RB_ADC_AUTO_OFS_RFIN1] <= 'd0;
   end
else if (adc_sumreg[RB_ADC_AUTO_OFS_RFIN1][29:28] == 2'b01) begin
   adc_sumreg[RB_ADC_AUTO_OFS_RFIN1] = 'd0;
   adc_offset[RB_ADC_AUTO_OFS_RFIN1] = adc_offset[RB_ADC_AUTO_OFS_RFIN1] + 1;
   end
else if (adc_sumreg[RB_ADC_AUTO_OFS_RFIN1][29:28] == 2'b10) begin
   adc_sumreg[RB_ADC_AUTO_OFS_RFIN1] = 'd0;
   adc_offset[RB_ADC_AUTO_OFS_RFIN1] = adc_offset[RB_ADC_AUTO_OFS_RFIN1] - 1;
   end
else
   adc_sumreg[RB_ADC_AUTO_OFS_RFIN1] = adc_sumreg[RB_ADC_AUTO_OFS_RFIN1] + adc_enhanced[RB_ADC_AUTO_OFS_RFIN1];
end

always @(posedge clk_adc_125mhz) begin                                 // assign adc_sumreg, adc_offset - adjust ADC offset values
if (!adc_rstn_i || !adc_auto_ofs) begin
   adc_sumreg[RB_ADC_AUTO_OFS_RFIN2] <= 'd0;
   adc_offset[RB_ADC_AUTO_OFS_RFIN2] <= 'd0;
   end
else if (adc_sumreg[RB_ADC_AUTO_OFS_RFIN2][29:28] == 2'b01) begin
   adc_sumreg[RB_ADC_AUTO_OFS_RFIN2] = 'd0;
   adc_offset[RB_ADC_AUTO_OFS_RFIN2] = adc_offset[RB_ADC_AUTO_OFS_RFIN2] + 1;
   end
else if (adc_sumreg[RB_ADC_AUTO_OFS_RFIN2][29:28] == 2'b10) begin
   adc_sumreg[RB_ADC_AUTO_OFS_RFIN2] = 'd0;
   adc_offset[RB_ADC_AUTO_OFS_RFIN2] = adc_offset[RB_ADC_AUTO_OFS_RFIN2] - 1;
   end
else
   adc_sumreg[RB_ADC_AUTO_OFS_RFIN2] = adc_sumreg[RB_ADC_AUTO_OFS_RFIN2] + adc_enhanced[RB_ADC_AUTO_OFS_RFIN2];
end

always @(posedge clk_adc_125mhz) begin                                 // assign adc_sumreg, adc_offset - adjust ADC offset values
if (!adc_rstn_i || !adc_auto_ofs) begin
   adc_sumreg[RB_ADC_AUTO_OFS_EXT_CH0] <= 'd0;
   adc_offset[RB_ADC_AUTO_OFS_EXT_CH0] <= 'd0;
   end
else if (adc_sumreg[RB_ADC_AUTO_OFS_EXT_CH0][29:28] == 2'b01) begin
   adc_sumreg[RB_ADC_AUTO_OFS_EXT_CH0] = 'd0;
   adc_offset[RB_ADC_AUTO_OFS_EXT_CH0] = adc_offset[RB_ADC_AUTO_OFS_EXT_CH0] + 1;
   end
else if (adc_sumreg[RB_ADC_AUTO_OFS_EXT_CH0][29:28] == 2'b10) begin
   adc_sumreg[RB_ADC_AUTO_OFS_EXT_CH0] = 'd0;
   adc_offset[RB_ADC_AUTO_OFS_EXT_CH0] = adc_offset[RB_ADC_AUTO_OFS_EXT_CH0] - 1;
   end
else
   adc_sumreg[RB_ADC_AUTO_OFS_EXT_CH0] = adc_sumreg[RB_ADC_AUTO_OFS_EXT_CH0] + adc_enhanced[RB_ADC_AUTO_OFS_EXT_CH0];
end

always @(posedge clk_adc_125mhz) begin                                 // assign adc_sumreg, adc_offset - adjust ADC offset values
if (!adc_rstn_i || !adc_auto_ofs) begin
   adc_sumreg[RB_ADC_AUTO_OFS_EXT_CH8] <= 'd0;
   adc_offset[RB_ADC_AUTO_OFS_EXT_CH8] <= 'd0;
   end
else if (adc_sumreg[RB_ADC_AUTO_OFS_EXT_CH8][29:28] == 2'b01) begin
   adc_sumreg[RB_ADC_AUTO_OFS_EXT_CH8] = 'd0;
   adc_offset[RB_ADC_AUTO_OFS_EXT_CH8] = adc_offset[RB_ADC_AUTO_OFS_EXT_CH8] + 1;
   end
else if (adc_sumreg[RB_ADC_AUTO_OFS_EXT_CH8][29:28] == 2'b10) begin
   adc_sumreg[RB_ADC_AUTO_OFS_EXT_CH8] = 'd0;
   adc_offset[RB_ADC_AUTO_OFS_EXT_CH8] = adc_offset[RB_ADC_AUTO_OFS_EXT_CH8] - 1;
   end
else
   adc_sumreg[RB_ADC_AUTO_OFS_EXT_CH8] = adc_sumreg[RB_ADC_AUTO_OFS_EXT_CH8] + adc_enhanced[RB_ADC_AUTO_OFS_EXT_CH8];
end

always @(posedge clk_adc_125mhz) begin                                 // assign adc_sumreg, adc_offset - adjust ADC offset values
if (!adc_rstn_i || !adc_auto_ofs) begin
   adc_sumreg[RB_ADC_AUTO_OFS_EXT_CH1] <= 'd0;
   adc_offset[RB_ADC_AUTO_OFS_EXT_CH1] <= 'd0;
   end
else if (adc_sumreg[RB_ADC_AUTO_OFS_EXT_CH1][29:28] == 2'b01) begin
   adc_sumreg[RB_ADC_AUTO_OFS_EXT_CH1] = 'd0;
   adc_offset[RB_ADC_AUTO_OFS_EXT_CH1] = adc_offset[RB_ADC_AUTO_OFS_EXT_CH1] + 1;
   end
else if (adc_sumreg[RB_ADC_AUTO_OFS_EXT_CH1][29:28] == 2'b10) begin
   adc_sumreg[RB_ADC_AUTO_OFS_EXT_CH1] = 'd0;
   adc_offset[RB_ADC_AUTO_OFS_EXT_CH1] = adc_offset[RB_ADC_AUTO_OFS_EXT_CH1] - 1;
   end
else
   adc_sumreg[RB_ADC_AUTO_OFS_EXT_CH1] = adc_sumreg[RB_ADC_AUTO_OFS_EXT_CH1] + adc_enhanced[RB_ADC_AUTO_OFS_EXT_CH1];
end

always @(posedge clk_adc_125mhz) begin                                 // assign adc_sumreg, adc_offset - adjust ADC offset values
if (!adc_rstn_i || !adc_auto_ofs) begin
   adc_sumreg[RB_ADC_AUTO_OFS_EXT_CH9] <= 'd0;
   adc_offset[RB_ADC_AUTO_OFS_EXT_CH9] <= 'd0;
   end
else if (adc_sumreg[RB_ADC_AUTO_OFS_EXT_CH9][29:28] == 2'b01) begin
   adc_sumreg[RB_ADC_AUTO_OFS_EXT_CH9] = 'd0;
   adc_offset[RB_ADC_AUTO_OFS_EXT_CH9] = adc_offset[RB_ADC_AUTO_OFS_EXT_CH9] + 1;
   end
else if (adc_sumreg[RB_ADC_AUTO_OFS_EXT_CH9][29:28] == 2'b10) begin
   adc_sumreg[RB_ADC_AUTO_OFS_EXT_CH9] = 'd0;
   adc_offset[RB_ADC_AUTO_OFS_EXT_CH9] = adc_offset[RB_ADC_AUTO_OFS_EXT_CH9] - 1;
   end
else
   adc_sumreg[RB_ADC_AUTO_OFS_EXT_CH9] = adc_sumreg[RB_ADC_AUTO_OFS_EXT_CH9] + adc_enhanced[RB_ADC_AUTO_OFS_EXT_CH9];
end

always @(posedge clk_adc_125mhz) begin                                 // assign adc_sumreg, adc_offset - adjust ADC offset values
if (!adc_rstn_i || !adc_auto_ofs) begin
   adc_sumreg[RB_ADC_AUTO_OFS_VpVn] <= 'd0;
   adc_offset[RB_ADC_AUTO_OFS_VpVn] <= 'd0;
   end
else if (adc_sumreg[RB_ADC_AUTO_OFS_VpVn][29:28] == 2'b01) begin
   adc_sumreg[RB_ADC_AUTO_OFS_VpVn] = 'd0;
   adc_offset[RB_ADC_AUTO_OFS_VpVn] = adc_offset[RB_ADC_AUTO_OFS_VpVn] + 1;
   end
else if (adc_sumreg[RB_ADC_AUTO_OFS_VpVn][29:28] == 2'b10) begin
   adc_sumreg[RB_ADC_AUTO_OFS_VpVn] = 'd0;
   adc_offset[RB_ADC_AUTO_OFS_VpVn] = adc_offset[RB_ADC_AUTO_OFS_VpVn] - 1;
   end
else
   adc_sumreg[RB_ADC_AUTO_OFS_VpVn] = adc_sumreg[RB_ADC_AUTO_OFS_VpVn] + adc_enhanced[RB_ADC_AUTO_OFS_VpVn];
end


/*
localparam CLK_48KHZ_CTR_MAX = 2604;                                   // long run max value
localparam CLK_48KHZ_FRC_MAX = 5;

reg  [ 11: 0] clk_48khz_ctr  = 'b0;
reg  [  2: 0] clk_48khz_frc  = 'b0;
reg           clk_48khz      = 'b0;
reg           clk_8khz       = 'b0;

always @(posedge clk_adc_125mhz)                                       // assign clk_48khz, clk_8khz
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
         clk_48khz_ctr <= 1'b0;                                        // overflow of the frac part makes a long run
         clk_8khz <= 1'b1;
         end
      else begin
         clk_48khz_frc <= clk_48khz_frc + 1;
         clk_48khz_ctr <= 12'b1;                                       // short run
         end
      end
   else begin
      clk_8khz  <= 1'b0;
      clk_48khz <= 1'b0;
      clk_48khz_ctr <= clk_48khz_ctr + 1;
      end
*/
assign clk_8khz  = ac97_clks_i[0];
assign clk_48khz = ac97_clks_i[1];


localparam CLK_200KHZ_CTR_MAX = 624;

reg                    clk_200khz     = 1'b0;
reg  unsigned [ 11: 0] clk_200khz_ctr =  'b0;

always @(posedge clk_adc_125mhz)                                       // assign clk_200khz
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

reg  unsigned [ 7: 0] clk_200khz_d  = 'b0;

always @(posedge clk_adc_125mhz)
if (!rb_clk_en)
   clk_200khz_d <= 'b0;
else
   clk_200khz_d <= { clk_200khz_d[6:0], clk_200khz };


//---------------------------------------------------------------------------------
//  Overdrive signals

reg  [ 19: 0] led_ctr  = 20'b0;
wire          rb_overdrive_tx_muxin;
reg           rb_overdrive_tx_muxin_d   = 1'b0;
reg           rb_overdrive_tx_muxin_mon = 1'b0;

always @(posedge clk_adc_125mhz)
if (!rb_clk_en) begin
   rb_overdrive_tx_muxin_mon <= 1'b0;
   rb_overdrive_tx_muxin_d <= 1'b0;
   end
else if (!led_ctr) begin
   rb_overdrive_tx_muxin_mon <= rb_overdrive_tx_muxin_d;
   rb_overdrive_tx_muxin_d <= 1'b0;
   end
else if (rb_overdrive_tx_muxin)
   rb_overdrive_tx_muxin_d <= 1'b1;

wire rb_overdrive_rx_muxin;
reg  rb_overdrive_rx_muxin_d   = 1'b0;
reg  rb_overdrive_rx_muxin_mon = 1'b0;

always @(posedge clk_adc_125mhz)
if (!rb_clk_en) begin
   rb_overdrive_rx_muxin_mon <= 1'b0;
   rb_overdrive_rx_muxin_d <= 1'b0;
   end
else if (!led_ctr) begin
   rb_overdrive_rx_muxin_mon <= rb_overdrive_rx_muxin_d;
   rb_overdrive_rx_muxin_d <= 1'b0;
   end
else if (rb_overdrive_rx_muxin)
   rb_overdrive_rx_muxin_d <= 1'b1;


// === Transmitter section ===
// === TX_MOD section ===

//---------------------------------------------------------------------------------
//  ADC modulation offset correction and gain

wire   signed [ 15: 0] tx_muxin_mix_in = (tx_muxin_src == 6'h20) ?  adc_enhanced[RB_ADC_AUTO_OFS_RFIN1]   :
                                         (tx_muxin_src == 6'h21) ?  adc_enhanced[RB_ADC_AUTO_OFS_RFIN2]   :
                                         (tx_muxin_src == 6'h18) ?  adc_enhanced[RB_ADC_AUTO_OFS_EXT_CH0] :      // swapped here due to pin connection warnings when swapping @ XADC <--> pins
                                         (tx_muxin_src == 6'h10) ?  adc_enhanced[RB_ADC_AUTO_OFS_EXT_CH8] :
                                         (tx_muxin_src == 6'h11) ?  adc_enhanced[RB_ADC_AUTO_OFS_EXT_CH1] :
                                         (tx_muxin_src == 6'h19) ?  adc_enhanced[RB_ADC_AUTO_OFS_EXT_CH9] :
                                         (tx_muxin_src == 6'h03) ?  adc_enhanced[RB_ADC_AUTO_OFS_VpVn]    :
                                         (tx_muxin_src == 6'h30) ?  rb_line_out_i[15: 0]                  :
                                         (tx_muxin_src == 6'h31) ?  rb_line_out_i[31:16]                  :
                                         16'b0                                                            ;
wire   signed [ 17: 0] tx_muxin_mix_in_se = { tx_muxin_mix_in[15], tx_muxin_mix_in[15:0], 1'b0 };           // max. 1Vpp
wire   signed [ 17: 0] tx_mod_adc_in = (tx_muxin_mix_in_se << tx_muxin_mix_log2);                           // signed value: input booster for
                                                                                                            // factor: 1x .. 2^3=7 shift postions=128x (16 mV --> full-scale)
wire   signed [ 17: 0] tx_mod_adc_ofs = { {2{tx_muxin_mix_ofs[15]}}, tx_muxin_mix_ofs[15:0] };              // strip of the DC component
wire   signed [ 17: 0] tx_muxin_mix_gain_in = { 2'b0, tx_muxin_mix_gain[15:0] };                            // unsigned value
wire   signed [ 36: 0] tx_mod_adc_out;

rb_dsp48_AaDmBaC_A18_D18_B18_C36_P37 i_rb_tx_mod_adc_dsp48 (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_pwr_tx_OSC_en            ),  // power down on request

  .A                       ( tx_mod_adc_in               ),  // MUX in signal:            SIGNED 18 bit
  .D                       ( tx_mod_adc_ofs              ),  // offset setting:           SIGNED 18 bit
  .B                       ( tx_muxin_mix_gain_in        ),  // gain setting:             SIGNED 18 bit
  .C                       ( 36'b0                       ),  // unused                    SIGNED 36 bit

  .P                       ( tx_mod_adc_out              )   // PreAmp output             SIGSIG 37 bit
);

wire          [  4: 0] rb_overdrive_tx_muxin_ovl = tx_mod_adc_out[35:31];
assign rb_overdrive_tx_muxin = (!tx_mod_adc_out[36] && (| rb_overdrive_tx_muxin_ovl)) || (tx_mod_adc_out[36] && !(& rb_overdrive_tx_muxin_ovl));


//---------------------------------------------------------------------------------
//  TX_MOD_OSC modulation oscillator and SSB weaver modulator

wire          tx_mod_osc_reset_n = rb_pwr_tx_OSC_rst_n & !tx_mod_osc_reset;

wire [ 47: 0] tx_mod_osc_inc_stream = 48'b0;  // not used, yet
wire [ 47: 0] tx_mod_osc_ofs_stream = 48'b0;  // not used, yet

wire [ 47: 0] tx_mod_osc_stream_inc = tx_mod_osc_inc_mux ?  tx_mod_osc_inc_stream :
                                                            tx_mod_osc_inc        ;
wire [ 47: 0] tx_mod_osc_stream_ofs = tx_mod_osc_ofs_mux ?  tx_mod_osc_ofs_stream :
                                                            tx_mod_osc_ofs        ;

wire          tx_mod_osc_axis_s_vld   = tx_mod_osc_reset_n;
wire [103: 0] tx_mod_osc_axis_s_phase = { 7'b0, tx_mod_osc_resync, tx_mod_osc_stream_ofs, tx_mod_osc_stream_inc };

wire          tx_mod_osc_axis_m_vld;
wire [ 31: 0] tx_mod_osc_axis_m_data;

rb_dds_48_16_125 i_rb_tx_mod_osc_dds (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_tx_OSC_clken         ),  // power down on request
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

wire [ 17: 0] tx_mod_qmix_in      =  (tx_muxin_src == 6'h00) ?  18'hffff : tx_mod_adc_out[32:15];  // when ADC source ID is zero take cos() from MOD_OSC only
wire [ 17: 0] tx_mod_qmix_cos_in  = { {2{tx_mod_osc_cos[15]}}, tx_mod_osc_cos[15:0] };  // signed expansion
wire [ 36: 0] tx_mod_qmix_i_s1_out;

rb_dsp48_AaDmBaC_A18_D18_B18_C36_P37 i_rb_tx_mod_qmix_I_s1_dsp48 (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_pwr_tx_OSC_clken         ),  // power down on request

  .A                       ( tx_mod_qmix_in              ),  // TX_MUX in signal:         SIGNED 18 bit
  .D                       ( 18'b0                       ),  // unused                    SIGNED 18 bit
  .B                       ( tx_mod_qmix_cos_in          ),  // TX_MOD_OSC cos:           SIGNED 18 bit
  .C                       ( 36'b0                       ),  // unused                    SIGNED 36 bit

  .P                       ( tx_mod_qmix_i_s1_out        )   // TX_QMIX I output:         SIGSIG 37 bit
);

wire [ 17: 0] tx_mod_qmix_gain_in  = { 2'b0, tx_mod_qmix_gain[15:0] };  // unsigned expansion
wire [ 36: 0] tx_mod_qmix_i_s2_out;

rb_dsp48_AaDmBaC_A18_D18_B18_C36_P37 i_rb_tx_mod_qmix_I_s2_dsp48 (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_pwr_tx_OSC_clken         ),  // power down on request

  .A                       ( tx_mod_qmix_i_s1_out[33:16] ),  // TX_MUX in signal:         SIGNED 18 bit
  .D                       ( 18'b0                       ),  // unused                    SIGNED 18 bit
  .B                       ( tx_mod_qmix_gain_in         ),  // gain setting:             SIGNED 18 bit
  .C                       ( 36'b0                       ),  // unused                    SIGNED 36 bit

  .P                       ( tx_mod_qmix_i_s2_out        )   // TX_QMIX I regulated:      SIGSIG 37 bit
);

wire [ 47: 0] tx_mod_qmix_i_s3_in = tx_car_osc_inc_mux ?  { {8{tx_mod_qmix_i_s2_out[36]}}, tx_mod_qmix_i_s2_out[36:0], 3'b0 } :  /* when FM is used, take finer resolution */
                                                          { tx_mod_qmix_i_s2_out[31:0], 16'b0 }                               ;
wire [ 47: 0] tx_mod_qmix_i_s3_out;

rb_addsub_48M48 i_rb_tx_mod_qmix_I_s3_addsub (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_pwr_tx_OSC_clken         ),  // power down on request

  .ADD                     ( 1'b1                        ),  // ADD
  .A                       ( tx_mod_qmix_i_s3_in         ),  // modulation:               SIGNED 48 bit
  .B                       ( tx_mod_qmix_ofs             ),  // offset:                   SIGNED 48 bit
  .S                       ( tx_mod_qmix_i_s3_out        )   // TX_QMIX I for OSC:        SIGNED 48 bit
);

wire [ 17: 0] tx_mod_qmix_sin_in  = { {2{tx_mod_osc_sin[15]}}, tx_mod_osc_sin[15:0] };  // signed expansion
wire [ 36: 0] tx_mod_qmix_q_s1_out;

rb_dsp48_AaDmBaC_A18_D18_B18_C36_P37 i_rb_tx_mod_qmix_Q_s1_dsp48 (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_pwr_tx_Q_clken           ),  // power down on request

  .A                       ( tx_mod_qmix_in              ),  // MUX in signal:            SIGNED 18 bit
  .D                       ( 18'b0                       ),  // unused                    SIGNED 18 bit
  .B                       ( tx_mod_qmix_sin_in          ),  // MOD_OSC sin:              SIGNED 18 bit
  .C                       ( 36'b0                       ),  // unused                    SIGNED 36 bit

  .P                       ( tx_mod_qmix_q_s1_out        )   // QMIX Q output:            SIGSIG 37 bit
);

wire [ 36: 0] tx_mod_qmix_q_s2_out;

rb_dsp48_AaDmBaC_A18_D18_B18_C36_P37 i_rb_tx_mod_qmix_Q_s2_dsp48 (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_pwr_tx_Q_clken           ),  // power down on request

  .A                       ( tx_mod_qmix_q_s1_out[33:16] ),  // MUX in signal:            SIGNED 18 bit
  .D                       ( 18'b0                       ),  // unused                    SIGNED 18 bit
  .B                       ( tx_mod_qmix_gain_in         ),  // gain setting:             SIGNED 18 bit
  .C                       ( 36'b0                       ),  // unused                    SIGNED 36 bit

  .P                       ( tx_mod_qmix_q_s2_out        )   // TX_QMIX Q regulated:      SIGSIG 37 bit
);

wire [ 47: 0] tx_mod_qmix_q_s3_in = { tx_mod_qmix_q_s2_out[31:0], 16'b0 };
wire [ 47: 0] tx_mod_qmix_q_s3_out;

rb_addsub_48M48 i_rb_tx_mod_qmix_Q_s3_addsub (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_pwr_tx_Q_clken           ),  // power down on request

  .ADD                     ( 1'b1                        ),  // ADD
  .A                       ( tx_mod_qmix_q_s3_in         ),  // modulation:               SIGNED 48 bit
  .B                       ( tx_mod_qmix_ofs             ),  // offset:                   SIGNED 48 bit
  .S                       ( tx_mod_qmix_q_s3_out        )   // TX_QMIX Q for OSC:        SIGNED 48 bit
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

rb_cic_48k_to_8k_18T18 i_rb_tx_mod_I_cic (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_tx_I_clken           ),  // power down on request
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

rb_cic_48k_to_8k_18T18 i_rb_tx_mod_Q_cic (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_tx_Q_clken           ),  // power down on request
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

wire [ 23: 0] tx_mod_fir_i_in = { 7'b0, tx_mod_cic_i_out[17:1] };
wire [ 39: 0] tx_mod_fir_i_out;
wire          tx_mod_fir_i_out_vld;
wire          tx_mod_fir_i_out_rdy;

rb_fir_8k_to_8k_25c23_17i16_35o33 i_rb_tx_mod_I_fir (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_tx_I_clken           ),  // power down on request
  .aresetn                 ( rb_pwr_tx_I_rst_n           ),

  .s_axis_data_tdata       ( tx_mod_fir_i_in             ),  // TX_MOD_CIC output I - 8 kHz (17.16 bit width)
  .s_axis_data_tvalid      ( tx_mod_cic_i_out_vld        ),
  .s_axis_data_tready      ( tx_mod_cic_i_out_rdy        ),

  .m_axis_data_tdata       ( tx_mod_fir_i_out            ),  // TX_MOD_FIR output I - 8kHz (35.33 bit width)
  .m_axis_data_tvalid      ( tx_mod_fir_i_out_vld        ),
  .m_axis_data_tready      ( tx_mod_fir_i_out_rdy        )
);

wire [ 23: 0] tx_mod_fir_q_in = { 7'b0, tx_mod_cic_q_out[17:1] };
wire [ 39: 0] tx_mod_fir_q_out;
wire          tx_mod_fir_q_out_vld;
wire          tx_mod_fir_q_out_rdy;

rb_fir_8k_to_8k_25c23_17i16_35o33 i_rb_tx_mod_Q_fir (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_tx_Q_clken           ),  // power down on request
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

wire [ 23: 0] tx_car_cic_41M664_i_in = { 6'b0, tx_mod_fir_i_out[33:16] };  // AXIS word expansion
wire [ 23: 0] tx_car_cic_41M664_i_out;
wire          tx_car_cic_41M664_i_out_vld;

rb_cic_8k_to_41M664_18T18 i_rb_tx_car_I_cic (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_tx_I_clken           ),  // power down on request
  .aresetn                 ( rb_pwr_tx_I_rst_n           ),

  .s_axis_data_tdata       ( tx_car_cic_41M664_i_in      ),  // TX_MOD_FIR I - 8 kHz
  .s_axis_data_tvalid      ( tx_mod_fir_i_out_vld        ),
  .s_axis_data_tready      ( tx_mod_fir_i_out_rdy        ),

  .m_axis_data_tdata       ( tx_car_cic_41M664_i_out     ),  // TX_CAR_CIC I stage 1 output - 41.664 MHz
  .m_axis_data_tvalid      ( tx_car_cic_41M664_i_out_vld )
);

wire [ 23: 0] tx_car_cic_41M664_q_in = { 6'b0, tx_mod_fir_q_out[33:16] };  // AXIS word expansion
wire [ 23: 0] tx_car_cic_41M664_q_out;
wire          tx_car_cic_41M664_q_out_vld;

rb_cic_8k_to_41M664_18T18 i_rb_tx_car_Q_cic (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_tx_Q_clken           ),  // power down on request
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
//  TX_CAR_OSC_INC_ACCU

wire [ 47: 0] tx_car_osc_inc_next;

rb_addsub_48M48 i_rb_tx_osc_inc_accu_addsub (
  // global signals
  .CLK                  ( clk_adc_125mhz            ),  // global 125 MHz clock
  .CE                   ( rb_clk_en                 ),  // power down on request

  .ADD                  ( 1'b1                      ),  // ADD
  .A                    ( tx_car_osc_inc            ),  // current TX_CAR_OSC_INC
  .B                    ( tx_car_osc_inc_scanner    ),  // scanner increment @200 kHz for TX_CAR_OSC_INC
  .S                    ( tx_car_osc_inc_next       )   // next TX_CAR_OSC_INC value when not overwritten by direct access
);


//---------------------------------------------------------------------------------
//  TX_CAR_OSC carrier frequency oscillator  (CW, FM, PM modulated)
wire          tx_car_osc_reset_n      = rb_pwr_tx_OSC_rst_n & !tx_car_osc_reset;

wire [ 47: 0] tx_car_osc_stream_inc   = tx_car_osc_inc_mux ?    tx_mod_qmix_i_s3_out[47:0]         :
                                                                tx_car_osc_inc                     ;
wire [ 47: 0] tx_car_osc_stream_ofs   = tx_car_osc_ofs_mux ?    tx_mod_qmix_i_s3_out[47:0]         :
                                                                tx_car_osc_ofs                     ;
wire          tx_car_osc_axis_s_vld   = tx_car_osc_reset_n;
wire [103: 0] tx_car_osc_axis_s_phase = { 7'b0, tx_car_osc_resync, tx_car_osc_stream_ofs, tx_car_osc_stream_inc };

wire          tx_car_osc_axis_m_vld;
wire [ 31: 0] tx_car_osc_axis_m_data;

rb_dds_48_16_125 i_rb_tx_car_osc_dds (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_tx_OSC_clken         ),  // power down on request
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

wire [ 17: 0] tx_car_qmix_i_in = (tx_car_osc_inc_mux || tx_car_osc_ofs_mux)                       ?  18'h7fff                                                       :  // FM and PM
                                  rb_pwr_tx_Q_rst_n                                               ?  { {2{tx_car_regs_i_data[15]}}, tx_car_regs_i_data[15:0] }      :  // SSB
                                                                                                     { {2{tx_mod_qmix_i_s3_out[47]}}, tx_mod_qmix_i_s3_out[47:32] } ;  // AM
wire [ 17: 0] tx_car_qmix_cos_in  = { {2{tx_car_osc_cos[15]}}, tx_car_osc_cos[15:0] };  // signed expansion
wire [ 36: 0] tx_car_qmix_i_out;

rb_dsp48_AaDmBaC_A18_D18_B18_C36_P37 i_rb_tx_car_qmix_I_dsp48 (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_pwr_tx_OSC_clken         ),  // power down on request

  .A                       ( tx_car_qmix_i_in            ),  // TX_MUX in signal:         SIGNED 18 bit
  .D                       ( 18'b0                       ),  // unused                    SIGNED 18 bit
  .B                       ( tx_car_qmix_cos_in          ),  // TX_CAR_OSC cos:           SIGNED 18 bit
  .C                       ( 36'b0                       ),  // unused                    SIGNED 36 bit

  .P                       ( tx_car_qmix_i_out           )   // TX_CAR_QMIX I output:     SIGSIG 37 bit
);

wire [ 17: 0] tx_car_qmix_q_in = (tx_car_osc_inc_mux || tx_car_osc_ofs_mux || !rb_pwr_tx_Q_rst_n) ?  18'h0000                                                       :  // FM, PM and AM
                                                                                                     { {2{tx_car_regs_q_data[15]}}, tx_car_regs_q_data[15:0] }      ;  // TX_MOD_QMIX/TX_MOD_CIC Q path keep quiet when Q is disabled - SSB uses TX_CIC Q instead
wire [ 17: 0] tx_car_qmix_sin_in  = { {2{tx_car_osc_sin[15]}}, tx_car_osc_sin[15:0] };  // signed expansion
wire [ 36: 0] tx_car_qmix_q_out;

rb_dsp48_AaDmBaC_A18_D18_B18_C36_P37 i_rb_tx_car_qmix_Q_dsp48 (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_pwr_tx_OSC_clken         ),  // power down on request

  .A                       ( tx_car_qmix_q_in            ),  // TX_MUX in signal:         SIGNED 18 bit
  .D                       ( 18'b0                       ),  // unused                    SIGNED 18 bit
  .B                       ( tx_car_qmix_sin_in          ),  // TX_CAR_OSC sin:           SIGNED 18 bit
  .C                       ( 36'b0                       ),  // unused                    SIGNED 36 bit

  .P                       ( tx_car_qmix_q_out           )   // TX_CAR_QMIX Q output:     SIGSIG 37 bit
);


//---------------------------------------------------------------------------------
//  TX_RF_AMP amplifier for the radio frequency output (CW, AM modulated)

wire [ 17: 0] tx_amp_rf_i_var =                      tx_car_qmix_i_out[32:15]    ;                          // halfed and sign corrected expansion
wire [ 17: 0] tx_amp_rf_q_var = rb_pwr_tx_Q_rst_n ?  tx_car_qmix_q_out[32:15]    :
                                                     18'b0                       ;                          // halfed and sign corrected expansion
wire [ 17: 0] tx_amp_rf_gain  = { 1'b0, tx_rf_amp_gain[15:0], 1'b0 };                                       // unsigned register value
wire [ 35: 0] tx_amp_rf_ofs   = { tx_rf_amp_ofs[15], tx_rf_amp_ofs[14:0], 20'b0 };                          //   signed register value
wire [ 36: 0] tx_amp_rf_out;

rb_dsp48_AaDmBaC_A18_D18_B18_C36_P37 i_rb_tx_amp_rf_dsp48 (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_pwr_tx_OSC_clken         ),  // power down on request

  .A                       ( tx_amp_rf_i_var             ),  // TX_QMIX_RF I      SIGNED 18 bit
  .D                       ( tx_amp_rf_q_var             ),  // TX_QMIX_RF Q      SIGNED 18 bit
  .B                       ( tx_amp_rf_gain              ),  // TX_RF_AMP gain    SIGNED 18 bit
  .C                       ( tx_amp_rf_ofs               ),  // TX_RF_AMP ofs     SIGSIG 36 bit

  .P                       ( tx_amp_rf_out               )   // TX_AMP RF output  SIGSIG 37 bit
);


// === Receiver section ===
// === RX_CAR section ===

//---------------------------------------------------------------------------------
//  RX_MUXIN amplifier

wire   signed [ 15: 0] rx_muxin_sig = (rx_muxin_src == 6'h20) ?  adc_enhanced[RB_ADC_AUTO_OFS_RFIN1]   :
                                      (rx_muxin_src == 6'h21) ?  adc_enhanced[RB_ADC_AUTO_OFS_RFIN2]   :
                                      (rx_muxin_src == 6'h18) ?  adc_enhanced[RB_ADC_AUTO_OFS_EXT_CH0] :  // swapped here due to pin connection warnings when swapping @ XADC <--> pins
                                      (rx_muxin_src == 6'h10) ?  adc_enhanced[RB_ADC_AUTO_OFS_EXT_CH8] :
                                      (rx_muxin_src == 6'h11) ?  adc_enhanced[RB_ADC_AUTO_OFS_EXT_CH1] :
                                      (rx_muxin_src == 6'h19) ?  adc_enhanced[RB_ADC_AUTO_OFS_EXT_CH9] :
                                      (rx_muxin_src == 6'h03) ?  adc_enhanced[RB_ADC_AUTO_OFS_VpVn]    :
                                      (rx_muxin_src == 6'h30) ?  rb_line_out_i[15: 0]                  :
                                      (rx_muxin_src == 6'h31) ?  rb_line_out_i[31:16]                  :
                                      16'b0                                                            ;
wire   signed [ 47: 0] rx_muxin_sig_in = { {3{rx_muxin_sig[15]}},     rx_muxin_sig[14:0],     30'b0 };      // signed expansion
wire   signed [ 47: 0] rx_muxin_ofs_in = { {3{rx_muxin_mix_ofs[15]}}, rx_muxin_mix_ofs[14:0], 30'b0 };
wire   signed [ 47: 0] rx_muxin_biased_out;

rb_addsub_48M48 i_rb_rx_muxin_offset_bias_addsub (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_pwr_rx_CAR_clken         ),  // power down on request

  .ADD                     ( 1'b0                        ),  // SUBTRACT
  .A                       ( rx_muxin_sig_in             ),  // ADC raw value           SIGNED 48 bit
  .B                       ( rx_muxin_ofs_in             ),  // ADC offset value        SIGNED 48 bit
  .S                       ( rx_muxin_biased_out         )   // biased output           SIGNED 48 bit
);

wire   signed [ 17: 0] rx_muxin_mix_in = (rx_muxin_biased_out[47:30] << rx_muxin_mix_log2);                 // unsigned value: input booster for
                                                                                                            // factor: 1x .. 2^3=7 shift postions=128x (16 mV --> full-scale)
wire   signed [ 17: 0] rx_muxin_mix_gain_in = { 2'b0, rx_muxin_mix_gain[15:0] };
wire   signed [ 36: 0] rx_muxin_mix_out;

rb_dsp48_AaDmBaC_A18_D18_B18_C36_P37 i_rb_rx_muxin_amplifier_dsp48 (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_pwr_rx_CAR_clken         ),  // power down on request

  .A                       ( rx_muxin_mix_in             ),  // input signal              SIGNED 18 bit
  .D                       ( 18'b0                       ),  // unused                    SIGNED 18 bit
  .B                       ( rx_muxin_mix_gain_in        ),  // RX amplifier gain         SIGNED 18 bit
  .C                       ( 36'b0                       ),  // unused                    SIGNED 36 bit

  .P                       ( rx_muxin_mix_out            )   // RX level adj. input       SIGSIG 37 bit
);

wire          [ 17: 0] rx_muxin_out = rx_muxin_mix_out[30:13];

assign rb_overdrive_rx_muxin = (!rx_muxin_mix_out[36] && (| rx_muxin_mix_out[35:28])) || (rx_muxin_mix_out[36] && !(& rx_muxin_mix_out[35:28]));


//---------------------------------------------------------------------------------
//  RX_CAR_OSC_INC_ACCU

wire [ 47: 0] rx_car_osc_inc_next;

rb_addsub_48M48 i_rb_rx_car_osc_inc_accu_addsub (
  // global signals
  .CLK                  ( clk_adc_125mhz            ),  // global 125 MHz clock
  .CE                   ( rb_clk_en                 ),  // power down on request

  .ADD                  ( 1'b1                      ),  // ADD
  .A                    ( rx_car_osc_inc            ),  // current RX_CAR_OSC_INC
  .B                    ( rx_car_osc_inc_scanner    ),  // scanner increment @200 kHz for RX_CAR_OSC_INC
  .S                    ( rx_car_osc_inc_next       )   // next RX_CAR_OSC_INC value when not overwritten by direct access
);


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

rb_dds_48_16_125 i_rb_rx_car_osc_dds (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_rx_CAR_clken         ),  // power down on request
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

wire [ 17: 0] rx_car_qmix_in     = rx_muxin_out[17:0];
wire [ 17: 0] rx_car_qmix_cos_in = { {2{rx_car_osc_cos[15]}}, rx_car_osc_cos[15:0] };
wire [ 36: 0] rx_car_qmix_i_out;

rb_dsp48_AaDmBaC_A18_D18_B18_C36_P37 i_rb_rx_car_qmix_I_dsp48 (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_pwr_rx_CAR_clken         ),  // power down on request

  .A                       ( rx_car_qmix_in              ),  // RX_MUX in signal:         SIGNED 18 bit
  .D                       ( 18'b0                       ),  // unused                    SIGNED 18 bit
  .B                       ( rx_car_qmix_cos_in          ),  // RX_CAR_OSC cos:           SIGNED 18 bit
  .C                       ( 36'b0                       ),  // unused                    SIGNED 36 bit

  .P                       ( rx_car_qmix_i_out           )   // RX_CAR_QMIX I output:     SIGSIG 37 bit
);

wire [ 17: 0] rx_car_qmix_sin_in = { {2{rx_car_osc_sin[15]}}, rx_car_osc_sin[15:0] };
wire [ 36: 0] rx_car_qmix_q_out;

rb_dsp48_AaDmBaC_A18_D18_B18_C36_P37 i_rb_rx_car_qmix_Q_dsp48 (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_pwr_rx_CAR_clken         ),  // power down on request

  .A                       ( rx_car_qmix_in              ),  // RX_MUX in signal:         SIGNED 18 bit
  .D                       ( 18'b0                       ),  // unused                    SIGNED 18 bit
  .B                       ( rx_car_qmix_sin_in          ),  // RX_CAR_OSC sin:           SIGNED 18 bit
  .C                       ( 36'b0                       ),  // unused                    SIGNED 36 bit

  .P                       ( rx_car_qmix_q_out           )   // RX_CAR_QMIX Q output:     SIGSIG 37 bit
);


//---------------------------------------------------------------------------------
//  RX_CAR_CIC1 sampling rate down convertion 125 MSPS to 5 MSPS

wire [ 23: 0] rx_car_cic1_i_in = { 6'b0, rx_car_qmix_i_out[31:14] };  // AXIS word expansion
wire [ 23: 0] rx_car_cic1_i_out;
wire          rx_car_cic1_i_out_vld;
wire          rx_car_cic1_i_out_rdy;

rb_cic_125M_to_5M_18T18 i_rb_rx_car_I1_cic (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_rx_CAR_clken         ),  // power down on request
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

rb_cic_125M_to_5M_18T18 i_rb_rx_car_Q1_cic (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_rx_CAR_clken         ),  // power down on request
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

wire [ 23: 0] rx_car_cic2_i_in = { 6'b0, rx_car_cic1_i_out[16:0], 1'b0 };  // AXIS word expansion
wire [ 23: 0] rx_car_cic2_i_out;
wire          rx_car_cic2_i_out_vld;

rb_cic_5M_to_200k_18T18 i_rb_rx_car_I2_cic (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_rx_CAR_clken         ),  // power down on request
  .aresetn                 ( rb_pwr_rx_CAR_rst_n         ),

  .s_axis_data_tdata       ( rx_car_cic2_i_in            ),
  .s_axis_data_tvalid      ( rx_car_cic1_i_out_vld       ),
  .s_axis_data_tready      ( rx_car_cic1_i_out_rdy       ),

  .m_axis_data_tdata       ( rx_car_cic2_i_out           ),  // RX_CAR_CIC2 output I
  .m_axis_data_tvalid      ( rx_car_cic2_i_out_vld       )
);

wire [ 23: 0] rx_car_cic2_q_in = { 6'b0, rx_car_cic1_q_out[16:0], 1'b0 };  // AXIS word expansion
wire [ 23: 0] rx_car_cic2_q_out;
wire          rx_car_cic2_q_out_vld;

rb_cic_5M_to_200k_18T18 i_rb_rx_car_Q2_cic (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_rx_CAR_clken         ),  // power down on request
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

wire unsigned [ 23: 0] rx_mod_cic1_i_in      = { 6'b0, rx_car_regs2_i_data[17:0] };  // AXIS word expansion
wire unsigned [ 23: 0] rx_mod_cic1_i_out;
wire                   rx_mod_cic1_i_out_vld;

rb_cic_200k_to_8k_18T18 i_rb_rx_mod_I1_cic (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_rx_MOD_clken         ),  // power down on request
  .aresetn                 ( rb_pwr_rx_MOD_rst_n         ),

  .s_axis_data_tdata       ( rx_mod_cic1_i_in            ),
  .s_axis_data_tvalid      ( rx_car_regs2_i_mod_vld      ),
  .s_axis_data_tready      ( rx_car_regs2_i_mod_rdy      ),

  .m_axis_data_tdata       ( rx_mod_cic1_i_out           ),  // RX_MOD_CIC1 output I
  .m_axis_data_tvalid      ( rx_mod_cic1_i_out_vld       )
);

wire unsigned [ 23: 0] rx_mod_cic1_q_in      = { 6'b0, rx_car_regs2_q_data[17:0] };  // AXIS word expansion
wire unsigned [ 23: 0] rx_mod_cic1_q_out;
wire                   rx_mod_cic1_q_out_vld;

rb_cic_200k_to_8k_18T18 i_rb_rx_mod_Q1_cic (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_rx_MOD_clken         ),  // power down on request
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

rb_fir1_8k_to_8k_25c_17i16_35o32 i_rb_rx_mod_I2_fir (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_rx_MOD_clken         ),  // power down on request
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

rb_fir1_8k_to_8k_25c_17i16_35o32 i_rb_rx_mod_Q2_fir (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_rx_MOD_clken         ),  // power down on request
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
                                              rx_mod_fir2_i_out[33:18] ;  // drive by factor 4 when modulation is AM-Sync, FM, PM, AM-Env
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
                                              rx_mod_fir2_q_out[33:18] ;  // drive by factor 4 when modulation is AM-Sync, FM, PM, AM-Env
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

rb_dds_48_16_125 i_rb_rx_mod_osc_dds (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_rx_MOD_clken         ),  // power down on request
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

wire [ 17: 0] rx_mod_qmix_i_in  = { {2{rx_mod_regs2_i_data[15]}}, rx_mod_regs2_i_data[15:0] };  // signed expansion
wire [ 17: 0] rx_mod_qmix_i_hld = { {2{rx_mod_hld_i_data[15]}},   rx_mod_hld_i_data[15:0]   };
wire [ 36: 0] rx_mod_qmix_i_out;

rb_dsp48_AaDmBaC_A18_D18_B18_C36_P37 i_rb_rx_mod_qmix_I_dsp48 (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_pwr_rx_MOD_clken         ),  // power down on request

  .A                       ( rx_mod_qmix_i_in            ),  // RX_MOD_FIR2 in I sig.:    SIGNED 18 bit
  .D                       ( 18'b0                       ),  // unused                    SIGNED 18 bit
  .B                       ( rx_mod_qmix_i_hld           ),  // RX_MOD_OSC cos:           SIGNED 18 bit
  .C                       ( 36'b0                       ),  // unused                    SIGNED 36 bit

  .P                       ( rx_mod_qmix_i_out           )   // RX_MOD_QMIX I output:     SIGSIG 37 bit
);

wire [ 17: 0] rx_mod_qmix_q_in  = { {2{rx_mod_regs2_q_data[15]}}, rx_mod_regs2_q_data[15:0] };  // signed expansion
wire [ 17: 0] rx_mod_qmix_q_hld = { {2{rx_mod_hld_q_data[15]}},   rx_mod_hld_q_data[15:0]   };  // signed expansion
wire [ 36: 0] rx_mod_qmix_q_out;

rb_dsp48_AaDmBaC_A18_D18_B18_C36_P37 i_rb_rx_mod_qmix_Q_dsp48 (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_pwr_rx_MOD_clken         ),  // power down on request

  .A                       ( rx_mod_qmix_q_in            ),  // RX_MOD_FIR2 in Q sig.:    SIGNED 18 bit
  .D                       ( 18'b0                       ),  // unused                    SIGNED 18 bit
  .B                       ( rx_mod_qmix_q_hld           ),  // RX_MOD_OSC sin:           SIGNED 18 bit
  .C                       ( 36'b0                       ),  // unused                    SIGNED 36 bit

  .P                       ( rx_mod_qmix_q_out           )   // RX_MOD_QMIX I output:     SIGSIG 37 bit
);


//---------------------------------------------------------------------------------
//  RX_MOD_FIR3 low pass filter for weaver upper/lower side-band selection
//
//  Coefficients built with Octave:
//  hn=fir2(126, [0/4000 900/4000 1700/4000 2500/4000 3300/4000  3350/4000 1], [1 1.5 2.25 3.375 5  0.000001 0.000001], 4096, kaiser(127, 4.5)); freqz(hn);

wire [ 23: 0] rx_mod_fir3_i_in = { 7'b0, rx_mod_qmix_i_out[30:14] };  // AXIS word expansion
wire [ 39: 0] rx_mod_fir3_i_out;
wire          rx_mod_fir3_i_out_vld;

rb_fir2_8k_to_8k_25c_17i16_35o32 i_rb_rx_mod_I3_fir (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_rx_MOD_clken         ),  // power down on request
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

rb_fir2_8k_to_8k_25c_17i16_35o32 i_rb_rx_mod_Q3_fir (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_rx_MOD_clken         ),  // power down on request
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

rb_cic_8k_to_48k_18T18 i_rb_rx_mod_I4_cic (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_rx_MOD_clken         ),  // power down on request
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

rb_cic_8k_to_48k_18T18 i_rb_rx_mod_Q4_cic (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_rx_MOD_clken         ),  // power down on request
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
      rx_mod_cic4_chk_rst_cnt    <= rx_mod_cic4_chk_rst_cnt - 1;
      end


//---------------------------------------------------------------------------------
//  RX_MOD_SSB_ADD reconstruction of the modulation

wire [ 17: 0] rx_mod_ssb_am_i_var = rx_mod_cic4_i_out[17:0];
wire [ 17: 0] rx_mod_ssb_am_q_var = rx_mod_cic4_q_out[17:0];
wire [ 17: 0] rx_mod_ssb_am_g_var = { 1'b0, rx_ssb_am_gain[15:0], 1'b0 };  // expand unsigned 16 bit to signed 18 bit. Amp: 0x..2x

wire [ 36: 0] rx_mod_ssb_mix_out;

rb_dsp48_AaDmBaC_A18_D18_B18_C36_P37 i_rb_rx_mod_ssb_am_dsp48 (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_pwr_rx_MOD_clken         ),  // power down on request

  .A                       ( rx_mod_ssb_am_i_var         ),  // RX_MOD_CIC4 I     SIGNED 18 bit
  .D                       ( rx_mod_ssb_am_q_var         ),  // RX_MOD_CIC4 Q     SIGNED 18 bit
  .B                       ( rx_mod_ssb_am_g_var         ),  // RX_MOD SSB gain   SIGNED 18 bit
  .C                       ( 36'b0                       ),  // no offset value specified

  .P                       ( rx_mod_ssb_mix_out          )   // RX_MOD SSB output SIGSIG 37 bit
);

wire [ 15: 0] rx_mod_ssb_am_out = rx_mod_ssb_mix_out[32:17];


// === RX_AFC section ===

//---------------------------------------------------------------------------------
//  RX_AFC_FIR low pass filter for carrier detection - @ 200 kSPS
//
//  Coefficients built with Octave:
//  0 = Set 1: AM-Sync (SSB) - band pass for 1700 Hz with abt. 440 Hz @ -40 dB band width
//  hn=fir2(253, [0 1660/1000000 1680/1000000 1700/1000000 1720/1000000 1740/1000000 1], [0.000000001 0.00001 0.001 114 0.001 0.00001 0.000000001], 65536, kaiser(254, 0.01)); freqz(hn);
//
//  1 = Set 2: FM / PM - low pass with abt. 2.56 kHz @ -6 dB
//  hn=fir1(254, 50000/200000, 'low', 'barthannwin'); freqz(hn);
//
//  2 = Set 3: AM-Env wide - low pass
//  hn=fir1(254, 8500/200000, 'low', 'barthannwin'); freqz(hn);
//
//  3= Set 4: AM-Env mid - low pass
//  hn=fir1(254, 6500/200000, 'low', 'barthannwin'); freqz(hn);
//
//  4 = Set 5: AM-Env narrow - low pass
//  hn=fir1(254, 4500/200000, 'low', 'barthannwin'); freqz(hn);

wire unsigned [  7: 0] rx_afc_fir_cfg_in    = ((rb_pwr_rx_modvar == RB_PWR_CTRL_RX_MOD_FM)  ||
                                               (rb_pwr_rx_modvar == RB_PWR_CTRL_RX_MOD_PM))    ?   8'd1                         :
                                               (rb_pwr_rx_modvar == RB_PWR_CTRL_RX_MOD_AM_ENV) ?  (8'd2 + rx_afc_amenv_filtvar) :
                                                                                                   8'd0                         ;
wire   signed [ 23: 0] rx_afc_fir_i_in      = { 7'b0, rx_car_regs2_i_data[17:1] };  // AXIS word expansion
wire unsigned [ 39: 0] rx_afc_fir_i_out;
wire                   rx_afc_fir_i_out_vld;

rb_fir3_200k_to_200k_24c_17i16_35o i_rb_rx_afc_I_fir (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_rx_AFC_clken         ),  // power down on request
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

wire   signed [ 23: 0] rx_afc_fir_q_in      = { 7'b0, rx_car_regs2_q_data[17:1] };                          // AXIS word expansion
wire unsigned [ 39: 0] rx_afc_fir_q_out;
wire                   rx_afc_fir_q_out_vld;

rb_fir3_200k_to_200k_24c_17i16_35o i_rb_rx_afc_Q_fir (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_rx_AFC_clken         ),  // power down on request
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

reg  [ 17: 0] rx_afc_fir_i_reg  =  'b0;
reg           rx_afc_fsm1_i_new = 1'b0;

always @(posedge clk_adc_125mhz)                                                                            // input register I
if (!rb_pwr_rx_AFC_rst_n) begin
   rx_afc_fir_i_reg  <=  'b0;
   rx_afc_fsm1_i_new <= 1'b0;
   end
else if (rx_afc_fir_i_out_vld) begin
   rx_afc_fir_i_reg  <= rx_afc_fir_i_out[32:15];
   rx_afc_fsm1_i_new <= 1'b1;
   end
else if (rx_afc_fsm1_ctr)
   rx_afc_fsm1_i_new <= 1'b0;

reg  [ 17: 0] rx_afc_fir_q_reg  =  'b0;
reg           rx_afc_fsm1_q_new = 1'b0;

always @(posedge clk_adc_125mhz)                                                                            // input register Q
if (!rb_pwr_rx_AFC_rst_n) begin
   rx_afc_fir_q_reg  <=  'b0;
   rx_afc_fsm1_q_new <= 1'b0;
   end
else if (rx_afc_fir_q_out_vld) begin
   rx_afc_fir_q_reg <= rx_afc_fir_q_out[32:15];
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
else if (!rx_afc_fsm1_ctr && (rx_afc_fsm1_i_new && rx_afc_fsm1_q_new && clk_200khz))                        // new data is available
   rx_afc_fsm1_ctr <= 2'd3;
else if (rx_afc_fsm1_ctr) begin
   rx_afc_cordic_cart_vld <= 1'b1;
   if (rx_afc_cordic_cart_rdy)                                                                              // data output handshake
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
//  MAGNITUDE data format: sign + 1 bit integers . fractions ==> sign = rx_afc_cordic_polar_out_mag[17], integer = rx_afc_cordic_polar_out_mag[16], fractions = rx_afc_cordic_polar_out_mag[15:0]
//
//  CORDIC:     -1 <= PHASE <= 1
//  PHASE data format: sign + 2 bit integers . fractions ==> sign = rx_afc_cordic_polar_out_phs[17], integer = rx_afc_cordic_polar_out_phs[16:15], fractions = rx_afc_cordic_polar_out_phs[14:0]

wire          [ 47: 0] rx_afc_cordic_cart_in = { 6'b0, ~rx_afc_fir_q_reg[17:0],  6'b0, rx_afc_fir_i_reg[17:0] };

wire          [ 47: 0] rx_afc_cordic_polar_out;
wire                   rx_afc_cordic_polar_out_vld;

rb_cordic_T_WS_O_SR_18T18_NE_CR_EM_B i_rb_rx_afc_cordic (
  // global signals
  .aclk                    ( clk_adc_125mhz              ),  // global 125 MHz clock
  .aclken                  ( rb_pwr_rx_AFC_clken         ),  // power down on request
  .aresetn                 ( rb_pwr_rx_AFC_rst_n         ),

  .s_axis_cartesian_tdata  ( rx_afc_cordic_cart_in       ),
  .s_axis_cartesian_tvalid ( rx_afc_cordic_cart_vld      ),
  .s_axis_cartesian_tready ( rx_afc_cordic_cart_rdy      ),

  .m_axis_dout_tdata       ( rx_afc_cordic_polar_out     ),
  .m_axis_dout_tvalid      ( rx_afc_cordic_polar_out_vld )
);

wire unsigned [ 15: 0] rx_afc_cordic_polar_out_mag =   rx_afc_cordic_polar_out[16:1];
wire   signed [ 15: 0] rx_afc_cordic_polar_out_phs = { rx_afc_cordic_polar_out[24+17], rx_afc_cordic_polar_out[24+14:24+0] };  // -0.999 .. +0.999 represents -180� .. +180�


always @(posedge clk_adc_125mhz)
if (!rb_pwr_rx_AFC_rst_n) begin
   regs[REG_RD_RB_RX_AFC_CORDIC_PHS_PREV] <= 32'h00000000;
   regs[REG_RD_RB_RX_AFC_CORDIC_MAG]      <= 32'h00000000;
   regs[REG_RD_RB_RX_AFC_CORDIC_PHS]      <= 32'h00000000;
   end
else if (rx_afc_cordic_dly_pulse) begin
   regs[REG_RD_RB_RX_AFC_CORDIC_PHS_PREV] <= regs[REG_RD_RB_RX_AFC_CORDIC_PHS];
   regs[REG_RD_RB_RX_AFC_CORDIC_MAG]      <= { 16'b0, rx_afc_cordic_polar_out_mag[15:0] };
   regs[REG_RD_RB_RX_AFC_CORDIC_PHS]      <= { {17{rx_afc_cordic_polar_out_phs[15]}}, rx_afc_cordic_polar_out_phs[14:0] };
   end


//---------------------------------------------------------------------------------
//  RX_AFC_CALC_PHASE_SUB

wire   signed [ 47: 0] rx_afc_cordic_phs_in      = { rx_afc_cordic_phs[15:0],      32'b0 };
wire   signed [ 47: 0] rx_afc_cordic_phs_prev_in = { rx_afc_cordic_phs_prev[15:0], 32'b0 };

wire   signed [ 47: 0] rx_afc_calc_phs_wvr_diff;

rb_addsub_48M48 i_rb_rx_afc_calc_phase_addsub (
  // global signals
  .CLK                  ( clk_adc_125mhz            ),  // global 125 MHz clock
  .CE                   ( rb_pwr_rx_AFC_clken       ),  // power down on request

  .ADD                  ( 1'b0                      ),  // SUBTRACT
  .A                    ( rx_afc_cordic_phs_in      ),  // current CORDIC phase           SIGNED 48 bit
  .B                    ( rx_afc_cordic_phs_prev_in ),  // previous CORDIC phase          SIGNED 48 bit
  .S                    ( rx_afc_calc_phs_wvr_diff  )   // phase difference               SIGNED 48 bit
);

wire   signed [ 47: 0] rx_afc_calc_phs_out;

rb_addsub_48M48 i_rb_rx_afc_calc_weaver_addsub (
  // global signals
  .CLK                  ( clk_adc_125mhz            ),  // global 125 MHz clock
  .CE                   ( rb_pwr_rx_AFC_clken       ),  // power down on request

  .ADD                  ( 1'b0                      ),  // SUBTRACT
  .A                    ( rx_afc_calc_phs_wvr_diff  ),  // phase difference               SIGNED 48 bit
  .B                    ( rx_afc_calc_weaver_inc    ),  // weaver frequency               SIGNED 48 bit
  .S                    ( rx_afc_calc_phs_out       )   // corrected phase                SIGNED 48 bit
);

always @(posedge clk_adc_125mhz)
if (!rb_pwr_rx_AFC_rst_n)
   regs[REG_RD_RB_RX_AFC_CORDIC_PHS_DIFF] <= 32'b0;
else
   regs[REG_RD_RB_RX_AFC_CORDIC_PHS_DIFF] <= rx_afc_calc_phs_out[47:16];                                    // =: rx_afc_cordic_phs_diff


//---------------------------------------------------------------------------------
//  RX_AFC_CALC_DIV

wire  signed [ 47: 0] rx_afc_div_afc_quot = { {28{rx_afc_calc_phs_out[47]}}, rx_afc_calc_phs_out[46:27] };


//---------------------------------------------------------------------------------
//  RX_AFC_CALC_AFCREG

wire   signed [ 47: 0] rx_afc_reg_inc_out;

rb_addsub_48M48 i_rb_rx_afc_calc_add_inc_addsub (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_pwr_rx_AFC_clken         ),  // power down on request

  .ADD                     ( 1'b1                        ),  // ADD
  .A                       ( rx_car_afc_inc              ),  // current AFC increment     SIGNED 48 bit
  .B                       ( rx_afc_div_afc_quot         ),  // AFC correction            SIGNED 48 bit
  .S                       ( rx_afc_reg_inc_out          )   // next AFC increment        SIGNED 48 bit
);

reg                    rx_afc_addsub_afc_pulse = 1'b0;

always @(posedge clk_adc_125mhz)
if (!rb_pwr_rx_AFC_rst_n)
   rx_afc_addsub_afc_pulse <= 1'b0;
else
   rx_afc_addsub_afc_pulse <= rx_afc_cordic_dly_pulse;                                                      // delayed by one clock

reg                    rx_afc_high_sig = 1'b0;

always @(posedge clk_adc_125mhz)
if (!rb_pwr_rx_AFC_rst_n) begin
   { regs[REG_RD_RB_RX_CAR_AFC_INC_HI][15:0], regs[REG_RD_RB_RX_CAR_AFC_INC_LO][31:0] } <= 48'b0;
   rx_afc_high_sig <= 1'b0;
   end
else if (rx_afc_addsub_afc_pulse)
   if (rx_afc_cordic_polar_out_mag[15:10] != 6'b000000) begin                                               // XXX change value for AFC sensitivness
      if (rx_afc_high_sig)                                                                                  // correct frequency only when phase monitoring is established
         { regs[REG_RD_RB_RX_CAR_AFC_INC_HI][15:0], regs[REG_RD_RB_RX_CAR_AFC_INC_LO][31:0] } <= rx_afc_reg_inc_out;
      rx_afc_high_sig <= 1'b1;                                                                              // delay assignment by one rx_afc_addsub_afc_pulse (200 kHz)
      end
   else begin                                                                                               // low signal, reset to center frequency
      { regs[REG_RD_RB_RX_CAR_AFC_INC_HI][15:0], regs[REG_RD_RB_RX_CAR_AFC_INC_LO][31:0] } <= 48'b0;
      rx_afc_high_sig <= 1'b0;
      end


//---------------------------------------------------------------------------------
//  RX_AFC_CALC_SUMREG

wire unsigned [ 47: 0] rx_car_sum_out_inc;

rb_addsub_48M48 i_rb_rx_afc_sum_add_inc_addsub (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_pwr_rx_AFC_clken         ),  // power down on request

  .ADD                     ( 1'b1                        ),  // ADD
  .A                       ( rx_car_osc_inc              ),  // carrier frequency:      UNSIGNED 48 bit
  .B                       ( rx_car_afc_inc              ),  // increment value:          SIGNED 48 bit
  .S                       ( rx_car_sum_out_inc          )   // RX_CAR_SUM INC:         UNSIGNED 48 bit
);

always @(posedge clk_adc_125mhz)
if (!rb_pwr_rx_AFC_rst_n)
   { regs[REG_RD_RB_RX_CAR_SUM_INC_HI][15:0], regs[REG_RD_RB_RX_CAR_SUM_INC_LO][31:0] } <= 48'b0;
else
   { regs[REG_RD_RB_RX_CAR_SUM_INC_HI][15:0], regs[REG_RD_RB_RX_CAR_SUM_INC_LO][31:0] } <= rx_car_sum_out_inc;


//---------------------------------------------------------------------------------
//  RX_MOD FM ouput - strategy: rx_afc_cordic_phs_diff = delta-f = FM modulation

wire   signed [ 17: 0] rx_mod_fm_mix_in = rx_afc_cordic_phs_diff[31:14];
wire   signed [ 17: 0] rx_fm_gain_in = { 2'b0, rx_fm_gain[15:0] };                                          // unsigned expansion
wire   signed [ 36: 0] rx_mod_fm_mix_out;

rb_dsp48_AaDmBaC_A18_D18_B18_C36_P37 i_rb_rx_mod_fm_mixer_dsp48 (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_pwr_rx_AFC_clken         ),  // power down on request

  .A                       ( rx_mod_fm_mix_in            ),  // FM modulation signal      SIGNED 18 bit
  .D                       ( 18'b0                       ),  // unused                    SIGNED 18 bit
  .B                       ( rx_fm_gain_in               ),  // FM mixer gain             SIGNED 18 bit
  .C                       ( 36'b0                       ),  // unused                    SIGNED 36 bit

  .P                       ( rx_mod_fm_mix_out           )   // FM demodulated output     SIGSIG 37 bit
);

wire signed [ 15: 0] rx_mod_fm_out = rx_mod_fm_mix_out[29:14];


//---------------------------------------------------------------------------------
//  RX_MOD PM ouput - strategy: lossy integrator

reg    signed [ 47: 0] rx_mod_pm_accu         = 'b0;                                                        // integration: FM --> PM

wire   signed [ 47: 0] rx_mod_fm_in           = { {7{rx_mod_fm_mix_out[30]}}, rx_mod_fm_mix_out[29:0], 11'b0 }; // signed expansion
wire   signed [ 47: 0] rx_mod_pm_s1_out;

wire   signed [ 47: 0] rx_mod_pm_accu_release = { {10{rx_mod_pm_accu[47]}}, rx_mod_pm_accu[46:9] };         // signed expansion, balance to zero within 400 ms
wire   signed [ 47: 0] rx_mod_pm_s2_out;

rb_addsub_48M48 i_rb_rx_mod_pm_accu_s1_addsub (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_reset_n                  ),  // power down on request

  .ADD                     ( 1'b1                        ),  // ADD
  .A                       ( rx_mod_pm_accu              ),  // accumulator               SIGNED 48 bit
  .B                       ( rx_mod_fm_in                ),  // FM modulation signal      SIGNED 48 bit
  .S                       ( rx_mod_pm_s1_out            )   // step 1                    SIGNED 48 bit
);

rb_addsub_48M48 i_rb_rx_mod_pm_accu_s2_addsub (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_reset_n                  ),  // power down on request

  .ADD                     ( 1'b0                        ),  // SUB
  .A                       ( rx_mod_pm_s1_out            ),  // accumulator               SIGNED 48 bit
  .B                       ( rx_mod_pm_accu_release      ),  // release                   SIGNED 48 bit
  .S                       ( rx_mod_pm_s2_out            )   // step 2                    SIGNED 48 bit
);

always @(posedge clk_adc_125mhz)
if (!rb_pwr_rx_AFC_rst_n)
   rx_mod_pm_accu <= 'b0;
else if (clk_200khz)
   rx_mod_pm_accu <= rx_mod_pm_s2_out;

wire   signed [ 17: 0] rx_pm_gain_in          = { 2'b0, rx_pm_gain[15:0] };
wire   signed [ 36: 0] rx_mod_pm_mix_out;

rb_dsp48_AaDmBaC_A18_D18_B18_C36_P37 i_rb_rx_mod_pm_mixer_dsp48 (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_pwr_rx_AFC_clken         ),  // power down on request

  .A                       ( rx_mod_pm_accu[47:30]       ),  // PM modulation signal      SIGNED 18 bit
  .D                       ( 18'b0                       ),  // unused                    SIGNED 18 bit
  .B                       ( rx_pm_gain_in               ),  // PM mixer gain             SIGNED 18 bit
  .C                       ( 36'b0                       ),  // unused                    SIGNED 36 bit

  .P                       ( rx_mod_pm_mix_out           )   // PM demodulated output     SIGSIG 37 bit
);

wire   signed [ 15: 0] rx_mod_pm_out = rx_mod_pm_mix_out[30:15];


//---------------------------------------------------------------------------------
//  RX_MOD_AMENV envelope ouput - strategy: difference to mean value of the magnitude

reg  unsigned [ 47: 0] rx_mod_amenv_accu         = 'b0;                                                     // mean value of magnitue = mean value of AM envelope
wire unsigned [ 47: 0] rx_mod_amenv_s1_sig_in    = { 14'b0, rx_afc_cordic_polar_out_mag[15:0], 18'b0 };     // unsigned expansion - ready to sum up 2^13 samples
wire unsigned [ 47: 0] rx_mod_amenv_s1_out;

rb_addsub_48M48 i_rb_rx_mod_amenv_accu_s1_addsub (                                                          // mean value - signal integrator
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_reset_n                  ),  // power down on request

  .ADD                     ( 1'b1                        ),  // ADD
  .A                       ( rx_mod_amenv_accu           ),  // accumulator             UNSIGNED 48 bit
  .B                       ( rx_mod_amenv_s1_sig_in      ),  // CORDIC mag signal       UNSIGNED 48 bit
  .S                       ( rx_mod_amenv_s1_out         )   // accu step 1             UNSIGNED 48 bit
);

reg  unsigned [ 12: 0] rx_mod_amenv_clk_cnt   =  'b0;
reg                    rx_mod_amenv_clk_pulse = 1'b0;

always @(posedge clk_adc_125mhz)
if (!rb_pwr_rx_AFC_rst_n) begin
   rx_mod_amenv_clk_cnt   <=  'b0;
   rx_mod_amenv_clk_pulse <= 1'b0;
   end
else begin
   if (rx_mod_amenv_clk_cnt == 13'b0)
      rx_mod_amenv_clk_pulse <= 1'b1;
   else
      rx_mod_amenv_clk_pulse <= 1'b0;
   rx_mod_amenv_clk_cnt <= rx_mod_amenv_clk_cnt - 1;
   end

reg  unsigned [ 47: 0] rx_mod_amenv_mean = 'b0;

always @(posedge clk_adc_125mhz)
if (!rb_pwr_rx_AFC_rst_n) begin
   rx_mod_amenv_mean <= 'b0;
   rx_mod_amenv_accu <= 'b0;
   end
else if (clk_200khz)
   if (rx_mod_amenv_clk_pulse) begin
      rx_mod_amenv_mean <= rx_mod_amenv_s1_out;
      rx_mod_amenv_accu <= 'b0;
      end
   else
      rx_mod_amenv_accu <= rx_mod_amenv_s1_out;

assign regs[REG_RD_RB_RX_SIGNAL_STRENGTH] = rx_mod_amenv_mean[47:32];

wire   signed [ 47: 0] rx_mod_amenv_sig_in    = { 2'b0, rx_afc_cordic_polar_out_mag[15:0], 29'b0 };
wire   signed [ 47: 0] rx_mod_amenv_mean_in   = { 2'b0, rx_mod_amenv_mean[47:2] };                          // 2^13 elements summed up, divide to get the average value
wire   signed [ 47: 0] rx_mod_amenv_diff_out;

rb_addsub_48M48 i_rb_rx_mod_amenv_accu_mod_addsub (                                                         // AM envelope demodulation
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_reset_n                  ),  // power down on request

  .ADD                     ( 1'b0                        ),  // SUB
  .A                       ( rx_mod_amenv_sig_in         ),  // current magnitude         SIGNED 48 bit
  .B                       ( rx_mod_amenv_mean_in        ),  // mean value                SIGNED 48 bit
  .S                       ( rx_mod_amenv_diff_out       )   // AM envelope               SIGNED 48 bit
);

wire   signed [ 17: 0] rx_mod_amenv_mix_in = rx_mod_amenv_diff_out[47:30];
wire   signed [ 17: 0] rx_amenv_gain_in = { 2'b0, rx_amenv_gain[15:0] };
wire   signed [ 36: 0] rx_mod_amenv_mix_out;

rb_dsp48_AaDmBaC_A18_D18_B18_C36_P37 i_rb_rx_mod_amenv_mixer_dsp48 (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_pwr_rx_AFC_clken         ),  // power down on request

  .A                       ( rx_mod_amenv_mix_in         ),  // AM-ENV modulation         SIGNED 18 bit
  .D                       ( 18'b0                       ),  // unused                    SIGNED 18 bit
  .B                       ( rx_amenv_gain_in            ),  // AM-ENV mixer gain         SIGNED 18 bit
  .C                       ( 36'b0                       ),  // unused                    SIGNED 36 bit

  .P                       ( rx_mod_amenv_mix_out        )   // AM-ENV demod. output      SIGSIG 37 bit
);

wire   signed [ 15: 0] rx_mod_amenv_out = rx_mod_amenv_mix_out[30:15];


// === RX_AUDIO section ===

//---------------------------------------------------------------------------------
//  RX_AUDIO_OUT audio output mixer

wire   signed [ 15: 0] rx_audio_mux_out = (rb_pwr_rx_modvar == RB_PWR_CTRL_RX_MOD_USB)         ||
                                          (rb_pwr_rx_modvar == RB_PWR_CTRL_RX_MOD_LSB)         ||
                                          (rb_pwr_rx_modvar == RB_PWR_CTRL_RX_MOD_AM_SYNC_USB) ||
                                          (rb_pwr_rx_modvar == RB_PWR_CTRL_RX_MOD_AM_SYNC_LSB) ?  rx_mod_ssb_am_out :
                                          (rb_pwr_rx_modvar == RB_PWR_CTRL_RX_MOD_FM)          ?  rx_mod_fm_out     :
                                          (rb_pwr_rx_modvar == RB_PWR_CTRL_RX_MOD_PM)          ?  rx_mod_pm_out     :
                                          (rb_pwr_rx_modvar == RB_PWR_CTRL_RX_MOD_AM_ENV)      ?  rx_mod_amenv_out  :
                                                                                                  16'b0             ;


// === Connection Matrix section ===

//---------------------------------------------------------------------------------
//  LEDs Magnitude indicator

function bit [7:0] fct_mag (input bit [15:0] val);
   automatic bit [7:0] leds = 8'b0;                                                                         // exact zero indicator

   if (!val[15]) begin                                                                                      // positive value
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

   else begin                                                                                               // negative value
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

always @(posedge clk_adc_125mhz)
if (!adc_rstn_i || !rb_reset_n) begin
   rb_leds_en    <=  1'b0;
   rb_leds_data  <=  8'b0;
   led_ctr       <= 20'b0;
   end

else if (led_src_con_pnt && rb_reset_n) begin
   rb_leds_en <=  1'b1;                                                                                     // LEDs magnitude indicator active
   case (led_src_con_pnt)

   RB_SRC_CON_PNT_NUM_OFF: begin
      rb_leds_data <=  8'b0;                                                                                // turn all LEDs off
      end

   RB_SRC_CON_PNT_NUM_TX_MUXIN_MIX_IN: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(tx_muxin_mix_in[15:0]);                                                    // updating about 120 Hz for reducing EMI
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_ADC_IN: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(tx_mod_adc_in[15:0]);
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_ADC_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(tx_mod_adc_out[31:16]);
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
         rb_leds_data <= fct_mag(tx_mod_qmix_i_s1_out[31:16]);
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_Q_S1_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(tx_mod_qmix_q_s1_out[31:16]);
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_I_S2_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(tx_mod_qmix_i_s2_out[31:16]);
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_Q_S2_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(tx_mod_qmix_q_s2_out[31:16]);
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
         rb_leds_data <= fct_mag(tx_mod_fir_i_out[33:18]);
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_FIR_Q_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(tx_mod_fir_q_out[33:18]);
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

   RB_SRC_CON_PNT_NUM_RX_MUXIN_MIX_IN: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(rx_muxin_sig[15:0]);
      end
   RB_SRC_CON_PNT_NUM_RX_MUXIN_MIX_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(rx_muxin_out[15:0]);
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
         rb_leds_data <= fct_mag(rx_afc_fir_i_out[32:17]);
      end
   RB_SRC_CON_PNT_NUM_RX_AFC_FIR_Q_OUT: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(rx_afc_fir_q_out[32:17]);
      end

   RB_SRC_CON_PNT_NUM_RX_AFC_CORDIC_MAG: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(rx_afc_cordic_polar_out_mag[15:0] - 16'h8000);
      end
   RB_SRC_CON_PNT_NUM_RX_AFC_CORDIC_PHS: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(rx_afc_cordic_polar_out_phs[15:0]);
      end
   RB_SRC_CON_PNT_NUM_RX_AFC_CORDIC_PHS_PREV: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(regs[REG_RD_RB_RX_AFC_CORDIC_PHS_PREV][15:0]);
      end
   RB_SRC_CON_PNT_NUM_RX_AFC_CORDIC_PHS_DIFF: begin
      if (!led_ctr)
         rb_leds_data <= fct_mag(regs[REG_RD_RB_RX_AFC_CORDIC_PHS_DIFF][15:0]);
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
         rb_leds_data <= fct_mag(rx_audio_mux_out);
      end

   RB_SRC_CON_PNT_NUM_TX_CAR_OSC_INC: begin
      rb_leds_data <= tx_car_osc_inc[28:21];
      end

   RB_SRC_CON_PNT_NUM_RX_CAR_OSC_INC: begin
      rb_leds_data <= rx_car_osc_inc[28:21];
      end

   RB_SRC_CON_PNT_NUM_ADC_AUTO_OFS_RFIN1: begin
      rb_leds_data <= adc_offset[RB_ADC_AUTO_OFS_RFIN1][7:0];
      end

   RB_SRC_CON_PNT_NUM_ADC_AUTO_OFS_RFIN2: begin
      rb_leds_data <= adc_offset[RB_ADC_AUTO_OFS_RFIN2][7:0];
      end

   RB_SRC_CON_PNT_NUM_ADC_AUTO_OFS_EXT_CH0: begin
      rb_leds_data <= adc_offset[RB_ADC_AUTO_OFS_EXT_CH0][7:0];
      end

   RB_SRC_CON_PNT_NUM_ADC_AUTO_OFS_EXT_CH8: begin
      rb_leds_data <= adc_offset[RB_ADC_AUTO_OFS_EXT_CH8][7:0];
      end

   RB_SRC_CON_PNT_NUM_ADC_AUTO_OFS_EXT_CH1: begin
      rb_leds_data <= adc_offset[RB_ADC_AUTO_OFS_EXT_CH1][7:0];
      end

   RB_SRC_CON_PNT_NUM_ADC_AUTO_OFS_EXT_CH9: begin
      rb_leds_data <= adc_offset[RB_ADC_AUTO_OFS_EXT_CH9][7:0];
      end

   RB_SRC_CON_PNT_NUM_ADC_AUTO_OFS_EXT_VpVn: begin
      rb_leds_data <= adc_offset[RB_ADC_AUTO_OFS_VpVn][7:0];
      end

   RB_SRC_CON_PNT_NUM_TEST_AC97: begin
      rb_leds_data <= ac97_leds_i;
      end

   RB_SRC_CON_PNT_NUM_TEST_OVERDRIVE: begin
      if (!led_ctr)
         //                LED7                    LED6                    LED5                    LED4                       LED3                    LED2                       LED1                    LED0
         rb_leds_data <= { ac97_irq_rec_i,         ac97_irq_play_i,        rb_overdrive_rx_muxin,  rb_overdrive_rx_muxin_mon, rb_overdrive_tx_muxin,  rb_overdrive_tx_muxin_mon, 1'b0,                   adc_auto_ofs };
      end

   RB_SRC_CON_PNT_NUM_TEST_VECTOR_OUT: begin
      if (!led_ctr)
         //                LED7                    LED6                    LED5                    LED4                       LED3                    LED2                       LED1                    LED0
         rb_leds_data <= { tx_car_osc_ofs_mux,     tx_car_osc_inc_mux,     rb_pwr_rx_AFC_rst_n,    rb_pwr_rx_MOD_rst_n,       rb_pwr_rx_CAR_rst_n,    rb_pwr_tx_Q_rst_n,         rb_pwr_tx_I_rst_n,      rb_pwr_tx_OSC_rst_n  };
      end

   default: begin
      rb_leds_data <= led_ctr[19:12];
      end

   endcase
   led_ctr <= led_ctr + 1;
   end

else begin                                                                                                  // RB_SRC_CON_PNT_NUM_DISABLED
   rb_leds_en   <=  1'b0;
   rb_leds_data <=  8'b0;
   led_ctr      <= 20'b0;
   end


//---------------------------------------------------------------------------------
//  RB RFOUT1 signal assignment

reg  [ 15: 0] rfout1_amp_in = 16'b0;

always @(posedge clk_adc_125mhz)
if (!adc_rstn_i || !rb_reset_n)
   rfout1_amp_in <= 16'b0;

else if (rfout1_src_con_pnt && rb_reset_n)
   case (rfout1_src_con_pnt)

   RB_SRC_CON_PNT_NUM_TX_MUXIN_MIX_IN: begin
      rfout1_amp_in <= tx_muxin_mix_in[15:0];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_ADC_IN: begin
      rfout1_amp_in <= tx_mod_adc_in[15:0];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_ADC_OUT: begin
      rfout1_amp_in <= tx_mod_adc_out[31:16];
      end

   RB_SRC_CON_PNT_NUM_TX_MOD_OSC_I_OUT: begin
      rfout1_amp_in <= tx_mod_osc_cos[15:0];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_OSC_Q_OUT: begin
      rfout1_amp_in <= tx_mod_osc_sin[15:0];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_I_S1_OUT: begin
      rfout1_amp_in <= tx_mod_qmix_i_s1_out[31:16];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_Q_S1_OUT: begin
      rfout1_amp_in <= tx_mod_qmix_q_s1_out[31:16];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_I_S2_OUT: begin
      rfout1_amp_in <= tx_mod_qmix_i_s2_out[31:16];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_Q_S2_OUT: begin
      rfout1_amp_in <= tx_mod_qmix_q_s2_out[31:16];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_I_S3_OUT: begin
      rfout1_amp_in <= tx_mod_qmix_i_s3_out[47:32];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_Q_S3_OUT: begin
      rfout1_amp_in <= tx_mod_qmix_q_s3_out[47:32];
      end

   RB_SRC_CON_PNT_NUM_TX_MOD_CIC_I_OUT: begin
      if (tx_mod_cic_i_out_vld)
         rfout1_amp_in <= tx_mod_cic_i_out[16:1];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_CIC_Q_OUT: begin
      if (tx_mod_cic_q_out_vld)
         rfout1_amp_in <= tx_mod_cic_q_out[16:1];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_FIR_I_OUT: begin
      if (tx_mod_fir_i_out_vld)
         rfout1_amp_in <= tx_mod_fir_i_out[32:17];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_FIR_Q_OUT: begin
      if (tx_mod_fir_q_out_vld)
         rfout1_amp_in <= tx_mod_fir_q_out[32:17];
      end
   RB_SRC_CON_PNT_NUM_TX_CAR_CIC_41M664_I_OUT: begin
      rfout1_amp_in <= tx_car_regs_i_data;
      end
   RB_SRC_CON_PNT_NUM_TX_CAR_CIC_41M664_Q_OUT: begin
      rfout1_amp_in <= tx_car_regs_q_data;
      end

   RB_SRC_CON_PNT_NUM_TX_CAR_OSC_I_OUT: begin
      rfout1_amp_in <= tx_car_osc_cos[15:0];
      end
   RB_SRC_CON_PNT_NUM_TX_CAR_OSC_Q_OUT: begin
      rfout1_amp_in <= tx_car_osc_sin[15:0];
      end
   RB_SRC_CON_PNT_NUM_TX_CAR_QMIX_I_OUT: begin
      rfout1_amp_in <= tx_car_qmix_i_out[30:15];
      end
   RB_SRC_CON_PNT_NUM_TX_CAR_QMIX_Q_OUT: begin
      rfout1_amp_in <= tx_car_qmix_q_out[30:15];
      end

   RB_SRC_CON_PNT_NUM_TX_RF_AMP_OUT: begin
      rfout1_amp_in <= tx_amp_rf_out[31:16];
      end

   RB_SRC_CON_PNT_NUM_RX_MUXIN_MIX_IN: begin
      rfout1_amp_in <= rx_muxin_sig[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_MUXIN_MIX_OUT: begin
      rfout1_amp_in <= rx_muxin_out[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_OSC_I_OUT: begin
      rfout1_amp_in <= rx_car_osc_cos[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_OSC_Q_OUT: begin
      rfout1_amp_in <= rx_car_osc_sin[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_QMIX_I_OUT: begin
      rfout1_amp_in <= rx_car_qmix_i_out[30:15];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_QMIX_Q_OUT: begin
      rfout1_amp_in <= rx_car_qmix_q_out[30:15];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_5M_I_OUT: begin
      if (rx_car_cic1_i_out_vld)
         rfout1_amp_in <= rx_car_cic1_i_out[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_5M_Q_OUT: begin
      if (rx_car_cic1_q_out_vld)
         rfout1_amp_in <= rx_car_cic1_q_out[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_200K_I_OUT: begin
      rfout1_amp_in <= rx_car_regs2_i_data[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_200K_Q_OUT: begin
      rfout1_amp_in <= rx_car_regs2_q_data[16:1];
      end

   RB_SRC_CON_PNT_NUM_RX_MOD_8K_I_OUT: begin
      rfout1_amp_in <= rx_mod_regs1_i_data[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_8K_Q_OUT: begin
      rfout1_amp_in <= rx_mod_regs1_q_data[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_VB1_I_OUT: begin
      rfout1_amp_in <= rx_mod_regs2_i_data[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_VB1_Q_OUT: begin
      rfout1_amp_in <= rx_mod_regs2_q_data[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_OSC_I_OUT: begin
      rfout1_amp_in <= rx_mod_osc_cos[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_OSC_Q_OUT: begin
      rfout1_amp_in <= rx_mod_osc_sin[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_HLD_I_OUT: begin
      rfout1_amp_in <= rx_mod_hld_i_data[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_HLD_Q_OUT: begin
      rfout1_amp_in <= rx_mod_hld_q_data[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_QMIX_I_OUT: begin
      rfout1_amp_in <= rx_mod_qmix_i_out[30:15];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_QMIX_Q_OUT: begin
      rfout1_amp_in <= rx_mod_qmix_q_out[30:15];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_VB2_I_OUT: begin
      rfout1_amp_in <= rx_mod_regs3_i_data[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_VB2_Q_OUT: begin
      rfout1_amp_in <= rx_mod_regs3_q_data[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_48K_I_OUT: begin
      if (rx_mod_cic4_i_out_vld)
         rfout1_amp_in <= rx_mod_cic4_i_out[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_48K_Q_OUT: begin
      if (rx_mod_cic4_q_out_vld)
         rfout1_amp_in <= rx_mod_cic4_q_out[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_SSB_AM_OUT: begin
      rfout1_amp_in <= rx_mod_ssb_am_out;
      end

   RB_SRC_CON_PNT_NUM_RX_AFC_FIR_I_OUT: begin
      if (rx_afc_fir_i_out_vld)
         rfout1_amp_in <= rx_afc_fir_i_out[32:17];
      end
   RB_SRC_CON_PNT_NUM_RX_AFC_FIR_Q_OUT: begin
      if (rx_afc_fir_q_out_vld)
         rfout1_amp_in <= rx_afc_fir_q_out[32:17];
      end

   RB_SRC_CON_PNT_NUM_RX_AFC_CORDIC_MAG: begin
      rfout1_amp_in <= (rx_afc_cordic_polar_out_mag[15:0] - 16'h8000);
      end
   RB_SRC_CON_PNT_NUM_RX_AFC_CORDIC_PHS: begin
      rfout1_amp_in <= rx_afc_cordic_polar_out_phs[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_AFC_CORDIC_PHS_PREV: begin
      rfout1_amp_in <= regs[REG_RD_RB_RX_AFC_CORDIC_PHS_PREV][15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_AFC_CORDIC_PHS_DIFF: begin
      rfout1_amp_in <= regs[REG_RD_RB_RX_AFC_CORDIC_PHS_DIFF][15:0];
      end

   RB_SRC_CON_PNT_NUM_RX_AFC_INC_REG: begin
      rfout1_amp_in <= { regs[REG_RD_RB_RX_CAR_AFC_INC_HI][2:0], regs[REG_RD_RB_RX_CAR_AFC_INC_LO][31:19] };
      end

   RB_SRC_CON_PNT_NUM_RX_SUM_INC_REG: begin
      rfout1_amp_in <= { regs[REG_RD_RB_RX_CAR_SUM_INC_HI][2:0], regs[REG_RD_RB_RX_CAR_SUM_INC_LO][31:19] };
      end

   RB_SRC_CON_PNT_NUM_RX_MOD_FM_OUT: begin
      rfout1_amp_in <= rx_mod_fm_out;
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_PM_OUT: begin
      rfout1_amp_in <= rx_mod_pm_out;
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_AMENV_OUT: begin
      rfout1_amp_in <= rx_mod_amenv_out;
      end

   RB_SRC_CON_PNT_NUM_RX_AUDIO_OUT: begin
      rfout1_amp_in <= rx_audio_mux_out;
      end

   RB_SRC_CON_PNT_NUM_TX_CAR_OSC_INC: begin
      rfout1_amp_in <= tx_car_osc_inc[36:21];
      end

   RB_SRC_CON_PNT_NUM_RX_CAR_OSC_INC: begin
      rfout1_amp_in <= rx_car_osc_inc[36:21];
      end

   RB_SRC_CON_PNT_NUM_TEST_VECTOR_OUT: begin
      // rfout1_amp_in <= { 1'b0, clk_200khz, 14'b0 };
      // rfout1_amp_in <= rx_mod_amenv_s1_out[43:28];
      // rfout1_amp_in <= { ~rx_mod_amenv_s1_out[43], rx_mod_amenv_s1_out[42:28] };                            // UNSIGNED --> SIGNED
      rfout1_amp_in <= { ~rx_mod_amenv_mean[47], rx_mod_amenv_mean[46:32] };
      end

   default: begin
      rfout1_amp_in <= led_ctr[19:4];                                                                       // error-ramp
      end

   endcase

else                                                                                                        // RB_SRC_CON_PNT_NUM_DISABLED
   rfout1_amp_in    <= 16'b0;                                                                               // silence


//---------------------------------------------------------------------------------
//  RB RFOUT1_AMP signal gain correction for different termnation variants

wire [ 17: 0] rfout1_amp_in18    = { {2{rfout1_amp_in[15]}}, rfout1_amp_in[15:0] };                         // signed expansion
wire [ 17: 0] rfout1_amp_gain_in = { 2'b0, rfout1_amp_gain[15:0] };                                         // 8 int . 8 frac
wire [ 35: 0] rfout1_amp_ofs_in  = { {12{rfout1_amp_ofs[15]}}, rfout1_amp_ofs[15:0], 8'b0 };                // signed register value, signed expansion
wire [ 36: 0] rfout1_amp_out;

rb_dsp48_AaDmBaC_A18_D18_B18_C36_P37 i_rb_rfout1_amp_dsp48 (
  // global signals
  .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
  .CE                      ( rb_clk_en                   ),  // power down on request

  .A                       ( rfout1_amp_in18             ),  // RFOUT1_AMP in             SIGNED 18 bit
  .D                       ( 18'b0                       ),  // unused                    SIGNED 18 bit
  .B                       ( rfout1_amp_gain_in          ),  // RFOUT1_AMP gain           SIGNED 18 bit
  .C                       ( rfout1_amp_ofs_in           ),  // RFOUT1_AMP ofs            SIGSIG 36 bit

  .P                       ( rfout1_amp_out              )   // RFOUT1_AMP out            SIGSIG 37 bit
);

assign rb_out_ch[0] = rfout1_amp_out[23:8];


//---------------------------------------------------------------------------------
//  RB RFOUT2 signal assignment

reg  [ 15: 0] rfout2_amp_in = 16'b0;

always @(posedge clk_adc_125mhz)
if (!adc_rstn_i || !rb_reset_n)
   rfout2_amp_in <= 16'b0;

else if (rfout2_src_con_pnt && rb_reset_n)
   case (rfout2_src_con_pnt)

   RB_SRC_CON_PNT_NUM_TX_MUXIN_MIX_IN: begin
      rfout2_amp_in <= tx_muxin_mix_in[15:0];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_ADC_IN: begin
      rfout2_amp_in <= tx_mod_adc_in[15:0];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_ADC_OUT: begin
      rfout2_amp_in <= tx_mod_adc_out[31:16];
      end

   RB_SRC_CON_PNT_NUM_TX_MOD_OSC_I_OUT: begin
      rfout2_amp_in <= tx_mod_osc_cos[15:0];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_OSC_Q_OUT: begin
      rfout2_amp_in <= tx_mod_osc_sin[15:0];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_I_S1_OUT: begin
      rfout2_amp_in <= tx_mod_qmix_i_s1_out[31:16];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_Q_S1_OUT: begin
      rfout2_amp_in <= tx_mod_qmix_q_s1_out[31:16];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_I_S2_OUT: begin
      rfout2_amp_in <= tx_mod_qmix_i_s2_out[31:16];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_Q_S2_OUT: begin
      rfout2_amp_in <= tx_mod_qmix_q_s2_out[31:16];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_I_S3_OUT: begin
      rfout2_amp_in <= tx_mod_qmix_i_s3_out[47:32];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_Q_S3_OUT: begin
      rfout2_amp_in <= tx_mod_qmix_q_s3_out[47:32];
      end

   RB_SRC_CON_PNT_NUM_TX_MOD_CIC_I_OUT: begin
      if (tx_mod_cic_i_out_vld)
         rfout2_amp_in <= tx_mod_cic_i_out[16:1];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_CIC_Q_OUT: begin
      if (tx_mod_cic_q_out_vld)
         rfout2_amp_in <= tx_mod_cic_q_out[16:1];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_FIR_I_OUT: begin
      if (tx_mod_fir_i_out_vld)
         rfout2_amp_in <= tx_mod_fir_i_out[32:17];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_FIR_Q_OUT: begin
      if (tx_mod_fir_q_out_vld)
         rfout2_amp_in <= tx_mod_fir_q_out[32:17];
      end
   RB_SRC_CON_PNT_NUM_TX_CAR_CIC_41M664_I_OUT: begin
      rfout2_amp_in <= tx_car_regs_i_data;
      end
   RB_SRC_CON_PNT_NUM_TX_CAR_CIC_41M664_Q_OUT: begin
      rfout2_amp_in <= tx_car_regs_q_data;
      end

   RB_SRC_CON_PNT_NUM_TX_CAR_OSC_I_OUT: begin
      rfout2_amp_in <= tx_car_osc_cos[15:0];
      end
   RB_SRC_CON_PNT_NUM_TX_CAR_OSC_Q_OUT: begin
      rfout2_amp_in <= tx_car_osc_sin[15:0];
      end
   RB_SRC_CON_PNT_NUM_TX_CAR_QMIX_I_OUT: begin
      rfout2_amp_in <= tx_car_qmix_i_out[30:15];
      end
   RB_SRC_CON_PNT_NUM_TX_CAR_QMIX_Q_OUT: begin
      rfout2_amp_in <= tx_car_qmix_q_out[30:15];
      end

   RB_SRC_CON_PNT_NUM_TX_RF_AMP_OUT: begin
      rfout2_amp_in <= tx_amp_rf_out[31:16];
      end

   RB_SRC_CON_PNT_NUM_RX_MUXIN_MIX_IN: begin
      rfout2_amp_in <= rx_muxin_sig[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_MUXIN_MIX_OUT: begin
      rfout2_amp_in <= rx_muxin_out[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_OSC_I_OUT: begin
      rfout2_amp_in <= rx_car_osc_cos[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_OSC_Q_OUT: begin
      rfout2_amp_in <= rx_car_osc_sin[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_QMIX_I_OUT: begin
      rfout2_amp_in <= rx_car_qmix_i_out[30:15];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_QMIX_Q_OUT: begin
      rfout2_amp_in <= rx_car_qmix_q_out[30:15];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_5M_I_OUT: begin
      if (rx_car_cic1_i_out_vld)
         rfout2_amp_in <= rx_car_cic1_i_out[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_5M_Q_OUT: begin
      if (rx_car_cic1_q_out_vld)
         rfout2_amp_in <= rx_car_cic1_q_out[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_200K_I_OUT: begin
      rfout2_amp_in <= rx_car_regs2_i_data[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_200K_Q_OUT: begin
      rfout2_amp_in <= rx_car_regs2_q_data[16:1];
      end

   RB_SRC_CON_PNT_NUM_RX_MOD_8K_I_OUT: begin
      rfout2_amp_in <= rx_mod_regs1_i_data[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_8K_Q_OUT: begin
      rfout2_amp_in <= rx_mod_regs1_q_data[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_VB1_I_OUT: begin
      rfout2_amp_in <= rx_mod_regs2_i_data[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_VB1_Q_OUT: begin
      rfout2_amp_in <= rx_mod_regs2_q_data[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_OSC_I_OUT: begin
      rfout2_amp_in <= rx_mod_osc_cos[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_OSC_Q_OUT: begin
      rfout2_amp_in <= rx_mod_osc_sin[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_HLD_I_OUT: begin
      rfout2_amp_in <= rx_mod_hld_i_data[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_HLD_Q_OUT: begin
      rfout2_amp_in <= rx_mod_hld_q_data[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_QMIX_I_OUT: begin
      rfout2_amp_in <= rx_mod_qmix_i_out[30:15];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_QMIX_Q_OUT: begin
      rfout2_amp_in <= rx_mod_qmix_q_out[30:15];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_VB2_I_OUT: begin
      rfout2_amp_in <= rx_mod_regs3_i_data[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_VB2_Q_OUT: begin
      rfout2_amp_in <= rx_mod_regs3_q_data[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_48K_I_OUT: begin
      if (rx_mod_cic4_i_out_vld)
         rfout2_amp_in <= rx_mod_cic4_i_out[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_48K_Q_OUT: begin
      if (rx_mod_cic4_q_out_vld)
         rfout2_amp_in <= rx_mod_cic4_q_out[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_SSB_AM_OUT: begin
      rfout2_amp_in <= rx_mod_ssb_am_out;
      end

   RB_SRC_CON_PNT_NUM_RX_AFC_FIR_I_OUT: begin
      if (rx_afc_fir_i_out_vld)
         rfout2_amp_in <= rx_afc_fir_i_out[32:17];
      end
   RB_SRC_CON_PNT_NUM_RX_AFC_FIR_Q_OUT: begin
      if (rx_afc_fir_q_out_vld)
         rfout2_amp_in <= rx_afc_fir_q_out[32:17];
      end

   RB_SRC_CON_PNT_NUM_RX_AFC_CORDIC_MAG: begin
      rfout2_amp_in <= (rx_afc_cordic_polar_out_mag[15:0] - 16'h8000);
      end
   RB_SRC_CON_PNT_NUM_RX_AFC_CORDIC_PHS: begin
      rfout2_amp_in <= rx_afc_cordic_polar_out_phs[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_AFC_CORDIC_PHS_PREV: begin
      rfout2_amp_in <= regs[REG_RD_RB_RX_AFC_CORDIC_PHS_PREV][15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_AFC_CORDIC_PHS_DIFF: begin
      rfout2_amp_in <= regs[REG_RD_RB_RX_AFC_CORDIC_PHS_DIFF][15:0];
      end

   RB_SRC_CON_PNT_NUM_RX_AFC_INC_REG: begin
      rfout2_amp_in <= { regs[REG_RD_RB_RX_CAR_AFC_INC_HI][2:0], regs[REG_RD_RB_RX_CAR_AFC_INC_LO][31:19] };
      end

   RB_SRC_CON_PNT_NUM_RX_SUM_INC_REG: begin
      rfout2_amp_in <= { regs[REG_RD_RB_RX_CAR_SUM_INC_HI][2:0], regs[REG_RD_RB_RX_CAR_SUM_INC_LO][31:19] };
      end

   RB_SRC_CON_PNT_NUM_RX_MOD_FM_OUT: begin
      rfout2_amp_in <= rx_mod_fm_out;
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_PM_OUT: begin
      rfout2_amp_in <= rx_mod_pm_out;
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_AMENV_OUT: begin
      rfout2_amp_in <= rx_mod_amenv_out;
      end

   RB_SRC_CON_PNT_NUM_RX_AUDIO_OUT: begin
      rfout2_amp_in <= rx_audio_mux_out;
      end

   RB_SRC_CON_PNT_NUM_TX_CAR_OSC_INC: begin
      rfout2_amp_in <= tx_car_osc_inc[36:21];
      end

   RB_SRC_CON_PNT_NUM_RX_CAR_OSC_INC: begin
      rfout2_amp_in <= rx_car_osc_inc[36:21];
      end

   RB_SRC_CON_PNT_NUM_TEST_VECTOR_OUT: begin
      // rfout2_amp_in <= { 1'b0, rx_mod_cic1_chk_rst, 14'b0 };
      // rfout2_amp_in <= rx_mod_amenv_mix_in[15:0];
      rfout2_amp_in <= { ~rx_mod_amenv_mean[47], rx_mod_amenv_mean[46:32] };
      end

   default: begin
      rfout2_amp_in <= led_ctr[19:4];                                                                       // error-ramp
      end

   endcase

else                                                                                                        // RB_SRC_CON_PNT_NUM_DISABLED
   rfout2_amp_in <= 16'b0;                                                                                  // silence


//---------------------------------------------------------------------------------
//  RB RFOUT2_AMP signal gain correction for different termnation variants

wire [ 17: 0] rfout2_amp_in18    = { {2{rfout2_amp_in[15]}}, rfout2_amp_in[15:0] };                         // signed expansion
wire [ 17: 0] rfout2_amp_gain_in = { 2'b0, rfout2_amp_gain[15:0] };                                         // 8 int . 8 frac
wire [ 35: 0] rfout2_amp_ofs_in  = { {12{rfout2_amp_ofs[15]}}, rfout2_amp_ofs[15:0], 8'b0 };                // signed register value, signed expansion
wire [ 36: 0] rfout2_amp_out;

rb_dsp48_AaDmBaC_A18_D18_B18_C36_P37 i_rb_rfout2_amp_dsp48 (
 // global signals
 .CLK                     ( clk_adc_125mhz              ),  // global 125 MHz clock
 .CE                      ( rb_clk_en                   ),  // power down on request

 .A                       ( rfout2_amp_in18             ),  // RFOUT2_AMP in             SIGNED 18 bit
 .D                       ( 18'b0                       ),  // unused                    SIGNED 18 bit
 .B                       ( rfout2_amp_gain_in          ),  // RFOUT2_AMP gain           SIGNED 18 bit
 .C                       ( rfout2_amp_ofs_in           ),  // RFOUT2_AMP ofs            SIGSIG 36 bit

 .P                       ( rfout2_amp_out              )   // RFOUT2_AMP out            SIGSIG 37 bit
);

assign rb_out_ch[1] = rfout2_amp_out[23:8];


//---------------------------------------------------------------------------------
//  Linux LINE-IN 1 left signal assignment

reg  [ 15: 0] rb_line_in1l_amp_in = 16'b0;

always @(posedge clk_adc_125mhz)
if (!adc_rstn_i || !rb_reset_n)
   rb_line_in1l_amp_in <= 16'b0;

else if (line_in1l_src_con_pnt && rb_reset_n)
   case (line_in1l_src_con_pnt)

   RB_SRC_CON_PNT_NUM_TX_MUXIN_MIX_IN: begin
      rb_line_in1l_amp_in <= tx_muxin_mix_in[15:0];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_ADC_IN: begin
      rb_line_in1l_amp_in <= tx_mod_adc_in[15:0];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_ADC_OUT: begin
      rb_line_in1l_amp_in <= tx_mod_adc_out[31:16];
      end

   RB_SRC_CON_PNT_NUM_TX_MOD_OSC_I_OUT: begin
      rb_line_in1l_amp_in <= tx_mod_osc_cos[15:0];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_OSC_Q_OUT: begin
      rb_line_in1l_amp_in <= tx_mod_osc_sin[15:0];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_I_S1_OUT: begin
      rb_line_in1l_amp_in <= tx_mod_qmix_i_s1_out[31:16];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_Q_S1_OUT: begin
      rb_line_in1l_amp_in <= tx_mod_qmix_q_s1_out[31:16];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_I_S2_OUT: begin
      rb_line_in1l_amp_in <= tx_mod_qmix_i_s2_out[31:16];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_Q_S2_OUT: begin
      rb_line_in1l_amp_in <= tx_mod_qmix_q_s2_out[31:16];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_I_S3_OUT: begin
      rb_line_in1l_amp_in <= tx_mod_qmix_i_s3_out[47:32];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_Q_S3_OUT: begin
      rb_line_in1l_amp_in <= tx_mod_qmix_q_s3_out[47:32];
      end

   RB_SRC_CON_PNT_NUM_TX_MOD_CIC_I_OUT: begin
      if (tx_mod_cic_i_out_vld)
         rb_line_in1l_amp_in <= tx_mod_cic_i_out[16:1];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_CIC_Q_OUT: begin
      if (tx_mod_cic_q_out_vld)
         rb_line_in1l_amp_in <= tx_mod_cic_q_out[16:1];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_FIR_I_OUT: begin
      if (tx_mod_fir_i_out_vld)
         rb_line_in1l_amp_in <= tx_mod_fir_i_out[32:17];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_FIR_Q_OUT: begin
      if (tx_mod_fir_q_out_vld)
         rb_line_in1l_amp_in <= tx_mod_fir_q_out[32:17];
      end
   RB_SRC_CON_PNT_NUM_TX_CAR_CIC_41M664_I_OUT: begin
      rb_line_in1l_amp_in <= tx_car_regs_i_data;
      end
   RB_SRC_CON_PNT_NUM_TX_CAR_CIC_41M664_Q_OUT: begin
      rb_line_in1l_amp_in <= tx_car_regs_q_data;
      end

   RB_SRC_CON_PNT_NUM_TX_CAR_OSC_I_OUT: begin
      rb_line_in1l_amp_in <= tx_car_osc_cos[15:0];
      end
   RB_SRC_CON_PNT_NUM_TX_CAR_OSC_Q_OUT: begin
      rb_line_in1l_amp_in <= tx_car_osc_sin[15:0];
      end
   RB_SRC_CON_PNT_NUM_TX_CAR_QMIX_I_OUT: begin
      rb_line_in1l_amp_in <= tx_car_qmix_i_out[30:15];
      end
   RB_SRC_CON_PNT_NUM_TX_CAR_QMIX_Q_OUT: begin
      rb_line_in1l_amp_in <= tx_car_qmix_q_out[30:15];
      end

   RB_SRC_CON_PNT_NUM_TX_RF_AMP_OUT: begin
      rb_line_in1l_amp_in <= tx_amp_rf_out[31:16];
      end

   RB_SRC_CON_PNT_NUM_RX_MUXIN_MIX_IN: begin
      rb_line_in1l_amp_in <= rx_muxin_sig[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_MUXIN_MIX_OUT: begin
      rb_line_in1l_amp_in <= rx_muxin_out[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_OSC_I_OUT: begin
      rb_line_in1l_amp_in <= rx_car_osc_cos[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_OSC_Q_OUT: begin
      rb_line_in1l_amp_in <= rx_car_osc_sin[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_QMIX_I_OUT: begin
      rb_line_in1l_amp_in <= rx_car_qmix_i_out[30:15];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_QMIX_Q_OUT: begin
      rb_line_in1l_amp_in <= rx_car_qmix_q_out[30:15];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_5M_I_OUT: begin
      if (rx_car_cic1_i_out_vld)
         rb_line_in1l_amp_in <= rx_car_cic1_i_out[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_5M_Q_OUT: begin
      if (rx_car_cic1_q_out_vld)
         rb_line_in1l_amp_in <= rx_car_cic1_q_out[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_200K_I_OUT: begin
      rb_line_in1l_amp_in <= rx_car_regs2_i_data[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_200K_Q_OUT: begin
      rb_line_in1l_amp_in <= rx_car_regs2_q_data[16:1];
      end

   RB_SRC_CON_PNT_NUM_RX_MOD_8K_I_OUT: begin
      rb_line_in1l_amp_in <= rx_mod_regs1_i_data[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_8K_Q_OUT: begin
      rb_line_in1l_amp_in <= rx_mod_regs1_q_data[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_VB1_I_OUT: begin
      rb_line_in1l_amp_in <= rx_mod_regs2_i_data[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_VB1_Q_OUT: begin
      rb_line_in1l_amp_in <= rx_mod_regs2_q_data[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_OSC_I_OUT: begin
      rb_line_in1l_amp_in <= rx_mod_osc_cos[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_OSC_Q_OUT: begin
      rb_line_in1l_amp_in <= rx_mod_osc_sin[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_HLD_I_OUT: begin
      rb_line_in1l_amp_in <= rx_mod_hld_i_data[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_HLD_Q_OUT: begin
      rb_line_in1l_amp_in <= rx_mod_hld_q_data[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_QMIX_I_OUT: begin
      rb_line_in1l_amp_in <= rx_mod_qmix_i_out[30:15];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_QMIX_Q_OUT: begin
      rb_line_in1l_amp_in <= rx_mod_qmix_q_out[30:15];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_VB2_I_OUT: begin
      rb_line_in1l_amp_in <= rx_mod_regs3_i_data[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_VB2_Q_OUT: begin
      rb_line_in1l_amp_in <= rx_mod_regs3_q_data[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_48K_I_OUT: begin
      if (rx_mod_cic4_i_out_vld)
         rb_line_in1l_amp_in <= rx_mod_cic4_i_out[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_48K_Q_OUT: begin
      if (rx_mod_cic4_q_out_vld)
         rb_line_in1l_amp_in <= rx_mod_cic4_q_out[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_SSB_AM_OUT: begin
      rb_line_in1l_amp_in <= rx_mod_ssb_am_out;
      end

   RB_SRC_CON_PNT_NUM_RX_AFC_FIR_I_OUT: begin
      if (rx_afc_fir_i_out_vld)
         rb_line_in1l_amp_in <= rx_afc_fir_i_out[32:17];
      end
   RB_SRC_CON_PNT_NUM_RX_AFC_FIR_Q_OUT: begin
      if (rx_afc_fir_q_out_vld)
         rb_line_in1l_amp_in <= rx_afc_fir_q_out[32:17];
      end

   RB_SRC_CON_PNT_NUM_RX_AFC_CORDIC_MAG: begin
      rb_line_in1l_amp_in <= (rx_afc_cordic_polar_out_mag[15:0] - 16'h8000);
      end
   RB_SRC_CON_PNT_NUM_RX_AFC_CORDIC_PHS: begin
      rb_line_in1l_amp_in <= rx_afc_cordic_polar_out_phs[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_AFC_CORDIC_PHS_PREV: begin
      rb_line_in1l_amp_in <= regs[REG_RD_RB_RX_AFC_CORDIC_PHS_PREV][15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_AFC_CORDIC_PHS_DIFF: begin
      rb_line_in1l_amp_in <= regs[REG_RD_RB_RX_AFC_CORDIC_PHS_DIFF][15:0];
      end

   RB_SRC_CON_PNT_NUM_RX_AFC_INC_REG: begin
      rb_line_in1l_amp_in <= { regs[REG_RD_RB_RX_CAR_AFC_INC_HI][2:0], regs[REG_RD_RB_RX_CAR_AFC_INC_LO][31:19] };
      end

   RB_SRC_CON_PNT_NUM_RX_SUM_INC_REG: begin
      rb_line_in1l_amp_in <= { regs[REG_RD_RB_RX_CAR_SUM_INC_HI][2:0], regs[REG_RD_RB_RX_CAR_SUM_INC_LO][31:19] };
      end

   RB_SRC_CON_PNT_NUM_RX_MOD_FM_OUT: begin
      rb_line_in1l_amp_in <= rx_mod_fm_out;
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_PM_OUT: begin
      rb_line_in1l_amp_in <= rx_mod_pm_out;
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_AMENV_OUT: begin
      rb_line_in1l_amp_in <= rx_mod_amenv_out;
      end

   RB_SRC_CON_PNT_NUM_RX_AUDIO_OUT: begin
      rb_line_in1l_amp_in <= rx_audio_mux_out;
      end

   RB_SRC_CON_PNT_NUM_TX_CAR_OSC_INC: begin
      rb_line_in1l_amp_in <= tx_car_osc_inc[36:21];
      end

   RB_SRC_CON_PNT_NUM_RX_CAR_OSC_INC: begin
      rb_line_in1l_amp_in <= rx_car_osc_inc[36:21];
      end

   RB_SRC_CON_PNT_NUM_TEST_VECTOR_OUT: begin
      // rb_line_in1l_amp_in <= { 1'b0, rx_mod_cic1_chk_rst, 14'b0 };
      // rb_line_in1l_amp_in <= rx_mod_amenv_mix_in[15:0];
      rb_line_in1l_amp_in <= { ~rx_mod_amenv_mean[47], rx_mod_amenv_mean[46:32] };
      end

   default: begin
      rb_line_in1l_amp_in <= led_ctr[19:4];                                                                 // error-ramp
      end

   endcase

else                                                                                                        // RB_SRC_CON_PNT_NUM_DISABLED
   rb_line_in1l_amp_in <= 16'b0;                                                                            // silence

assign rb_line_in_o[15: 0] = rb_line_in1l_amp_in;                                                           // TODO


//---------------------------------------------------------------------------------
//  Linux LINE-IN 1 right signal assignment

reg  [ 15: 0] rb_line_in1r_amp_in = 16'b0;

always @(posedge clk_adc_125mhz)
if (!adc_rstn_i || !rb_reset_n)
   rb_line_in1r_amp_in <= 16'b0;

else if (line_in1r_src_con_pnt && rb_reset_n)
   case (line_in1r_src_con_pnt)

   RB_SRC_CON_PNT_NUM_TX_MUXIN_MIX_IN: begin
      rb_line_in1r_amp_in <= tx_muxin_mix_in[15:0];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_ADC_IN: begin
      rb_line_in1r_amp_in <= tx_mod_adc_in[15:0];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_ADC_OUT: begin
      rb_line_in1r_amp_in <= tx_mod_adc_out[31:16];
      end

   RB_SRC_CON_PNT_NUM_TX_MOD_OSC_I_OUT: begin
      rb_line_in1r_amp_in <= tx_mod_osc_cos[15:0];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_OSC_Q_OUT: begin
      rb_line_in1r_amp_in <= tx_mod_osc_sin[15:0];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_I_S1_OUT: begin
      rb_line_in1r_amp_in <= tx_mod_qmix_i_s1_out[31:16];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_Q_S1_OUT: begin
      rb_line_in1r_amp_in <= tx_mod_qmix_q_s1_out[31:16];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_I_S2_OUT: begin
      rb_line_in1r_amp_in <= tx_mod_qmix_i_s2_out[31:16];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_Q_S2_OUT: begin
      rb_line_in1r_amp_in <= tx_mod_qmix_q_s2_out[31:16];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_I_S3_OUT: begin
      rb_line_in1r_amp_in <= tx_mod_qmix_i_s3_out[47:32];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_QMIX_Q_S3_OUT: begin
      rb_line_in1r_amp_in <= tx_mod_qmix_q_s3_out[47:32];
      end

   RB_SRC_CON_PNT_NUM_TX_MOD_CIC_I_OUT: begin
      if (tx_mod_cic_i_out_vld)
         rb_line_in1r_amp_in <= tx_mod_cic_i_out[16:1];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_CIC_Q_OUT: begin
      if (tx_mod_cic_q_out_vld)
         rb_line_in1r_amp_in <= tx_mod_cic_q_out[16:1];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_FIR_I_OUT: begin
      if (tx_mod_fir_i_out_vld)
         rb_line_in1r_amp_in <= tx_mod_fir_i_out[32:17];
      end
   RB_SRC_CON_PNT_NUM_TX_MOD_FIR_Q_OUT: begin
      if (tx_mod_fir_q_out_vld)
         rb_line_in1r_amp_in <= tx_mod_fir_q_out[32:17];
      end
   RB_SRC_CON_PNT_NUM_TX_CAR_CIC_41M664_I_OUT: begin
      rb_line_in1r_amp_in <= tx_car_regs_i_data;
      end
   RB_SRC_CON_PNT_NUM_TX_CAR_CIC_41M664_Q_OUT: begin
      rb_line_in1r_amp_in <= tx_car_regs_q_data;
      end

   RB_SRC_CON_PNT_NUM_TX_CAR_OSC_I_OUT: begin
      rb_line_in1r_amp_in <= tx_car_osc_cos[15:0];
      end
   RB_SRC_CON_PNT_NUM_TX_CAR_OSC_Q_OUT: begin
      rb_line_in1r_amp_in <= tx_car_osc_sin[15:0];
      end
   RB_SRC_CON_PNT_NUM_TX_CAR_QMIX_I_OUT: begin
      rb_line_in1r_amp_in <= tx_car_qmix_i_out[30:15];
      end
   RB_SRC_CON_PNT_NUM_TX_CAR_QMIX_Q_OUT: begin
      rb_line_in1r_amp_in <= tx_car_qmix_q_out[30:15];
      end

   RB_SRC_CON_PNT_NUM_TX_RF_AMP_OUT: begin
      rb_line_in1r_amp_in <= tx_amp_rf_out[31:16];
      end

   RB_SRC_CON_PNT_NUM_RX_MUXIN_MIX_IN: begin
      rb_line_in1r_amp_in <= rx_muxin_sig[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_MUXIN_MIX_OUT: begin
      rb_line_in1r_amp_in <= rx_muxin_out[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_OSC_I_OUT: begin
      rb_line_in1r_amp_in <= rx_car_osc_cos[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_OSC_Q_OUT: begin
      rb_line_in1r_amp_in <= rx_car_osc_sin[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_QMIX_I_OUT: begin
      rb_line_in1r_amp_in <= rx_car_qmix_i_out[30:15];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_QMIX_Q_OUT: begin
      rb_line_in1r_amp_in <= rx_car_qmix_q_out[30:15];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_5M_I_OUT: begin
      if (rx_car_cic1_i_out_vld)
         rb_line_in1r_amp_in <= rx_car_cic1_i_out[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_5M_Q_OUT: begin
      if (rx_car_cic1_q_out_vld)
         rb_line_in1r_amp_in <= rx_car_cic1_q_out[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_200K_I_OUT: begin
      rb_line_in1r_amp_in <= rx_car_regs2_i_data[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_CAR_200K_Q_OUT: begin
      rb_line_in1r_amp_in <= rx_car_regs2_q_data[16:1];
      end

   RB_SRC_CON_PNT_NUM_RX_MOD_8K_I_OUT: begin
      rb_line_in1r_amp_in <= rx_mod_regs1_i_data[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_8K_Q_OUT: begin
      rb_line_in1r_amp_in <= rx_mod_regs1_q_data[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_VB1_I_OUT: begin
      rb_line_in1r_amp_in <= rx_mod_regs2_i_data[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_VB1_Q_OUT: begin
      rb_line_in1r_amp_in <= rx_mod_regs2_q_data[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_OSC_I_OUT: begin
      rb_line_in1r_amp_in <= rx_mod_osc_cos[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_OSC_Q_OUT: begin
      rb_line_in1r_amp_in <= rx_mod_osc_sin[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_HLD_I_OUT: begin
      rb_line_in1r_amp_in <= rx_mod_hld_i_data[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_HLD_Q_OUT: begin
      rb_line_in1r_amp_in <= rx_mod_hld_q_data[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_QMIX_I_OUT: begin
      rb_line_in1r_amp_in <= rx_mod_qmix_i_out[30:15];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_QMIX_Q_OUT: begin
      rb_line_in1r_amp_in <= rx_mod_qmix_q_out[30:15];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_VB2_I_OUT: begin
      rb_line_in1r_amp_in <= rx_mod_regs3_i_data[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_VB2_Q_OUT: begin
      rb_line_in1r_amp_in <= rx_mod_regs3_q_data[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_48K_I_OUT: begin
      if (rx_mod_cic4_i_out_vld)
         rb_line_in1r_amp_in <= rx_mod_cic4_i_out[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_48K_Q_OUT: begin
      if (rx_mod_cic4_q_out_vld)
         rb_line_in1r_amp_in <= rx_mod_cic4_q_out[16:1];
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_SSB_AM_OUT: begin
      rb_line_in1r_amp_in <= rx_mod_ssb_am_out;
      end

   RB_SRC_CON_PNT_NUM_RX_AFC_FIR_I_OUT: begin
      if (rx_afc_fir_i_out_vld)
         rb_line_in1r_amp_in <= rx_afc_fir_i_out[32:17];
      end
   RB_SRC_CON_PNT_NUM_RX_AFC_FIR_Q_OUT: begin
      if (rx_afc_fir_q_out_vld)
         rb_line_in1r_amp_in <= rx_afc_fir_q_out[32:17];
      end

   RB_SRC_CON_PNT_NUM_RX_AFC_CORDIC_MAG: begin
      rb_line_in1r_amp_in <= (rx_afc_cordic_polar_out_mag[15:0] - 16'h8000);
      end
   RB_SRC_CON_PNT_NUM_RX_AFC_CORDIC_PHS: begin
      rb_line_in1r_amp_in <= rx_afc_cordic_polar_out_phs[15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_AFC_CORDIC_PHS_PREV: begin
      rb_line_in1r_amp_in <= regs[REG_RD_RB_RX_AFC_CORDIC_PHS_PREV][15:0];
      end
   RB_SRC_CON_PNT_NUM_RX_AFC_CORDIC_PHS_DIFF: begin
      rb_line_in1r_amp_in <= regs[REG_RD_RB_RX_AFC_CORDIC_PHS_DIFF][15:0];
      end

   RB_SRC_CON_PNT_NUM_RX_AFC_INC_REG: begin
      rb_line_in1r_amp_in <= { regs[REG_RD_RB_RX_CAR_AFC_INC_HI][2:0], regs[REG_RD_RB_RX_CAR_AFC_INC_LO][31:19] };
      end

   RB_SRC_CON_PNT_NUM_RX_SUM_INC_REG: begin
      rb_line_in1r_amp_in <= { regs[REG_RD_RB_RX_CAR_SUM_INC_HI][2:0], regs[REG_RD_RB_RX_CAR_SUM_INC_LO][31:19] };
      end

   RB_SRC_CON_PNT_NUM_RX_MOD_FM_OUT: begin
      rb_line_in1r_amp_in <= rx_mod_fm_out;
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_PM_OUT: begin
      rb_line_in1r_amp_in <= rx_mod_pm_out;
      end
   RB_SRC_CON_PNT_NUM_RX_MOD_AMENV_OUT: begin
      rb_line_in1r_amp_in <= rx_mod_amenv_out;
      end

   RB_SRC_CON_PNT_NUM_RX_AUDIO_OUT: begin
      rb_line_in1r_amp_in <= rx_audio_mux_out;
      end

   RB_SRC_CON_PNT_NUM_TX_CAR_OSC_INC: begin
      rb_line_in1r_amp_in <= tx_car_osc_inc[36:21];
      end

   RB_SRC_CON_PNT_NUM_RX_CAR_OSC_INC: begin
      rb_line_in1r_amp_in <= rx_car_osc_inc[36:21];
      end

   RB_SRC_CON_PNT_NUM_TEST_VECTOR_OUT: begin
      // rb_line_in1r_amp_in <= { 1'b0, rx_mod_cic1_chk_rst, 14'b0 };
      // rb_line_in1r_amp_in <= rx_mod_amenv_mix_in[15:0];
      rb_line_in1r_amp_in <= { ~rx_mod_amenv_mean[47], rx_mod_amenv_mean[46:32] };
      end

   default: begin
      rb_line_in1r_amp_in <= led_ctr[19:4];                                                                 // error-ramp
      end

   endcase

else                                                                                                        // RB_SRC_CON_PNT_NUM_DISABLED
   rb_line_in1r_amp_in <= 16'b0;                                                                            // silence

assign rb_line_in_o[31:16] = rb_line_in1r_amp_in;                                                           // TODO


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

   regs[REG_RD_RB_STATUS][RB_STAT_TX_CAR_OSC_ZERO]           <= !tx_car_osc_sin;                            // when phase is 0 deg
   regs[REG_RD_RB_STATUS][RB_STAT_TX_CAR_OSC_VALID]          <=  tx_car_osc_axis_m_vld;

   regs[REG_RD_RB_STATUS][RB_STAT_TX_MOD_OSC_ZERO]           <= !tx_mod_osc_sin;                            // when phase is 0 deg
   regs[REG_RD_RB_STATUS][RB_STAT_TX_MOD_OSC_VALID]          <=  tx_mod_osc_axis_m_vld;

   regs[REG_RD_RB_STATUS][RB_STAT_RX_CAR_OSC_ZERO]           <= !rx_car_osc_sin;                            // when phase is 0 deg
   regs[REG_RD_RB_STATUS][RB_STAT_RX_CAR_OSC_VALID]          <=  rx_car_osc_axis_m_vld;

   regs[REG_RD_RB_STATUS][RB_STAT_RX_MOD_OSC_ZERO]           <= !rx_mod_osc_sin;                            // when phase is 0 deg
   regs[REG_RD_RB_STATUS][RB_STAT_RX_MOD_OSC_VALID]          <=  rx_mod_osc_axis_m_vld;

   regs[REG_RD_RB_STATUS][RB_STAT_RX_AFC_HIGH_SIG]           <=  rx_afc_high_sig;

   regs[REG_RD_RB_STATUS][RB_STAT_OVERDRIVE_TX]              <=  rb_overdrive_tx_muxin_mon;
   regs[REG_RD_RB_STATUS][RB_STAT_OVERDRIVE_RX]              <=  rb_overdrive_rx_muxin_mon;

   regs[REG_RD_RB_STATUS][RB_STAT_LED7_ON : RB_STAT_LED0_ON] <= rb_leds_data;
   end


//---------------------------------------------------------------------------------
//  Readout signal registers

always @(posedge clk_adc_125mhz)
if (!adc_rstn_i) begin
   regs[REG_RD_RB_READOUT_RFIN1]             <= 32'b0;
   regs[REG_RD_RB_READOUT_RFIN2]             <= 32'b0;
   regs[REG_RD_RB_READOUT_RFOUT1]            <= 32'b0;
   regs[REG_RD_RB_READOUT_RFOUT2]            <= 32'b0;
   end
else begin
   regs[REG_RD_RB_READOUT_RFIN1]             <=  { 16'b0, adc_i[0][13:0], 2'b0 };
   regs[REG_RD_RB_READOUT_RFIN2]             <=  { 16'b0, adc_i[1][13:0], 2'b0 };
   regs[REG_RD_RB_READOUT_RFOUT1]            <=  { 16'b0, rb_out_ch[0][15:0] };
   regs[REG_RD_RB_READOUT_RFOUT2]            <=  { 16'b0, rb_out_ch[1][15:0] };
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
   regs[REG_RW_RB_LINE_IN_SRC_CON_PNT]       <= 32'h00000000;
   regs[REG_RW_RB_TX_CAR_OSC_INC_LO]         <= 32'h00000000;
   regs[REG_RW_RB_TX_CAR_OSC_INC_HI]         <= 32'h00000000;
   regs[REG_RW_RB_TX_CAR_OSC_OFS_LO]         <= 32'h00000000;
   regs[REG_RW_RB_TX_CAR_OSC_OFS_HI]         <= 32'h00000000;
   regs[REG_RW_RB_TX_CAR_OSC_INC_SCNR_LO]    <= 32'h00000000;
   regs[REG_RW_RB_TX_CAR_OSC_INC_SCNR_HI]    <= 32'h00000000;
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
   regs[REG_RW_RB_TX_MUXIN_OFS]              <= 32'h00000000;
   regs[REG_RW_RB_RX_CAR_CALC_WEAVER_INC_LO] <= 32'h00000000;
   regs[REG_RW_RB_RX_CAR_CALC_WEAVER_INC_HI] <= 32'h00000000;
   regs[REG_RW_RB_RX_CAR_OSC_INC_LO]         <= 32'h00000000;
   regs[REG_RW_RB_RX_CAR_OSC_INC_HI]         <= 32'h00000000;
   regs[REG_RW_RB_RX_CAR_OSC_OFS_LO]         <= 32'h00000000;
   regs[REG_RW_RB_RX_CAR_OSC_OFS_HI]         <= 32'h00000000;
   regs[REG_RW_RB_RX_CAR_OSC_INC_SCNR_LO]    <= 32'h00000000;
   regs[REG_RW_RB_RX_CAR_OSC_INC_SCNR_HI]    <= 32'h00000000;
   regs[REG_RW_RB_RX_MOD_OSC_INC_LO]         <= 32'h00000000;
   regs[REG_RW_RB_RX_MOD_OSC_INC_HI]         <= 32'h00000000;
   regs[REG_RW_RB_RX_MOD_OSC_OFS_LO]         <= 32'h00000000;
   regs[REG_RW_RB_RX_MOD_OSC_OFS_HI]         <= 32'h00000000;
   regs[REG_RW_RB_RX_MUXIN_SRC]              <= 32'h00000000;
   regs[REG_RW_RB_RX_MUXIN_GAIN]             <= 32'h00000000;
   regs[REG_RW_RB_RX_MUXIN_OFS]              <= 32'h00000000;
   regs[REG_RW_RB_RX_SSB_AM_GAIN]            <= 32'h00000000;
   regs[REG_RW_RB_RX_AMENV_GAIN]             <= 32'h00000000;
   regs[REG_RW_RB_RX_FM_GAIN]                <= 32'h00000000;
   regs[REG_RW_RB_RX_PM_GAIN]                <= 32'h00000000;
   regs[REG_RW_RB_RFOUT1_AMP_GAIN]           <= 32'h00000000;
   regs[REG_RW_RB_RFOUT1_AMP_OFS]            <= 32'h00000000;
   regs[REG_RW_RB_RFOUT2_AMP_GAIN]           <= 32'h00000000;
   regs[REG_RW_RB_RFOUT2_AMP_OFS]            <= 32'h00000000;
   end

else begin
   /* preset every point of time of the 200 kHz clock, when not overwritten by a bus access */
   if (clk_200khz_d[7]) begin
      regs[REG_RW_RB_TX_CAR_OSC_INC_LO]      <=          tx_car_osc_inc_next[31: 0]  ;
      regs[REG_RW_RB_TX_CAR_OSC_INC_HI]      <= { 16'b0, tx_car_osc_inc_next[47:32] };

      regs[REG_RW_RB_RX_CAR_OSC_INC_LO]      <=          rx_car_osc_inc_next[31: 0]  ;
      regs[REG_RW_RB_RX_CAR_OSC_INC_HI]      <= { 16'b0, rx_car_osc_inc_next[47:32] };
   end

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
      20'h00014: begin
         regs[REG_RW_RB_PWR_CTRL]                 <= { 16'b0, sys_wdata[15:0] };
         end
      20'h00018: begin
         regs[REG_RW_RB_RFOUTx_LED_SRC_CON_PNT]   <= sys_wdata[31:0] & 32'hFFFF00FF;
         end
      20'h0001C: begin
         regs[REG_RW_RB_LINE_IN_SRC_CON_PNT]      <= sys_wdata[31:0];
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
         regs[REG_RW_RB_TX_CAR_OSC_INC_SCNR_LO]   <= sys_wdata[31:0];
         end
      20'h00034: begin
         regs[REG_RW_RB_TX_CAR_OSC_INC_SCNR_HI]   <= { 16'b0, sys_wdata[15:0] };
         end
      20'h00038: begin
         regs[REG_RW_RB_TX_RF_AMP_GAIN]           <= { 16'b0, sys_wdata[15:0] };
         end
      20'h0003C: begin
         regs[REG_RW_RB_TX_RF_AMP_OFS]            <= { 16'b0, sys_wdata[15:0] };
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
         regs[REG_RW_RB_TX_MOD_QMIX_GAIN]         <= { 16'b0, sys_wdata[15:0] };
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
      20'h00068: begin
         regs[REG_RW_RB_TX_MUXIN_OFS]             <= { 16'b0, sys_wdata[15:0] };
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
      20'h00130: begin
         regs[REG_RW_RB_RX_CAR_OSC_INC_SCNR_LO]   <= sys_wdata[31:0];
         end
      20'h00134: begin
         regs[REG_RW_RB_RX_CAR_OSC_INC_SCNR_HI]   <= { 16'b0, sys_wdata[15:0] };
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

      /* RX filter variants */
      20'h0015C: begin
         regs[REG_RW_RB_RX_EMENV_FILT_VARIANT]    <= { 30'b0, sys_wdata[ 1:0] };
         end

      /* RX_MUX */
      20'h00160: begin
         regs[REG_RW_RB_RX_MUXIN_SRC]             <= sys_wdata[31:0];
         end
      20'h00164: begin
         regs[REG_RW_RB_RX_MUXIN_GAIN]            <= { 13'b0, sys_wdata[18:0] };
         end
      20'h00168: begin
         regs[REG_RW_RB_RX_MUXIN_OFS]             <= { 16'b0, sys_wdata[15:0] };
         end

      /* RX_DEMOD_GAIN */
      20'h00180: begin
        regs[REG_RW_RB_RX_SSB_AM_GAIN]            <= { 16'b0, sys_wdata[15:0] };
        end
      20'h00184: begin
        regs[REG_RW_RB_RX_AMENV_GAIN]             <= { 16'b0, sys_wdata[15:0] };
        end
      20'h00188: begin
        regs[REG_RW_RB_RX_FM_GAIN]                <= { 16'b0, sys_wdata[15:0] };
        end
      20'h0018C: begin
        regs[REG_RW_RB_RX_PM_GAIN]                <= { 16'b0, sys_wdata[15:0] };
        end

      /* RFOUTx AMP section */
      20'h00190: begin
         regs[REG_RW_RB_RFOUT1_AMP_GAIN]          <= { 16'b0, sys_wdata[15:0] };
         end
      20'h00194: begin
         regs[REG_RW_RB_RFOUT1_AMP_OFS]           <= { 16'b0, sys_wdata[15:0] };
         end
      20'h00198: begin
         regs[REG_RW_RB_RFOUT2_AMP_GAIN]          <= { 16'b0, sys_wdata[15:0] };
         end
      20'h0019C: begin
         regs[REG_RW_RB_RFOUT2_AMP_OFS]           <= { 16'b0, sys_wdata[15:0] };
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
      20'h00014: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_PWR_CTRL];
         end
      20'h00018: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_RFOUTx_LED_SRC_CON_PNT];
         end
      20'h0001C: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_LINE_IN_SRC_CON_PNT];
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
         sys_rdata <= regs[REG_RW_RB_TX_CAR_OSC_INC_SCNR_LO];
         end
      20'h00034: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_TX_CAR_OSC_INC_SCNR_HI];
         end
      20'h00038: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_TX_RF_AMP_GAIN];
         end
      20'h0003C: begin
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
      20'h00068: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_TX_MUXIN_OFS];
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
      20'h00130: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_RX_CAR_OSC_INC_SCNR_LO];
         end
      20'h00134: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_RX_CAR_OSC_INC_SCNR_HI];
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

      /* RX filter variants */
      20'h0015C: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_RX_EMENV_FILT_VARIANT];
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
      20'h00168: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_RX_MUXIN_OFS];
         end
      20'h0016C: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RD_RB_RX_SIGNAL_STRENGTH];
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
         sys_rdata <= regs[REG_RW_RB_RX_AMENV_GAIN];
         end
      20'h00188: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_RX_FM_GAIN];
         end
      20'h0018C: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_RX_PM_GAIN];
         end

      /* RFOUTx AMP section */
      20'h00190: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_RFOUT1_AMP_GAIN];
         end
      20'h00194: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_RFOUT1_AMP_OFS];
         end
      20'h00198: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_RFOUT2_AMP_GAIN];
         end
      20'h0019C: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_RFOUT2_AMP_OFS];
         end

      /* READOUT SIGNALS section */
      20'h001A0: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RD_RB_READOUT_RFIN1];
         end
      20'h001A4: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RD_RB_READOUT_RFIN2];
         end
      20'h001A8: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RD_RB_READOUT_RFOUT1];
         end
      20'h001AC: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RD_RB_READOUT_RFOUT2];
         end

      default:   begin
         sys_ack   <= sys_en;
         sys_rdata <= 32'h00000000;
         end

      endcase
      end

   else if (sys_wen) begin                                                                                  // keep sys_ack assignment in this process
      sys_ack <= sys_en;
      end

   else begin
      sys_ack <= 1'b0;
      end
   end

endmodule
