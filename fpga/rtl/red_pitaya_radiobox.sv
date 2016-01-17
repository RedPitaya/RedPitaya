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
   output reg            rb_activated    ,      // RB sub-module is activated

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
    //REG_RD_RB_RSVD_H018,
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

    REG_RW_RB_TX_MUXIN_SRC,                     // h060: RB analog TX MUX input selector:  d3=VpVn,
                                                //      d16=EXT-CH0,  d24=EXT-CH8,
                                                //      d17=EXT-CH1,  d25=EXT-CH9,
                                                //      d32=adc_i[0], d33=adc_i[1]
    REG_RW_RB_TX_MUXIN_GAIN,                    // h064: RB analog TX MUX gain for input amplifier


    /* RX section */
    //REG_RD_RB_RX_RSVD_H100,
    //REG_RD_RB_RX_RSVD_H104,
    //REG_RD_RB_RX_RSVD_H108,
    //REG_RD_RB_RX_RSVD_H10C,
    //REG_RD_RB_RX_RSVD_H110,
    //REG_RD_RB_RX_RSVD_H114,
    //REG_RD_RB_RX_RSVD_H118,
    //REG_RD_RB_RX_RSVD_H11C,

    REG_RW_RB_RX_CAR_OSC_INC_LO,                // h120: RB RX_CAR_OSC increment register              LSB:        (Bit 31: 0)
    REG_RW_RB_RX_CAR_OSC_INC_HI,                // h124: RB RX_CAR_OSC increment register              MSB: 16'b0, (Bit 47:32)
    REG_RW_RB_RX_CAR_OSC_OFS_LO,                // h128: RB RX_CAR_OSC offset register                 LSB:        (Bit 31: 0)
    REG_RW_RB_RX_CAR_OSC_OFS_HI,                // h12C: RB RX_CAR_OSC offset register                 MSB: 16'b0, (Bit 47:32)

    REG_RW_RB_RX_MOD_ADD_GAIN,                  // h130: RB RX_MOD_OSC mixer gain:     SIGNED 16 bit
    //REG_RD_RB_RX_RSVD_H134,
    REG_RW_RB_RX_MOD_ADD_OFS,                   // h138: RB RX_MOD_OSC mixer offset:   SIGNED 17 bit
    //REG_RD_RB_RX_RSVD_H13C,

    REG_RW_RB_RX_MOD_OSC_INC_LO,                // h140: RB RX_MOD_OSC increment register              LSB:        (Bit 31: 0)
    REG_RW_RB_RX_MOD_OSC_INC_HI,                // h144: RB RX_MOD_OSC increment register              MSB: 16'b0, (Bit 47:32)
    REG_RW_RB_RX_MOD_OSC_OFS_LO,                // h148: RB RX_MOD_OSC offset register                 LSB:        (Bit 31: 0)
    REG_RW_RB_RX_MOD_OSC_OFS_HI,                // h14C: RB RX_MOD_OSC offset register                 MSB: 16'b0, (Bit 47:32)

    //REG_RD_RB_RX_RSVD_H150,
    //REG_RD_RB_RX_RSVD_H154,
    //REG_RD_RB_RX_RSVD_H158,
    //REG_RD_RB_RX_RSVD_H15C,

    REG_RW_RB_RX_MUXIN_SRC,                     // h160: RB analog RX MUX input selector:  d0=RF Input 1, d1=RF Input 2
    REG_RW_RB_RX_MUXIN_GAIN,                    // h164: RB analog RX MUX gain for input amplifier

    REG_RB_COUNT
} REG_RB_ENUMS;

reg  [31: 0]    regs    [REG_RB_COUNT];         // registers to be accessed by the system bus

enum {
    RB_CTRL_ENABLE                        =  0, // enabling the RadioBox sub-module
    RB_CTRL_RESET_TX_CAR_OSC,                   // reset TX_CAR_OSC, does not touch clock enable
    RB_CTRL_RESET_TX_MOD_OSC,                   // reset TX_MOD_OSC, does not touch clock enable
    RB_CTRL_RSVD_D03,

    RB_CTRL_TX_CAR_OSC_RESYNC,                  // TX_CAR_OSC restart with phase register = 0
    RB_CTRL_TX_CAR_OSC_INC_SRC_STREAM,          // TX_CAR_OSC OSC incrementing: use stream instead of register setting
    RB_CTRL_TX_CAR_OSC_OFS_SRC_STREAM,          // TX_CAR_OSC OSC offset: use stream instead of register setting
    RB_CTRL_RSVD_D07,

    RB_CTRL_RSVD_D08,
    RB_CTRL_RSVD_D09,
    RB_CTRL_RSVD_D10,
    RB_CTRL_RSVD_D11,

    RB_CTRL_TX_MOD_OSC_RESYNC,                  // TX_MOD_OSC restart with phase register = 0
    RB_CTRL_TX_MOD_OSC_INC_SRC_STREAM,          // TX_MOD_OSC OSC incrementing: use stream instead of register setting
    RB_CTRL_TX_MOD_OSC_OFS_SRC_STREAM,          // TX_MOD_OSC OSC offset: use stream instead of register setting
    RB_CTRL_TX_RF_AMP_Q_EN,                     // TX_RF_AMP enable the TX_CAR_QMIX Q path for the SSB modulation

    RB_CTRL_RSVD_D16,
    RB_CTRL_RESET_RX_CAR_OSC,                   // reset RX_CAR_OSC, does not touch clock enable
    RB_CTRL_RESET_RX_MOD_OSC,                   // reset RX_MOD_OSC, does not touch clock enable
    RB_CTRL_RSVD_D19,

    RB_CTRL_RX_CAR_OSC_RESYNC,                  // RX_CAR_OSC restart with phase register = 0
    RB_CTRL_RSVD_D21,
    RB_CTRL_RSVD_D22,
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

    RB_STAT_RSVD_D20,
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
    RB_SRC_CON_PNT_NUM_RX_CAR_CIC1_I_OUT,                 // RX_CAR_CIC1 I output
    RB_SRC_CON_PNT_NUM_RX_CAR_CIC1_Q_OUT,                 // RX_CAR_CIC1 Q output
    RB_SRC_CON_PNT_NUM_RX_CAR_CIC2_I_OUT,                 // RX_CAR_CIC2 I output
    RB_SRC_CON_PNT_NUM_RX_CAR_CIC2_Q_OUT,                 // RX_CAR_CIC2 Q output

    RB_SRC_CON_PNT_NUM_RX_MOD_OSC_I_OUT             = 40, // RX_MOD_OSC I output
    RB_SRC_CON_PNT_NUM_RX_MOD_OSC_Q_OUT,                  // RX_MOD_OSC Q output
    RB_SRC_CON_PNT_NUM_RX_MOD_QMIX_I_OUT,                 // RX_MOD_QMIX I output
    RB_SRC_CON_PNT_NUM_RX_MOD_QMIX_Q_OUT,                 // RX_MOD_QMIX Q output
    RB_SRC_CON_PNT_NUM_RX_MOD_FIR_I_OUT,                  // RX_MOD_FIR I output
    RB_SRC_CON_PNT_NUM_RX_MOD_FIR_Q_OUT,                  // RX_MOD_FIR Q output
    RB_SRC_CON_PNT_NUM_RX_MOD_CIC_I_OUT,                  // RX_MOD_CIC I output
    RB_SRC_CON_PNT_NUM_RX_MOD_CIC_Q_OUT,                  // RX_MOD_CIC Q output

    RB_SRC_CON_PNT_NUM_RX_MOD_ADD_OUT               = 48, // RX_MOD_ADD output

    RB_SRC_CON_PNT_NUM_RX_CAR_FIR_I_OUT             = 52, // RX_CAR_FIR I output
    RB_SRC_CON_PNT_NUM_RX_CAR_FIR_Q_OUT,                  // RX_CAR_FIR Q output
    RB_SRC_CON_PNT_NUM_RX_MOD_HLD_I_OUT,                  // RX_MOD_OSC_HLD I output
    RB_SRC_CON_PNT_NUM_RX_MOD_HLD_Q_OUT,                  // RX_MOD_OSC_HLD I output

    RB_SRC_CON_PNT_NUM_TEST_VECTOR_OUT              = 63  // Current test vector, look at assignments within this file
} RB_SRC_CON_PNT_ENUM;                                    // 64 entries = 2^6 --> 6 bit field

enum {
    RB_XADC_MAPPING_EXT_CH0               =  0, // CH0 and CH8 are sampled simultaneously, mapped to: vinp_i[0]/vinn_i[0]
    RB_XADC_MAPPING_EXT_CH8,                    // CH0 and CH8 are sampled simultaneously, mapped to: vinp_i[1]/vinn_i[1]
    RB_XADC_MAPPING_EXT_CH1,                    // CH1 and CH9 are sampled simultaneously, mapped to: vinp_i[2]/vinn_i[2]
    RB_XADC_MAPPING_EXT_CH9,                    // CH1 and CH9 are sampled simultaneously, mapped to: vinp_i[3]/vinn_i[3]
    RB_XADC_MAPPING_VpVn,                       // The dedicated Vp/Vn input mapped to: vinp_i[4]/vinn_i[4]
    RB_XADC_MAPPING__COUNT
} RB_XADC_MAPPING_ENUM;


// === OMNI section ===

wire rb_enable = regs[REG_RW_RB_CTRL][RB_CTRL_ENABLE];

reg          rb_enable_last     = 1'b0;
reg          rb_clk_en          = 1'b0;
reg          rb_reset_n         = 1'b0;
reg  [ 1: 0] rb_enable_ctr      = 2'b0;


//---------------------------------------------------------------------------------
//  RadioBox sub-module activation

always @(posedge clk_adc_125mhz)
begin
   if (!adc_rstn_i) begin
      rb_activated  <= 1'b0;
      rb_clk_en     <= 1'b0;
      rb_reset_n    <= 1'b0;
      rb_enable_ctr <= 2'b0;
      end

   else begin
      if (rb_enable != rb_enable_last) begin
         rb_enable_ctr <= 2'b11;                // load timer on enable bit change
         if (rb_enable)                         // just enabled
            rb_clk_en <= 1'b1;                  // firing up
         else begin                             // just disabled
            rb_activated <= 1'b0;               // RB sub-module is no more activated
            rb_reset_n   <= 1'b0;               // resetting before sleep
            end
         end
      else if (rb_enable_ctr)                   // counter runs
         rb_enable_ctr <= rb_enable_ctr - 1;

      if (rb_enable == rb_enable_last && 
          !rb_enable_ctr)                       // after the counter has stopped
         if (rb_enable) begin                   // when enabling counter elapsed
            rb_reset_n   <= 1'b1;               // release reset
            rb_activated <= 1'b1;               // RB sub-module is activated
            end
         else                                   // when disabling counter elapsed
            rb_clk_en <= 1'b0;                  // going to sleep
      end

   rb_enable_last <= rb_enable;
end


//---------------------------------------------------------------------------------
//  Signal input matrix

// AXI streaming master from XADC

reg  [15:0] rb_xadc[RB_XADC_MAPPING__COUNT - 1: 0];

always @(posedge xadc_axis_aclk)                                       // CLOCK_DOMAIN: FCLK_CLK0 (125 MHz) phase asynchron to clk_adc_125mhz
begin
   if (!adc_rstn_i) begin
      rb_xadc[RB_XADC_MAPPING_EXT_CH0] <= 16'b0;
      rb_xadc[RB_XADC_MAPPING_EXT_CH8] <= 16'b0;
      rb_xadc[RB_XADC_MAPPING_EXT_CH1] <= 16'b0;
      rb_xadc[RB_XADC_MAPPING_EXT_CH9] <= 16'b0;
      rb_xadc[RB_XADC_MAPPING_VpVn]    <= 16'b0;
      xadc_axis_tready <= 0;
      end

   else begin
      xadc_axis_tready <= 1;                                           // no reason for signaling not to be ready
      if (xadc_axis_tvalid) begin
         casez (xadc_axis_tid)                                         // @see ug480_7Series_XADC.pdf for XADC channel mapping
         5'h10: begin                                                  // channel ID d16 for EXT-CH#0
            rb_xadc[RB_XADC_MAPPING_EXT_CH0]  <= xadc_axis_tdata;      // CH0 and CH8 are sampled simultaneously, mapped to: vinp_i[1]/vinn_i[1]
            end
         5'h18: begin                                                  // channel ID d24 for EXT-CH#8
            rb_xadc[RB_XADC_MAPPING_EXT_CH8]  <= xadc_axis_tdata;      // CH0 and CH8 are sampled simultaneously, mapped to: vinp_i[0]/vinn_i[0]
            end

         5'h11: begin                                                  // channel ID d17 for EXT-CH#1
            rb_xadc[RB_XADC_MAPPING_EXT_CH1]  <= xadc_axis_tdata;      // CH1 and CH9 are sampled simultaneously, mapped to: vinp_i[2]/vinn_i[2]
            end
         5'h19: begin                                                  // channel ID d25 for EXT-CH#9
            rb_xadc[RB_XADC_MAPPING_EXT_CH9]  <= xadc_axis_tdata;      // CH1 and CH9 are sampled simultaneously, mapped to: vinp_i[3]/vinn_i[3]
            end

         5'h03: begin                                                  // channel ID d3 for dedicated Vp/Vn input lines
            rb_xadc[RB_XADC_MAPPING_VpVn]     <= xadc_axis_tdata;      // The dedicated Vp/Vn input mapped to: vinp_i[4]/vinn_i[4]
            end

         default:   begin
            end
         endcase
         end
      end
end


parameter CLK_48KHZ_CTR_MAX = 2604;             // long run max value
parameter CLK_48KHZ_FRC_MAX = 5;

reg  [11:0] clk_48khz_ctr  = 0;
reg  [ 2:0] clk_48khz_frc  = 0;
reg         clk_48khz      = 'b0;

always @(posedge clk_adc_125mhz)                // assign clk_48khz
begin
   if (!rb_clk_en) begin
      clk_48khz_ctr <= 'b0;
      clk_48khz_frc <= 'b0;
      clk_48khz <= 'b0;
      end
   else begin
      if (clk_48khz_ctr == CLK_48KHZ_CTR_MAX) begin
         clk_48khz <= 'b1;
         if (clk_48khz_frc == CLK_48KHZ_FRC_MAX) begin
            clk_48khz_frc <= 'b0;
            clk_48khz_ctr <= 'b0;               // overflow of the frac part makes a long run
            end
         else begin
            clk_48khz_frc = clk_48khz_frc + 1;
            clk_48khz_ctr <= 12'b1;             // short run
            end
         end
      else begin
         clk_48khz <= 'b0;
         clk_48khz_ctr = clk_48khz_ctr + 1;
         end
      end
end


// === Transmitter section ===

//---------------------------------------------------------------------------------
//  ADC modulation offset correction and gain

wire [15:0] tx_muxin_mix_in = (regs[REG_RW_RB_TX_MUXIN_SRC][5:0] == 6'h20) ?  {~adc_i[0], 2'b0}                :
                              (regs[REG_RW_RB_TX_MUXIN_SRC][5:0] == 6'h21) ?  {~adc_i[1], 2'b0}                :
                              (regs[REG_RW_RB_TX_MUXIN_SRC][5:0] == 6'h18) ?  rb_xadc[RB_XADC_MAPPING_EXT_CH0] :  // swapped here due to pin connection warnings when swapping @ XADC <--> pins
                              (regs[REG_RW_RB_TX_MUXIN_SRC][5:0] == 6'h10) ?  rb_xadc[RB_XADC_MAPPING_EXT_CH8] :
                              (regs[REG_RW_RB_TX_MUXIN_SRC][5:0] == 6'h11) ?  rb_xadc[RB_XADC_MAPPING_EXT_CH1] :
                              (regs[REG_RW_RB_TX_MUXIN_SRC][5:0] == 6'h19) ?  rb_xadc[RB_XADC_MAPPING_EXT_CH9] :
                              (regs[REG_RW_RB_TX_MUXIN_SRC][5:0] == 6'h03) ?  rb_xadc[RB_XADC_MAPPING_VpVn]    :
                              16'b0;
wire [ 2:0] tx_muxin_mix_log2 = regs[REG_RW_RB_TX_MUXIN_GAIN][18:16];
wire [15:0] tx_muxin_mix_gain = regs[REG_RW_RB_TX_MUXIN_GAIN][15: 0];

wire [15:0] tx_mod_adc_in = (tx_muxin_mix_in << tx_muxin_mix_log2);  // unsigned value: input booster for
                                                                     // factor: 1x .. 2^3=7 shift postions=128x (16 mV --> full-scale)
wire [15:0] tx_mod_adc_ofs = 16'b0;                                  // TODO: FSM for calculating mean value to strip of the DC component
wire [31:0] tx_mod_adc_out;

rb_dsp48_AaDmB_A16_D16_B16_P32 i_rb_tx_mod_adc_dsp48 (
  // global signals
  .CLK                  ( clk_adc_125mhz    ),  // global 125 MHz clock
  .CE                   ( rb_clk_en         ),  // enable part 1 of RadioBox sub-module

  // ADC/XADC modulation input
  .A                    ( tx_mod_adc_in     ),  // MUX in signal:    SIGNED 16 bit
  // modulation offset input
  .D                    ( tx_mod_adc_ofs    ),  // offset setting:   SIGNED 16 bit
  // modulation gain input
  .B                    ( tx_muxin_mix_gain ),  // gain setting:     SIGNED 16 bit

  // multiplier output
  .P                    ( tx_mod_adc_out    )   // PreAmp output     SIGSIG 32 bit
);


//---------------------------------------------------------------------------------
//  TX_MOD_OSC modulation oscillator and SSB weaver modulator


wire         tx_mod_osc_inc_mux = regs[REG_RW_RB_CTRL][RB_CTRL_TX_MOD_OSC_INC_SRC_STREAM];
wire         tx_mod_osc_ofs_mux = regs[REG_RW_RB_CTRL][RB_CTRL_TX_MOD_OSC_OFS_SRC_STREAM];
wire         tx_mod_osc_reset_n = rb_reset_n & !regs[REG_RW_RB_CTRL][RB_CTRL_RESET_TX_MOD_OSC];
wire         tx_mod_osc_resync  = regs[REG_RW_RB_CTRL][RB_CTRL_TX_MOD_OSC_RESYNC];

wire [ 47:0] tx_mod_osc_inc_stream = 48'b0;  // TODO: ADC
wire [ 47:0] tx_mod_osc_ofs_stream = 48'b0;  // TODO: ADC

wire [ 47:0] tx_mod_osc_inc = ( tx_mod_osc_inc_mux ?  tx_mod_osc_inc_stream : {regs[REG_RW_RB_TX_MOD_OSC_INC_HI][15:0], regs[REG_RW_RB_TX_MOD_OSC_INC_LO][31:0]});
wire [ 47:0] tx_mod_osc_ofs = ( tx_mod_osc_ofs_mux ?  tx_mod_osc_ofs_stream : {regs[REG_RW_RB_TX_MOD_OSC_OFS_HI][15:0], regs[REG_RW_RB_TX_MOD_OSC_OFS_LO][31:0]});

wire         tx_mod_osc_axis_s_vld   = rb_reset_n;  // TODO: ADC
wire [103:0] tx_mod_osc_axis_s_phase = {7'b0, tx_mod_osc_resync, tx_mod_osc_ofs, tx_mod_osc_inc};

wire         tx_mod_osc_axis_m_vld;
wire [ 31:0] tx_mod_osc_axis_m_data;
wire [ 15:0] tx_mod_osc_cos = tx_mod_osc_axis_m_data[15: 0];
wire [ 15:0] tx_mod_osc_sin = tx_mod_osc_axis_m_data[31:16];

rb_dds_48_16_125 i_rb_tx_mod_osc (
  // global signals
  .aclk                 ( clk_adc_125mhz          ),  // global 125 MHz clock
  .aclken               ( rb_clk_en               ),  // clock enable of RadioBox sub-module
  .aresetn              ( tx_mod_osc_reset_n      ),  // reset of TX_MOD_OSC

  // AXI-Stream slave in port: streaming data for TX_MOD_OSC modulation
  .s_axis_phase_tvalid  ( tx_mod_osc_axis_s_vld   ),  // AXIS slave data valid
  .s_axis_phase_tdata   ( tx_mod_osc_axis_s_phase ),  // AXIS slave data

  // AXI-Stream master out port: TX_MOD_OSC signal
  .m_axis_data_tvalid   ( tx_mod_osc_axis_m_vld   ),  // AXIS master TX_MOD_OSC data valid
  .m_axis_data_tdata    ( tx_mod_osc_axis_m_data  )   // AXIS master TX_MOD_OSC output: 2x SIGNED 16 bit
);


//---------------------------------------------------------------------------------
//  TX_MOD_QMIX quadrature mixer for the base band

wire        tx_amp_rf_q_en = regs[REG_RW_RB_CTRL][RB_CTRL_TX_RF_AMP_Q_EN];

wire [15:0] tx_mod_qmix_in      = (regs[REG_RW_RB_TX_MUXIN_SRC][5:0] == 6'h00) ?  16'h7fff : tx_mod_adc_out[30:15];  // when ADC source ID is zero take cos() from MOD_OSC only
wire [15:0] tx_mod_qmix_gain    =  regs[REG_RW_RB_TX_MOD_QMIX_GAIN][15:0];
wire [47:0] tx_mod_qmix_ofs     = {regs[REG_RW_RB_TX_MOD_QMIX_OFS_HI][15:0], regs[REG_RW_RB_TX_MOD_QMIX_OFS_LO][31:0]};

wire [31:0] tx_mod_qmix_i_s1_out;
wire [31:0] tx_mod_qmix_q_s1_out;

wire [31:0] tx_mod_qmix_i_s2_out;
wire [31:0] tx_mod_qmix_q_s2_out;

wire [47:0] tx_mod_qmix_i_s3_in = regs[REG_RW_RB_CTRL][RB_CTRL_TX_CAR_OSC_INC_SRC_STREAM] ?
                               {{15{tx_mod_qmix_i_s2_out[30]}}, tx_mod_qmix_i_s2_out[29:0], 3'b0} :  /* when FM is used, take 2^14 finer resolution */
                                   {tx_mod_qmix_i_s2_out[30:0], 17'b0};
wire [47:0] tx_mod_qmix_q_s3_in = {tx_mod_qmix_q_s2_out[30:0], 17'b0};
wire [47:0] tx_mod_qmix_i_s3_out;
wire [47:0] tx_mod_qmix_q_s3_out;

rb_dsp48_AmB_A16_B16_P32 i_rb_tx_mod_qmix_I_s1_dsp48 (
  // global signals
  .CLK                  ( clk_adc_125mhz       ),  // global 125 MHz clock
  .CE                   ( rb_clk_en            ),  // enable part 1 of RadioBox sub-module

  // modulation input
  .A                    ( tx_mod_qmix_in       ),  // TX_MUX in signal:         SIGNED 16 bit
  // TX_MOD_OSC cos input
  .B                    ( tx_mod_osc_cos       ),  // TX_MOD_OSC cos:           SIGNED 16 bit

  // multiplier output stage 1
  .P                    ( tx_mod_qmix_i_s1_out )   // TX_QMIX I output:         SIGSIG 32 bit
);

rb_dsp48_AmB_A16_B16_P32 i_rb_tx_mod_qmix_I_s2_dsp48 (
  // global signals
  .CLK                  ( clk_adc_125mhz       ),  // global 125 MHz clock
  .CE                   ( rb_clk_en            ),  // enable part 1 of RadioBox sub-module

  // TX_QMIX I input
  .A                    ( tx_mod_qmix_i_s1_out[30:15] ),  // TX_MUX in signal:  SIGNED 16 bit
  // gain setting input
  .B                    ( tx_mod_qmix_gain     ),  // gain setting:             SIGNED 16 bit

  // multiplier output stage 2
  .P                    ( tx_mod_qmix_i_s2_out )   // TX_QMIX I regulated:      SIGSIG 32 bit
);

rb_dsp48_CONaC_CON48_C48_P48 i_rb_tx_mod_qmix_I_s3_dsp48 (
  // global signals
  .CLK                  ( clk_adc_125mhz       ),  // global 125 MHz clock
  .CE                   ( rb_clk_en            ),  // enable part 1 of RadioBox sub-module

  // modulation input
  .C                    ( tx_mod_qmix_i_s3_in  ),  // modulation:               SIGNED 48 bit
  // offset value for OSC control
  .CONCAT               ( tx_mod_qmix_ofs      ),  // offset:                   SIGNED 48 bit

  // adder output
  .P                    ( tx_mod_qmix_i_s3_out )   // TX_QMIX I for OSC:        SIGNED 48 bit
);


rb_dsp48_AmB_A16_B16_P32 i_rb_tx_mod_qmix_Q_s1_dsp48 (
  // global signals
  .CLK                  ( clk_adc_125mhz       ),  // global 125 MHz clock
  .CE                   ( rb_clk_en & tx_amp_rf_q_en ),  // enable part 1 of RadioBox sub-module and the Q path

  // modulation input
  .A                    ( tx_mod_qmix_in       ),  // MUX in signal:            SIGNED 16 bit
  // TX_MOD_OSC sin input
  .B                    ( tx_mod_osc_sin       ),  // MOD_OSC sin:              SIGNED 16 bit

  // multiplier output stage 1
  .P                    ( tx_mod_qmix_q_s1_out )   // QMIX Q output:            SIGSIG 32 bit
);

rb_dsp48_AmB_A16_B16_P32 i_rb_tx_mod_qmix_Q_s2_dsp48 (
  // global signals
  .CLK                  ( clk_adc_125mhz       ),  // global 125 MHz clock
  .CE                   ( rb_clk_en & tx_amp_rf_q_en ),  // enable part 1 of RadioBox sub-module and the Q path

  // TX_QMIX Q input
  .A                    ( tx_mod_qmix_q_s1_out[30:15] ),  // MUX in signal:     SIGNED 16 bit
  // gain setting input
  .B                    ( tx_mod_qmix_gain     ),  // gain setting:             SIGNED 16 bit

  // multiplier output stage 2
  .P                    ( tx_mod_qmix_q_s2_out )   // TX_QMIX Q regulated:      SIGSIG 32 bit
);

rb_dsp48_CONaC_CON48_C48_P48 i_rb_tx_mod_qmix_Q_s3_dsp48 (
  // global signals
  .CLK                  ( clk_adc_125mhz       ),  // global 125 MHz clock
  .CE                   ( rb_clk_en & tx_amp_rf_q_en ),  // enable part 1 of RadioBox sub-module and the Q path

  // modulation input
  .C                    ( tx_mod_qmix_q_s3_in  ),  // modulation:               SIGNED 48 bit
  // offset value for OSC control
  .CONCAT               ( tx_mod_qmix_ofs      ),  // offset:                   SIGNED 48 bit

  // adder output
  .P                    ( tx_mod_qmix_q_s3_out )   // TX_QMIX Q for OSC:        SIGNED 48 bit
);


//---------------------------------------------------------------------------------
//  TX_MOD_CIC sampling rate down convertion 48 kSPS to 8 kSPS

reg          tx_mod_cic_s_vld_i = 'b0;
wire         tx_mod_cic_s_rdy_i;
wire [ 31:0] tx_mod_cic_i_out;
wire         tx_mod_cic_i_vld;
wire         tx_mod_cic_i_rdy;

reg          tx_mod_cic_s_vld_q = 'b0;
wire         tx_mod_cic_s_rdy_q;
wire [ 31:0] tx_mod_cic_q_out;
wire         tx_mod_cic_q_vld;
wire         tx_mod_cic_q_rdy;

always @(posedge clk_adc_125mhz)                // assign tx_mod_cic_s_vld_i
begin
   if (!rb_clk_en)
      tx_mod_cic_s_vld_i <= 'b0;
   else begin
      if (tx_mod_cic_s_vld_i && tx_mod_cic_s_rdy_i)
         tx_mod_cic_s_vld_i <= 'b0;             // falling back to non-active state

      if (clk_48khz)                            // trigger able to overwrite tx_mod_cic_s_vld_i
         tx_mod_cic_s_vld_i <= 'b1;             // entering active state
      end
end

always @(posedge clk_adc_125mhz)                // assign tx_mod_cic_s_vld_q
begin
   if (!rb_clk_en)
      tx_mod_cic_s_vld_q <= 'b0;
   else begin
      if (tx_mod_cic_s_vld_q && tx_mod_cic_s_rdy_q)
         tx_mod_cic_s_vld_q <= 'b0;             // falling back to non-active state

      if (clk_48khz)                            // trigger able to overwrite tx_mod_cic_s_vld_q
         tx_mod_cic_s_vld_q <= 'b1;             // entering active state
      end
end

rb_cic_48k_to_8k_32T32_lat13 i_rb_tx_mod_cic_I (
  // global signals
  .aclk                 ( clk_adc_125mhz       ),  // global 125 MHz clock
  .aclken               ( rb_clk_en            ),  // enable RadioBox sub-module
  .aresetn              ( rb_reset_n           ),

  .s_axis_data_tdata    ( tx_mod_qmix_i_s2_out ),  // TX_QMIX I stage 2
  .s_axis_data_tvalid   ( tx_mod_cic_s_vld_i   ),
  .s_axis_data_tready   ( tx_mod_cic_s_rdy_i   ),

  .m_axis_data_tdata    ( tx_mod_cic_i_out     ),  // TX_MOD_CIC output I
  .m_axis_data_tvalid   ( tx_mod_cic_i_vld     ),
  .m_axis_data_tready   ( tx_mod_cic_i_rdy     ),
  .event_halted         (                      )
);

rb_cic_48k_to_8k_32T32_lat13 i_rb_tx_mod_cic_Q (
  // global signals
  .aclk                 ( clk_adc_125mhz       ),  // global 125 MHz clock
  .aclken               ( rb_clk_en            ),  // enable RadioBox sub-module
  .aresetn              ( rb_reset_n           ),

  .s_axis_data_tdata    ( tx_mod_qmix_q_s2_out ),  // TX_QMIX Q stage 2
  .s_axis_data_tvalid   ( tx_mod_cic_s_vld_q   ),
  .s_axis_data_tready   ( tx_mod_cic_s_rdy_q   ),

  .m_axis_data_tdata    ( tx_mod_cic_q_out     ),  // TX_MOD_CIC output Q
  .m_axis_data_tvalid   ( tx_mod_cic_q_vld     ),
  .m_axis_data_tready   ( tx_mod_cic_q_rdy     ),
  .event_halted         (                      )
);


//---------------------------------------------------------------------------------
//  TX_MOD_FIR low pass filter for side-band selection
//
//  TX_MOD_FIR/RX_CAR_FIR/RX_MOD_FIR coefficients built with Octave:
//  hn = fir2(62, [0 0.38 0.39 1], [1 1 0.000001 0.000001], 512, kaiser(63,4));

wire [ 23:0] tx_mod_fir_i_in = {3'b0, tx_mod_cic_i_out[30:14]};
wire [ 39:0] tx_mod_fir_i_out;
wire         tx_mod_fir_i_vld;
wire         tx_mod_fir_i_rdy;

wire [ 23:0] tx_mod_fir_q_in = {3'b0, tx_mod_cic_q_out[30:14]};
wire [ 39:0] tx_mod_fir_q_out;
wire         tx_mod_fir_q_vld;
wire         tx_mod_fir_q_rdy;

rb_fir_8k_8k_25c23_17i16_35o33_lat42 i_rb_tx_mod_fir_I (
  // global signals
  .aclk                 ( clk_adc_125mhz       ),  // global 125 MHz clock
  .aclken               ( rb_clk_en            ),  // enable RadioBox sub-module
  .aresetn              ( rb_reset_n           ),

  .s_axis_data_tdata    ( tx_mod_fir_i_in      ),  // TX_MOD_CIC output I - 8 kHz (17.16 bit width)
  .s_axis_data_tvalid   ( tx_mod_cic_i_vld     ),
  .s_axis_data_tready   ( tx_mod_cic_i_rdy     ),

  .m_axis_data_tdata    ( tx_mod_fir_i_out     ),  // TX_MOD_FIR output I - 8kHz (35.30 bit width)
  .m_axis_data_tvalid   ( tx_mod_fir_i_vld     ),
  .m_axis_data_tready   ( tx_mod_fir_i_rdy     )
);

rb_fir_8k_8k_25c23_17i16_35o33_lat42 i_rb_tx_mod_fir_Q (
  // global signals
  .aclk                 ( clk_adc_125mhz       ),  // global 125 MHz clock
  .aclken               ( rb_clk_en            ),  // enable RadioBox sub-module
  .aresetn              ( rb_reset_n           ),

  .s_axis_data_tdata    ( tx_mod_fir_q_in      ),  // TX_MOD_CIC output Q - 8 kHz (17.16 bit width)
  .s_axis_data_tvalid   ( tx_mod_cic_q_vld     ),
  .s_axis_data_tready   ( tx_mod_cic_q_rdy     ),

  .m_axis_data_tdata    ( tx_mod_fir_q_out     ),  // TX_MOD_FIR output Q - 8 kHz (35.30 bit width)
  .m_axis_data_tvalid   ( tx_mod_fir_q_vld     ),
  .m_axis_data_tready   ( tx_mod_fir_q_rdy     )
);


//---------------------------------------------------------------------------------
//  TX_CAR_CIC sampling rate up convertion 8 kSPS to 41.664 MSPS

wire [ 31:0] tx_car_cic_41M664_i_out;
wire         tx_car_cic_41M664_i_vld;

wire [ 31:0] tx_car_cic_41M664_q_out;
wire         tx_car_cic_41M664_q_vld;

rb_cic_8k_to_41M664_32T32_lat14 i_rb_tx_car_cic_I (
  // global signals
  .aclk                 ( clk_adc_125mhz          ),  // global 125 MHz clock
  .aclken               ( rb_clk_en               ),  // enable RadioBox sub-module
  .aresetn              ( rb_reset_n              ),

  .s_axis_data_tdata    ( tx_mod_fir_i_out[32:1]  ),  // TX_MOD_FIR I - 8 kHz
  .s_axis_data_tvalid   ( tx_mod_fir_i_vld        ),
  .s_axis_data_tready   ( tx_mod_fir_i_rdy        ),

  .m_axis_data_tdata    ( tx_car_cic_41M664_i_out ),  // TX_CAR_CIC I stage 1 output - 1 MHz
  .m_axis_data_tvalid   ( tx_car_cic_41M664_i_vld )
);

rb_cic_8k_to_41M664_32T32_lat14 i_rb_tx_car_cic_Q (
  // global signals
  .aclk                 ( clk_adc_125mhz          ),  // global 125 MHz clock
  .aclken               ( rb_clk_en               ),  // enable RadioBox sub-module
  .aresetn              ( rb_reset_n              ),

  .s_axis_data_tdata    ( tx_mod_fir_q_out[32:1]  ),  // TX_MOD_FIR Q - 8 kHz
  .s_axis_data_tvalid   ( tx_mod_fir_q_vld        ),
  .s_axis_data_tready   ( tx_mod_fir_q_rdy        ),

  .m_axis_data_tdata    ( tx_car_cic_41M664_q_out ),  // TX_CAR_CIC Q stage 1 output - 1 MHz
  .m_axis_data_tvalid   ( tx_car_cic_41M664_q_vld )
);


//---------------------------------------------------------------------------------
//  TX_CAR_OSC carrier frequency oscillator  (CW, FM, PM modulated)
wire         tx_car_osc_inc_mux         = regs[REG_RW_RB_CTRL][RB_CTRL_TX_CAR_OSC_INC_SRC_STREAM];
wire         tx_car_osc_ofs_mux         = regs[REG_RW_RB_CTRL][RB_CTRL_TX_CAR_OSC_OFS_SRC_STREAM];
wire         tx_car_osc_reset_n         = rb_reset_n & !regs[REG_RW_RB_CTRL][RB_CTRL_RESET_TX_CAR_OSC];
wire         tx_car_osc_resync          = regs[REG_RW_RB_CTRL][RB_CTRL_TX_CAR_OSC_RESYNC];

wire [ 47:0] tx_car_osc_inc             = (tx_car_osc_inc_mux ?  tx_mod_qmix_i_s3_out : {regs[REG_RW_RB_TX_CAR_OSC_INC_HI][15:0], regs[REG_RW_RB_TX_CAR_OSC_INC_LO][31:0]});
wire [ 47:0] tx_car_osc_ofs             = (tx_car_osc_ofs_mux ?  tx_mod_qmix_i_s3_out : {regs[REG_RW_RB_TX_CAR_OSC_OFS_HI][15:0], regs[REG_RW_RB_TX_CAR_OSC_OFS_LO][31:0]});

wire         tx_car_osc_axis_s_vld      = rb_reset_n;  // TODO
wire [103:0] tx_car_osc_axis_s_phase    = {7'b0, tx_car_osc_resync, tx_car_osc_ofs, tx_car_osc_inc};

wire         tx_car_osc_axis_m_vld;
wire [ 31:0] tx_car_osc_axis_m_data;

wire [ 15:0] tx_car_osc_cos             = tx_car_osc_axis_m_data[15: 0];
wire [ 15:0] tx_car_osc_sin             = tx_car_osc_axis_m_data[31:16];

rb_dds_48_16_125 i_rb_tx_car_osc (
  // global signals
  .aclk                 ( clk_adc_125mhz          ),  // global 125 MHz clock
  .aclken               ( rb_clk_en               ),  // enable RadioBox sub-module
  .aresetn              ( tx_car_osc_reset_n      ),  // reset of TX_CAR_OSC

  // simple-AXI slave in port: streaming data for TX_CAR_OSC modulation
  .s_axis_phase_tvalid  ( tx_car_osc_axis_s_vld   ),  // AXIS slave data valid
  .s_axis_phase_tdata   ( tx_car_osc_axis_s_phase ),  // AXIS slave data

  // simple-AXI master out port: TX_CAR_OSC signal
  .m_axis_data_tvalid   ( tx_car_osc_axis_m_vld   ),  // AXIS master TX_CAR_OSC data valid
  .m_axis_data_tdata    ( tx_car_osc_axis_m_data  )   // AXIS master TX_CAR_OSC output: Q SIGNED 16 bit, I SIGNED 16 bit
);


//---------------------------------------------------------------------------------
//  TX_CAR_QMIX quadrature mixer for the radio frequency

wire [ 15:0] tx_car_qmix_i_in = (tx_car_osc_inc_mux || tx_car_osc_ofs_mux)                    ?  16'h7fff : tx_amp_rf_q_en ?  tx_car_cic_41M664_i_out[30:15] : tx_mod_qmix_i_s3_out[47:32];  // TX_MOD_QMIX/TX_MOD_CIC uses full scale constant for CW, FM and PM modulations - SSB uses TX_CIC I instead
wire [ 15:0] tx_car_qmix_q_in = (tx_car_osc_inc_mux || tx_car_osc_ofs_mux || !tx_amp_rf_q_en) ?  16'h0000 :                   tx_car_cic_41M664_q_out[30:15]                              ;  // TX_MOD_QMIX/TX_MOD_CIC Q path keep quiet when Q is disabled - SSB uses TX_CIC Q instead

wire [ 31:0] tx_car_qmix_i_out;
wire [ 31:0] tx_car_qmix_q_out;

rb_dsp48_AmB_A16_B16_P32 i_rb_tx_car_qmix_I_dsp48 (
  // global signals
  .CLK                  ( clk_adc_125mhz       ),  // global 125 MHz clock
  .CE                   ( rb_clk_en            ),  // enable part 1 of RadioBox sub-module

  // modulation input
  .A                    ( tx_car_qmix_i_in     ),  // TX_MUX in signal:     SIGNED 16 bit
  // TX_CAR_OSC cos input
  .B                    ( tx_car_osc_cos       ),  // TX_CAR_OSC cos:       SIGNED 16 bit

  // multiplier output
  .P                    ( tx_car_qmix_i_out    )   // TX_CAR_QMIX I output: SIGSIG 32 bit
);

rb_dsp48_AmB_A16_B16_P32 i_rb_tx_car_qmix_Q_dsp48 (
  // global signals
  .CLK                  ( clk_adc_125mhz       ),  // global 125 MHz clock
  .CE                   ( rb_clk_en & tx_amp_rf_q_en ),  // enable part 1 of RadioBox sub-module and the Q path

  // modulation input
  .A                    ( tx_car_qmix_q_in     ),  // TX_MUX in signal:     SIGNED 16 bit
  // TX_CAR_OSC sin input
  .B                    ( tx_car_osc_sin       ),  // TX_CAR_OSC sin:       SIGNED 16 bit

  // multiplier output
  .P                    ( tx_car_qmix_q_out    )   // TX_CAR_QMIX Q output: SIGSIG 32 bit
);


//---------------------------------------------------------------------------------
//  TX_AMP_CTL amplifier control for setting gain of the radio frequency amplifier


//---------------------------------------------------------------------------------
//  TX_RF_AMP amplifier for the radio frequency output (CW, AM modulated)

wire [16:0] tx_amp_rf_i_var =                   {tx_car_qmix_i_out[30], tx_car_qmix_i_out[30:15]}        ;  // halfed and sign corrected 17 bit extension
wire [16:0] tx_amp_rf_q_var = tx_amp_rf_q_en ?  {tx_car_qmix_q_out[30], tx_car_qmix_q_out[30:15]} : 17'b0;  // halfed and sign corrected 17 bit extension
wire [16:0] tx_amp_rf_gain  = {regs[REG_RW_RB_TX_RF_AMP_GAIN][15:0],  1'b0};  // signed register value
wire [34:0] tx_amp_rf_ofs   = {regs[REG_RW_RB_TX_RF_AMP_OFS ][15:0], 19'b0};  // signed register value

wire [35:0] tx_amp_rf_out;

rb_dsp48_AaDmBaC_A17_D17_B17_C35_P36 i_rb_tx_amp_rf_dsp48 (
  // global signals
  .CLK                  ( clk_adc_125mhz       ),  // global 125 MHz clock
  .CE                   ( rb_clk_en            ),  // enable part 1 of RadioBox sub-module
  .SCLR                 ( !rb_enable           ),  // put output to neutral when activated

  // TX_QMIX RF I output
  .A                    ( tx_amp_rf_i_var      ),  // TX_QMIX_RF I      SIGNED 17 bit
  // TX_QMIX RF Q output
  .D                    ( tx_amp_rf_q_var      ),  // TX_QMIX_RF Q      SIGNED 17 bit
  // TX_AMP RF gain
  .B                    ( tx_amp_rf_gain       ),  // TX_RF_AMP gain    SIGNED 17 bit
  // TX_AMP RF offset
  .C                    ( tx_amp_rf_ofs        ),  // TX_RF_AMP ofs     SIGSIG 35 bit

  // TX_AMP RF output
  .P                    ( tx_amp_rf_out        )   // TX_AMP RF output  SIGSIG 36 bit
);


// === Receiver section ===

//---------------------------------------------------------------------------------
//  RX_CAR_OSC carrier oscillator

wire         rx_car_osc_reset_n         = rb_reset_n & !regs[REG_RW_RB_CTRL][RB_CTRL_RESET_RX_CAR_OSC];
wire         rx_car_osc_resync          = regs[REG_RW_RB_CTRL][RB_CTRL_RX_CAR_OSC_RESYNC];

wire [ 47:0] rx_car_osc_inc             = {regs[REG_RW_RB_RX_CAR_OSC_INC_HI][15:0], regs[REG_RW_RB_RX_CAR_OSC_INC_LO][31:0]};
wire [ 47:0] rx_car_osc_ofs             = {regs[REG_RW_RB_RX_CAR_OSC_OFS_HI][15:0], regs[REG_RW_RB_RX_CAR_OSC_OFS_LO][31:0]};

wire         rx_car_osc_axis_s_vld      = rb_reset_n;  // TODO
wire [103:0] rx_car_osc_axis_s_phase    = {7'b0, rx_car_osc_resync, rx_car_osc_ofs, rx_car_osc_inc};

wire         rx_car_osc_axis_m_vld;
wire [ 31:0] rx_car_osc_axis_m_data;

wire [ 15:0] rx_car_osc_cos             = rx_car_osc_axis_m_data[15: 0];
wire [ 15:0] rx_car_osc_sin             = rx_car_osc_axis_m_data[31:16];

rb_dds_48_16_125 i_rb_rx_car_osc (
  // global signals
  .aclk                 ( clk_adc_125mhz          ),  // global 125 MHz clock
  .aclken               ( rb_clk_en               ),  // enable RadioBox sub-module
  .aresetn              ( rx_car_osc_reset_n      ),  // reset of RX_CAR_OSC

  // simple-AXI slave in port: streaming data for RX_CAR_OSC modulation
  .s_axis_phase_tvalid  ( rx_car_osc_axis_s_vld   ),  // AXIS slave data valid
  .s_axis_phase_tdata   ( rx_car_osc_axis_s_phase ),  // AXIS slave data

  // simple-AXI master out port: RX_CAR_OSC signal
  .m_axis_data_tvalid   ( rx_car_osc_axis_m_vld   ),  // AXIS master TX_CAR_OSC data valid
  .m_axis_data_tdata    ( rx_car_osc_axis_m_data  )   // AXIS master TX_CAR_OSC output: Q SIGNED 16 bit, I SIGNED 16 bit
);


//---------------------------------------------------------------------------------
//  RX_CAR_QMIX quadrature mixer for the radio frequency

//wire [ 15:0] rx_mod_qmix_in_gain = regs[REG_RW_RB_RX_MUXIN_GAIN][15:0];

wire [ 15:0] rx_car_qmix_in = regs[REG_RW_RB_RX_MUXIN_SRC] == 1 ?  {~adc_i[0], 2'b0} :
                              regs[REG_RW_RB_RX_MUXIN_SRC] == 2 ?  {~adc_i[1], 2'b0} :
                                                                    16'b0            ;

wire [ 31:0] rx_car_qmix_i_out;
wire [ 31:0] rx_car_qmix_q_out;

rb_dsp48_AmB_A16_B16_P32 i_rb_rx_car_qmix_I_dsp48 (
  // global signals
  .CLK                  ( clk_adc_125mhz       ),  // global 125 MHz clock
  .CE                   ( rb_clk_en            ),  // enable part 1 of RadioBox sub-module

  // RF input
  .A                    ( rx_car_qmix_in       ),  // RX_MUX in signal:     SIGNED 16 bit
  // RX_CAR_OSC cos input
  .B                    ( rx_car_osc_cos       ),  // RX_CAR_OSC cos:       SIGNED 16 bit

  // multiplier output
  .P                    ( rx_car_qmix_i_out    )   // RX_CAR_QMIX I output: SIGSIG 32 bit
);

rb_dsp48_AmB_A16_B16_P32 i_rb_rx_car_qmix_Q_dsp48 (
  // global signals
  .CLK                  ( clk_adc_125mhz       ),  // global 125 MHz clock
  .CE                   ( rb_clk_en            ),  // enable part 1 of RadioBox sub-module  - TODO and the Q path

  // RF input
  .A                    ( rx_car_qmix_in       ),  // RX_MUX in signal:     SIGNED 16 bit
  // RX_CAR_OSC sin input
  .B                    ( rx_car_osc_sin       ),  // RX_CAR_OSC sin:       SIGNED 16 bit

  // multiplier output
  .P                    ( rx_car_qmix_q_out    )   // RX_CAR_QMIX Q output: SIGSIG 32 bit
);


//---------------------------------------------------------------------------------
//  RX_CAR_CIC1 sampling rate down convertion 125 MSPS to 5 MSPS

wire [ 31:0] rx_car_cic1_i_out;
wire         rx_car_cic1_i_vld;
wire         rx_car_cic1_i_rdy;
wire         rx_car_cic1_i_hlt;

wire [ 31:0] rx_car_cic1_q_out;
wire         rx_car_cic1_q_vld;
wire         rx_car_cic1_q_rdy;
wire         rx_car_cic1_q_hlt;

rb_cic_125M_to_5M_32T32_lat18 i_rb_rx_car_cic1_I (
  // global signals
  .aclk                 ( clk_adc_125mhz       ),  // global 125 MHz clock
  .aclken               ( rb_clk_en            ),  // enable RadioBox sub-module
  .aresetn              ( rb_reset_n           ),

  .s_axis_data_tdata    ( rx_car_qmix_i_out    ),  // RX_CAR_QMIX I
  .s_axis_data_tvalid   ( 1'b1                 ),
  .s_axis_data_tready   (                      ),

  .m_axis_data_tdata    ( rx_car_cic1_i_out    ),  // RX_CAR_CIC1 output I
  .m_axis_data_tvalid   ( rx_car_cic1_i_vld    ),
  .m_axis_data_tready   ( rx_car_cic1_i_rdy    ),
  .event_halted         ( rx_car_cic1_i_hlt    )
);

rb_cic_125M_to_5M_32T32_lat18 i_rb_rx_car_cic1_Q (
  // global signals
  .aclk                 ( clk_adc_125mhz       ),  // global 125 MHz clock
  .aclken               ( rb_clk_en            ),  // enable RadioBox sub-module
  .aresetn              ( rb_reset_n           ),

  .s_axis_data_tdata    ( rx_car_qmix_q_out    ),  // RX_CAR_QMIX Q
  .s_axis_data_tvalid   ( 1'b1                 ),
  .s_axis_data_tready   (                      ),

  .m_axis_data_tdata    ( rx_car_cic1_q_out    ),  // RX_CAR_CIC1 output Q
  .m_axis_data_tvalid   ( rx_car_cic1_q_vld    ),
  .m_axis_data_tready   ( rx_car_cic1_q_rdy    ),
  .event_halted         ( rx_car_cic1_q_hlt    )
);


//---------------------------------------------------------------------------------
//  RX_CAR_CIC2 sampling rate down convertion 4.992 MSPS to 48 kSPS

wire [ 31:0] rx_car_cic2_i_out;
wire         rx_car_cic2_i_vld;
wire         rx_car_cic2_i_rdy;
wire         rx_car_cic2_i_hlt;

wire [ 31:0] rx_car_cic2_q_out;
wire         rx_car_cic2_q_vld;
wire         rx_car_cic2_q_rdy;
wire         rx_car_cic2_q_hlt;

rb_cic_4M992_to_48k_32T32 i_rb_rx_car_cic2_I (
  // global signals
  .aclk                 ( clk_adc_125mhz       ),  // global 125 MHz clock
  .aclken               ( rb_clk_en            ),  // enable RadioBox sub-module
  .aresetn              ( rb_reset_n           ),

  .s_axis_data_tdata    ( {rx_car_cic1_i_out[30:0], 1'b0}),
  .s_axis_data_tvalid   ( rx_car_cic1_i_vld    ),
  .s_axis_data_tready   ( rx_car_cic1_i_rdy    ),

  .m_axis_data_tdata    ( rx_car_cic2_i_out    ),  // RX_CAR_CIC2 output I
  .m_axis_data_tvalid   ( rx_car_cic2_i_vld    ),
  .m_axis_data_tready   ( rx_car_cic2_i_rdy    ),
  .event_halted         ( rx_car_cic2_i_hlt    )
);

rb_cic_4M992_to_48k_32T32 i_rb_rx_car_cic2_Q (
  // global signals
  .aclk                 ( clk_adc_125mhz       ),  // global 125 MHz clock
  .aclken               ( rb_clk_en            ),  // enable RadioBox sub-module
  .aresetn              ( rb_reset_n           ),

  .s_axis_data_tdata    ( {rx_car_cic1_q_out[30:0], 1'b0}),
  .s_axis_data_tvalid   ( rx_car_cic1_q_vld    ),
  .s_axis_data_tready   ( rx_car_cic1_q_rdy    ),

  .m_axis_data_tdata    ( rx_car_cic2_q_out    ),  // RX_CAR_CIC2 output Q
  .m_axis_data_tvalid   ( rx_car_cic2_q_vld    ),
  .m_axis_data_tready   ( rx_car_cic2_q_rdy    ),
  .event_halted         ( rx_car_cic2_q_hlt    )
);


//---------------------------------------------------------------------------------
//  RX_CAR_FIFO

wire [ 31:0] rx_car_fifo_i_out;
wire         rx_car_fifo_i_vld;
wire         rx_car_fifo_i_rdy;
wire         rx_car_fifo_i_ovl;
wire         rx_car_fifo_i_ufl;

wire [ 31:0] rx_car_fifo_q_out;
wire         rx_car_fifo_q_vld;
wire         rx_car_fifo_q_rdy;
wire         rx_car_fifo_q_ovl;
wire         rx_car_fifo_q_ufl;

rb_fifo_axis_W32_D16 i_rb_rx_car_fifo_I (
  // global signals
  .s_aclk               ( clk_adc_125mhz       ),  // global 125 MHz clock
  .s_aclk_en            ( rb_clk_en            ),  // enable RadioBox sub-module
  .s_aresetn            ( rb_reset_n           ),

  .s_axis_tdata         ( rx_car_cic2_i_out    ),  // RX_MOD_QMIX output I
  .s_axis_tvalid        ( rx_car_cic2_i_vld    ),
  .s_axis_tready        ( rx_car_cic2_i_rdy    ),

  .m_axis_tdata         ( rx_car_fifo_i_out    ),
  .m_axis_tvalid        ( rx_car_fifo_i_vld    ),
  .m_axis_tready        ( rx_car_fifo_i_rdy    ),

  .axis_overflow        ( rx_car_fifo_i_ovl    ),
  .axis_underflow       ( rx_car_fifo_i_ufl    )
);

rb_fifo_axis_W32_D16 i_rb_rx_car_fifo_Q (
  // global signals
  .s_aclk               ( clk_adc_125mhz       ),  // global 125 MHz clock
  .s_aclk_en            ( rb_clk_en            ),  // enable RadioBox sub-module
  .s_aresetn            ( rb_reset_n           ),

  .s_axis_tdata         ( rx_car_cic2_q_out    ),  // RX_MOD_QMIX output Q
  .s_axis_tvalid        ( rx_car_cic2_q_vld    ),
  .s_axis_tready        ( rx_car_cic2_q_rdy    ),

  .m_axis_tdata         ( rx_car_fifo_q_out    ),
  .m_axis_tvalid        ( rx_car_fifo_q_vld    ),
  .m_axis_tready        ( rx_car_fifo_q_rdy    ),

  .axis_overflow        ( rx_car_fifo_q_ovl    ),
  .axis_underflow       ( rx_car_fifo_q_ufl    )
);


//---------------------------------------------------------------------------------
//  RX_CAR_FIR low pass filter for side-band selection
//
//  TX_MOD_FIR/RX_CAR_FIR/RX_MOD_FIR coefficients built with Octave:
//  hn = fir2(62, [0 0.38 0.39 1], [1 1 0.000001 0.000001], 512, kaiser(63,4));

wire [ 23:0] rx_car_fir_i_in = {3'b0, rx_car_fifo_i_out[30:14]};  // bus width is multiple of 8
wire [ 39:0] rx_car_fir_i_out;
wire         rx_car_fir_i_vld;
wire         rx_car_fir_i_rdy;

wire [ 23:0] rx_car_fir_q_in = {3'b0, rx_car_fifo_q_out[30:14]};
wire [ 39:0] rx_car_fir_q_out;
wire         rx_car_fir_q_vld;
wire         rx_car_fir_q_rdy;

rb_fir_48k_48k_25c23_17i16_35o33 i_rb_rx_car_fir_I (
  // global signals
  .aclk                 ( clk_adc_125mhz       ),   // global 125 MHz clock
  .aclken               ( rb_clk_en            ),   // enable RadioBox sub-module
  .aresetn              ( rb_reset_n           ),

  .s_axis_data_tdata    ( rx_car_fir_i_in      ),
  .s_axis_data_tvalid   ( rx_car_fifo_i_vld    ),
  .s_axis_data_tready   ( rx_car_fifo_i_rdy    ),

  .m_axis_data_tdata    ( rx_car_fir_i_out     ),   // RX_CAR_FIR output I - 48kHz (35.33 bit width)
  .m_axis_data_tvalid   ( rx_car_fir_i_vld     ),
  .m_axis_data_tready   ( rx_car_fir_i_rdy     )
);

rb_fir_48k_48k_25c23_17i16_35o33 i_rb_rx_car_fir_Q (
  // global signals
  .aclk                 ( clk_adc_125mhz       ),   // global 125 MHz clock
  .aclken               ( rb_clk_en            ),   // enable RadioBox sub-module
  .aresetn              ( rb_reset_n           ),

  .s_axis_data_tdata    ( rx_car_fir_q_in      ),
  .s_axis_data_tvalid   ( rx_car_fifo_q_vld    ),
  .s_axis_data_tready   ( rx_car_fifo_q_rdy    ),

  .m_axis_data_tdata    ( rx_car_fir_q_out     ),   // RX_CAR_FIR output Q - 48 kHz (35.33 bit width)
  .m_axis_data_tvalid   ( rx_car_fir_q_vld     ),
  .m_axis_data_tready   ( rx_car_fir_q_rdy     )
);


//---------------------------------------------------------------------------------
//  RX_MOD_OSC modulation oscillator and SSB weaver modulator

wire         rx_mod_osc_reset_n = rb_reset_n & !regs[REG_RW_RB_CTRL][RB_CTRL_RESET_RX_MOD_OSC];
wire         rx_mod_osc_resync  = regs[REG_RW_RB_CTRL][RB_CTRL_RX_MOD_OSC_RESYNC];

wire [ 47:0] rx_mod_osc_inc = {regs[REG_RW_RB_RX_MOD_OSC_INC_HI][15:0], regs[REG_RW_RB_RX_MOD_OSC_INC_LO][31:0]};
wire [ 47:0] rx_mod_osc_ofs = {regs[REG_RW_RB_RX_MOD_OSC_OFS_HI][15:0], regs[REG_RW_RB_RX_MOD_OSC_OFS_LO][31:0]};

wire         rx_mod_osc_axis_s_vld   = rb_reset_n;  // TODO: ADC
wire [103:0] rx_mod_osc_axis_s_phase = {7'b0, rx_mod_osc_resync, rx_mod_osc_ofs, rx_mod_osc_inc};

wire         rx_mod_osc_axis_m_vld;
wire [ 31:0] rx_mod_osc_axis_m_data;
wire [ 15:0] rx_mod_osc_cos = rx_mod_osc_axis_m_data[15: 0];
wire [ 15:0] rx_mod_osc_sin = rx_mod_osc_axis_m_data[31:16];

rb_dds_48_16_125 i_rb_rx_mod_osc (
  // global signals
  .aclk                 ( clk_adc_125mhz          ),  // global 125 MHz clock
  .aclken               ( rb_clk_en               ),  // clock enable of RadioBox sub-module
  .aresetn              ( rx_mod_osc_reset_n      ),  // reset of RX_MOD_OSC

  // AXI-Stream slave in port: streaming data for RX_MOD_OSC modulation
  .s_axis_phase_tvalid  ( rx_mod_osc_axis_s_vld   ),  // AXIS slave data valid
  .s_axis_phase_tdata   ( rx_mod_osc_axis_s_phase ),  // AXIS slave data

  // AXI-Stream master out port: RX_MOD_OSC signal
  .m_axis_data_tvalid   ( rx_mod_osc_axis_m_vld   ),  // AXIS master RX_MOD_OSC data valid
  .m_axis_data_tdata    ( rx_mod_osc_axis_m_data  )   // AXIS master RX_MOD_OSC output: 2x SIGNED 16 bit
);


//---------------------------------------------------------------------------------
//  RX_MOD_OSC_HLD data hold

reg  [ 15:0] rx_mod_osc_hld_i = 'b0;
reg  [ 15:0] rx_mod_osc_hld_q = 'b0;

always @(posedge clk_adc_125mhz)
if (!rx_mod_osc_reset_n)
   rx_mod_osc_hld_i <= 'b0;
else if (rx_car_fir_i_vld && rx_car_fir_i_rdy)
   rx_mod_osc_hld_i <= rx_mod_osc_cos;

always @(posedge clk_adc_125mhz)
if (!rx_mod_osc_reset_n)
   rx_mod_osc_hld_q <= 'b0;
else if (rx_car_fir_q_vld && rx_car_fir_q_rdy)
   rx_mod_osc_hld_q <= rx_mod_osc_sin;


//---------------------------------------------------------------------------------
//  RX_MOD_QMIX quadrature mixer for the base band

//wire        tx_amp_rf_q_en = regs[REG_RW_RB_CTRL][RB_CTRL_TX_RF_AMP_Q_EN];

wire [15:0] rx_mod_qmix_i_in = rx_car_fir_i_out[33:18];
wire [15:0] rx_mod_qmix_q_in = rx_car_fir_q_out[33:18];

wire [31:0] rx_mod_qmix_i_out;
wire [31:0] rx_mod_qmix_q_out;

rb_dsp48_AmB_A16_B16_P32 i_rb_rx_mod_qmix_I_dsp48 (
  // global signals
  .CLK                  ( clk_adc_125mhz       ),  // global 125 MHz clock
  .CE                   ( rb_clk_en            ),  // enable part 1 of RadioBox sub-module

  // modulation input
  .A                    ( rx_mod_qmix_i_in     ),  // RX_CAR_QMIX in I signal:  SIGNED 16 bit
  // TX_MOD_OSC cos input
  .B                    ( rx_mod_osc_hld_i     ),  // RX_MOD_OSC cos:           SIGNED 16 bit

  // multiplier output stage 1
  .P                    ( rx_mod_qmix_i_out    )   // RX_MOD_QMIX I output:     SIGSIG 32 bit
);

rb_dsp48_AmB_A16_B16_P32 i_rb_rx_mod_qmix_Q_dsp48 (
  // global signals
  .CLK                  ( clk_adc_125mhz       ),  // global 125 MHz clock
  .CE                   ( rb_clk_en            ),  // - TODO enable part 1 of RadioBox sub-module and the Q path

  // modulation input
  .A                    ( rx_mod_qmix_q_in     ),  // RX_CAR_QMIX in Q signal:  SIGNED 16 bit
  // TX_MOD_OSC sin input
  .B                    ( rx_mod_osc_hld_q     ),  // RX_MOD_OSC sin:           SIGNED 16 bit

  // multiplier output stage 1
  .P                    ( rx_mod_qmix_q_out    )   // RX_MOD_QMIX I output:     SIGSIG 32 bit
);


//---------------------------------------------------------------------------------
//  RX_MOD_FIFO1

wire [ 31:0] rx_mod_fifo1_i_out;
wire         rx_mod_fifo1_i_vld;
wire         rx_mod_fifo1_i_rdy;
wire         rx_mod_fifo1_i_ovl;
wire         rx_mod_fifo1_i_ufl;

wire [ 31:0] rx_mod_fifo1_q_out;
wire         rx_mod_fifo1_q_vld;
wire         rx_mod_fifo1_q_rdy;
wire         rx_mod_fifo1_q_ovl;
wire         rx_mod_fifo1_q_ufl;

rb_fifo_axis_W32_D16 i_rb_rx_mod_fifo1_I (
  // global signals
  .s_aclk               ( clk_adc_125mhz       ),  // global 125 MHz clock
  .s_aclk_en            ( rb_clk_en            ),  // enable RadioBox sub-module
  .s_aresetn            ( rb_reset_n           ),

  .s_axis_tdata         ( rx_mod_qmix_i_out    ),  // RX_MOD_QMIX output I
  .s_axis_tvalid        ( rx_car_fir_i_vld     ),
  .s_axis_tready        ( rx_car_fir_i_rdy     ),

  .m_axis_tdata         ( rx_mod_fifo1_i_out   ),
  .m_axis_tvalid        ( rx_mod_fifo1_i_vld   ),
  .m_axis_tready        ( rx_mod_fifo1_i_rdy   ),

  .axis_overflow        ( rx_mod_fifo1_i_ovl   ),
  .axis_underflow       ( rx_mod_fifo1_i_ufl   )
);

rb_fifo_axis_W32_D16 i_rb_rx_mod_fifo1_Q (
  // global signals
  .s_aclk               ( clk_adc_125mhz       ),  // global 125 MHz clock
  .s_aclk_en            ( rb_clk_en            ),  // enable RadioBox sub-module
  .s_aresetn            ( rb_reset_n           ),

  .s_axis_tdata         ( rx_mod_qmix_i_out    ),  // RX_MOD_QMIX output Q
  .s_axis_tvalid        ( rx_car_fir_q_vld     ),
  .s_axis_tready        ( rx_car_fir_q_rdy     ),

  .m_axis_tdata         ( rx_mod_fifo1_q_out   ),
  .m_axis_tvalid        ( rx_mod_fifo1_q_vld   ),
  .m_axis_tready        ( rx_mod_fifo1_q_rdy   ),

  .axis_overflow        ( rx_mod_fifo1_q_ovl   ),
  .axis_underflow       ( rx_mod_fifo1_q_ufl   )
);


//---------------------------------------------------------------------------------
//  RX_MOD_FIR low pass filter for side-band selection
//
//  TX_MOD_FIR/RX_CAR_FIR/RX_MOD_FIR coefficients built with Octave:
//  hn = fir2(62, [0 0.38 0.39 1], [1 1 0.000001 0.000001], 512, kaiser(63,4));

wire [ 23:0] rx_mod_fir_i_in = {3'b0, rx_mod_fifo1_i_out[30:14]};  // bus width is multiple of 8
wire [ 39:0] rx_mod_fir_i_out;
wire         rx_mod_fir_i_vld;
wire         rx_mod_fir_i_rdy;

wire [ 23:0] rx_mod_fir_q_in = {3'b0, rx_mod_fifo1_q_out[30:14]};
wire [ 39:0] rx_mod_fir_q_out;
wire         rx_mod_fir_q_vld;
wire         rx_mod_fir_q_rdy;

rb_fir_8k_8k_25c23_17i16_35o33_lat42 i_rb_rx_mod_fir_I (
  // global signals
  .aclk                 ( clk_adc_125mhz       ),   // global 125 MHz clock
  .aclken               ( rb_clk_en            ),   // enable RadioBox sub-module
  .aresetn              ( rb_reset_n           ),

  .s_axis_data_tdata    ( rx_mod_fir_i_in      ),   // RX_MOD_QMIX output I - 8 kHz
  .s_axis_data_tvalid   ( rx_mod_fifo1_i_vld   ),
  .s_axis_data_tready   ( rx_mod_fifo1_i_rdy   ),

  .m_axis_data_tdata    ( rx_mod_fir_i_out     ),   // RX_MOD_FIR output I - 8kHz (35.33 bit width)
  .m_axis_data_tvalid   ( rx_mod_fir_i_vld     ),
  .m_axis_data_tready   ( rx_mod_fir_i_rdy     )
);

rb_fir_8k_8k_25c23_17i16_35o33_lat42 i_rb_rx_mod_fir_Q (
  // global signals
  .aclk                 ( clk_adc_125mhz       ),   // global 125 MHz clock
  .aclken               ( rb_clk_en            ),   // enable RadioBox sub-module
  .aresetn              ( rb_reset_n           ),

  .s_axis_data_tdata    ( rx_mod_fir_q_in      ),   // RX_MOD_QMIX output Q - 8 kHz
  .s_axis_data_tvalid   ( rx_mod_fifo1_q_vld   ),
  .s_axis_data_tready   ( rx_mod_fifo1_q_rdy   ),

  .m_axis_data_tdata    ( rx_mod_fir_q_out     ),   // RX_MOD_FIR output Q - 8 kHz (35.33 bit width)
  .m_axis_data_tvalid   ( rx_mod_fir_q_vld     ),
  .m_axis_data_tready   ( rx_mod_fir_q_rdy     )
);


//---------------------------------------------------------------------------------
//  RX_MOD_CIC sampling rate up convertion 8 kSPS to 48 kSPS

wire [ 31:0] rx_mod_cic_i_out;
wire         rx_mod_cic_i_vld;
wire         rx_mod_cic_i_rdy;

wire [ 31:0] rx_mod_cic_q_out;
wire         rx_mod_cic_q_vld;
wire         rx_mod_cic_q_rdy;

rb_cic_8k_to_48k_32T32_lat16 i_rb_rx_mod_cic_I (
  // global signals
  .aclk                 ( clk_adc_125mhz       ),  // global 125 MHz clock
  .aclken               ( rb_clk_en            ),  // enable RadioBox sub-module
  .aresetn              ( rb_reset_n           ),

  .s_axis_data_tdata    ( rx_mod_fir_i_out[33:2]),
  .s_axis_data_tvalid   ( rx_mod_fir_i_vld     ),
  .s_axis_data_tready   ( rx_mod_fir_i_rdy     ),
//.s_axis_data_tdata    ( rx_car_fir_i_out[33:2]),
//.s_axis_data_tvalid   ( rx_car_fir_i_vld     ),
//.s_axis_data_tready   ( rx_car_fir_i_rdy     ),

  .m_axis_data_tdata    ( rx_mod_cic_i_out     ),  // RX_MOD_CIC output I
  .m_axis_data_tvalid   ( rx_mod_cic_i_vld     )
);

rb_cic_8k_to_48k_32T32_lat16 i_rb_rx_mod_cic_Q (
  // global signals
  .aclk                 ( clk_adc_125mhz       ),  // global 125 MHz clock
  .aclken               ( rb_clk_en            ),  // enable RadioBox sub-module
  .aresetn              ( rb_reset_n           ),

  .s_axis_data_tdata    ( rx_mod_fir_q_out[33:2]),
  .s_axis_data_tvalid   ( rx_mod_fir_q_vld     ),
  .s_axis_data_tready   ( rx_mod_fir_q_rdy     ),
//.s_axis_data_tdata    ( rx_car_fir_q_out[33:2]),
//.s_axis_data_tvalid   ( rx_car_fir_q_vld     ),
//.s_axis_data_tready   ( rx_car_fir_q_rdy     ),

  .m_axis_data_tdata    ( rx_mod_cic_q_out     ),  // RX_MOD_CIC output Q
  .m_axis_data_tvalid   ( rx_mod_cic_q_vld     )
);


//---------------------------------------------------------------------------------
//  RX_MOD_ADD reconstruction of the modulation

//wire        rx_mod_add_en = regs[REG_RW_RB_CTRL][RB_CTRL_RX_MOD_ADD_Q_EN];

wire [16:0] rx_mod_add_i_var = {rx_mod_cic_i_out[30], rx_mod_cic_i_out[30:15]};  // halfed and sign corrected 17 bit extension
wire [16:0] rx_mod_add_q_var = {rx_mod_cic_q_out[30], rx_mod_cic_i_out[30:15]};  // halfed and sign corrected 17 bit extension
wire [16:0] rx_mod_add_gain  = {regs[REG_RW_RB_RX_MOD_ADD_GAIN][15:0],  1'b0};   // signed register value
wire [34:0] rx_mod_add_ofs   = {regs[REG_RW_RB_RX_MOD_ADD_OFS ][15:0], 19'b0};   // signed register value

wire [35:0] rx_mod_add_out;

rb_dsp48_AaDmBaC_A17_D17_B17_C35_P36 i_rb_rx_mod_decoder_dsp48 (
  // global signals
  .CLK                  ( clk_adc_125mhz       ),  // global 125 MHz clock
  .CE                   ( rb_clk_en            ),  // enable part 1 of RadioBox sub-module
  .SCLR                 ( !rb_enable           ),  // put output to neutral when activated

  // RX_MOD_ADD I input
  .A                    ( rx_mod_add_i_var     ),  // TX_QMIX_RF I      SIGNED 17 bit
  // RX_MOD_ADD Q input
  .D                    ( rx_mod_add_q_var     ),  // TX_QMIX_RF Q      SIGNED 17 bit
  // RX_MOD_ADD gain
  .B                    ( rx_mod_add_gain      ),  // RX_MOD_ADD gain   SIGNED 17 bit
  // RX_MOD_ADD offset
  .C                    ( rx_mod_add_ofs       ),  // RX_MOD_ADD ofs    SIGSIG 35 bit

  // RX_MOD_ADD output
  .P                    ( rx_mod_add_out       )   // RX_MOD_ADD output SIGSIG 36 bit
);


// === Connection Matrix section ===

//---------------------------------------------------------------------------------
//  LEDs Magnitude indicator

reg  [19: 0] led_ctr  = 20'b0;

wire [ 5: 0] led_src_con_pnt = regs[REG_RW_RB_RFOUTx_LED_SRC_CON_PNT][ 5: 0];

function bit [7:0] fct_mag (input bit [15:0] val);
   automatic bit [7:0] leds = 8'b0;             // exact zero indicator

   if (!val[15]) begin                          // positive value
      if (val[14])
         leds = 8'b11110000;
      else if (val[14:12] >= 3'b001)
         leds = 8'b01110000;
      else if (val[14:10] >= 5'b00001)
         leds = 8'b00110000;
      else if (val)
         leds = 8'b00010000;
      end

   else begin                                   // negative value
      if (!val[14])
         leds = 8'b00001111;
      else if (val[14:12] <= 3'b110)
         leds = 8'b00001110;
      else if (val[14:10] <= 5'b11110)
         leds = 8'b00001100;
      else
         leds = 8'b00001000;
      end

   fct_mag = leds;
endfunction: fct_mag

always @(posedge clk_adc_125mhz)
if (!adc_rstn_i || !rb_reset_n) begin
   rb_leds_en    <=  1'b0;
   rb_leds_data  <=  8'b0;
   led_ctr       <= 20'b0;
   end

else begin
   if (led_src_con_pnt && rb_activated) begin
      rb_leds_en <=  1'b1;                      // LEDs magnitude indicator active
       case (led_src_con_pnt)

       RB_SRC_CON_PNT_NUM_DISABLED: begin
          rb_leds_data <=  8'b0;
          end
       RB_SRC_CON_PNT_NUM_OFF: begin
          rb_leds_data <=  8'b0;                // turn all LEDs off
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
          if (!led_ctr && tx_mod_cic_i_vld)
             rb_leds_data <= fct_mag(tx_mod_cic_i_out[30:15]);
          end
       RB_SRC_CON_PNT_NUM_TX_MOD_CIC_Q_OUT: begin
          if (!led_ctr && tx_mod_cic_q_vld)
             rb_leds_data <= fct_mag(tx_mod_cic_q_out[30:15]);
          end
       RB_SRC_CON_PNT_NUM_TX_MOD_FIR_I_OUT: begin
          if (!led_ctr && tx_mod_fir_i_vld)
             rb_leds_data <= fct_mag(tx_mod_fir_i_out[32:17]);
          end
       RB_SRC_CON_PNT_NUM_TX_MOD_FIR_Q_OUT: begin
          if (!led_ctr && tx_mod_fir_q_vld)
             rb_leds_data <= fct_mag(tx_mod_fir_q_out[32:17]);
          end
       RB_SRC_CON_PNT_NUM_TX_CAR_CIC_41M664_I_OUT: begin
          if (!led_ctr && tx_car_cic_41M664_i_vld)
             rb_leds_data <= fct_mag(tx_car_cic_41M664_i_out[30:15]);
          end
       RB_SRC_CON_PNT_NUM_TX_CAR_CIC_41M664_Q_OUT: begin
          if (!led_ctr && tx_car_cic_41M664_q_vld)
             rb_leds_data <= fct_mag(tx_car_cic_41M664_q_out[30:15]);
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
          if (!led_ctr && tx_car_cic_41M664_i_vld)
             rb_leds_data <= fct_mag(tx_car_qmix_i_out[30:15]);
          end
       RB_SRC_CON_PNT_NUM_TX_CAR_QMIX_Q_OUT: begin
          if (!led_ctr && tx_car_cic_41M664_q_vld)
             rb_leds_data <= fct_mag(tx_car_qmix_q_out[30:15]);
          end

       RB_SRC_CON_PNT_NUM_TX_RF_AMP_OUT: begin
          if (!led_ctr && tx_car_cic_41M664_i_vld)
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
       RB_SRC_CON_PNT_NUM_RX_CAR_CIC1_I_OUT: begin
          if (!led_ctr && rx_car_cic1_i_vld)
             rb_leds_data <= fct_mag(rx_car_cic1_i_out[30:15]);
          end
       RB_SRC_CON_PNT_NUM_RX_CAR_CIC1_Q_OUT: begin
          if (!led_ctr && rx_car_cic1_q_vld)
             rb_leds_data <= fct_mag(rx_car_cic1_q_out[30:15]);
          end
       RB_SRC_CON_PNT_NUM_RX_CAR_CIC2_I_OUT: begin
          if (!led_ctr && rx_car_cic2_i_vld)
             rb_leds_data <= fct_mag(rx_car_cic2_i_out[30:15]);
          end
       RB_SRC_CON_PNT_NUM_RX_CAR_CIC2_Q_OUT: begin
          if (!led_ctr && rx_car_cic2_q_vld)
             rb_leds_data <= fct_mag(rx_car_cic2_q_out[30:15]);
          end
       RB_SRC_CON_PNT_NUM_RX_CAR_FIR_I_OUT: begin
          if (!led_ctr && rx_car_fir_i_vld)
             rb_leds_data <= fct_mag(rx_car_fir_i_out[33:18]);
          end
       RB_SRC_CON_PNT_NUM_RX_CAR_FIR_Q_OUT: begin
          if (!led_ctr && rx_car_fir_q_vld)
             rb_leds_data <= fct_mag(rx_car_fir_q_out[33:18]);
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
             rb_leds_data <= fct_mag(rx_mod_osc_hld_i[15:0]);
          end
       RB_SRC_CON_PNT_NUM_RX_MOD_HLD_Q_OUT: begin
          if (!led_ctr)
             rb_leds_data <= fct_mag(rx_mod_osc_hld_q[15:0]);
          end
       RB_SRC_CON_PNT_NUM_RX_MOD_QMIX_I_OUT: begin
          if (!led_ctr)
             rb_leds_data <= fct_mag(rx_mod_qmix_i_out[30:15]);
          end
       RB_SRC_CON_PNT_NUM_RX_MOD_QMIX_Q_OUT: begin
          if (!led_ctr)
             rb_leds_data <= fct_mag(rx_mod_qmix_q_out[30:15]);
          end
       RB_SRC_CON_PNT_NUM_RX_MOD_FIR_I_OUT: begin
          if (!led_ctr && rx_mod_fir_i_vld)
             rb_leds_data <= fct_mag(rx_mod_fir_i_out[33:18]);
          end
       RB_SRC_CON_PNT_NUM_RX_MOD_FIR_Q_OUT: begin
          if (!led_ctr && rx_mod_fir_q_vld)
             rb_leds_data <= fct_mag(rx_mod_fir_q_out[33:18]);
          end
       RB_SRC_CON_PNT_NUM_RX_MOD_CIC_I_OUT: begin
          if (!led_ctr && rx_mod_cic_i_vld)
             rb_leds_data <= fct_mag(rx_mod_cic_i_out[30:15]);
          end
       RB_SRC_CON_PNT_NUM_RX_MOD_CIC_Q_OUT: begin
          if (!led_ctr && rx_mod_cic_q_vld)
             rb_leds_data <= fct_mag(rx_mod_cic_q_out[30:15]);
          end

       RB_SRC_CON_PNT_NUM_RX_MOD_ADD_OUT: begin
          if (!led_ctr && rx_mod_cic_i_vld)
             rb_leds_data <= fct_mag(rx_mod_add_out[31:16]);
          end

       RB_SRC_CON_PNT_NUM_TEST_VECTOR_OUT: begin
          if (!led_ctr)
             rb_leds_data <= { rx_car_fifo_q_ovl, rx_car_fifo_q_ufl, rx_mod_fifo1_i_ovl, rx_mod_fifo1_i_ufl, rx_car_fifo_i_vld, rx_car_fir_i_vld, 2'b0 };
          end

       default: begin
          rb_leds_data <=  8'b0;
          end

       endcase
      led_ctr <= led_ctr + 1;
      end
   else begin                                   // RB_SRC_CON_PNT_NUM_DISABLED
      rb_leds_en   <=  1'b0;
      rb_leds_data <=  8'b0;
      led_ctr      <= 20'b0;
      end
   end


//---------------------------------------------------------------------------------
//  RB RFOUT1 signal assignment

wire [ 5: 0] rfout1_con_src_pnt = regs[REG_RW_RB_RFOUTx_LED_SRC_CON_PNT][21:16];

always @(posedge clk_adc_125mhz)
if (!adc_rstn_i || !rb_reset_n) begin
   rb_out_ch[0] <= 16'b0;
   end

else begin
   if (rfout1_con_src_pnt && rb_activated) begin
       case (rfout1_con_src_pnt)

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
          if (tx_mod_cic_i_vld)
             rb_out_ch[0] <= tx_mod_cic_i_out[30:15];
          end
       RB_SRC_CON_PNT_NUM_TX_MOD_CIC_Q_OUT: begin
          if (tx_mod_cic_q_vld)
             rb_out_ch[0] <= tx_mod_cic_q_out[30:15];
          end
       RB_SRC_CON_PNT_NUM_TX_MOD_FIR_I_OUT: begin
          if (tx_mod_fir_i_vld)
             rb_out_ch[0] <= tx_mod_fir_i_out[32:17];
          end
       RB_SRC_CON_PNT_NUM_TX_MOD_FIR_Q_OUT: begin
          if (tx_mod_fir_q_vld)
             rb_out_ch[0] <= tx_mod_fir_q_out[32:17];
          end
       RB_SRC_CON_PNT_NUM_TX_CAR_CIC_41M664_I_OUT: begin
          if (tx_car_cic_41M664_i_vld)
             rb_out_ch[0] <= tx_car_cic_41M664_i_out[30:15];
          end
       RB_SRC_CON_PNT_NUM_TX_CAR_CIC_41M664_Q_OUT: begin
          if (tx_car_cic_41M664_q_vld)
             rb_out_ch[0] <= tx_car_cic_41M664_q_out[30:15];
          end

       RB_SRC_CON_PNT_NUM_TX_CAR_OSC_I_OUT: begin
          rb_out_ch[0] <= tx_car_osc_cos[15:0];
          end
       RB_SRC_CON_PNT_NUM_TX_CAR_OSC_Q_OUT: begin
          rb_out_ch[0] <= tx_car_osc_sin[15:0];
          end
       RB_SRC_CON_PNT_NUM_TX_CAR_QMIX_I_OUT: begin
          if (tx_car_cic_41M664_i_vld)
             rb_out_ch[0] <= tx_car_qmix_i_out[30:15];
          end
       RB_SRC_CON_PNT_NUM_TX_CAR_QMIX_Q_OUT: begin
          if (tx_car_cic_41M664_q_vld)
             rb_out_ch[0] <= tx_car_qmix_q_out[30:15];
          end

       RB_SRC_CON_PNT_NUM_TX_RF_AMP_OUT: begin
          if (tx_car_cic_41M664_i_vld)
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
       RB_SRC_CON_PNT_NUM_RX_CAR_CIC1_I_OUT: begin
          if (rx_car_cic1_i_vld)
             rb_out_ch[0] <= rx_car_cic1_i_out[30:15];
          end
       RB_SRC_CON_PNT_NUM_RX_CAR_CIC1_Q_OUT: begin
          if (rx_car_cic1_q_vld)
             rb_out_ch[0] <= rx_car_cic1_q_out[30:15];
          end
       RB_SRC_CON_PNT_NUM_RX_CAR_CIC2_I_OUT: begin
          if (rx_car_cic2_i_vld)
             rb_out_ch[0] <= rx_car_cic2_i_out[30:15];
          end
       RB_SRC_CON_PNT_NUM_RX_CAR_CIC2_Q_OUT: begin
          if (rx_car_cic2_q_vld)
             rb_out_ch[0] <= rx_car_cic2_q_out[30:15];
          end
       RB_SRC_CON_PNT_NUM_RX_CAR_FIR_I_OUT: begin
          if (rx_car_fir_i_vld)
             rb_out_ch[0] <= rx_car_fir_i_out[33:18];
          end
       RB_SRC_CON_PNT_NUM_RX_CAR_FIR_Q_OUT: begin
          if (rx_car_fir_q_vld)
             rb_out_ch[0] <= rx_car_fir_q_out[33:18];
          end

       RB_SRC_CON_PNT_NUM_RX_MOD_OSC_I_OUT: begin
          rb_out_ch[0] <= rx_mod_osc_cos[15:0];
          end
       RB_SRC_CON_PNT_NUM_RX_MOD_OSC_Q_OUT: begin
          rb_out_ch[0] <= rx_mod_osc_sin[15:0];
          end
       RB_SRC_CON_PNT_NUM_RX_MOD_HLD_I_OUT: begin
          rb_out_ch[0] <= rx_mod_osc_hld_i[15:0];
          end
       RB_SRC_CON_PNT_NUM_RX_MOD_HLD_Q_OUT: begin
          rb_out_ch[0] <= rx_mod_osc_hld_q[15:0];
          end
       RB_SRC_CON_PNT_NUM_RX_MOD_QMIX_I_OUT: begin
          rb_out_ch[0] <= rx_mod_qmix_i_out[30:15];
          end
       RB_SRC_CON_PNT_NUM_RX_MOD_QMIX_Q_OUT: begin
          rb_out_ch[0] <= rx_mod_qmix_q_out[30:15];
          end
       RB_SRC_CON_PNT_NUM_RX_MOD_FIR_I_OUT: begin
          if (rx_mod_fir_i_vld)
             rb_out_ch[0] <= rx_mod_fir_i_out[33:18];
          end
       RB_SRC_CON_PNT_NUM_RX_MOD_FIR_Q_OUT: begin
          if (rx_mod_fir_q_vld)
             rb_out_ch[0] <= rx_mod_fir_q_out[33:18];
          end
       RB_SRC_CON_PNT_NUM_RX_MOD_CIC_I_OUT: begin
          if (rx_mod_cic_i_vld)
             rb_out_ch[0] <= rx_mod_cic_i_out[30:15];
          end
       RB_SRC_CON_PNT_NUM_RX_MOD_CIC_Q_OUT: begin
          if (rx_mod_cic_q_vld)
             rb_out_ch[0] <= rx_mod_cic_q_out[30:15];
          end

       RB_SRC_CON_PNT_NUM_RX_MOD_ADD_OUT: begin
          if (rx_mod_cic_i_vld)
             rb_out_ch[0] <= rx_mod_add_out[31:16];
          end

       RB_SRC_CON_PNT_NUM_TEST_VECTOR_OUT: begin
          rb_out_ch[0] <= { 1'b0, rx_car_fifo_i_vld, 14'b0 };
          end

       default: begin
          rb_out_ch[0] <= 16'b0;                // silence
          end

       endcase
      end
   else begin                                   // RB_SRC_CON_PNT_NUM_DISABLED
      rb_out_ch[0]     <= 16'b0;                // silence
      end
   end


//---------------------------------------------------------------------------------
//  RB RFOUT2 signal assignment

wire [ 5: 0] rfout2_con_src_pnt = regs[REG_RW_RB_RFOUTx_LED_SRC_CON_PNT][29:24];

always @(posedge clk_adc_125mhz)
if (!adc_rstn_i || !rb_reset_n) begin
   rb_out_ch[1] <= 16'b0;
   end

else begin
   if (rfout2_con_src_pnt && rb_activated) begin
       case (rfout2_con_src_pnt)

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
          if (tx_mod_cic_i_vld)
             rb_out_ch[1] <= tx_mod_cic_i_out[30:15];
          end
       RB_SRC_CON_PNT_NUM_TX_MOD_CIC_Q_OUT: begin
          if (tx_mod_cic_q_vld)
             rb_out_ch[1] <= tx_mod_cic_q_out[30:15];
          end
       RB_SRC_CON_PNT_NUM_TX_MOD_FIR_I_OUT: begin
          if (tx_mod_fir_i_vld)
             rb_out_ch[1] <= tx_mod_fir_i_out[32:17];
          end
       RB_SRC_CON_PNT_NUM_TX_MOD_FIR_Q_OUT: begin
          if (tx_mod_fir_q_vld)
             rb_out_ch[1] <= tx_mod_fir_q_out[32:17];
          end
       RB_SRC_CON_PNT_NUM_TX_CAR_CIC_41M664_I_OUT: begin
          if (tx_car_cic_41M664_i_vld)
             rb_out_ch[1] <= tx_car_cic_41M664_i_out[30:15];
          end
       RB_SRC_CON_PNT_NUM_TX_CAR_CIC_41M664_Q_OUT: begin
          if (tx_car_cic_41M664_q_vld)
             rb_out_ch[1] <= tx_car_cic_41M664_q_out[30:15];
          end

       RB_SRC_CON_PNT_NUM_TX_CAR_OSC_I_OUT: begin
          rb_out_ch[1] <= tx_car_osc_cos[15:0];
          end
       RB_SRC_CON_PNT_NUM_TX_CAR_OSC_Q_OUT: begin
          rb_out_ch[1] <= tx_car_osc_sin[15:0];
          end
       RB_SRC_CON_PNT_NUM_TX_CAR_QMIX_I_OUT: begin
          if (tx_car_cic_41M664_i_vld)
             rb_out_ch[1] <= tx_car_qmix_i_out[30:15];
          end
       RB_SRC_CON_PNT_NUM_TX_CAR_QMIX_Q_OUT: begin
          if (tx_car_cic_41M664_q_vld)
             rb_out_ch[1] <= tx_car_qmix_q_out[30:15];
          end

       RB_SRC_CON_PNT_NUM_TX_RF_AMP_OUT: begin
          if (tx_car_cic_41M664_i_vld)
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
       RB_SRC_CON_PNT_NUM_RX_CAR_CIC1_I_OUT: begin
          if (rx_car_cic1_i_vld)
             rb_out_ch[1] <= rx_car_cic1_i_out[30:15];
          end
       RB_SRC_CON_PNT_NUM_RX_CAR_CIC1_Q_OUT: begin
          if (rx_car_cic1_q_vld)
             rb_out_ch[1] <= rx_car_cic1_q_out[30:15];
          end
       RB_SRC_CON_PNT_NUM_RX_CAR_CIC2_I_OUT: begin
          if (rx_car_cic2_i_vld)
             rb_out_ch[1] <= rx_car_cic2_i_out[30:15];
          end
       RB_SRC_CON_PNT_NUM_RX_CAR_CIC2_Q_OUT: begin
          if (rx_car_cic2_q_vld)
             rb_out_ch[1] <= rx_car_cic2_q_out[30:15];
          end
       RB_SRC_CON_PNT_NUM_RX_CAR_FIR_I_OUT: begin
           if (rx_car_fir_i_vld)
             rb_out_ch[1] <= rx_car_fir_i_out[33:18];
           end
       RB_SRC_CON_PNT_NUM_RX_CAR_FIR_Q_OUT: begin
          if (rx_car_fir_q_vld)
             rb_out_ch[1] <= rx_car_fir_q_out[33:18];
          end

       RB_SRC_CON_PNT_NUM_RX_MOD_OSC_I_OUT: begin
          rb_out_ch[1] <= rx_mod_osc_cos[15:0];
          end
       RB_SRC_CON_PNT_NUM_RX_MOD_OSC_Q_OUT: begin
          rb_out_ch[1] <= rx_mod_osc_sin[15:0];
          end
       RB_SRC_CON_PNT_NUM_RX_MOD_HLD_I_OUT: begin
          rb_out_ch[1] <= rx_mod_osc_hld_i[15:0];
          end
       RB_SRC_CON_PNT_NUM_RX_MOD_HLD_Q_OUT: begin
          rb_out_ch[1] <= rx_mod_osc_hld_q[15:0];
          end
       RB_SRC_CON_PNT_NUM_RX_MOD_QMIX_I_OUT: begin
          rb_out_ch[1] <= rx_mod_qmix_i_out[30:15];
          end
       RB_SRC_CON_PNT_NUM_RX_MOD_QMIX_Q_OUT: begin
          rb_out_ch[1] <= rx_mod_qmix_q_out[30:15];
          end
       RB_SRC_CON_PNT_NUM_RX_MOD_FIR_I_OUT: begin
          if (rx_mod_fir_i_vld)
             rb_out_ch[1] <= rx_mod_fir_i_out[33:18];
          end
       RB_SRC_CON_PNT_NUM_RX_MOD_FIR_Q_OUT: begin
          if (rx_mod_fir_q_vld)
             rb_out_ch[1] <= rx_mod_fir_q_out[33:18];
          end
       RB_SRC_CON_PNT_NUM_RX_MOD_CIC_I_OUT: begin
          if (rx_mod_cic_i_vld)
             rb_out_ch[1] <= rx_mod_cic_i_out[30:15];
          end
       RB_SRC_CON_PNT_NUM_RX_MOD_CIC_Q_OUT: begin
          if (rx_mod_cic_q_vld)
             rb_out_ch[1] <= rx_mod_cic_q_out[30:15];
          end

       RB_SRC_CON_PNT_NUM_RX_MOD_ADD_OUT: begin
          if (rx_mod_cic_i_vld)
             rb_out_ch[1] <= rx_mod_add_out[31:16];
          end

       RB_SRC_CON_PNT_NUM_TEST_VECTOR_OUT: begin
          rb_out_ch[1] <= { 1'b0, rx_car_fir_i_vld, 14'b0 };
          end

       default: begin
          rb_out_ch[1] <= 16'b0;                // silence
          end

       endcase
      end
   else begin                                   // RB_SRC_CON_PNT_NUM_DISABLED
      rb_out_ch[1]     <= 16'b0;                // silence
      end
   end


// === Bus handling ===

//---------------------------------------------------------------------------------
//  Status register

always @(posedge clk_adc_125mhz)
if (!adc_rstn_i) begin
  regs[REG_RD_RB_STATUS] <= 32'b0;
  end

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

  regs[REG_RD_RB_STATUS][RB_STAT_LED7_ON : RB_STAT_LED0_ON] <= rb_leds_data;
  end


//---------------------------------------------------------------------------------
//  System bus connection

// write access to the registers
always @(posedge clk_adc_125mhz)
if (!adc_rstn_i) begin
   regs[REG_RW_RB_CTRL]                   <= 32'h00000000;
   regs[REG_RW_RB_ICR]                    <= 32'h00000000;
   regs[REG_RD_RB_ISR]                    <= 32'h00000000;
   regs[REG_RW_RB_DMA_CTRL]               <= 32'h00000000;
   regs[REG_RW_RB_RFOUTx_LED_SRC_CON_PNT] <= 32'h00000000;
   regs[REG_RW_RB_TX_CAR_OSC_INC_LO]      <= 32'h00000000;
   regs[REG_RW_RB_TX_CAR_OSC_INC_HI]      <= 32'h00000000;
   regs[REG_RW_RB_TX_CAR_OSC_OFS_LO]      <= 32'h00000000;
   regs[REG_RW_RB_TX_CAR_OSC_OFS_HI]      <= 32'h00000000;
   regs[REG_RW_RB_TX_RF_AMP_GAIN]         <= 32'h00000000;
   regs[REG_RW_RB_TX_RF_AMP_OFS]          <= 32'h00000000;
   regs[REG_RW_RB_TX_MOD_OSC_INC_LO]      <= 32'h00000000;
   regs[REG_RW_RB_TX_MOD_OSC_INC_HI]      <= 32'h00000000;
   regs[REG_RW_RB_TX_MOD_OSC_OFS_LO]      <= 32'h00000000;
   regs[REG_RW_RB_TX_MOD_OSC_OFS_HI]      <= 32'h00000000;
   regs[REG_RW_RB_TX_MOD_QMIX_GAIN]       <= 32'h00000000;
   regs[REG_RW_RB_TX_MOD_QMIX_OFS_LO]     <= 32'h00000000;
   regs[REG_RW_RB_TX_MOD_QMIX_OFS_HI]     <= 32'h00000000;
   regs[REG_RW_RB_TX_MUXIN_SRC]           <= 32'h00000000;
   regs[REG_RW_RB_TX_MUXIN_GAIN]          <= 32'h00000000;
   regs[REG_RW_RB_RX_CAR_OSC_INC_LO]      <= 32'h00000000;
   regs[REG_RW_RB_RX_CAR_OSC_INC_HI]      <= 32'h00000000;
   regs[REG_RW_RB_RX_CAR_OSC_OFS_LO]      <= 32'h00000000;
   regs[REG_RW_RB_RX_CAR_OSC_OFS_HI]      <= 32'h00000000;
   regs[REG_RW_RB_RX_MOD_ADD_GAIN]        <= 32'h00000000;
   regs[REG_RW_RB_RX_MOD_ADD_OFS]         <= 32'h00000000;
   regs[REG_RW_RB_RX_MOD_OSC_INC_LO]      <= 32'h00000000;
   regs[REG_RW_RB_RX_MOD_OSC_INC_HI]      <= 32'h00000000;
   regs[REG_RW_RB_RX_MOD_OSC_OFS_LO]      <= 32'h00000000;
   regs[REG_RW_RB_RX_MOD_OSC_OFS_HI]      <= 32'h00000000;
   regs[REG_RW_RB_RX_MUXIN_SRC]           <= 32'h00000000;
   regs[REG_RW_RB_RX_MUXIN_GAIN]          <= 32'h00000000;
   end

else begin
   if (sys_wen) begin
      casez (sys_addr[19:0])

      /* control */
      20'h00000: begin
         regs[REG_RW_RB_CTRL]                  <= sys_wdata[31:0];
         end
      20'h00008: begin
         regs[REG_RW_RB_ICR]                   <= sys_wdata[31:0];
         end
      20'h00010: begin
         regs[REG_RW_RB_DMA_CTRL]              <= sys_wdata[31:0];
         end
      20'h0001C: begin
         regs[REG_RW_RB_RFOUTx_LED_SRC_CON_PNT]<= sys_wdata[31:0] & 32'h3F3F003F;
         end

      /* TX_CAR_OSC */
      20'h00020: begin
         regs[REG_RW_RB_TX_CAR_OSC_INC_LO]     <= sys_wdata[31:0];
         end
      20'h00024: begin
         regs[REG_RW_RB_TX_CAR_OSC_INC_HI]     <= {16'b0, sys_wdata[15:0]};
         end
      20'h00028: begin
         regs[REG_RW_RB_TX_CAR_OSC_OFS_LO]     <= sys_wdata[31:0];
         end
      20'h0002C: begin
         regs[REG_RW_RB_TX_CAR_OSC_OFS_HI]     <= {16'b0, sys_wdata[15:0]};
         end
      20'h00030: begin
         regs[REG_RW_RB_TX_RF_AMP_GAIN]        <= sys_wdata[15:0];
         end
      20'h00038: begin
         regs[REG_RW_RB_TX_RF_AMP_OFS]         <= sys_wdata[15:0];
         end

      /* TX_MOD_OSC */
      20'h00040: begin
         regs[REG_RW_RB_TX_MOD_OSC_INC_LO]     <= sys_wdata[31:0];
         end
      20'h00044: begin
         regs[REG_RW_RB_TX_MOD_OSC_INC_HI]     <= {16'b0, sys_wdata[15:0]};
         end
      20'h00048: begin
         regs[REG_RW_RB_TX_MOD_OSC_OFS_LO]     <= sys_wdata[31:0];
         end
      20'h0004C: begin
         regs[REG_RW_RB_TX_MOD_OSC_OFS_HI]     <= {16'b0, sys_wdata[15:0]};
         end
      20'h00050: begin
         regs[REG_RW_RB_TX_MOD_QMIX_GAIN]      <= sys_wdata[31:0];
         end
      20'h00058: begin
         regs[REG_RW_RB_TX_MOD_QMIX_OFS_LO]    <= sys_wdata[31:0];
         end
      20'h0005C: begin
         regs[REG_RW_RB_TX_MOD_QMIX_OFS_HI]    <= {16'b0, sys_wdata[15:0]};
         end

      /* Input TX_MUXIN */
      20'h00060: begin
         regs[REG_RW_RB_TX_MUXIN_SRC]          <= {regs[REG_RW_RB_TX_MUXIN_SRC][31:6], sys_wdata[5:0]};
         end
      20'h00064: begin
         regs[REG_RW_RB_TX_MUXIN_GAIN]         <= sys_wdata[31:0];
         end

      /* RX_CAR_OSC */
      20'h00120: begin
         regs[REG_RW_RB_RX_CAR_OSC_INC_LO]     <= sys_wdata[31:0];
         end
      20'h00124: begin
         regs[REG_RW_RB_RX_CAR_OSC_INC_HI]     <= {16'b0, sys_wdata[15:0]};
         end
      20'h00128: begin
         regs[REG_RW_RB_RX_CAR_OSC_OFS_LO]     <= sys_wdata[31:0];
         end
      20'h0012C: begin
         regs[REG_RW_RB_RX_CAR_OSC_OFS_HI]     <= {16'b0, sys_wdata[15:0]};
         end

      /* RX_MOD_ADD */
      20'h00130: begin
         regs[REG_RW_RB_RX_MOD_ADD_GAIN]       <= sys_wdata[15:0];
         end
      20'h00138: begin
         regs[REG_RW_RB_RX_MOD_ADD_OFS]        <= sys_wdata[15:0];
         end

      /* RX_MOD_OSC */
      20'h00140: begin
         regs[REG_RW_RB_RX_MOD_OSC_INC_LO]     <= sys_wdata[31:0];
         end
      20'h00144: begin
         regs[REG_RW_RB_RX_MOD_OSC_INC_HI]     <= {16'b0, sys_wdata[15:0]};
         end
      20'h00148: begin
         regs[REG_RW_RB_RX_MOD_OSC_OFS_LO]     <= sys_wdata[31:0];
         end
      20'h0014C: begin
         regs[REG_RW_RB_RX_MOD_OSC_OFS_HI]     <= {16'b0, sys_wdata[15:0]};
         end

      /* RX_MUX */
      20'h00160: begin
         regs[REG_RW_RB_RX_MUXIN_SRC]          <= sys_wdata[31:0];
         end
      20'h00164: begin
         regs[REG_RW_RB_RX_MUXIN_GAIN]         <= {16'b0, sys_wdata[15:0]};
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
      casez (sys_addr[19:0])

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
         sys_rdata <= {26'b0, regs[REG_RW_RB_TX_MUXIN_SRC][5:0]};
         end
      20'h00064: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_TX_MUXIN_GAIN];
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

      /* RX_MOD_ADD */
      20'h00130: begin
        sys_ack   <= sys_en;
        sys_rdata <= regs[REG_RW_RB_RX_MOD_ADD_GAIN];
        end
      20'h00138: begin
        sys_ack   <= sys_en;
        sys_rdata <= regs[REG_RW_RB_RX_MOD_ADD_OFS];
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
