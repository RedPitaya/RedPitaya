/**
 * $Id: red_pitaya_ac97.v 001 2016-04-12 18:55:00Z DF4IAH $
 *
 * @brief Red Pitaya AC97 adapter, used to import and export AC97 audio streams
 * for connection to and from the Red Pitaya FPGA.
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
 * The AC97 streams are connected to internal register nodes. Any input node can be used
 * as an additional audio/signal source, any output node can be used as an additional
 * audio/signal sink clocked with 48 kHz. This gives the Linux kernel a good chance to
 * attach an ALSA driver to this FPGA configuration.
 *
 * This module emulates the register map behaviour seen by the CPUs of the XILINX ML403 board,
 * which includes a Texas Instruments ML4550 AC97-Codec. The ML403 has an IP available called
 * "OPB AC97 Sound Controller" (opb_ac97_controller_ref) to interface the FPGA to the AC97-Codec.
 *
 * This module is a re-implementation of such a AC97-Controller with the interface described in
 * UG082, chapter 6 in detail.
 *
 * On the Linux kernel side a ALSA sound driver implements the interfacing of the ALSA sound
 * system to this FPGA configuration register map. Due to already working driver implementation
 * the module  (linux-xlnx)/sound/drivers/ml403-ac97cr.c  is selected to to this job.
 */


`timescale 1ns / 1ps

module red_pitaya_ac97ctrl #(
  // parameter RSZ = 14  // RAM size 2^RSZ
)(
   // ADC clock & reset
   input                 clk_adc_125mhz  ,      // ADC based clock, 125 MHz
   input                 adc_rstn_i      ,      // ADC reset - active low
   input                 clk_48khz       ,      // sound frame clock from RadioBox

   // AC97 lines
   input        [ 15: 0] ac97_line_i[0:0],      // AC97 line input nodes
   output reg   [ 15: 0] ac97_line_o[1:0],      // AC97 line output nodes

   // Interrupts
   output reg            ac97_irq_o      ,      // IRQ line signaling any pending interrupts

   // System bus - slave
   input        [ 31: 0] sys_addr        ,      // bus saddress
   input        [ 31: 0] sys_wdata       ,      // bus write data
   input        [  3: 0] sys_sel         ,      // bus write byte select
   input                 sys_wen         ,      // bus write enable
   input                 sys_ren         ,      // bus read enable
   output reg   [ 31: 0] sys_rdata       ,      // bus read data
   output reg            sys_err         ,      // bus error indicator
   output reg            sys_ack                // bus acknowledge signal
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

    REG_CTRL_COUNT
} REG_CTRL_ENUMS;

reg  [31: 0]    regs    [REG_CTRL_COUNT];       // AC97-Controller registers interface to be accessed by the system bus


// === OMNI section ===

//---------------------------------------------------------------------------------
// Short hand names

//wire                   rb_enable              = regs[REG_RW_RB_CTRL][RB_CTRL_ENABLE];
//wire unsigned [  7: 0] rb_pwr_rx_modvar       = regs[REG_RW_RB_PWR_CTRL][ 7:0];



/*
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

*/


// === Bus handling ===

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
