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
    REG_RW_RB_CTRL                        =  0, // h00: RB control register
    REG_RD_RB_STATUS,                           // h04: EB status register
    REG_RW_RB_ICR,                              // h08: RB interrupt control register
    REG_RD_RB_ISR,                              // h0C: RB interrupt status register
    REG_RW_RB_DMA_CTRL,                         // h10: RB DMA control register
    //REG_RD_RB_RSVD_H14,
    //REG_RD_RB_RSVD_H18,
    REG_RW_RB_RFOUTx_LED_SRC_CON_PNT,           // h1C: RB_LED, RB_RFOUT1 and RB_RFOUT2 connection matrix

    REG_RW_RB_CAR_OSC_INC_LO,                   // h20: RB CAR_OSC increment register              LSB:        (Bit 31: 0)
    REG_RW_RB_CAR_OSC_INC_HI,                   // h24: RB CAR_OSC increment register              MSB: 16'b0, (Bit 47:32)
    REG_RW_RB_CAR_OSC_OFS_LO,                   // h28: RB CAR_OSC offset register                 LSB:        (Bit 31: 0)
    REG_RW_RB_CAR_OSC_OFS_HI,                   // h2C: RB CAR_OSC offset register                 MSB: 16'b0, (Bit 47:32)

    REG_RW_RB_AMP_RF_GAIN,                      // h30: RB CAR_OSC mixer gain:     SIGNED 16 bit
    //REG_RD_RB_RSVD_H34,
    REG_RW_RB_AMP_RF_OFS,                       // h38: RB CAR_OSC mixer offset:   SIGNED 17 bit
    //REG_RD_RB_RSVD_H3C,

    REG_RW_RB_MOD_OSC_INC_LO,                   // h40: RB MOD_OSC increment register              LSB:        (Bit 31: 0)
    REG_RW_RB_MOD_OSC_INC_HI,                   // h44: RB MOD_OSC increment register              MSB: 16'b0, (Bit 47:32)
    REG_RW_RB_MOD_OSC_OFS_LO,                   // h48: RB MOD_OSC offset register                 LSB:        (Bit 31: 0)
    REG_RW_RB_MOD_OSC_OFS_HI,                   // h4C: RB MOD_OSC offset register                 MSB: 16'b0, (Bit 47:32)

    REG_RW_RB_MOD_QMIX_GAIN,                    // h50: RB MOD_OSC mixer gain:     SIGNED 16 bit
    //REG_RD_RB_RSVD_H54,
    REG_RW_RB_MOD_QMIX_OFS_LO,                  // h58: RB MOD_OSC mixer offset:   SIGNED 48 bit   LSB:        (Bit 31: 0)
    REG_RW_RB_MOD_QMIX_OFS_HI,                  // h5C: RB MOD_OSC mixer offset:   SIGNED 48 bit   MSB: 16'b0, (Bit 47:32)

    REG_RW_RB_MUXIN_SRC,                        // h60: RB analog MUX input selector:  d3=VpVn,
                                                //      d16=EXT-CH0,  d24=EXT-CH8,
                                                //      d17=EXT-CH1,  d25=EXT-CH9,
                                                //      d32=adc_i[0], d33=adc_i[1]
    REG_RW_RB_MUXIN_GAIN,                       // h64: RB analog MUX gain for input amplifier

    REG_RB_COUNT
} REG_RB_ENUMS;

reg  [31: 0]    regs    [REG_RB_COUNT];         // registers to be accessed by the system bus

enum {
    RB_CTRL_ENABLE                        =  0, // enabling the RadioBox sub-module
    RB_CTRL_RESET_CAR_OSC,                      // reset CAR_OSC, does not touch clock enable
    RB_CTRL_RESET_MOD_OSC,                      // reset MOD_OSC, does not touch clock enable
    RB_CTRL_RSVD_D03,

    RB_CTRL_CAR_OSC_RESYNC,                     // CAR_OSC restart with phase register = 0
    RB_CTRL_CAR_OSC_INC_SRC_STREAM,             // CAR_OSC OSC incrementing: use stream instead of register setting
    RB_CTRL_CAR_OSC_OFS_SRC_STREAM,             // CAR_OSC OSC offset: use stream instead of register setting
    RB_CTRL_RSVD_D07,

    RB_CTRL_RSVD_D08,
    RB_CTRL_RSVD_D09,
    RB_CTRL_RSVD_D10,
    RB_CTRL_RSVD_D11,

    RB_CTRL_MOD_OSC_RESYNC,                     // MOD_OSC restart with phase register = 0
    RB_CTRL_MOD_OSC_INC_SRC_STREAM,             // MOD_OSC OSC incrementing: use stream instead of register setting
    RB_CTRL_MOD_OSC_OFS_SRC_STREAM,             // MOD_OSC OSC offset: use stream instead of register setting
    RB_CTRL_RSVD_D15,

    RB_CTRL_RSVD_D16,
    RB_CTRL_RSVD_D17,
    RB_CTRL_RSVD_D18,
    RB_CTRL_RSVD_D19,

    RB_CTRL_AMP_RF_Q_EN,                        // AMP_RF enable the CAR_QMIX Q path for the SSB modulation
    RB_CTRL_RSVD_D21,
    RB_CTRL_RSVD_D22,
    RB_CTRL_RSVD_D23,

    RB_CTRL_RSVD_D24,
    RB_CTRL_RSVD_D25,
    RB_CTRL_RSVD_D26,
    RB_CTRL_RSVD_D27,

    RB_CTRL_RSVD_D28,
    RB_CTRL_RSVD_D29,
    RB_CTRL_RSVD_D30,
    RB_CTRL_RSVD_D31
} RB_CTRL_BITS_ENUM;

enum {
    RB_STAT_CLK_EN                        =  0, // RB clock enable
    RB_STAT_RESET,                              // RB reset
    RB_STAT_LEDS_EN,                            // RB LEDs enabled
    RB_STAT_RSVD_D03,

    RB_STAT_CAR_OSC_ZERO,                       // CAR_OSC output is zero
    RB_STAT_CAR_OSC_VALID,                      // CAR_OSC output valid
    RB_STAT_RSVD_D06,
    RB_STAT_RSVD_D07,

    RB_STAT_MOD_OSC_ZERO,                       // MOD_OSC output is zero
    RB_STAT_MOD_OSC_VALID,                      // MOD_OSC output valid
    RB_STAT_RSVD_D10,
    RB_STAT_RSVD_D11,

    RB_STAT_RSVD_D12,
    RB_STAT_RSVD_D13,
    RB_STAT_RSVD_D14,
    RB_STAT_RSVD_D15,

    RB_STAT_RSVD_D16,
    RB_STAT_RSVD_D17,
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
    RB_SRC_CON_PNT_NUM_DISABLED           =  0, // LEDs not driven by RB,   RFOUTx silence
    RB_SRC_CON_PNT_NUM_OFF,                     // all LEDs driven but off, RFOUTx silence

    RB_SRC_CON_PNT_NUM_MUXIN_MIX_IN       =  4, // ADC selector input
    RB_SRC_CON_PNT_NUM_MOD_ADC_IN,              // modulation amplifier input
    RB_SRC_CON_PNT_NUM_MOD_ADC_OUT,             // modulation amplifier output

    RB_SRC_CON_PNT_NUM_MOD_QMIX_I_S1_OUT  =  8, // MOD_QMIX I output at stage 1
    RB_SRC_CON_PNT_NUM_MOD_QMIX_Q_S1_OUT,       // MOD_QMIX Q output at stage 1
    RB_SRC_CON_PNT_NUM_MOD_QMIX_I_S2_OUT,       // MOD_QMIX I output at stage 2
    RB_SRC_CON_PNT_NUM_MOD_QMIX_Q_S2_OUT,       // MOD_QMIX Q output at stage 2
    RB_SRC_CON_PNT_NUM_MOD_QMIX_I_S3_OUT,       // MOD_QMIX I output at stage 3
    RB_SRC_CON_PNT_NUM_MOD_QMIX_Q_S3_OUT,       // MOD_QMIX Q output at stage 3

    RB_SRC_CON_PNT_NUM_MOD_CIC_I_OUT      = 16, // MOD_CIC I output
    RB_SRC_CON_PNT_NUM_MOD_CIC_Q_OUT,           // MOD_CIC Q output
    RB_SRC_CON_PNT_NUM_MOD_FIR_I_OUT,           // MOD_FIR I output
    RB_SRC_CON_PNT_NUM_MOD_FIR_Q_OUT,           // MOD_FIR Q output
    RB_SRC_CON_PNT_NUM_CAR_CIC_41M664_I_OUT,    // CAR_CIC I stage 1 - 41.664 MHz output
    RB_SRC_CON_PNT_NUM_CAR_CIC_41M664_Q_OUT,    // CAR_CIC Q stage 1 - 41.664 MHz output

    RB_SRC_CON_PNT_NUM_CAR_QMIX_I_OUT     = 24, // CAR_QMIX I output
    RB_SRC_CON_PNT_NUM_CAR_QMIX_Q_OUT,          // CAR_QMIX Q output

    RB_SRC_CON_PNT_NUM_AMP_RF_OUT         = 28, // AMP_RF output

    RB_SRC_CON_PNT_NUM_TEST_VECTOR_OUT    = 63  // Current test vector, look at assignments within this file
} RB_SRC_CON_PNT_ENUM;                          // 64 entries = 2^6 --> 6 bit field

enum {
    RB_XADC_MAPPING_EXT_CH0               =  0, // CH0 and CH8 are sampled simultaneously, mapped to: vinp_i[0]/vinn_i[0]
    RB_XADC_MAPPING_EXT_CH8,                    // CH0 and CH8 are sampled simultaneously, mapped to: vinp_i[1]/vinn_i[1]
    RB_XADC_MAPPING_EXT_CH1,                    // CH1 and CH9 are sampled simultaneously, mapped to: vinp_i[2]/vinn_i[2]
    RB_XADC_MAPPING_EXT_CH9,                    // CH1 and CH9 are sampled simultaneously, mapped to: vinp_i[3]/vinn_i[3]
    RB_XADC_MAPPING_VpVn,                       // The dedicated Vp/Vn input mapped to: vinp_i[4]/vinn_i[4]
    RB_XADC_MAPPING__COUNT
} RB_XADC_MAPPING_ENUM;


wire rb_enable = regs[REG_RW_RB_CTRL][RB_CTRL_ENABLE];

reg          rb_enable_last     = 1'b0;
reg          rb_clk_en          = 1'b0;
reg          rb_reset_n         = 1'b0;
reg  [ 1: 0] rb_enable_ctr      = 2'b0;

wire         rb_reset_car_osc_n = rb_reset_n & !regs[REG_RW_RB_CTRL][RB_CTRL_RESET_CAR_OSC];
wire         rb_reset_mod_osc_n = rb_reset_n & !regs[REG_RW_RB_CTRL][RB_CTRL_RESET_MOD_OSC];


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


// Transmitter

//---------------------------------------------------------------------------------
//  ADC modulation offset correction and gain

wire [15:0] muxin_mix_in = (regs[REG_RW_RB_MUXIN_SRC][5:0] == 6'h20) ?  {~adc_i[0], 2'b0} :
                           (regs[REG_RW_RB_MUXIN_SRC][5:0] == 6'h21) ?  {~adc_i[1], 2'b0} :
                           (regs[REG_RW_RB_MUXIN_SRC][5:0] == 6'h18) ?  rb_xadc[RB_XADC_MAPPING_EXT_CH0] :  // swapped here due to pin connection warnings when swapping @ XADC <--> pins
                           (regs[REG_RW_RB_MUXIN_SRC][5:0] == 6'h10) ?  rb_xadc[RB_XADC_MAPPING_EXT_CH8] :
                           (regs[REG_RW_RB_MUXIN_SRC][5:0] == 6'h11) ?  rb_xadc[RB_XADC_MAPPING_EXT_CH1] :
                           (regs[REG_RW_RB_MUXIN_SRC][5:0] == 6'h19) ?  rb_xadc[RB_XADC_MAPPING_EXT_CH9] :
                           (regs[REG_RW_RB_MUXIN_SRC][5:0] == 6'h03) ?  rb_xadc[RB_XADC_MAPPING_VpVn]    :
                           16'b0;
wire [ 2:0] muxin_mix_log2 = regs[REG_RW_RB_MUXIN_GAIN][18:16];
wire [15:0] muxin_mix_gain = regs[REG_RW_RB_MUXIN_GAIN][15: 0];

wire [15:0] mod_adc_in = (muxin_mix_in << muxin_mix_log2);  // unsigned value: input booster for
                                                            // factor: 1x .. 2^3=7 shift postions=128x (16 mV --> full-scale)
wire [15:0] mod_adc_ofs = 16'b0;                            // TODO: FSM for calculating mean value to strip of the DC component
wire [31:0] mod_adc_out;

rb_dsp48_AaDmB_A16_D16_B16_P32 i_rb_mod_adc_dsp48 (
  // global signals
  .CLK                  ( clk_adc_125mhz    ),  // global 125 MHz clock
  .CE                   ( rb_clk_en         ),  // enable part 1 of RadioBox sub-module

  // ADC/XADC modulation input
  .A                    ( mod_adc_in        ),  // MUX in signal:    SIGNED 16 bit
  // modulation offset input
  .D                    ( mod_adc_ofs       ),  // offset setting:   SIGNED 16 bit
  // modulation gain input
  .B                    ( muxin_mix_gain    ),  // gain setting:     SIGNED 16 bit

  // multiplier output
  .P                    ( mod_adc_out       )   // PreAmp output     SIGSIG 32 bit
);


//---------------------------------------------------------------------------------
//  MOD_OSC modulation generator and SSB weaver modulator

wire         mod_osc_inc_mux = regs[REG_RW_RB_CTRL][RB_CTRL_MOD_OSC_INC_SRC_STREAM];
wire         mod_osc_ofs_mux = regs[REG_RW_RB_CTRL][RB_CTRL_MOD_OSC_OFS_SRC_STREAM];
wire         mod_osc_resync  = regs[REG_RW_RB_CTRL][RB_CTRL_MOD_OSC_RESYNC];

wire [ 47:0] mod_osc_inc_stream = 48'b0;  // TODO: ADC
wire [ 47:0] mod_osc_ofs_stream = 48'b0;  // TODO: ADC

wire [ 47:0] mod_osc_inc = ( mod_osc_inc_mux ?  mod_osc_inc_stream : {regs[REG_RW_RB_MOD_OSC_INC_HI][15:0], regs[REG_RW_RB_MOD_OSC_INC_LO][31:0]});
wire [ 47:0] mod_osc_ofs = ( mod_osc_ofs_mux ?  mod_osc_ofs_stream : {regs[REG_RW_RB_MOD_OSC_OFS_HI][15:0], regs[REG_RW_RB_MOD_OSC_OFS_LO][31:0]});

wire         mod_osc_axis_s_vld   = rb_reset_n;  // TODO: ADC
wire [103:0] mod_osc_axis_s_phase = {7'b0, mod_osc_resync, mod_osc_ofs, mod_osc_inc};

wire         mod_osc_axis_m_vld;
wire [ 31:0] mod_osc_axis_m_data;
wire [ 15:0] mod_osc_cos = mod_osc_axis_m_data[15: 0];
wire [ 15:0] mod_osc_sin = mod_osc_axis_m_data[31:16];

rb_dds_48_16_125 i_rb_mod_osc (
  // global signals
  .aclk                 ( clk_adc_125mhz       ),  // global 125 MHz clock
  .aclken               ( rb_clk_en            ),  // clock enable of RadioBox sub-module
  .aresetn              ( rb_reset_mod_osc_n   ),  // reset of MOD_OSC

  // AXI-Stream slave in port: streaming data for MOD_OSC modulation
  .s_axis_phase_tvalid  ( mod_osc_axis_s_vld   ),  // AXIS slave data valid
  .s_axis_phase_tdata   ( mod_osc_axis_s_phase ),  // AXIS slave data

  // AXI-Stream master out port: MOD_OSC signal
  .m_axis_data_tvalid   ( mod_osc_axis_m_vld   ),  // AXIS master MOD_OSC data valid
  .m_axis_data_tdata    ( mod_osc_axis_m_data  )   // AXIS master MOD_OSC output: 2x SIGNED 16 bit
);


//---------------------------------------------------------------------------------
//  MOD_QMIX quadrature mixer for the base band

wire        amp_rf_q_en    = regs[REG_RW_RB_CTRL][RB_CTRL_AMP_RF_Q_EN];

wire [15:0] mod_qmix_in    = (regs[REG_RW_RB_MUXIN_SRC][5:0] == 6'h00) ?  16'h7fff : mod_adc_out[30:15];  // when ADC source ID is zero take cos() from MOD_OSC only
wire [15:0] mod_qmix_gain  =  regs[REG_RW_RB_MOD_QMIX_GAIN][15:0];
wire [47:0] mod_qmix_ofs   = {regs[REG_RW_RB_MOD_QMIX_OFS_HI][15:0], regs[REG_RW_RB_MOD_QMIX_OFS_LO][31:0]};

wire [31:0] mod_qmix_i_s1_out;
wire [31:0] mod_qmix_q_s1_out;

wire [31:0] mod_qmix_i_s2_out;
wire [31:0] mod_qmix_q_s2_out;

wire [47:0] mod_qmix_i_s3_in = regs[REG_RW_RB_CTRL][RB_CTRL_CAR_OSC_INC_SRC_STREAM] ?
                               {{15{mod_qmix_i_s2_out[30]}}, mod_qmix_i_s2_out[29:0], 3'b0} :  /* when FM is used, take 2^14 finer resolution */
                               {mod_qmix_i_s2_out[30:0], 17'b0};
wire [47:0] mod_qmix_q_s3_in = {mod_qmix_q_s2_out[30:0], 17'b0};
wire [47:0] mod_qmix_i_s3_out;
wire [47:0] mod_qmix_q_s3_out;

rb_dsp48_AmB_A16_B16_P32 i_rb_mod_qmix_I_s1_dsp48 (
  // global signals
  .CLK                  ( clk_adc_125mhz    ),  // global 125 MHz clock
  .CE                   ( rb_clk_en         ),  // enable part 1 of RadioBox sub-module

  // modulation input
  .A                    ( mod_qmix_in       ),  // MUX in signal:    SIGNED 16 bit
  // MOD_OSC cos input
  .B                    ( mod_osc_cos       ),  // MOD_OSC cos:      SIGNED 16 bit

  // multiplier output stage 1
  .P                    ( mod_qmix_i_s1_out )   // QMIX I output:    SIGSIG 32 bit
);

rb_dsp48_AmB_A16_B16_P32 i_rb_mod_qmix_I_s2_dsp48 (
  // global signals
  .CLK                  ( clk_adc_125mhz    ),  // global 125 MHz clock
  .CE                   ( rb_clk_en         ),  // enable part 1 of RadioBox sub-module

  // QMIX I input
  .A                    ( mod_qmix_i_s1_out[30:15] ),  // MUX in signal:    SIGNED 16 bit
  // gain setting input
  .B                    ( mod_qmix_gain     ),  // gain setting:     SIGNED 16 bit

  // multiplier output stage 2
  .P                    ( mod_qmix_i_s2_out )   // QMIX I regulated: SIGSIG 32 bit
);

rb_dsp48_CONaC_CON48_C48_P48 i_rb_mod_qmix_I_s3_dsp48 (
  // global signals
  .CLK                  ( clk_adc_125mhz    ),  // global 125 MHz clock
  .CE                   ( rb_clk_en         ),  // enable part 1 of RadioBox sub-module

  // modulation input
  .C                    ( mod_qmix_i_s3_in  ),  // modulation:       SIGNED 48 bit
  // offset value for OSC control
  .CONCAT               ( mod_qmix_ofs      ),  // offset:           SIGNED 48 bit

  // adder output
  .P                    ( mod_qmix_i_s3_out )   // QMIX I for OSC:   SIGNED 48 bit
);


rb_dsp48_AmB_A16_B16_P32 i_rb_mod_qmix_Q_s1_dsp48 (
  // global signals
  .CLK                  ( clk_adc_125mhz    ),  // global 125 MHz clock
  .CE                   ( rb_clk_en & amp_rf_q_en ),  // enable part 1 of RadioBox sub-module and the Q path

  // modulation input
  .A                    ( mod_qmix_in       ),  // MUX in signal:    SIGNED 16 bit
  // MOD_OSC sin input
  .B                    ( mod_osc_sin       ),  // MOD_OSC sin:      SIGNED 16 bit

  // multiplier output stage 1
  .P                    ( mod_qmix_q_s1_out )   // QMIX Q output:    SIGSIG 32 bit
);

rb_dsp48_AmB_A16_B16_P32 i_rb_mod_qmix_Q_s2_dsp48 (
  // global signals
  .CLK                  ( clk_adc_125mhz    ),  // global 125 MHz clock
  .CE                   ( rb_clk_en & amp_rf_q_en ),  // enable part 1 of RadioBox sub-module and the Q path

  // QMIX Q input
  .A                    ( mod_qmix_q_s1_out[30:15] ),  // MUX in signal:    SIGNED 16 bit
  // gain setting input
  .B                    ( mod_qmix_gain     ),  // gain setting:     SIGNED 16 bit

  // multiplier output stage 2
  .P                    ( mod_qmix_q_s2_out )   // QMIX Q regulated: SIGSIG 32 bit
);

rb_dsp48_CONaC_CON48_C48_P48 i_rb_mod_qmix_Q_s3_dsp48 (
  // global signals
  .CLK                  ( clk_adc_125mhz    ),  // global 125 MHz clock
  .CE                   ( rb_clk_en & amp_rf_q_en ),  // enable part 1 of RadioBox sub-module and the Q path

  // modulation input
  .C                    ( mod_qmix_q_s3_in  ),  // modulation:       SIGNED 48 bit
  // offset value for OSC control
  .CONCAT               ( mod_qmix_ofs      ),  // offset:           SIGNED 48 bit

  // adder output
  .P                    ( mod_qmix_q_s3_out )   // QMIX Q for OSC:   SIGNED 48 bit
);


//---------------------------------------------------------------------------------
//  MOD_CIC sampling rate down convertion 48 kSPS to 8 kSPS

reg          mod_cic_s_vld_i = 'b0;
wire         mod_cic_s_rdy_i;
wire [ 31:0] mod_cic_i_out;
wire         mod_cic_i_vld;
wire         mod_cic_i_rdy;

reg          mod_cic_s_vld_q = 'b0;
wire         mod_cic_s_rdy_q;
wire [ 31:0] mod_cic_q_out;
wire         mod_cic_q_vld;
wire         mod_cic_q_rdy;

always @(posedge clk_adc_125mhz)                // assign mod_cic_s_vld_i
begin
   if (!rb_clk_en)
      mod_cic_s_vld_i <= 'b0;
   else begin
      if (mod_cic_s_vld_i && mod_cic_s_rdy_i)
         mod_cic_s_vld_i <= 'b0;                // falling back to non-active state

      if (clk_48khz)                            // trigger able to overwrite mod_cic_s_vld_i
         mod_cic_s_vld_i <= 'b1;                // entering active state
      end
end

always @(posedge clk_adc_125mhz)                // assign mod_cic_s_vld_q
begin
   if (!rb_clk_en)
      mod_cic_s_vld_q <= 'b0;
   else begin
      if (mod_cic_s_vld_q && mod_cic_s_rdy_q)
         mod_cic_s_vld_q <= 'b0;                // falling back to non-active state

      if (clk_48khz)                            // trigger able to overwrite mod_cic_s_vld_q
         mod_cic_s_vld_q <= 'b1;                // entering active state
      end
end

rb_cic_48k_to_8k_32T32_lat13 i_rb_mod_cic_I (
  // global signals
  .aclk                 ( clk_adc_125mhz    ),  // global 125 MHz clock
  .aclken               ( rb_clk_en         ),  // enable RadioBox sub-module
  .aresetn              ( rb_reset_n        ),

  .s_axis_data_tdata    ( mod_qmix_i_s2_out ),  // QMIX I stage 2
  .s_axis_data_tvalid   ( mod_cic_s_vld_i   ),
  .s_axis_data_tready   ( mod_cic_s_rdy_i   ),

  .m_axis_data_tdata    ( mod_cic_i_out     ),  // MOD_CIC output I
  .m_axis_data_tvalid   ( mod_cic_i_vld     ),
  .m_axis_data_tready   ( mod_cic_i_rdy     )
);

rb_cic_48k_to_8k_32T32_lat13 i_rb_mod_cic_Q (
  // global signals
  .aclk                 ( clk_adc_125mhz    ),  // global 125 MHz clock
  .aclken               ( rb_clk_en         ),  // enable RadioBox sub-module
  .aresetn              ( rb_reset_n        ),

  .s_axis_data_tdata    ( mod_qmix_q_s2_out ),  // QMIX Q stage 2
  .s_axis_data_tvalid   ( mod_cic_s_vld_q   ),
  .s_axis_data_tready   ( mod_cic_s_rdy_q   ),

  .m_axis_data_tdata    ( mod_cic_q_out     ),  // MOD_CIC output Q
  .m_axis_data_tvalid   ( mod_cic_q_vld     ),
  .m_axis_data_tready   ( mod_cic_q_rdy     )
);


//---------------------------------------------------------------------------------
//  MOD_FIR high pass filter for CIC compensation in the voice band
//
//  FIR coefficients built with Octave:
//  hn = fir2(62, [0 0.38 0.39 1], [1 1 0.000001 0.000001], 512, kaiser(63,4));

wire [ 34:0] mod_fir_i_out;
wire         mod_fir_i_vld;
wire         mod_fir_i_rdy;

wire [ 34:0] mod_fir_q_out;
wire         mod_fir_q_vld;
wire         mod_fir_q_rdy;

rb_fir_8k_8k_25c23_17i16_35o33_lat42 i_rb_mod_fir_I (
  // global signals
  .aclk                 ( clk_adc_125mhz    ),  // global 125 MHz clock
  .aclken               ( rb_clk_en         ),  // enable RadioBox sub-module
  .aresetn              ( rb_reset_n        ),

  .s_axis_data_tdata    ( mod_cic_i_out[30:14] ),  // MOD_CIC output I - 8 kHz (17.16 bit width)
  .s_axis_data_tvalid   ( mod_cic_i_vld     ),
  .s_axis_data_tready   ( mod_cic_i_rdy     ),

  .m_axis_data_tdata    ( mod_fir_i_out     ),  // MOD_FIR output I - 8kHz (35.30 bit width)
  .m_axis_data_tvalid   ( mod_fir_i_vld     ),
  .m_axis_data_tready   ( mod_fir_i_rdy     )
);

rb_fir_8k_8k_25c23_17i16_35o33_lat42 i_rb_mod_fir_Q (
  // global signals
  .aclk                 ( clk_adc_125mhz    ),  // global 125 MHz clock
  .aclken               ( rb_clk_en         ),  // enable RadioBox sub-module
  .aresetn              ( rb_reset_n        ),

  .s_axis_data_tdata    ( mod_cic_q_out[30:14] ),  // MOD_CIC output Q - 8 kHz (17.16 bit width)
  .s_axis_data_tvalid   ( mod_cic_q_vld     ),
  .s_axis_data_tready   ( mod_cic_q_rdy     ),

  .m_axis_data_tdata    ( mod_fir_q_out     ),  // MOD_FIR output Q - 8 kHz (35.30 bit width)
  .m_axis_data_tvalid   ( mod_fir_q_vld     ),
  .m_axis_data_tready   ( mod_fir_q_rdy     )
);


//---------------------------------------------------------------------------------
//  CAR_CIC sampling rate up convertion 8 kSPS to 41.664 MSPS

wire [ 31:0] car_cic_41M664_i_out;
wire         car_cic_41M664_i_vld;

wire [ 31:0] car_cic_41M664_q_out;
wire         car_cic_41M664_q_vld;

rb_cic_8k_to_41M664_32T32_lat14 i_rb_car_cic_I (
  // global signals
  .aclk                 ( clk_adc_125mhz    ),  // global 125 MHz clock
  .aclken               ( rb_clk_en         ),  // enable RadioBox sub-module
  .aresetn              ( rb_reset_n        ),

  .s_axis_data_tdata    ( mod_fir_i_out[32:1]),  // MOD_FIR I - 8 kHz
  .s_axis_data_tvalid   ( mod_fir_i_vld     ),
  .s_axis_data_tready   ( mod_fir_i_rdy     ),

  .m_axis_data_tdata    ( car_cic_41M664_i_out  ),  // CAR_CIC I stage 1 output - 1 MHz
  .m_axis_data_tvalid   ( car_cic_41M664_i_vld  )
);

rb_cic_8k_to_41M664_32T32_lat14 i_rb_car_cic_Q (
  // global signals
  .aclk                 ( clk_adc_125mhz    ),  // global 125 MHz clock
  .aclken               ( rb_clk_en         ),  // enable RadioBox sub-module
  .aresetn              ( rb_reset_n        ),

  .s_axis_data_tdata    ( mod_fir_q_out[32:1]),  // MOD_FIR Q - 8 kHz
  .s_axis_data_tvalid   ( mod_fir_q_vld     ),
  .s_axis_data_tready   ( mod_fir_q_rdy     ),

  .m_axis_data_tdata    ( car_cic_41M664_q_out  ),  // CAR_CIC Q stage 1 output - 1 MHz
  .m_axis_data_tvalid   ( car_cic_41M664_q_vld  )
);


//---------------------------------------------------------------------------------
//  CAR_OSC carrier frequency generator  (CW, FM, PM modulated)

wire         car_osc_inc_mux = regs[REG_RW_RB_CTRL][RB_CTRL_CAR_OSC_INC_SRC_STREAM];
wire         car_osc_ofs_mux = regs[REG_RW_RB_CTRL][RB_CTRL_CAR_OSC_OFS_SRC_STREAM];
wire         car_osc_resync  = regs[REG_RW_RB_CTRL][RB_CTRL_CAR_OSC_RESYNC];

wire [ 47:0] car_osc_inc = ( car_osc_inc_mux ?  mod_qmix_i_s3_out : {regs[REG_RW_RB_CAR_OSC_INC_HI][15:0], regs[REG_RW_RB_CAR_OSC_INC_LO][31:0]});
wire [ 47:0] car_osc_ofs = ( car_osc_ofs_mux ?  mod_qmix_i_s3_out : {regs[REG_RW_RB_CAR_OSC_OFS_HI][15:0], regs[REG_RW_RB_CAR_OSC_OFS_LO][31:0]});

wire         car_osc_axis_s_vld   = rb_reset_n;  // TODO
wire [103:0] car_osc_axis_s_phase = {7'b0, car_osc_resync, car_osc_ofs, car_osc_inc};

wire         car_osc_axis_m_vld;
wire [ 31:0] car_osc_axis_m_data;

wire [ 15:0] car_osc_cos = car_osc_axis_m_data[15: 0];
wire [ 15:0] car_osc_sin = car_osc_axis_m_data[31:16];

rb_dds_48_16_125 i_rb_car_osc (
  // global signals
  .aclk                 ( clk_adc_125mhz       ),  // global 125 MHz clock
  .aclken               ( rb_clk_en            ),  // enable RadioBox sub-module
  .aresetn              ( rb_reset_car_osc_n   ),  // reset of CAR_OSC

  // simple-AXI slave in port: streaming data for CAR_OSC modulation
  .s_axis_phase_tvalid  ( car_osc_axis_s_vld   ),  // AXIS slave data valid
  .s_axis_phase_tdata   ( car_osc_axis_s_phase ),  // AXIS slave data

  // simple-AXI master out port: CAR_OSC signal
  .m_axis_data_tvalid   ( car_osc_axis_m_vld   ),  // AXIS master CAR_OSC data valid
  .m_axis_data_tdata    ( car_osc_axis_m_data  )   // AXIS master CAR_OSC output: Q SIGNED 16 bit, I SIGNED 16 bit
);


//---------------------------------------------------------------------------------
//  CAR_QMIX quadrature mixer for the radio frequency

wire [ 15:0] car_qmix_i_in = (car_osc_inc_mux || car_osc_ofs_mux)                 ?  16'h7fff : amp_rf_q_en ?  car_cic_41M664_i_out[30:15] : mod_qmix_i_s3_out[47:32];  // MOD_QMIX/MOD_CIC uses full scale constant for CW, FM and PM modulations - SSB uses CIC I instead
wire [ 15:0] car_qmix_q_in = (car_osc_inc_mux || car_osc_ofs_mux || !amp_rf_q_en) ?  16'h0000 :                car_cic_41M664_q_out[30:15]                           ;  // MOD_QMIX/MOD_CIC Q path keep quiet when Q is disabled - SSB uses CIC Q instead

wire [ 31:0] car_qmix_i_out;
wire [ 31:0] car_qmix_q_out;

rb_dsp48_AmB_A16_B16_P32 i_rb_car_qmix_I_dsp48 (
  // global signals
  .CLK                  ( clk_adc_125mhz    ),  // global 125 MHz clock
  .CE                   ( rb_clk_en         ),  // enable part 1 of RadioBox sub-module

  // modulation input
  .A                    ( car_qmix_i_in     ),  // MUX in signal:        SIGNED 16 bit
  // CAR_OSC cos input
  .B                    ( car_osc_cos       ),  // CAR_OSC cos:          SIGNED 16 bit

  // multiplier output
  .P                    ( car_qmix_i_out    )   // CAR_QMIX I output:    SIGSIG 32 bit
);

rb_dsp48_AmB_A16_B16_P32 i_rb_car_qmix_Q_dsp48 (
  // global signals
  .CLK                  ( clk_adc_125mhz    ),  // global 125 MHz clock
  .CE                   ( rb_clk_en & amp_rf_q_en ),  // enable part 1 of RadioBox sub-module and the Q path

  // modulation input
  .A                    ( car_qmix_q_in     ),  // MUX in signal:        SIGNED 16 bit
  // CAR_OSC sin input
  .B                    ( car_osc_sin       ),  // CAR_OSC sin:          SIGNED 16 bit

  // multiplier output
  .P                    ( car_qmix_q_out    )   // CAR_QMIX Q output:    SIGSIG 32 bit
);


//---------------------------------------------------------------------------------
//  AMP_CTL amplifier control for setting gain of the radio frequency amplifier


//---------------------------------------------------------------------------------
//  AMP_RF amplifier for the radio frequency output (CW, AM modulated)

wire [16:0] amp_rf_i_var =                {car_qmix_i_out[30], car_qmix_i_out[30:15]}        ;  // halfed and sign corrected 17 bit extension
wire [16:0] amp_rf_q_var = amp_rf_q_en ?  {car_qmix_q_out[30], car_qmix_q_out[30:15]} : 17'b0;  // halfed and sign corrected 17 bit extension
wire [16:0] amp_rf_gain  = {regs[REG_RW_RB_AMP_RF_GAIN][15:0], 1'b0};  // signed register value
wire [34:0] amp_rf_ofs   = {regs[REG_RW_RB_AMP_RF_OFS][15:0], 19'b0};  // signed register value

wire [35:0] amp_rf_out;

rb_dsp48_AaDmBaC_A17_D17_B17_C35_P36 i_rb_amp_rf_dsp48 (
  // global signals
  .CLK                  ( clk_adc_125mhz    ),  // global 125 MHz clock
  .CE                   ( rb_clk_en         ),  // enable part 1 of RadioBox sub-module
  .SCLR                 ( !rb_enable        ),  // put output to neutral when activated

  // QMIX RF I output
  .A                    ( amp_rf_i_var      ),  // QMIX_RF I         SIGNED 17 bit
  // QMIX RF Q output
  .D                    ( amp_rf_q_var      ),  // QMIX_RF Q         SIGNED 17 bit
  // AMP RF gain
  .B                    ( amp_rf_gain       ),  // AMP_RF gain       SIGNED 17 bit
  // AMP RF offset
  .C                    ( amp_rf_ofs        ),  // AMP_RF ofs        SIGSIG 35 bit

  // AMP RF output
  .P                    ( amp_rf_out        )   // AMP RF output     SIGSIG 36 bit
);


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

       RB_SRC_CON_PNT_NUM_MUXIN_MIX_IN: begin
          if (!led_ctr)
             rb_leds_data <= fct_mag(muxin_mix_in[15:0]);  // updating about 120 Hz for reducing EMI
          end
       RB_SRC_CON_PNT_NUM_MOD_ADC_IN: begin
          if (!led_ctr)
             rb_leds_data <= fct_mag(mod_adc_in[15:0]);
          end
       RB_SRC_CON_PNT_NUM_MOD_ADC_OUT: begin
          if (!led_ctr)
             rb_leds_data <= fct_mag(mod_adc_out[30:15]);
          end

       RB_SRC_CON_PNT_NUM_MOD_QMIX_I_S1_OUT: begin
          if (!led_ctr)
             rb_leds_data <= fct_mag(mod_qmix_i_s1_out[30:15]);
          end
       RB_SRC_CON_PNT_NUM_MOD_QMIX_Q_S1_OUT: begin
          if (!led_ctr)
             rb_leds_data <= fct_mag(mod_qmix_q_s1_out[30:15]);
          end
       RB_SRC_CON_PNT_NUM_MOD_QMIX_I_S2_OUT: begin
          if (!led_ctr)
             rb_leds_data <= fct_mag(mod_qmix_i_s2_out[30:15]);
          end
       RB_SRC_CON_PNT_NUM_MOD_QMIX_Q_S2_OUT: begin
          if (!led_ctr)
             rb_leds_data <= fct_mag(mod_qmix_q_s2_out[30:15]);
          end
       RB_SRC_CON_PNT_NUM_MOD_QMIX_I_S3_OUT: begin
          if (!led_ctr)
             rb_leds_data <= fct_mag(mod_qmix_i_s3_out[47:32]);
          end
       RB_SRC_CON_PNT_NUM_MOD_QMIX_Q_S3_OUT: begin
          if (!led_ctr)
             rb_leds_data <= fct_mag(mod_qmix_q_s3_out[47:32]);
          end

       RB_SRC_CON_PNT_NUM_MOD_CIC_I_OUT: begin
          if (!led_ctr)
             rb_leds_data <= fct_mag(mod_cic_i_out[30:15]);
          end
       RB_SRC_CON_PNT_NUM_MOD_CIC_Q_OUT: begin
          if (!led_ctr)
             rb_leds_data <= fct_mag(mod_cic_q_out[30:15]);
          end
       RB_SRC_CON_PNT_NUM_MOD_FIR_I_OUT: begin
          if (!led_ctr)
             rb_leds_data <= fct_mag(mod_fir_i_out[32:17]);
          end
       RB_SRC_CON_PNT_NUM_MOD_FIR_Q_OUT: begin
          if (!led_ctr)
             rb_leds_data <= fct_mag(mod_fir_q_out[32:17]);
          end

       RB_SRC_CON_PNT_NUM_CAR_CIC_41M664_I_OUT: begin
          if (!led_ctr)
             rb_leds_data <= fct_mag(car_cic_41M664_i_out[30:15]);
          end
       RB_SRC_CON_PNT_NUM_CAR_CIC_41M664_Q_OUT: begin
          if (!led_ctr)
             rb_leds_data <= fct_mag(car_cic_41M664_q_out[30:15]);
          end

       RB_SRC_CON_PNT_NUM_CAR_QMIX_I_OUT: begin
          if (!led_ctr)
             rb_leds_data <= fct_mag(car_qmix_i_out[30:15]);
          end
       RB_SRC_CON_PNT_NUM_CAR_QMIX_Q_OUT: begin
          if (!led_ctr)
             rb_leds_data <= fct_mag(car_qmix_q_out[30:15]);
          end

       RB_SRC_CON_PNT_NUM_AMP_RF_OUT: begin
          if (!led_ctr)
             rb_leds_data <= fct_mag(amp_rf_out[31:16]);
          end

       RB_SRC_CON_PNT_NUM_TEST_VECTOR_OUT: begin
          if (!led_ctr)
             rb_leds_data <= { mod_cic_s_vld_i, mod_cic_i_vld, mod_fir_i_vld, car_cic_41M664_i_vld,  mod_cic_s_vld_q, mod_cic_q_vld, mod_fir_q_vld, car_cic_41M664_q_vld };
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

       RB_SRC_CON_PNT_NUM_MUXIN_MIX_IN: begin
          rb_out_ch[0] <= muxin_mix_in[15:0];
          end
       RB_SRC_CON_PNT_NUM_MOD_ADC_IN: begin
          rb_out_ch[0] <= mod_adc_in[15:0];
          end
       RB_SRC_CON_PNT_NUM_MOD_ADC_OUT: begin
          rb_out_ch[0] <= mod_adc_out[30:15];
          end

       RB_SRC_CON_PNT_NUM_MOD_QMIX_I_S1_OUT: begin
          rb_out_ch[0] <= mod_qmix_i_s1_out[30:15];
          end
       RB_SRC_CON_PNT_NUM_MOD_QMIX_Q_S1_OUT: begin
          rb_out_ch[0] <= mod_qmix_q_s1_out[30:15];
          end
       RB_SRC_CON_PNT_NUM_MOD_QMIX_I_S2_OUT: begin
          rb_out_ch[0] <= mod_qmix_i_s2_out[30:15];
          end
       RB_SRC_CON_PNT_NUM_MOD_QMIX_Q_S2_OUT: begin
          rb_out_ch[0] <= mod_qmix_q_s2_out[30:15];
          end
       RB_SRC_CON_PNT_NUM_MOD_QMIX_I_S3_OUT: begin
          rb_out_ch[0] <= mod_qmix_i_s3_out[47:32];
          end
       RB_SRC_CON_PNT_NUM_MOD_QMIX_Q_S3_OUT: begin
          rb_out_ch[0] <= mod_qmix_q_s3_out[47:32];
          end

       RB_SRC_CON_PNT_NUM_MOD_CIC_I_OUT: begin
          rb_out_ch[0] <= mod_cic_i_out[30:15];
          end
       RB_SRC_CON_PNT_NUM_MOD_CIC_Q_OUT: begin
          rb_out_ch[0] <= mod_cic_q_out[30:15];
          end
       RB_SRC_CON_PNT_NUM_MOD_FIR_I_OUT: begin
          rb_out_ch[0] <= mod_fir_i_out[32:17];
          end
       RB_SRC_CON_PNT_NUM_MOD_FIR_Q_OUT: begin
          rb_out_ch[0] <= mod_fir_q_out[32:17];
          end

       RB_SRC_CON_PNT_NUM_CAR_CIC_41M664_I_OUT: begin
          rb_out_ch[0] <= car_cic_41M664_i_out[30:15];
          end
       RB_SRC_CON_PNT_NUM_CAR_CIC_41M664_Q_OUT: begin
          rb_out_ch[0] <= car_cic_41M664_q_out[30:15];
          end

       RB_SRC_CON_PNT_NUM_CAR_QMIX_I_OUT: begin
          rb_out_ch[0] <= car_qmix_i_out[30:15];
          end
       RB_SRC_CON_PNT_NUM_CAR_QMIX_Q_OUT: begin
          rb_out_ch[0] <= car_qmix_q_out[30:15];
          end

       RB_SRC_CON_PNT_NUM_AMP_RF_OUT: begin
          rb_out_ch[0] <= amp_rf_out[31:16];
          end

       RB_SRC_CON_PNT_NUM_TEST_VECTOR_OUT: begin
          rb_out_ch[0] <= { 1'b0, mod_fir_i_vld, 14'b0};
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

       RB_SRC_CON_PNT_NUM_MUXIN_MIX_IN: begin
          rb_out_ch[1] <= muxin_mix_in[15:0];
          end
       RB_SRC_CON_PNT_NUM_MOD_ADC_IN: begin
          rb_out_ch[1] <= mod_adc_in[15:0];
          end
       RB_SRC_CON_PNT_NUM_MOD_ADC_OUT: begin
          rb_out_ch[1] <= mod_adc_out[30:15];
          end

       RB_SRC_CON_PNT_NUM_MOD_QMIX_I_S1_OUT: begin
          rb_out_ch[1] <= mod_qmix_i_s1_out[30:15];
          end
       RB_SRC_CON_PNT_NUM_MOD_QMIX_Q_S1_OUT: begin
          rb_out_ch[1] <= mod_qmix_q_s1_out[30:15];
          end
       RB_SRC_CON_PNT_NUM_MOD_QMIX_I_S2_OUT: begin
          rb_out_ch[1] <= mod_qmix_i_s2_out[30:15];
          end
       RB_SRC_CON_PNT_NUM_MOD_QMIX_Q_S2_OUT: begin
          rb_out_ch[1] <= mod_qmix_q_s2_out[30:15];
          end
       RB_SRC_CON_PNT_NUM_MOD_QMIX_I_S3_OUT: begin
          rb_out_ch[1] <= mod_qmix_i_s3_out[47:32];
          end
       RB_SRC_CON_PNT_NUM_MOD_QMIX_Q_S3_OUT: begin
          rb_out_ch[1] <= mod_qmix_q_s3_out[47:32];
          end

       RB_SRC_CON_PNT_NUM_MOD_CIC_I_OUT: begin
          rb_out_ch[1] <= mod_cic_i_out[30:15];
          end
       RB_SRC_CON_PNT_NUM_MOD_CIC_Q_OUT: begin
          rb_out_ch[1] <= mod_cic_q_out[30:15];
          end
       RB_SRC_CON_PNT_NUM_MOD_FIR_I_OUT: begin
          rb_out_ch[1] <= mod_fir_i_out[32:17];
          end
       RB_SRC_CON_PNT_NUM_MOD_FIR_Q_OUT: begin
          rb_out_ch[1] <= mod_fir_q_out[32:17];
          end

       RB_SRC_CON_PNT_NUM_CAR_CIC_41M664_I_OUT: begin
          rb_out_ch[1] <= car_cic_41M664_i_out[30:15];
          end
       RB_SRC_CON_PNT_NUM_CAR_CIC_41M664_Q_OUT: begin
          rb_out_ch[1] <= car_cic_41M664_q_out[30:15];
          end

       RB_SRC_CON_PNT_NUM_CAR_QMIX_I_OUT: begin
          rb_out_ch[1] <= car_qmix_i_out[30:15];
          end
       RB_SRC_CON_PNT_NUM_CAR_QMIX_Q_OUT: begin
          rb_out_ch[1] <= car_qmix_q_out[30:15];
          end

       RB_SRC_CON_PNT_NUM_AMP_RF_OUT: begin
          rb_out_ch[1] <= amp_rf_out[31:16];
          end

       RB_SRC_CON_PNT_NUM_TEST_VECTOR_OUT: begin
          rb_out_ch[1] <= { 1'b0, mod_fir_q_vld, 14'b0};
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


// Bus handling

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

  regs[REG_RD_RB_STATUS][RB_STAT_CAR_OSC_ZERO]              <= !car_osc_sin;  // when phase is 0 deg
  regs[REG_RD_RB_STATUS][RB_STAT_CAR_OSC_VALID]             <= car_osc_axis_m_vld;

  regs[REG_RD_RB_STATUS][RB_STAT_MOD_OSC_ZERO]              <= !mod_osc_sin;  // when phase is 0 deg
  regs[REG_RD_RB_STATUS][RB_STAT_MOD_OSC_VALID]             <= mod_osc_axis_m_vld;

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
   regs[REG_RW_RB_CAR_OSC_INC_LO]         <= 32'h00000000;
   regs[REG_RW_RB_CAR_OSC_INC_HI]         <= 32'h00000000;
   regs[REG_RW_RB_CAR_OSC_OFS_LO]         <= 32'h00000000;
   regs[REG_RW_RB_CAR_OSC_OFS_HI]         <= 32'h00000000;
   regs[REG_RW_RB_AMP_RF_GAIN]            <= 32'h00000000;
   regs[REG_RW_RB_AMP_RF_OFS]             <= 32'h00000000;
   regs[REG_RW_RB_MOD_OSC_INC_LO]         <= 32'h00000000;
   regs[REG_RW_RB_MOD_OSC_INC_HI]         <= 32'h00000000;
   regs[REG_RW_RB_MOD_OSC_OFS_LO]         <= 32'h00000000;
   regs[REG_RW_RB_MOD_OSC_OFS_HI]         <= 32'h00000000;
   regs[REG_RW_RB_MOD_QMIX_GAIN]          <= 32'h00000000;
   regs[REG_RW_RB_MOD_QMIX_OFS_LO]        <= 32'h00000000;
   regs[REG_RW_RB_MOD_QMIX_OFS_HI]        <= 32'h00000000;
   regs[REG_RW_RB_MUXIN_SRC]              <= 32'h00000000;
   regs[REG_RW_RB_MUXIN_GAIN]             <= 32'h00000000;
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

      /* CAR_OSC */
      20'h00020: begin
         regs[REG_RW_RB_CAR_OSC_INC_LO]        <= sys_wdata[31:0];
         end
      20'h00024: begin
         regs[REG_RW_RB_CAR_OSC_INC_HI]        <= {16'b0, sys_wdata[15:0]};
         end
      20'h00028: begin
         regs[REG_RW_RB_CAR_OSC_OFS_LO]        <= sys_wdata[31:0];
         end
      20'h0002C: begin
         regs[REG_RW_RB_CAR_OSC_OFS_HI]        <= {16'b0, sys_wdata[15:0]};
         end
      20'h00030: begin
         regs[REG_RW_RB_AMP_RF_GAIN]           <= sys_wdata[15:0];
         end
      20'h00038: begin
         regs[REG_RW_RB_AMP_RF_OFS]            <= sys_wdata[15:0];
         end

      /* MOD_OSC */
      20'h00040: begin
         regs[REG_RW_RB_MOD_OSC_INC_LO]        <= sys_wdata[31:0];
         end
      20'h00044: begin
         regs[REG_RW_RB_MOD_OSC_INC_HI]        <= {16'b0, sys_wdata[15:0]};
         end
      20'h00048: begin
         regs[REG_RW_RB_MOD_OSC_OFS_LO]        <= sys_wdata[31:0];
         end
      20'h0004C: begin
         regs[REG_RW_RB_MOD_OSC_OFS_HI]        <= {16'b0, sys_wdata[15:0]};
         end
      20'h00050: begin
         regs[REG_RW_RB_MOD_QMIX_GAIN]         <= sys_wdata[31:0];
         end
      20'h00058: begin
         regs[REG_RW_RB_MOD_QMIX_OFS_LO]       <= sys_wdata[31:0];
         end
      20'h0005C: begin
         regs[REG_RW_RB_MOD_QMIX_OFS_HI]       <= {16'b0, sys_wdata[15:0]};
         end

      /* Input MUX */
      20'h00060: begin
         regs[REG_RW_RB_MUXIN_SRC]             <= {regs[REG_RW_RB_MUXIN_SRC][31:6], sys_wdata[5:0]};
         end

      20'h00064: begin
         regs[REG_RW_RB_MUXIN_GAIN]            <= sys_wdata[31:0];
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

      /* CAR_OSC */
      20'h00020: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_CAR_OSC_INC_LO];
         end
      20'h00024: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_CAR_OSC_INC_HI];
         end
      20'h00028: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_CAR_OSC_OFS_LO];
         end
      20'h0002C: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_CAR_OSC_OFS_HI];
         end
      20'h00030: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_AMP_RF_GAIN];
         end
      20'h00038: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_AMP_RF_OFS];
         end

      /* MOD_OSC */
      20'h00040: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_MOD_OSC_INC_LO];
         end
      20'h00044: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_MOD_OSC_INC_HI];
         end
      20'h00048: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_MOD_OSC_OFS_LO];
         end
      20'h0004C: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_MOD_OSC_OFS_HI];
         end
      20'h00050: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_MOD_QMIX_GAIN];
         end
      20'h00058: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_MOD_QMIX_OFS_LO];
         end
      20'h0005C: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_MOD_QMIX_OFS_HI];
         end

      /* Input MUX */
      20'h00060: begin
         sys_ack   <= sys_en;
         sys_rdata <= {26'b0, regs[REG_RW_RB_MUXIN_SRC][5:0]};
         end

      20'h00064: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_MUXIN_GAIN];
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
