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
  // ADC
  input  logic signed [DWI-1:0] sti_dat,  // data
  input  logic                  sti_vld,  // valid
  output logic                  sti_rdy,  // ready
  // triggers
  input  logic                  trg_ext,  // external input
  output logic                  trg_out,  // output

   // AXI0 master
   output logic            axi_clk_o      ,  // global clock
   output logic            axi_rstn_o     ,  // global reset
   output logic [ 32-1: 0] axi_waddr_o    ,  // system write address
   output logic [ 64-1: 0] axi_wdata_o    ,  // system write data
   output logic [  8-1: 0] axi_wsel_o     ,  // system write byte select
   output logic            axi_wvalid_o   ,  // system write data valid
   output logic [  4-1: 0] axi_wlen_o     ,  // system write burst length
   output logic            axi_wfixed_o   ,  // system write burst type (fixed / incremental)
   input  logic            axi_werr_i     ,  // system write error
   input  logic            axi_wrdy_i     ,  // system write ready
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

reg             adc_arm_do   ;
reg             adc_rst_do   ;

//---------------------------------------------------------------------------------
//  Input filtering

wire [ 14-1: 0] adc_a_filt_in  ;
wire [ 14-1: 0] adc_a_filt_out ;
reg  [ 18-1: 0] set_filt_aa  ;
reg  [ 25-1: 0] set_filt_bb  ;
reg  [ 25-1: 0] set_filt_kk  ;
reg  [ 25-1: 0] set_filt_pp  ;

assign adc_a_filt_in = sti_dat ;
assign adc_b_filt_in = adc_b_i ;

red_pitaya_dfilt1 i_dfilt1_cha (
   // ADC
  .clk   ( clk       ),  // ADC clock
  .rstn  ( rstn      ),  // ADC reset - active low
  .adc_dat_i   ( adc_a_filt_in   ),  // ADC data
  .adc_dat_o   ( adc_a_filt_out  ),  // ADC data
   // configuration
  .cfg_aa_i    ( set_filt_aa   ),  // config AA coefficient
  .cfg_bb_i    ( set_filt_bb   ),  // config BB coefficient
  .cfg_kk_i    ( set_filt_kk   ),  // config KK coefficient
  .cfg_pp_i    ( set_filt_pp   )   // config PP coefficient
);

//---------------------------------------------------------------------------------
//  Decimate input data

reg  [ 14-1: 0] adc_a_dat     ;
reg  [ 32-1: 0] adc_a_sum     ;
reg  [ 17-1: 0] set_dec       ;
reg  [ 17-1: 0] adc_dec_cnt   ;
reg             set_avg_en    ;
reg             adc_dv        ;

always @(posedge clk)
if (rstn == 1'b0) begin
   adc_a_sum   <= 32'h0 ;
   adc_dec_cnt <= 17'h0 ;
   adc_dv      <=  1'b0 ;
end else begin
   if ((adc_dec_cnt >= set_dec) || adc_arm_do) begin // start again or arm
      adc_dec_cnt <= 17'h1                   ;
      adc_a_sum   <= $signed(adc_a_filt_out) ;
   end else begin
      adc_dec_cnt <= adc_dec_cnt + 17'h1 ;
      adc_a_sum   <= $signed(adc_a_sum) + $signed(adc_a_filt_out) ;
   end

   adc_dv <= (adc_dec_cnt >= set_dec) ;

   case (set_dec & {17{set_avg_en}})
      17'h0     : begin adc_a_dat <= adc_a_filt_out;        end
      17'h1     : begin adc_a_dat <= adc_a_sum[15+0 :  0];  end
      17'h8     : begin adc_a_dat <= adc_a_sum[15+3 :  3];  end
      17'h40    : begin adc_a_dat <= adc_a_sum[15+6 :  6];  end
      17'h400   : begin adc_a_dat <= adc_a_sum[15+10: 10];  end
      17'h2000  : begin adc_a_dat <= adc_a_sum[15+13: 13];  end
      17'h10000 : begin adc_a_dat <= adc_a_sum[15+16: 16];  end
      default   : begin adc_a_dat <= adc_a_sum[15+0 :  0];  end
   endcase
end

//---------------------------------------------------------------------------------
//
//  AXI CHA connection

reg  [ 32-1: 0] set_axi_start    ;
reg  [ 32-1: 0] set_axi_stop     ;
reg  [ 32-1: 0] set_axi_dly      ;
reg             set_axi_en       ;
reg  [ 32-1: 0] set_axi_trig     ;
reg  [ 32-1: 0] set_axi_cur      ;
reg             axi_a_we           ;
reg  [ 64-1: 0] axi_a_dat          ;
reg  [  2-1: 0] axi_a_dat_sel      ;
reg  [  1-1: 0] axi_a_dat_dv       ;
reg  [ 32-1: 0] axi_a_dly_cnt      ;
reg             axi_a_dly_do       ;
wire            axi_a_clr          ;
wire [ 32-1: 0] axi_a_cur_addr     ;

assign axi_a_clr = adc_rst_do ;


always @(posedge axi_clk_o) begin
   if (axi_rstn_o == 1'b0) begin
      axi_a_dat_sel <=  2'h0 ;
      axi_a_dat_dv  <=  1'b0 ;
      axi_a_dly_cnt <= 32'h0 ;
      axi_a_dly_do  <=  1'b0 ;
   end
   else begin
      if (adc_arm_do && set_axi_en)
         axi_a_we <= 1'b1 ;
      else if (((axi_a_dly_do || adc_trig) && (axi_a_dly_cnt == 32'h0)) || adc_rst_do) //delayed reached or reset
         axi_a_we <= 1'b0 ;

      if (adc_trig && axi_a_we)
         axi_a_dly_do  <= 1'b1 ;
      else if ((axi_a_dly_do && (axi_a_dly_cnt == 32'b0)) || axi_a_clr || adc_arm_do) //delayed reached or reset
         axi_a_dly_do  <= 1'b0 ;

      if (axi_a_dly_do && axi_a_we && adc_dv)
         axi_a_dly_cnt <= axi_a_dly_cnt - 1;
      else if (!axi_a_dly_do)
         axi_a_dly_cnt <= set_axi_dly ;

      if (axi_a_clr)
         axi_a_dat_sel <= 2'h0 ;
      else if (axi_a_we && adc_dv)
         axi_a_dat_sel <= axi_a_dat_sel + 2'h1 ;

      axi_a_dat_dv <= axi_a_we && (axi_a_dat_sel == 2'b11) && adc_dv ;
   end

   if (axi_a_we && adc_dv) begin
      if (axi_a_dat_sel == 2'b00) axi_a_dat[ 16-1:  0] <= $signed(adc_a_dat);
      if (axi_a_dat_sel == 2'b01) axi_a_dat[ 32-1: 16] <= $signed(adc_a_dat);
      if (axi_a_dat_sel == 2'b10) axi_a_dat[ 48-1: 32] <= $signed(adc_a_dat);
      if (axi_a_dat_sel == 2'b11) axi_a_dat[ 64-1: 48] <= $signed(adc_a_dat);
   end

   if (axi_a_clr)
      set_axi_trig <= {RSZ{1'b0}};
   else if (adc_trig && !axi_a_dly_do && axi_a_we)
      set_axi_trig <= {axi_a_cur_addr[32-1:3],axi_a_dat_sel,1'b0} ; // save write pointer at trigger arrival

   if (axi_a_clr)
      set_axi_cur <= set_axi_start ;
   else if (axi_wvalid_o)
      set_axi_cur <= axi_a_cur_addr ;
end

axi_wr_fifo #(
  .DW  (  64    ), // data width (8,16,...,1024)
  .AW  (  32    ), // address width
  .FW  (   8    )  // address width of FIFO pointers
) i_wr0 (
   // global signals
  .axi_clk_i          (  axi_clk_o        ), // global clock
  .axi_rstn_i         (  axi_rstn_o       ), // global reset

   // Connection to AXI master
  .axi_waddr_o        (  axi_waddr_o      ), // write address
  .axi_wdata_o        (  axi_wdata_o      ), // write data
  .axi_wsel_o         (  axi_wsel_o       ), // write byte select
  .axi_wvalid_o       (  axi_wvalid_o     ), // write data valid
  .axi_wlen_o         (  axi_wlen_o       ), // write burst length
  .axi_wfixed_o       (  axi_wfixed_o     ), // write burst type (fixed / incremental)
  .axi_werr_i         (  axi_werr_i       ), // write error
  .axi_wrdy_i         (  axi_wrdy_i       ), // write ready

   // data and configuration
  .wr_data_i          (  axi_a_dat         ), // write data
  .wr_val_i           (  axi_a_dat_dv      ), // write data valid
  .ctrl_start_addr_i  (  set_axi_start   ), // range start address
  .ctrl_stop_addr_i   (  set_axi_stop    ), // range stop address
  .ctrl_trg_size_i   (  4'hF              ), // trigger level
  .ctrl_wrap_i        (  1'b1              ), // start from begining when reached stop
  .ctrl_clr_i         (  axi_a_clr         ), // clear / flush
  .stat_overflow_o    (                    ), // overflow indicator
  .stat_cur_addr_o    (  axi_a_cur_addr    ), // current write address
  .stat_write_data_o  (                    )  // write data indicator
);

assign axi_clk_o  = clk ;
assign axi_rstn_o = rstn;

//---------------------------------------------------------------------------------
//  Trigger source selector

reg               adc_trg_ap      ;
reg               adc_trg_an      ;
reg               adc_trg_bp      ;
reg               adc_trg_bn      ;
reg               adc_trg_sw      ;
reg   [   4-1: 0] set_trg_src     ;
wire              ext_trg_p       ;
wire              ext_trg_n       ;
wire              asg_trg_p       ;
wire              asg_trg_n       ;

always @(posedge clk)
if (rstn == 1'b0) begin
   adc_arm_do    <= 1'b0 ;
   adc_rst_do    <= 1'b0 ;
   adc_trg_sw   <= 1'b0 ;
   set_trg_src  <= 4'h0 ;
   adc_trig      <= 1'b0 ;
end else begin
   adc_arm_do  <= sys_wen && (sys_addr[19:0]==20'h0) && sys_wdata[0] ; // SW ARM
   adc_rst_do  <= sys_wen && (sys_addr[19:0]==20'h0) && sys_wdata[1] ;
   adc_trg_sw <= sys_wen && (sys_addr[19:0]==20'h4) && (sys_wdata[3:0]==4'h1); // SW trigger

      if (sys_wen && (sys_addr[19:0]==20'h4))
         set_trg_src <= sys_wdata[3:0] ;
      else if (((adc_dly_do || adc_trig) && (adc_dly_cnt == 32'h0)) || adc_rst_do) //delayed reached or reset
         set_trg_src <= 4'h0 ;

   case (set_trg_src)
       4'd1 : adc_trig <= adc_trg_sw   ; // manual
       4'd2 : adc_trig <= adc_trg_ap   ; // A ch rising edge
       4'd3 : adc_trig <= adc_trg_an   ; // A ch falling edge
       4'd4 : adc_trig <= adc_trg_bp   ; // B ch rising edge
       4'd5 : adc_trig <= adc_trg_bn   ; // B ch falling edge
       4'd6 : adc_trig <= ext_trg_p    ; // external - rising edge
       4'd7 : adc_trig <= ext_trg_n    ; // external - falling edge
       4'd8 : adc_trig <= asg_trg_p    ; // ASG - rising edge
       4'd9 : adc_trig <= asg_trg_n    ; // ASG - falling edge
    default : adc_trig <= 1'b0          ;
   endcase
end

//---------------------------------------------------------------------------------
//  System bus connection

always @(posedge clk)
if (rstn == 1'b0) begin
   adc_we_keep <=   1'b0      ;
   set_tresh   <=  14'd5000   ;
   set_dly     <=  32'd0      ;
   set_dec     <=  17'd1      ;
   set_hyst    <=  14'd20     ;
   set_avg_en  <=   1'b1      ;
   set_filt_aa <=  18'h0      ;
   set_filt_bb <=  25'h0      ;
   set_filt_kk <=  25'hFFFFFF ;
   set_filt_pp <=  25'h0      ;
   set_deb_len <=  20'd62500  ;
   set_axi_en  <=   1'b0      ;
end else begin
   if (sys_wen) begin
      if (sys_addr[19:0]==20'h00)   adc_we_keep   <= sys_wdata[     3] ;

      if (sys_addr[19:0]==20'h08)   set_tresh   <= sys_wdata[14-1:0] ;
      if (sys_addr[19:0]==20'h0C)   set_b_tresh   <= sys_wdata[14-1:0] ;
      if (sys_addr[19:0]==20'h10)   set_dly       <= sys_wdata[32-1:0] ;
      if (sys_addr[19:0]==20'h14)   set_dec       <= sys_wdata[17-1:0] ;
      if (sys_addr[19:0]==20'h20)   set_hyst    <= sys_wdata[14-1:0] ;
      if (sys_addr[19:0]==20'h28)   set_avg_en    <= sys_wdata[     0] ;

      if (sys_addr[19:0]==20'h30)   set_filt_aa <= sys_wdata[18-1:0] ;
      if (sys_addr[19:0]==20'h34)   set_filt_bb <= sys_wdata[25-1:0] ;
      if (sys_addr[19:0]==20'h38)   set_filt_kk <= sys_wdata[25-1:0] ;
      if (sys_addr[19:0]==20'h3C)   set_filt_pp <= sys_wdata[25-1:0] ;

      if (sys_addr[19:0]==20'h50)   set_axi_start <= sys_wdata[32-1:0] ;
      if (sys_addr[19:0]==20'h54)   set_axi_stop  <= sys_wdata[32-1:0] ;
      if (sys_addr[19:0]==20'h58)   set_axi_dly   <= sys_wdata[32-1:0] ;
      if (sys_addr[19:0]==20'h5C)   set_axi_en    <= sys_wdata[     0] ;

      if (sys_addr[19:0]==20'h90)   set_deb_len <= sys_wdata[20-1:0] ;
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
     20'h00000 : begin sys_ack <= sys_en;          sys_rdata <= {{32- 4{1'b0}}, adc_we_keep               // do not disarm on 
                                                                              , adc_dly_do                // trigger status
                                                                              , 1'b0                      // reset
                                                                              , adc_we}             ; end // arm

     20'h00004 : begin sys_ack <= sys_en;          sys_rdata <= {{32- 4{1'b0}}, set_trg_src}       ; end 

     20'h00008 : begin sys_ack <= sys_en;          sys_rdata <= {{32-14{1'b0}}, set_tresh}        ; end
     20'h00010 : begin sys_ack <= sys_en;          sys_rdata <= {               set_dly}            ; end
     20'h00014 : begin sys_ack <= sys_en;          sys_rdata <= {{32-17{1'b0}}, set_dec}            ; end

     20'h00018 : begin sys_ack <= sys_en;          sys_rdata <= {{32-RSZ{1'b0}}, adc_wp_cur}        ; end
     20'h0001C : begin sys_ack <= sys_en;          sys_rdata <= {{32-RSZ{1'b0}}, adc_wp_trig}       ; end

     20'h00020 : begin sys_ack <= sys_en;          sys_rdata <= {{32-14{1'b0}}, set_hyst}         ; end

     20'h00028 : begin sys_ack <= sys_en;          sys_rdata <= {{32- 1{1'b0}}, set_avg_en}         ; end

     20'h0002C : begin sys_ack <= sys_en;          sys_rdata <=                 adc_we_cnt          ; end

     20'h00030 : begin sys_ack <= sys_en;          sys_rdata <= {{32-18{1'b0}}, set_filt_aa}      ; end
     20'h00034 : begin sys_ack <= sys_en;          sys_rdata <= {{32-25{1'b0}}, set_filt_bb}      ; end
     20'h00038 : begin sys_ack <= sys_en;          sys_rdata <= {{32-25{1'b0}}, set_filt_kk}      ; end
     20'h0003C : begin sys_ack <= sys_en;          sys_rdata <= {{32-25{1'b0}}, set_filt_pp}      ; end

     20'h00050 : begin sys_ack <= sys_en;          sys_rdata <=                 set_axi_start     ; end
     20'h00054 : begin sys_ack <= sys_en;          sys_rdata <=                 set_axi_stop      ; end
     20'h00058 : begin sys_ack <= sys_en;          sys_rdata <=                 set_axi_dly       ; end
     20'h0005C : begin sys_ack <= sys_en;          sys_rdata <= {{32- 1{1'b0}}, set_axi_en}       ; end
     20'h00060 : begin sys_ack <= sys_en;          sys_rdata <=                 set_axi_trig      ; end
     20'h00064 : begin sys_ack <= sys_en;          sys_rdata <=                 set_axi_cur       ; end

     20'h00090 : begin sys_ack <= sys_en;          sys_rdata <= {{32-20{1'b0}}, set_deb_len}        ; end

     20'h1???? : begin sys_ack <= adc_rd_dv;       sys_rdata <= {16'h0, 2'h0,adc_a_rd}              ; end

       default : begin sys_ack <= sys_en;          sys_rdata <=  32'h0                              ; end
   endcase
end

endmodule: scope_top
