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
   output       [ 15: 0] rb_out_ch [1:0] ,      // RadioBox output signals

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
    REG_RW_RB_CTRL                      = 0,    // h00: RB control register
    REG_RD_RB_STATUS,                           // h04: EB status register
    REG_RW_RB_ICR,                              // h08: RB interrupt control register
    REG_RD_RB_ISR,                              // h0C: RB interrupt status register
    REG_RW_RB_DMA_CTRL,                         // h10: RB DMA control register
    //REG_RD_RB_RSVD_H14,
    //REG_RD_RB_RSVD_H18,
    REG_RW_RB_LED_CTRL,                         // h1C: RB LED magnitude indicator

    REG_RW_RB_OSC_CAR_INC_LO,                   // h20: RB OSC_CAR increment register              LSB:        (Bit 31: 0)
    REG_RW_RB_OSC_CAR_INC_HI,                   // h24: RB OSC_CAR increment register              MSB: 16'b0, (Bit 47:32)
    REG_RW_RB_OSC_CAR_OFS_LO,                   // h28: RB OSC_CAR offset register                 LSB:        (Bit 31: 0)
    REG_RW_RB_OSC_CAR_OFS_HI,                   // h2C: RB OSC_CAR offset register                 MSB: 16'b0, (Bit 47:32)

    REG_RW_RB_AMP_RF_GAIN,                      // h30: RB OSC_CAR mixer gain:     SIGNED 16 bit
    //REG_RD_RB_RSVD_H34,
    REG_RW_RB_AMP_RF_OFS,                       // h38: RB OSC_CAR mixer offset:   SIGNED 17 bit
    //REG_RD_RB_RSVD_H3C,

    REG_RW_RB_OSC_MOD_INC_LO,                   // h40: RB OSC_MOD increment register              LSB:        (Bit 31: 0)
    REG_RW_RB_OSC_MOD_INC_HI,                   // h44: RB OSC_MOD increment register              MSB: 16'b0, (Bit 47:32)
    REG_RW_RB_OSC_MOD_OFS_LO,                   // h48: RB OSC_MOD offset register                 LSB:        (Bit 31: 0)
    REG_RW_RB_OSC_MOD_OFS_HI,                   // h4C: RB OSC_MOD offset register                 MSB: 16'b0, (Bit 47:32)

    REG_RW_RB_QMIX_MOD_GAIN,                    // h50: RB OSC_MOD mixer gain:     SIGNED 16 bit
    //REG_RD_RB_RSVD_H54,
    REG_RW_RB_QMIX_MOD_OFS_LO,                  // h58: RB OSC_MOD mixer offset:   SIGNED 48 bit   LSB:        (Bit 31: 0)
    REG_RW_RB_QMIX_MOD_OFS_HI,                  // h5C: RB OSC_MOD mixer offset:   SIGNED 48 bit   MSB: 16'b0, (Bit 47:32)

    REG_RW_RB_MUXIN_SRC,                        // h60: RB analog MUX input selector:  d3=VpVn,
                                                //      d16=EXT-CH0,  d24=EXT-CH8,
                                                //      d17=EXT-CH1,  d25=EXT-CH9,
                                                //      d32=adc_i[0], d33=adc_i[1]
    REG_RW_RB_MUXIN_GAIN,                       // h64: RB analog MUX gain for input amplifier

    REG_RB_COUNT
} REG_RB_ENUMS;

reg  [31: 0]    regs    [REG_RB_COUNT];         // registers to be accessed by the system bus

enum {
    RB_CTRL_ENABLE                      = 0,    // enabling the RadioBox sub-module
    RB_CTRL_RESET_OSC_CAR,                      // reset OSC_CAR, does not touch clock enable
    RB_CTRL_RESET_OSC_MOD,                      // reset OSC_MOD, does not touch clock enable
    RB_CTRL_RSVD_D03,

    RB_CTRL_OSC_CAR_RESYNC,                     // OSC_CAR restart with phase register = 0
    RB_CTRL_OSC_CAR_INC_SRC_STREAM,             // OSC_CAR OSC incrementing: use stream instead of register setting
    RB_CTRL_OSC_CAR_OFS_SRC_STREAM,             // OSC_CAR OSC offset: use stream instead of register setting
    RB_CTRL_RSVD_D07,

    RB_CTRL_RSVD_D08,
    RB_CTRL_RSVD_D09,
    RB_CTRL_RSVD_D10,
    RB_CTRL_RSVD_D11,

    RB_CTRL_OSC_MOD_RESYNC,                     // OSC_MOD restart with phase register = 0
    RB_CTRL_OSC_MOD_INC_SRC_STREAM,             // OSC_MOD OSC incrementing: use stream instead of register setting
    RB_CTRL_OSC_MOD_OFS_SRC_STREAM,             // OSC_MOD OSC offset: use stream instead of register setting
    RB_CTRL_RSVD_D15,

    RB_CTRL_RSVD_D16,
    RB_CTRL_RSVD_D17,
    RB_CTRL_RSVD_D18,
    RB_CTRL_RSVD_D19,

    RB_CTRL_AMP_RF_Q_EN,                        // AMP_RF enable the QMIX_CAR Q path for the SSB modulation
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
    RB_STAT_CLK_EN                      = 0,    // RB clock enable
    RB_STAT_RESET,                              // RB reset
    RB_STAT_LEDS_EN,                            // RB LEDs enabled
    RB_STAT_RSVD_D03,

    RB_STAT_OSC_CAR_ZERO,                       // OSC_CAR output is zero
    RB_STAT_OSC_CAR_VALID,                      // OSC_CAR output valid
    RB_STAT_RSVD_D06,
    RB_STAT_RSVD_D07,

    RB_STAT_OSC_MOD_ZERO,                       // OSC_MOD output is zero
    RB_STAT_OSC_MOD_VALID,                      // OSC_MOD output valid
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
    RB_LED_CTRL_NUM_DISABLED            =  0,   // LEDs not touched
    RB_LED_CTRL_NUM_OFF,                        // all LEDs off (ro be used before switching to DISABLED)

    RB_LED_CTRL_NUM_MUXIN_MIX_IN        =  4,   // Magnitude indicator @ ADC selector input
    RB_LED_CTRL_NUM_ADC_MOD_IN,                 // Magnitude indicator @ modulation amplifier input
    RB_LED_CTRL_NUM_ADC_MOD_OUT,                // Magnitude indicator @ modulation amplifier output

    RB_LED_CTRL_NUM_QMIX_MOD_I_S1_OUT   =  8,   // Magnitude indicator @ QMIX_MOD I output at stage 1
    RB_LED_CTRL_NUM_QMIX_MOD_Q_S1_OUT,          // Magnitude indicator @ QMIX_MOD Q output at stage 1
    RB_LED_CTRL_NUM_QMIX_MOD_I_S2_OUT,          // Magnitude indicator @ QMIX_MOD I output at stage 2
    RB_LED_CTRL_NUM_QMIX_MOD_Q_S2_OUT,          // Magnitude indicator @ QMIX_MOD Q output at stage 2
    RB_LED_CTRL_NUM_QMIX_MOD_I_S3_OUT,          // Magnitude indicator @ QMIX_MOD I output at stage 3
    RB_LED_CTRL_NUM_QMIX_MOD_Q_S3_OUT,          // Magnitude indicator @ QMIX_MOD Q output at stage 3

    RB_LED_CTRL_NUM_CIC_MOD_I_OUT       = 16,   // Magnitude indicator @ CIC_MOD I output
    RB_LED_CTRL_NUM_CIC_MOD_Q_OUT,              // Magnitude indicator @ CIC_MOD Q output
    RB_LED_CTRL_NUM_FIR_MOD_I_OUT,              // Magnitude indicator @ FIR_MOD I output
    RB_LED_CTRL_NUM_FIR_MOD_Q_OUT,              // Magnitude indicator @ FIR_MOD Q output
    RB_LED_CTRL_NUM_CIC_CAR_I_OUT,              // Magnitude indicator @ CIC_CAR I output
    RB_LED_CTRL_NUM_CIC_CAR_Q_OUT,              // Magnitude indicator @ CIC_CAR Q output

    RB_LED_CTRL_NUM_QMIX_CAR_I_S1_OUT   = 24,   // Magnitude indicator @ QMIX_CAR I output at stage 1
    RB_LED_CTRL_NUM_QMIX_CAR_Q_S1_OUT,          // Magnitude indicator @ QMIX_CAR Q output at stage 1

    RB_LED_CTRL_NUM_AMP_RF_OUT          = 28    // Magnitude indicator @ AMP_RF output
} RB_LED_CTRL_ENUM;

enum {
    RB_XADC_MAPPING_EXT_CH0             = 0,    // CH0 and CH8 are sampled simultaneously, mapped to: vinp_i[0]/vinn_i[0]
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

wire         rb_reset_osc_car_n    = rb_reset_n & !regs[REG_RW_RB_CTRL][RB_CTRL_RESET_OSC_CAR];
wire         rb_reset_osc_mod_n    = rb_reset_n & !regs[REG_RW_RB_CTRL][RB_CTRL_RESET_OSC_MOD];


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

wire [15:0] adc_mod_in = (muxin_mix_in << muxin_mix_log2);  // unsigned value: input booster for
                                                            // factor: 1x .. 2^3=7 shift postions=128x (16 mV --> full-scale)
wire [31:0] adc_mod_out;

rb_dsp48_AaDmB_A16_D16_B16_P32 i_rb_dsp48_adc_mod (
  // global signals
  .CLK                  ( clk_adc_125mhz    ),  // global 125 MHz clock
  .CE                   ( rb_clk_en         ),  // enable part 1 of RadioBox sub-module

  // ADC/XADC modulation input
  .A                    ( adc_mod_in        ),  // MUX in signal:    SIGNED 16 bit
  // modulation offset input
  .D                    ( 16'b0             ),  // offset setting:   SIGNED 16 bit
  // modulation gain input
  .B                    ( muxin_mix_gain    ),  // gain setting:     SIGNED 16 bit

  // multiplier output
  .P                    ( adc_mod_out       )   // PreAmp output     SIGSIG 32 bit
);


//---------------------------------------------------------------------------------
//  OSC_MOD modulation generator and SSB weaver modulator

wire         osc_mod_inc_mux = regs[REG_RW_RB_CTRL][RB_CTRL_OSC_MOD_INC_SRC_STREAM];
wire         osc_mod_ofs_mux = regs[REG_RW_RB_CTRL][RB_CTRL_OSC_MOD_OFS_SRC_STREAM];
wire         osc_mod_resync  = regs[REG_RW_RB_CTRL][RB_CTRL_OSC_MOD_RESYNC];

wire [ 47:0] osc_mod_inc_stream = 48'b0;  // TODO: ADC
wire [ 47:0] osc_mod_ofs_stream = 48'b0;  // TODO: ADC

wire [ 47:0] osc_mod_inc = ( osc_mod_inc_mux ?  osc_mod_inc_stream : {regs[REG_RW_RB_OSC_MOD_INC_HI][15:0], regs[REG_RW_RB_OSC_MOD_INC_LO][31:0]});
wire [ 47:0] osc_mod_ofs = ( osc_mod_ofs_mux ?  osc_mod_ofs_stream : {regs[REG_RW_RB_OSC_MOD_OFS_HI][15:0], regs[REG_RW_RB_OSC_MOD_OFS_LO][31:0]});

wire         osc_mod_axis_s_vld   = rb_reset_n;  // TODO: ADC
wire [103:0] osc_mod_axis_s_phase = {7'b0, osc_mod_resync, osc_mod_ofs, osc_mod_inc};

wire         osc_mod_axis_m_vld;
wire [ 31:0] osc_mod_axis_m_data;
wire [ 15:0] osc_mod_cos = osc_mod_axis_m_data[15: 0];
wire [ 15:0] osc_mod_sin = osc_mod_axis_m_data[31:16];

rb_dds_48_16_125 i_rb_osc_mod (
  // global signals
  .aclk                 ( clk_adc_125mhz       ),  // global 125 MHz clock
  .aclken               ( rb_clk_en            ),  // clock enable of RadioBox sub-module
  .aresetn              ( rb_reset_osc_mod_n   ),  // reset of OSC_MOD

  // AXI-Stream slave in port: streaming data for OSC_MOD modulation
  .s_axis_phase_tvalid  ( osc_mod_axis_s_vld   ),  // AXIS slave data valid
  .s_axis_phase_tdata   ( osc_mod_axis_s_phase ),  // AXIS slave data

  // AXI-Stream master out port: OSC_MOD signal
  .m_axis_data_tvalid   ( osc_mod_axis_m_vld   ),  // AXIS master OSC_MOD data valid
  .m_axis_data_tdata    ( osc_mod_axis_m_data  )   // AXIS master OSC_MOD output: 2x SIGNED 16 bit
);


//---------------------------------------------------------------------------------
//  QMIX_MOD quadrature mixer for the base band

wire        amp_rf_q_en    = regs[REG_RW_RB_CTRL][RB_CTRL_AMP_RF_Q_EN];

wire [15:0] qmix_mod_in    = (regs[REG_RW_RB_MUXIN_SRC][5:0] == 6'h00) ?  16'h7fff : adc_mod_out[30:15];  // when ADC source ID is zero take cos() from OSC_MOD only
wire [15:0] qmix_mod_gain  =  regs[REG_RW_RB_QMIX_MOD_GAIN][15:0];
wire [47:0] qmix_mod_ofs   = {regs[REG_RW_RB_QMIX_MOD_OFS_HI][15:0], regs[REG_RW_RB_QMIX_MOD_OFS_LO][31:0]};

wire [31:0] qmix_mod_i_s1_out;
wire [31:0] qmix_mod_q_s1_out;

wire [31:0] qmix_mod_i_s2_out;
wire [31:0] qmix_mod_q_s2_out;

wire [47:0] qmix_mod_i_s3_in = regs[REG_RW_RB_CTRL][RB_CTRL_OSC_CAR_INC_SRC_STREAM] ?
                               {{15{qmix_mod_i_s2_out[30]}}, qmix_mod_i_s2_out[29:0], 3'b0} :  /* when FM is used, take 2^14 finer resolution */
                               {qmix_mod_i_s2_out[30:0], 17'b0};
wire [47:0] qmix_mod_q_s3_in = {qmix_mod_q_s2_out[30:0], 17'b0};
wire [47:0] qmix_mod_i_s3_out;
wire [47:0] qmix_mod_q_s3_out;

rb_dsp48_AmB_A16_B16_P32 i_rb_dsp48_qmix_mod_s1_I (
  // global signals
  .CLK                  ( clk_adc_125mhz    ),  // global 125 MHz clock
  .CE                   ( rb_clk_en         ),  // enable part 1 of RadioBox sub-module

  // modulation input
  .A                    ( qmix_mod_in       ),  // MUX in signal:    SIGNED 16 bit
  // OSC_MOD cos input
  .B                    ( osc_mod_cos       ),  // OSC_MOD cos:      SIGNED 16 bit

  // multiplier output stage 1
  .P                    ( qmix_mod_i_s1_out )   // QMIX I output:    SIGSIG 32 bit
);

rb_dsp48_AmB_A16_B16_P32 i_rb_dsp48_qmix_mod_s2_I (
  // global signals
  .CLK                  ( clk_adc_125mhz    ),  // global 125 MHz clock
  .CE                   ( rb_clk_en         ),  // enable part 1 of RadioBox sub-module

  // QMIX I input
  .A                    ( qmix_mod_i_s1_out[30:15] ),  // MUX in signal:    SIGNED 16 bit
  // gain setting input
  .B                    ( qmix_mod_gain     ),  // gain setting:     SIGNED 16 bit

  // multiplier output stage 2
  .P                    ( qmix_mod_i_s2_out )   // QMIX I regulated: SIGSIG 32 bit
);

rb_dsp48_CONaC_CON48_C48_P48 i_rb_dsp48_qmix_mod_s3_I (
  // global signals
  .CLK                  ( clk_adc_125mhz    ),  // global 125 MHz clock
  .CE                   ( rb_clk_en         ),  // enable part 1 of RadioBox sub-module

  // modulation input
  .C                    ( qmix_mod_i_s3_in  ),  // modulation:       SIGNED 48 bit
  // offset value for OSC control
  .CONCAT               ( qmix_mod_ofs      ),  // offset:           SIGNED 48 bit

  // adder output
  .P                    ( qmix_mod_i_s3_out )   // QMIX I 4_OSC:     SIGNED 48 bit
);


rb_dsp48_AmB_A16_B16_P32 i_rb_dsp48_qmix_mod_s1_Q (
  // global signals
  .CLK                  ( clk_adc_125mhz    ),  // global 125 MHz clock
  .CE                   ( rb_clk_en & amp_rf_q_en ),  // enable part 1 of RadioBox sub-module and the Q path

  // modulation input
  .A                    ( qmix_mod_in       ),  // MUX in signal:    SIGNED 16 bit
  // OSC_MOD sin input
  .B                    ( osc_mod_sin       ),  // OSC_MOD sin:      SIGNED 16 bit

  // multiplier output stage 1
  .P                    ( qmix_mod_q_s1_out )   // QMIX Q output:    SIGSIG 32 bit
);

rb_dsp48_AmB_A16_B16_P32 i_rb_dsp48_qmix_mod_s2_Q (
  // global signals
  .CLK                  ( clk_adc_125mhz    ),  // global 125 MHz clock
  .CE                   ( rb_clk_en & amp_rf_q_en ),  // enable part 1 of RadioBox sub-module and the Q path

  // QMIX Q input
  .A                    ( qmix_mod_q_s1_out[30:15] ),  // MUX in signal:    SIGNED 16 bit
  // gain setting input
  .B                    ( qmix_mod_gain     ),  // gain setting:     SIGNED 16 bit

  // multiplier output stage 2
  .P                    ( qmix_mod_q_s2_out )   // QMIX Q regulated: SIGSIG 32 bit
);

rb_dsp48_CONaC_CON48_C48_P48 i_rb_dsp48_qmix_mod_s3_Q (
  // global signals
  .CLK                  ( clk_adc_125mhz    ),  // global 125 MHz clock
  .CE                   ( rb_clk_en & amp_rf_q_en ),  // enable part 1 of RadioBox sub-module and the Q path

  // modulation input
  .C                    ( qmix_mod_q_s3_in  ),  // modulation:       SIGNED 48 bit
  // offset value for OSC control
  .CONCAT               ( qmix_mod_ofs      ),  // offset:           SIGNED 48 bit

  // adder output
  .P                    ( qmix_mod_q_s3_out )   // QMIX Q 4_OSC:     SIGNED 48 bit
);


//---------------------------------------------------------------------------------
//  CIC_MOD low pass filters

wire [31:0] cic_mod_i_out;
wire [31:0] cic_mod_q_out;

rb_cic_48k_to_8k_32T32_lat13 i_rb_cic_mod_I (
  // global signals
  .aclk                 ( clk_adc_125mhz    ),  // global 125 MHz clock
  .aclken               ( rb_clk_en         ),  // enable RadioBox sub-module

  .s_axis_data_tdata    ( qmix_mod_i_s2_out ),  // QMIX I stage 2
  .s_axis_data_tvalid   ( 1'b1              ),
  .s_axis_data_tready   (                   ),

  .m_axis_data_tdata    ( cic_mod_i_out     ),  // CIC_MOD output I
  .m_axis_data_tvalid   (                   )
);

rb_cic_48k_to_8k_32T32_lat13 i_rb_cic_mod_Q (
  // global signals
  .aclk                 ( clk_adc_125mhz    ),  // global 125 MHz clock
  .aclken               ( rb_clk_en         ),  // enable RadioBox sub-module

  .s_axis_data_tdata    ( qmix_mod_q_s2_out ),  // QMIX Q stage 2
  .s_axis_data_tvalid   ( 1'b1              ),
  .s_axis_data_tready   (                   ),

  .m_axis_data_tdata    ( cic_mod_q_out     ),  // CIC_MOD output Q
  .m_axis_data_tvalid   (                   )
);


//---------------------------------------------------------------------------------
//  FIR_MOD high pass filter for CIC compensation in the voice band
/*
wire [ 15:0] fir_mod_i_out;
wire [ 15:0] fir_mod_q_out;

rb_fir i_rb_fir_mod_I (
  // global signals
  .aclk                 ( clk_adc_125mhz    ),  // global 125 MHz clock
  .aclken               ( rb_clk_en         ),  // enable RadioBox sub-module

  .s_axis_data_tdata    ( cic_mod_i_out     ),  // CIC_MOD output I
  .s_axis_data_tvalid   ( 1'b1              ),
  .s_axis_data_tready   (                   ),

  .m_axis_data_tdata    ( fir_mod_i_out     ),  // FIR_MOD output I
  .m_axis_data_tvalid   (                   )
);

rb_fir i_rb_fir_mod_Q (
  // global signals
  .aclk                 ( clk_adc_125mhz    ),  // global 125 MHz clock
  .aclken               ( rb_clk_en         ),  // enable RadioBox sub-module

  .s_axis_data_tdata    ( cic_mod_q_out     ),  // CIC_MOD output Q
  .s_axis_data_tvalid   ( 1'b1              ),
  .s_axis_data_tready   (                   ),

  .m_axis_data_tdata    ( fir_mod_q_out     ),  // FIR_MOD output Q
  .m_axis_data_tvalid   (                   )
);
*/

//---------------------------------------------------------------------------------
//  OSC_CAR carrier frequency generator  (CW, FM, PM modulated)

wire         osc_car_inc_mux = regs[REG_RW_RB_CTRL][RB_CTRL_OSC_CAR_INC_SRC_STREAM];
wire         osc_car_ofs_mux = regs[REG_RW_RB_CTRL][RB_CTRL_OSC_CAR_OFS_SRC_STREAM];
wire         osc_car_resync  = regs[REG_RW_RB_CTRL][RB_CTRL_OSC_CAR_RESYNC];

wire [ 47:0] osc_car_inc = ( osc_car_inc_mux ?  qmix_mod_i_s3_out : {regs[REG_RW_RB_OSC_CAR_INC_HI][15:0], regs[REG_RW_RB_OSC_CAR_INC_LO][31:0]});
wire [ 47:0] osc_car_ofs = ( osc_car_ofs_mux ?  qmix_mod_i_s3_out : {regs[REG_RW_RB_OSC_CAR_OFS_HI][15:0], regs[REG_RW_RB_OSC_CAR_OFS_LO][31:0]});

wire         osc_car_axis_s_vld   = rb_reset_n;  // TODO
wire [103:0] osc_car_axis_s_phase = {7'b0, osc_car_resync, osc_car_ofs, osc_car_inc};

wire         osc_car_axis_m_vld;
wire [ 31:0] osc_car_axis_m_data;

wire [ 15:0] osc_car_cos = osc_car_axis_m_data[15: 0];
wire [ 15:0] osc_car_sin = osc_car_axis_m_data[31:16];

rb_dds_48_16_125 i_rb_osc_car (
  // global signals
  .aclk                 ( clk_adc_125mhz       ),  // global 125 MHz clock
  .aclken               ( rb_clk_en            ),  // enable RadioBox sub-module
  .aresetn              ( rb_reset_osc_car_n   ),  // reset of OSC_CAR

  // simple-AXI slave in port: streaming data for OSC_CAR modulation
  .s_axis_phase_tvalid  ( osc_car_axis_s_vld   ),  // AXIS slave data valid
  .s_axis_phase_tdata   ( osc_car_axis_s_phase ),  // AXIS slave data

  // simple-AXI master out port: OSC_CAR signal
  .m_axis_data_tvalid   ( osc_car_axis_m_vld   ),  // AXIS master OSC_CAR data valid
  .m_axis_data_tdata    ( osc_car_axis_m_data  )   // AXIS master OSC_CAR output: Q SIGNED 16 bit, I SIGNED 16 bit
);


//---------------------------------------------------------------------------------
//  QMIX_CAR quadrature mixer for the radio frequency

wire [ 15:0] qmix_car_i_in = (osc_car_inc_mux || osc_car_ofs_mux)                 ?  16'h7fff : amp_rf_q_en ?  cic_mod_i_out[29:14] : qmix_mod_i_s3_out[47:32];  // QMIX_MOD/CIC_MOD uses full scale constant for CW, FM and PM modulations - SSB uses CIC I instead
wire [ 15:0] qmix_car_q_in = (osc_car_inc_mux || osc_car_ofs_mux || !amp_rf_q_en) ?  16'h0000 :                cic_mod_q_out[29:14]                           ;  // QMIX_MOD/CIC_MOD Q path keep quiet when Q is disabled - SSB uses CIC Q instead

wire [ 31:0] qmix_car_i_s1_out;
wire [ 31:0] qmix_car_q_s1_out;

rb_dsp48_AmB_A16_B16_P32 i_rb_dsp48_qmix_car_s1_I (
  // global signals
  .CLK                  ( clk_adc_125mhz    ),  // global 125 MHz clock
  .CE                   ( rb_clk_en         ),  // enable part 1 of RadioBox sub-module

  // modulation input
  .A                    ( qmix_car_i_in     ),  // MUX in signal:        SIGNED 16 bit
  // OSC_CAR cos input
  .B                    ( osc_car_cos       ),  // OSC_CAR cos:          SIGNED 16 bit

  // multiplier output
  .P                    ( qmix_car_i_s1_out )   // QMIX_CAR I output:    SIGSIG 32 bit
);

rb_dsp48_AmB_A16_B16_P32 i_rb_dsp48_qmix_car_s1_Q (
  // global signals
  .CLK                  ( clk_adc_125mhz    ),  // global 125 MHz clock
  .CE                   ( rb_clk_en & amp_rf_q_en ),  // enable part 1 of RadioBox sub-module and the Q path

  // modulation input
  .A                    ( qmix_car_q_in     ),  // MUX in signal:        SIGNED 16 bit
  // OSC_CAR sin input
  .B                    ( osc_car_sin       ),  // OSC_CAR sin:          SIGNED 16 bit

  // multiplier output
  .P                    ( qmix_car_q_s1_out )   // QMIX_CAR Q output:    SIGSIG 32 bit
);


//---------------------------------------------------------------------------------
//  AMP_CTL amplifier control for setting gain of the radio frequency amplifier


//---------------------------------------------------------------------------------
//  AMP_RF amplifier for the radio frequency output (CW, AM modulated)

wire [16:0] amp_rf_i_var =                {qmix_car_i_s1_out[30], qmix_car_i_s1_out[30:15]}        ;  // halfed and sign corrected 17 bit extension
wire [16:0] amp_rf_q_var = amp_rf_q_en ?  {qmix_car_q_s1_out[30], qmix_car_q_s1_out[30:15]} : 17'b0;  // halfed and sign corrected 17 bit extension
wire [16:0] amp_rf_gain  = {regs[REG_RW_RB_AMP_RF_GAIN][15:0], 1'b0};  // signed register value
wire [34:0] amp_rf_ofs   = {regs[REG_RW_RB_AMP_RF_OFS][15:0], 19'b0};  // signed register value

wire [35:0] amp_rf_out;

rb_dsp48_AaDmBaC_A17_D17_B17_C35_P36 i_rb_dsp48_amp_rf (
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
reg  [15: 0] monitor  = 16'b0;

wire [ 4: 0] led_ctrl = regs[REG_RW_RB_LED_CTRL][4:0];

function bit [7:0] fct_mag (input bit [15:0] val);
   automatic bit [7:0] leds = 8'b0;             // exakt zero indicator

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
   monitor       <= 16'b0;
   end

else begin
   if (led_ctrl && rb_activated) begin
      rb_leds_en <=  1'b1;                      // LEDs magnitude indicator active

       case (led_ctrl)

       RB_LED_CTRL_NUM_DISABLED: begin
          rb_leds_data <=  8'b0;
          monitor      <= 16'b0;
          end
       RB_LED_CTRL_NUM_OFF: begin
          rb_leds_data <=  8'b0;                // turn all LEDs off
          monitor      <= 16'b0;
          end
       RB_LED_CTRL_NUM_MUXIN_MIX_IN: begin
          if (!led_ctr) rb_leds_data <= fct_mag(muxin_mix_in[15:0]);  // updating about 120 Hz for reducing EMI
          monitor <= muxin_mix_in[15:0];
          end
       RB_LED_CTRL_NUM_ADC_MOD_IN: begin
          if (!led_ctr) rb_leds_data <= fct_mag(adc_mod_in[15:0]);
          monitor <= adc_mod_in[15:0];
          end
       RB_LED_CTRL_NUM_ADC_MOD_OUT: begin
          if (!led_ctr) rb_leds_data <= fct_mag(adc_mod_out[30:15]);
          monitor <= adc_mod_out[30:15];
          end
       RB_LED_CTRL_NUM_QMIX_MOD_I_S1_OUT: begin
          if (!led_ctr) rb_leds_data <= fct_mag(qmix_mod_i_s1_out[30:15]);
          monitor <= qmix_mod_i_s1_out[30:15];
          end
       RB_LED_CTRL_NUM_QMIX_MOD_Q_S1_OUT: begin
          if (!led_ctr) rb_leds_data <= fct_mag(qmix_mod_q_s1_out[30:15]);
          monitor <= qmix_mod_q_s1_out[30:15];
          end
       RB_LED_CTRL_NUM_QMIX_MOD_I_S2_OUT: begin
          if (!led_ctr) rb_leds_data <= fct_mag(qmix_mod_i_s2_out[30:15]);
          monitor <= qmix_mod_i_s2_out[30:15];
          end
       RB_LED_CTRL_NUM_QMIX_MOD_Q_S2_OUT: begin
          if (!led_ctr) rb_leds_data <= fct_mag(qmix_mod_q_s2_out[30:15]);
          monitor <= qmix_mod_q_s2_out[30:15];
          end
       RB_LED_CTRL_NUM_QMIX_MOD_I_S3_OUT: begin
          if (!led_ctr) rb_leds_data <= fct_mag(qmix_mod_i_s3_out[47:32]);
          monitor <= qmix_mod_i_s3_out[47:32];
          end
       RB_LED_CTRL_NUM_QMIX_MOD_Q_S3_OUT: begin
          if (!led_ctr) rb_leds_data <= fct_mag(qmix_mod_q_s3_out[47:32]);
          monitor <= qmix_mod_q_s3_out[47:32];
          end
       RB_LED_CTRL_NUM_CIC_MOD_I_OUT: begin
          if (!led_ctr) rb_leds_data <= fct_mag(cic_mod_i_out[29:14]);
          monitor <= cic_mod_i_out[29:14];
          end
       RB_LED_CTRL_NUM_CIC_MOD_Q_OUT: begin
          if (!led_ctr) rb_leds_data <= fct_mag(cic_mod_q_out[29:14]);
          monitor <= cic_mod_q_out[29:14];
          end
/*
       RB_LED_CTRL_NUM_FIR_MOD_I_OUT: begin
          if (!led_ctr) rb_leds_data <= fct_mag(fir_mod_i_out[15:0]);
          monitor <= fir_mod_i_out[15:0];
          end
       RB_LED_CTRL_NUM_FIR_MOD_Q_OUT: begin
          if (!led_ctr) rb_leds_data <= fct_mag(fir_mod_q_out[15:0]);
          monitor <= fir_mod_q_out[15:0];
          end
       RB_LED_CTRL_NUM_CIC_CAR_I_OUT: begin
          if (!led_ctr) rb_leds_data <= fct_mag(cic_car_i_out[15:0]);
          monitor <= cic_car_i_out[15:0];
          end
       RB_LED_CTRL_NUM_CIC_CAR_Q_OUT: begin
          if (!led_ctr) rb_leds_data <= fct_mag(cic_car_q_out[15:0]);
          monitor <= cic_car_q_out[15:0];
          end
*/
       RB_LED_CTRL_NUM_QMIX_CAR_I_S1_OUT: begin
          if (!led_ctr) rb_leds_data <= fct_mag(qmix_car_i_s1_out[30:15]);
          monitor <= qmix_car_i_s1_out[30:15];
          end
       RB_LED_CTRL_NUM_QMIX_CAR_Q_S1_OUT: begin
          if (!led_ctr) rb_leds_data <= fct_mag(qmix_car_q_s1_out[30:15]);
          monitor <= qmix_car_q_s1_out[30:15];
          end
       RB_LED_CTRL_NUM_AMP_RF_OUT: begin
          if (!led_ctr) rb_leds_data <= fct_mag(amp_rf_out[31:16]);
          monitor <= amp_rf_out[31:16];
          end

       default: begin
          rb_leds_data <=  8'b0;
          monitor      <= 16'b0;
          end

       endcase
      led_ctr <= led_ctr + 1;
      end
   else begin                                   // RB_LED_CTRL_NUM_DISABLED
      rb_leds_en   <=  1'b0;
      rb_leds_data <=  8'b0;
      led_ctr      <= 20'b0;
      monitor      <= 16'b0;
      end
   end


//---------------------------------------------------------------------------------
//  RB output signal assignments

   assign rb_out_ch[0] = amp_rf_out[31:16];
   assign rb_out_ch[1] = monitor;



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

  regs[REG_RD_RB_STATUS][RB_STAT_OSC_CAR_ZERO]              <= !osc_car_sin;  // when phase is 0 deg
  regs[REG_RD_RB_STATUS][RB_STAT_OSC_CAR_VALID]             <= osc_car_axis_m_vld;

  regs[REG_RD_RB_STATUS][RB_STAT_OSC_MOD_ZERO]              <= !osc_mod_sin;  // when phase is 0 deg
  regs[REG_RD_RB_STATUS][RB_STAT_OSC_MOD_VALID]             <= osc_mod_axis_m_vld;

  regs[REG_RD_RB_STATUS][RB_STAT_LED7_ON : RB_STAT_LED0_ON] <= rb_leds_data;
  end


//---------------------------------------------------------------------------------
//  System bus connection

// write access to the registers
always @(posedge clk_adc_125mhz)
if (!adc_rstn_i) begin
   regs[REG_RW_RB_CTRL]                <= 32'h00000000;
   regs[REG_RW_RB_ICR]                 <= 32'h00000000;
   regs[REG_RD_RB_ISR]                 <= 32'h00000000;
   regs[REG_RW_RB_DMA_CTRL]            <= 32'h00000000;
   regs[REG_RW_RB_LED_CTRL]            <= 32'h00000000;
   regs[REG_RW_RB_OSC_CAR_INC_LO]      <= 32'h00000000;
   regs[REG_RW_RB_OSC_CAR_INC_HI]      <= 32'h00000000;
   regs[REG_RW_RB_OSC_CAR_OFS_LO]      <= 32'h00000000;
   regs[REG_RW_RB_OSC_CAR_OFS_HI]      <= 32'h00000000;
   regs[REG_RW_RB_AMP_RF_GAIN]         <= 32'h00000000;
   regs[REG_RW_RB_AMP_RF_OFS]          <= 32'h00000000;
   regs[REG_RW_RB_OSC_MOD_INC_LO]      <= 32'h00000000;
   regs[REG_RW_RB_OSC_MOD_INC_HI]      <= 32'h00000000;
   regs[REG_RW_RB_OSC_MOD_OFS_LO]      <= 32'h00000000;
   regs[REG_RW_RB_OSC_MOD_OFS_HI]      <= 32'h00000000;
   regs[REG_RW_RB_QMIX_MOD_GAIN]    <= 32'h00000000;
   regs[REG_RW_RB_QMIX_MOD_OFS_LO]  <= 32'h00000000;
   regs[REG_RW_RB_QMIX_MOD_OFS_HI]  <= 32'h00000000;
   regs[REG_RW_RB_MUXIN_SRC]           <= 32'h00000000;
   regs[REG_RW_RB_MUXIN_GAIN]          <= 32'h00000000;
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
         regs[REG_RW_RB_LED_CTRL]              <= sys_wdata[31:0];
         end

      /* OSC_CAR */
      20'h00020: begin
         regs[REG_RW_RB_OSC_CAR_INC_LO]        <= sys_wdata[31:0];
         end
      20'h00024: begin
         regs[REG_RW_RB_OSC_CAR_INC_HI]        <= {16'b0, sys_wdata[15:0]};
         end
      20'h00028: begin
         regs[REG_RW_RB_OSC_CAR_OFS_LO]        <= sys_wdata[31:0];
         end
      20'h0002C: begin
         regs[REG_RW_RB_OSC_CAR_OFS_HI]        <= {16'b0, sys_wdata[15:0]};
         end
      20'h00030: begin
         regs[REG_RW_RB_AMP_RF_GAIN]           <= sys_wdata[15:0];
         end
      20'h00038: begin
         regs[REG_RW_RB_AMP_RF_OFS]            <= sys_wdata[15:0];
         end

      /* OSC_MOD */
      20'h00040: begin
         regs[REG_RW_RB_OSC_MOD_INC_LO]        <= sys_wdata[31:0];
         end
      20'h00044: begin
         regs[REG_RW_RB_OSC_MOD_INC_HI]        <= {16'b0, sys_wdata[15:0]};
         end
      20'h00048: begin
         regs[REG_RW_RB_OSC_MOD_OFS_LO]        <= sys_wdata[31:0];
         end
      20'h0004C: begin
         regs[REG_RW_RB_OSC_MOD_OFS_HI]        <= {16'b0, sys_wdata[15:0]};
         end
      20'h00050: begin
         regs[REG_RW_RB_QMIX_MOD_GAIN]         <= sys_wdata[31:0];
         end
      20'h00058: begin
         regs[REG_RW_RB_QMIX_MOD_OFS_LO]       <= sys_wdata[31:0];
         end
      20'h0005C: begin
         regs[REG_RW_RB_QMIX_MOD_OFS_HI]       <= {16'b0, sys_wdata[15:0]};
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
         sys_rdata <= regs[REG_RW_RB_LED_CTRL];
         end

      /* OSC_CAR */
      20'h00020: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_OSC_CAR_INC_LO];
         end
      20'h00024: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_OSC_CAR_INC_HI];
         end
      20'h00028: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_OSC_CAR_OFS_LO];
         end
      20'h0002C: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_OSC_CAR_OFS_HI];
         end
      20'h00030: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_AMP_RF_GAIN];
         end
      20'h00038: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_AMP_RF_OFS];
         end

      /* OSC_MOD */
      20'h00040: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_OSC_MOD_INC_LO];
         end
      20'h00044: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_OSC_MOD_INC_HI];
         end
      20'h00048: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_OSC_MOD_OFS_LO];
         end
      20'h0004C: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_OSC_MOD_OFS_HI];
         end
      20'h00050: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_QMIX_MOD_GAIN];
         end
      20'h00058: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_QMIX_MOD_OFS_LO];
         end
      20'h0005C: begin
         sys_ack   <= sys_en;
         sys_rdata <= regs[REG_RW_RB_QMIX_MOD_OFS_HI];
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
