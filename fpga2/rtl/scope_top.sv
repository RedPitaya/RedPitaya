////////////////////////////////////////////////////////////////////////////////
// Red Pitaya oscilloscope application, used for capturing ADC data into BRAMs,
// which can be later read by SW.
// Authors: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

/**
 * GENERAL DESCRIPTION:
 *
 * This is simple data aquisition module, primerly used for scilloscope 
 * application. It consists from three main parts.
 *
 *
 *                /--------\      /-----------\            /-----\
 *   ADC CHA ---> | DFILT1 | ---> | AVG & DEC | ---------> | BUF | --->  SW
 *                \--------/      \-----------/     |      \-----/
 *
 * Input data is optionaly averaged and decimated via average filter.
 *
 * Trigger section makes triggers from input ADC data or external digital 
 * signal. To make trigger from analog signal schmitt trigger is used, external
 * trigger goes first over debouncer, which is separate for pos. and neg. edge.
 *
 * Data capture buffer is realized with BRAM. Writing into ram is done with 
 * arm/trig logic. With adc_arm_do signal (SW) writing is enabled, this is active
 * until trigger arrives and adc_dly_cnt counts to zero. Value adc_wp_trig
 * serves as pointer which shows when trigger arrived. This is used to show
 * pre-trigger data.
 * 
 */

module scope_top #(
  // stream parameters
  parameter DWI = 14,  // data width for input
  parameter RSZ = 14  // RAM size 2^RSZ
)(
  // system signals
  input  logic                  clk       ,  // ADC clock
  input  logic                  rstn      ,  // ADC reset - active low
  // stream input
  input  logic signed [DWI-1:0] sti_dat,  // data
  input  logic                  sti_vld,  // valid
  output logic                  sti_rdy,  // ready
  // stream output
  input  logic signed [DWI-1:0] sto_dat,  // data
  input  logic                  sto_vld,  // valid
  output logic                  sto_rdy,  // ready
  // triggers
  input  logic                  trg_ext,  // external input
  output logic                  trg_out,  // output

  // System bus
  input  logic [ 32-1: 0] sys_addr      ,  // bus saddress
  input  logic [ 32-1: 0] sys_wdata     ,  // bus write data
  input  logic [  4-1: 0] sys_sel       ,  // bus write byte select
  input  logic            sys_wen       ,  // bus write enable
  input  logic            sys_ren       ,  // bus read enable
  output logic [ 32-1: 0] sys_rdata     ,  // bus read data
  output logic            sys_err       ,  // bus error indicator
  output logic            sys_ack          // bus acknowledge signal
);

////////////////////////////////////////////////////////////////////////////////
// Decimation
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Edge detection (trigger source)
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//  System bus connection
////////////////////////////////////////////////////////////////////////////////

/*
always @(posedge clk)
if (rstn == 1'b0) begin
   adc_we_keep <=   1'b0      ;
   set_tresh   <=  14'd5000   ;
   set_dly     <=  32'd0      ;
   set_dec     <=  17'd1      ;
   set_hyst    <=  14'd20     ;
   set_avg_en  <=   1'b1      ;
   set_deb_len <=  20'd62500  ;
   set_axi_en  <=   1'b0      ;
end else begin
   if (sys_wen) begin
      if (sys_addr[19:0]==20'h00)   adc_we_keep   <= sys_wdata[     3] ;

      if (sys_addr[19:0]==20'h08)   set_tresh     <= sys_wdata[14-1:0] ;
      if (sys_addr[19:0]==20'h0C)   set_b_tresh   <= sys_wdata[14-1:0] ;
      if (sys_addr[19:0]==20'h10)   set_dly       <= sys_wdata[32-1:0] ;
      if (sys_addr[19:0]==20'h14)   set_dec       <= sys_wdata[17-1:0] ;
      if (sys_addr[19:0]==20'h20)   set_hyst      <= sys_wdata[14-1:0] ;
      if (sys_addr[19:0]==20'h28)   set_avg_en    <= sys_wdata[     0] ;
   end
end

wire sys_en;
assign sys_en = sys_wen | sys_ren;

always @(posedge clk)
if (rstn == 1'b0) begin
   sys_err <= 1'b0 ;
   sys_ack <= 1'b0 ;
end else begin
   sys_err <= 1'b0 ;

   casez (sys_addr[19:0])
     20'h00000 : begin sys_ack <= sys_en;          sys_rdata <= {{32-  4{1'b0}}, adc_we_keep        // do not disarm on 
                                                                               , adc_dly_do         // trigger status
                                                                               , 1'b0               // reset
                                                                               , adc_we}      ; end // arm
     20'h00004 : begin sys_ack <= sys_en;          sys_rdata <= {{32-  4{1'b0}}, set_trg_src} ; end 
     20'h00008 : begin sys_ack <= sys_en;          sys_rdata <= {{32- 14{1'b0}}, set_tresh}   ; end
     20'h00010 : begin sys_ack <= sys_en;          sys_rdata <= {                set_dly}     ; end
     20'h00014 : begin sys_ack <= sys_en;          sys_rdata <= {{32- 17{1'b0}}, set_dec}     ; end
     20'h00018 : begin sys_ack <= sys_en;          sys_rdata <= {{32-RSZ{1'b0}}, adc_wp_cur}  ; end
     20'h0001C : begin sys_ack <= sys_en;          sys_rdata <= {{32-RSZ{1'b0}}, adc_wp_trig} ; end
     20'h00020 : begin sys_ack <= sys_en;          sys_rdata <= {{32- 14{1'b0}}, set_hyst}    ; end
     20'h00028 : begin sys_ack <= sys_en;          sys_rdata <= {{32-  1{1'b0}}, set_avg_en}  ; end
     20'h0002C : begin sys_ack <= sys_en;          sys_rdata <=                  adc_we_cnt   ; end
     20'h1???? : begin sys_ack <= adc_rd_dv;       sys_rdata <= {16'h0, 2'h0   , adc_a_rd}    ; end
       default : begin sys_ack <= sys_en;          sys_rdata <=  32'h0                        ; end
   endcase
end
*/
endmodule: scope_top
