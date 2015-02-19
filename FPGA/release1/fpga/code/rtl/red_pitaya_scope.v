/**
 * $Id: red_pitaya_scope.v 965 2014-01-24 13:39:56Z matej.oblak $
 *
 * @brief Red Pitaya oscilloscope application, used for capturing ADC data
 *        into BRAMs, which can be later read by SW.
 *
 * @Author Matej Oblak
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in Verilog hardware description language (HDL).
 * Please visit http://en.wikipedia.org/wiki/Verilog
 * for more details on the language used herein.
 */

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
 *                                                  ˇ         ^
 *                                              /------\      |
 *   ext trigger -----------------------------> | TRIG | -----+
 *                                              \------/      |
 *                                                  ^         ˇ
 *                /--------\      /-----------\     |      /-----\
 *   ADC CHB ---> | DFILT1 | ---> | AVG & DEC | ---------> | BUF | --->  SW
 *                \--------/      \-----------/            \-----/ 
 *
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

module red_pitaya_scope #(
  parameter RSZ = 14  // RAM size 2^RSZ
)(
   // ADC
   input                 adc_clk_i       ,  // ADC clock
   input                 adc_rstn_i      ,  // ADC reset - active low
   input      [ 14-1: 0] adc_a_i         ,  // ADC data CHA
   input      [ 14-1: 0] adc_b_i         ,  // ADC data CHB
   // trigger sources
   input                 trig_ext_i      ,  // external trigger
   input                 trig_asg_i      ,  // ASG trigger

   // AXI0 master
   output                axi0_clk_o      ,  // global clock
   output                axi0_rstn_o     ,  // global reset
   output     [ 32-1: 0] axi0_waddr_o    ,  // system write address
   output     [ 64-1: 0] axi0_wdata_o    ,  // system write data
   output     [  8-1: 0] axi0_wsel_o     ,  // system write byte select
   output                axi0_wvalid_o   ,  // system write data valid
   output     [  4-1: 0] axi0_wlen_o     ,  // system write burst length
   output                axi0_wfixed_o   ,  // system write burst type (fixed / incremental)
   input                 axi0_werr_i     ,  // system write error
   input                 axi0_wrdy_i     ,  // system write ready
   input                 axi0_rstn_i     ,  // reset from PS

   // AXI1 master
   output                axi1_clk_o      ,  // global clock
   output                axi1_rstn_o     ,  // global reset
   output     [ 32-1: 0] axi1_waddr_o    ,  // system write address
   output     [ 64-1: 0] axi1_wdata_o    ,  // system write data
   output     [  8-1: 0] axi1_wsel_o     ,  // system write byte select
   output                axi1_wvalid_o   ,  // system write data valid
   output     [  4-1: 0] axi1_wlen_o     ,  // system write burst length
   output                axi1_wfixed_o   ,  // system write burst type (fixed / incremental)
   input                 axi1_werr_i     ,  // system write error
   input                 axi1_wrdy_i     ,  // system write ready
   input                 axi1_rstn_i     ,  // reset from PS
  
   // System bus
   input                 sys_clk_i       ,  // bus clock
   input                 sys_rstn_i      ,  // bus reset - active low
   input      [ 32-1: 0] sys_addr_i      ,  // bus saddress
   input      [ 32-1: 0] sys_wdata_i     ,  // bus write data
   input      [  4-1: 0] sys_sel_i       ,  // bus write byte select
   input                 sys_wen_i       ,  // bus write enable
   input                 sys_ren_i       ,  // bus read enable
   output     [ 32-1: 0] sys_rdata_o     ,  // bus read data
   output                sys_err_o       ,  // bus error indicator
   output                sys_ack_o          // bus acknowledge signal
);

wire [ 32-1: 0] addr         ;
wire [ 32-1: 0] wdata        ;
wire            wen          ;
wire            ren          ;
reg  [ 32-1: 0] rdata        ;
reg             err          ;
reg             ack          ;
reg             adc_arm_do   ;
reg             adc_rst_do   ;

//---------------------------------------------------------------------------------
//  Input filtering

wire [ 14-1: 0] adc_a_filt_in  ;
wire [ 14-1: 0] adc_a_filt_out ;
wire [ 14-1: 0] adc_b_filt_in  ;
wire [ 14-1: 0] adc_b_filt_out ;
reg  [ 18-1: 0] set_a_filt_aa  ;
reg  [ 25-1: 0] set_a_filt_bb  ;
reg  [ 25-1: 0] set_a_filt_kk  ;
reg  [ 25-1: 0] set_a_filt_pp  ;
reg  [ 18-1: 0] set_b_filt_aa  ;
reg  [ 25-1: 0] set_b_filt_bb  ;
reg  [ 25-1: 0] set_b_filt_kk  ;
reg  [ 25-1: 0] set_b_filt_pp  ;

assign adc_a_filt_in = adc_a_i ;
assign adc_b_filt_in = adc_b_i ;

red_pitaya_dfilt1 i_dfilt1_cha (
   // ADC
  .adc_clk_i   ( adc_clk_i       ),  // ADC clock
  .adc_rstn_i  ( adc_rstn_i      ),  // ADC reset - active low
  .adc_dat_i   ( adc_a_filt_in   ),  // ADC data
  .adc_dat_o   ( adc_a_filt_out  ),  // ADC data
   // configuration
  .cfg_aa_i    ( set_a_filt_aa   ),  // config AA coefficient
  .cfg_bb_i    ( set_a_filt_bb   ),  // config BB coefficient
  .cfg_kk_i    ( set_a_filt_kk   ),  // config KK coefficient
  .cfg_pp_i    ( set_a_filt_pp   )   // config PP coefficient
);

red_pitaya_dfilt1 i_dfilt1_chb (
   // ADC
  .adc_clk_i   ( adc_clk_i       ),  // ADC clock
  .adc_rstn_i  ( adc_rstn_i      ),  // ADC reset - active low
  .adc_dat_i   ( adc_b_filt_in   ),  // ADC data
  .adc_dat_o   ( adc_b_filt_out  ),  // ADC data
   // configuration
  .cfg_aa_i    ( set_b_filt_aa   ),  // config AA coefficient
  .cfg_bb_i    ( set_b_filt_bb   ),  // config BB coefficient
  .cfg_kk_i    ( set_b_filt_kk   ),  // config KK coefficient
  .cfg_pp_i    ( set_b_filt_pp   )   // config PP coefficient
);

//---------------------------------------------------------------------------------
//  Decimate input data

reg  [ 14-1: 0] adc_a_dat     ;
reg  [ 14-1: 0] adc_b_dat     ;
reg  [ 32-1: 0] adc_a_sum     ;
reg  [ 32-1: 0] adc_b_sum     ;
reg  [ 17-1: 0] set_dec       ;
reg  [ 17-1: 0] adc_dec_cnt   ;
reg             set_avg_en    ;
reg             adc_dv        ;

always @(posedge adc_clk_i)
if (adc_rstn_i == 1'b0) begin
   adc_a_sum   <= 32'h0 ;
   adc_b_sum   <= 32'h0 ;
   adc_dec_cnt <= 17'h0 ;
   adc_dv      <=  1'b0 ;
end else begin
   if ((adc_dec_cnt >= set_dec) || adc_arm_do) begin // start again or arm
      adc_dec_cnt <= 17'h1                   ;
      adc_a_sum   <= $signed(adc_a_filt_out) ;
      adc_b_sum   <= $signed(adc_b_filt_out) ;
   end else begin
      adc_dec_cnt <= adc_dec_cnt + 17'h1 ;
      adc_a_sum   <= $signed(adc_a_sum) + $signed(adc_a_filt_out) ;
      adc_b_sum   <= $signed(adc_b_sum) + $signed(adc_b_filt_out) ;
   end

   adc_dv <= (adc_dec_cnt >= set_dec) ;

   case (set_dec & {17{set_avg_en}})
      17'h0     : begin adc_a_dat <= adc_a_filt_out;            adc_b_dat <= adc_b_filt_out;        end
      17'h1     : begin adc_a_dat <= adc_a_sum[15+0 :  0];      adc_b_dat <= adc_b_sum[15+0 :  0];  end
      17'h8     : begin adc_a_dat <= adc_a_sum[15+3 :  3];      adc_b_dat <= adc_b_sum[15+3 :  3];  end
      17'h40    : begin adc_a_dat <= adc_a_sum[15+6 :  6];      adc_b_dat <= adc_b_sum[15+6 :  6];  end
      17'h400   : begin adc_a_dat <= adc_a_sum[15+10: 10];      adc_b_dat <= adc_b_sum[15+10: 10];  end
      17'h2000  : begin adc_a_dat <= adc_a_sum[15+13: 13];      adc_b_dat <= adc_b_sum[15+13: 13];  end
      17'h10000 : begin adc_a_dat <= adc_a_sum[15+16: 16];      adc_b_dat <= adc_b_sum[15+16: 16];  end
      default   : begin adc_a_dat <= adc_a_sum[15+0 :  0];      adc_b_dat <= adc_b_sum[15+0 :  0];  end
   endcase
end

//---------------------------------------------------------------------------------
//  ADC buffer RAM

//reg   [  14-1: 0] adc_a_buf [0:2**RSZ-1] ;
//reg   [  14-1: 0] adc_b_buf [0:2**RSZ-1] ;
reg   [  14-1: 0] adc_a_rd      ;
reg   [  14-1: 0] adc_b_rd      ;
reg   [ RSZ-1: 0] adc_wp        ;
reg   [ RSZ-1: 0] adc_raddr     ;
reg   [ RSZ-1: 0] adc_a_raddr   ;
reg   [ RSZ-1: 0] adc_b_raddr   ;
reg   [   4-1: 0] adc_rval      ;
wire              adc_rd_dv     ;
reg               adc_we        ;
reg               adc_trig      ;

reg   [ RSZ-1: 0] adc_wp_trig   ;
reg   [ RSZ-1: 0] adc_wp_cur    ;
reg   [  32-1: 0] set_dly       ;
reg   [  32-1: 0] adc_dly_cnt   ;
reg               adc_dly_do    ;

reg    [ 20-1: 0] set_deb_len; // debouncing length (glitch free time after a posedge)
reg               set_acu_ena; // accumulation enable
reg    [ 32-1: 0] set_acu_cnt; // accumulation counter length
wire   [ 32-1: 0] set_sts_cnt; // accumulation counter current status
reg    [  5-1: 0] set_acu_shf; // accumulation output shift
reg    [ 14-1: 0] set_acu_len; // accumulation sequence length

// Write
always @(posedge adc_clk_i)
if (adc_rstn_i == 1'b0) begin
   adc_wp      <= {RSZ{1'b0}};
   adc_we      <=  1'b0      ;
   adc_wp_trig <= {RSZ{1'b0}};
   adc_wp_cur  <= {RSZ{1'b0}};
   adc_dly_cnt <= 32'h0      ;
   adc_dly_do  <=  1'b0      ;
end
else if (~set_acu_ena) begin
   if (adc_arm_do)
      adc_we <= 1'b1 ;
   else if (((adc_dly_do || adc_trig) && (adc_dly_cnt == 32'h0)) || adc_rst_do) //delayed reached or reset
      adc_we <= 1'b0 ;

   if (adc_rst_do)
      adc_wp <= {RSZ{1'b0}};
   else if (adc_we && adc_dv)
      adc_wp <= adc_wp + 1'b1 ;

   if (adc_rst_do)
      adc_wp_trig <= {RSZ{1'b0}};
   else if (adc_trig && !adc_dly_do)
      adc_wp_trig <= adc_wp_cur ; // save write pointer at trigger arrival

   if (adc_rst_do)
      adc_wp_cur <= {RSZ{1'b0}};
   else if (adc_we && adc_dv)
      adc_wp_cur <= adc_wp ; // save current write pointer

   if (adc_trig)
      adc_dly_do  <= 1'b1 ;
   else if ((adc_dly_do && (adc_dly_cnt == 32'b0)) || adc_rst_do || adc_arm_do) //delayed reached or reset
      adc_dly_do  <= 1'b0 ;

   if (adc_dly_do && adc_we && adc_dv)
      adc_dly_cnt <= adc_dly_cnt + {32{1'b1}} ; // -1
   else if (!adc_dly_do)
      adc_dly_cnt <= set_dly ;
end

//always @(posedge adc_clk_i)
//if (adc_we && adc_dv) begin
//   adc_a_buf[adc_wp] <= adc_a_dat ;
//   adc_b_buf[adc_wp] <= adc_b_dat ;
//end

// Read
always @(posedge adc_clk_i)
if (adc_rstn_i == 1'b0)
   adc_rval <= 4'h0 ;
else
   adc_rval <= {adc_rval[2:0], (ren || wen)};

assign adc_rd_dv = adc_rval[3];

always @(posedge adc_clk_i)
begin
   adc_raddr   <= addr[RSZ+1:2] ; // address synchronous to clock
   adc_a_raddr <= adc_raddr     ; // double register 
   adc_b_raddr <= adc_raddr     ; // otherwise memory corruption at reading
   adc_a_rd    <= 0; // adc_a_buf[adc_a_raddr] ;
   adc_b_rd    <= 0; // adc_b_buf[adc_b_raddr] ;
end




//---------------------------------------------------------------------------------
//
//  AXI CHA connection

reg  [ 32-1: 0] set_a_axi_start    ;
reg  [ 32-1: 0] set_a_axi_stop     ;
reg  [ 32-1: 0] set_a_axi_dly      ;
reg             set_a_axi_en       ;
reg  [ 32-1: 0] set_a_axi_trig     ;
reg  [ 32-1: 0] set_a_axi_cur      ;
reg             axi_a_we           ;
reg  [ 64-1: 0] axi_a_dat          ;
reg  [  2-1: 0] axi_a_dat_sel      ;
reg  [  1-1: 0] axi_a_dat_dv       ;
reg  [ 32-1: 0] axi_a_dly_cnt      ;
reg             axi_a_dly_do       ;
wire            axi_a_clr          ;
wire [ 32-1: 0] axi_a_cur_addr     ;

assign axi_a_clr = adc_rst_do ;


always @(posedge axi0_clk_o) begin
   if (axi0_rstn_o == 1'b0) begin
      axi_a_dat_sel <=  2'h0 ;
      axi_a_dat_dv  <=  1'b0 ;
      axi_a_dly_cnt <= 32'h0 ;
      axi_a_dly_do  <=  1'b0 ;
   end
   else begin
      if (adc_arm_do && set_a_axi_en)
         axi_a_we <= 1'b1 ;
      else if (((axi_a_dly_do || adc_trig) && (axi_a_dly_cnt == 32'h0)) || adc_rst_do) //delayed reached or reset
         axi_a_we <= 1'b0 ;

      if (adc_trig && axi_a_we)
         axi_a_dly_do  <= 1'b1 ;
      else if ((axi_a_dly_do && (axi_a_dly_cnt == 32'b0)) || axi_a_clr || adc_arm_do) //delayed reached or reset
         axi_a_dly_do  <= 1'b0 ;

      if (adc_dly_do && axi_a_we && adc_dv)
         axi_a_dly_cnt <= axi_a_dly_cnt + {32{1'b1}} ; // -1
      else if (!axi_a_dly_do)
         axi_a_dly_cnt <= set_a_axi_dly ;

      if (axi_a_clr)
         axi_a_dat_sel <= 2'h0 ;
      else if (axi_a_we && adc_dv)
         axi_a_dat_sel <= axi_a_dat_sel + 2'h1 ;

      axi_a_dat_dv <= axi_a_we && (axi_a_dat_sel == 2'b11) && adc_dv ;
   end

   if (axi_a_we && adc_dv) begin
      if (axi_a_dat_sel == 2'b00) axi_a_dat[ 16-1:  0] <= {2'h0,adc_a_dat};
      if (axi_a_dat_sel == 2'b01) axi_a_dat[ 32-1: 16] <= {2'h0,adc_a_dat};
      if (axi_a_dat_sel == 2'b10) axi_a_dat[ 48-1: 32] <= {2'h0,adc_a_dat};
      if (axi_a_dat_sel == 2'b11) axi_a_dat[ 64-1: 48] <= {2'h0,adc_a_dat};
   end

   if (axi_a_clr)
      set_a_axi_trig <= {RSZ{1'b0}};
   else if (adc_trig && !axi_a_dly_do && axi_a_we)
      set_a_axi_trig <= {axi_a_cur_addr[32-1:3],axi_a_dat_sel,1'b0} ; // save write pointer at trigger arrival

   if (axi_a_clr)
      set_a_axi_cur <= set_a_axi_start ;
   else if (axi0_wvalid_o)
      set_a_axi_cur <= axi_a_cur_addr ;
end

axi_wr_fifo #(
  .DW  (  64    ), // data width (8,16,...,1024)
  .AW  (  32    ), // address width
  .FW  (   8    )  // address width of FIFO pointers
)
i_wr0
(
   // global signals
  .axi_clk_i          (  axi0_clk_o        ), // global clock
  .axi_rstn_i         (  axi0_rstn_o       ), // global reset

   // Connection to AXI master
  .axi_waddr_o        (  axi0_waddr_o      ), // write address
  .axi_wdata_o        (  axi0_wdata_o      ), // write data
  .axi_wsel_o         (  axi0_wsel_o       ), // write byte select
  .axi_wvalid_o       (  axi0_wvalid_o     ), // write data valid
  .axi_wlen_o         (  axi0_wlen_o       ), // write burst length
  .axi_wfixed_o       (  axi0_wfixed_o     ), // write burst type (fixed / incremental)
  .axi_werr_i         (  axi0_werr_i       ), // write error
  .axi_wrdy_i         (  axi0_wrdy_i       ), // write ready

   // data and configuration
  .wr_data_i          (  axi_a_dat         ), // write data
  .wr_val_i           (  axi_a_dat_dv      ), // write data valid
  .ctrl_start_addr_i  (  set_a_axi_start   ), // range start address
  .ctrl_stop_addr_i   (  set_a_axi_stop    ), // range stop address
  .ctrl_trig_size_i   (  4'hF              ), // trigger level
  .ctrl_wrap_i        (  1'b1              ), // start from begining when reached stop
  .ctrl_clr_i         (  axi_a_clr         ), // clear / flush
  .stat_overflow_o    (                    ), // overflow indicator
  .stat_cur_addr_o    (  axi_a_cur_addr    ), // current write address
  .stat_write_data_o  (                    )  // write data indicator
);

assign axi0_clk_o  = adc_clk_i ;
assign axi0_rstn_o = (adc_rstn_i != 1'b0) && axi0_rstn_i ;









//---------------------------------------------------------------------------------
//
//  AXI CHB connection

reg  [ 32-1: 0] set_b_axi_start    ;
reg  [ 32-1: 0] set_b_axi_stop     ;
reg  [ 32-1: 0] set_b_axi_dly      ;
reg             set_b_axi_en       ;
reg  [ 32-1: 0] set_b_axi_trig     ;
reg  [ 32-1: 0] set_b_axi_cur      ;
reg             axi_b_we           ;
reg  [ 64-1: 0] axi_b_dat          ;
reg  [  2-1: 0] axi_b_dat_sel      ;
reg  [  1-1: 0] axi_b_dat_dv       ;
reg  [ 32-1: 0] axi_b_dly_cnt      ;
reg             axi_b_dly_do       ;
wire            axi_b_clr          ;
wire [ 32-1: 0] axi_b_cur_addr     ;

assign axi_b_clr = adc_rst_do ;


always @(posedge axi1_clk_o) begin
   if (axi1_rstn_o == 1'b0) begin
      axi_b_dat_sel <=  2'h0 ;
      axi_b_dat_dv  <=  1'b0 ;
      axi_b_dly_cnt <= 32'h0 ;
      axi_b_dly_do  <=  1'b0 ;
   end
   else begin
      if (adc_arm_do && set_b_axi_en)
         axi_b_we <= 1'b1 ;
      else if (((axi_b_dly_do || adc_trig) && (axi_b_dly_cnt == 32'h0)) || adc_rst_do) //delayed reached or reset
         axi_b_we <= 1'b0 ;

      if (adc_trig && axi_b_we)
         axi_b_dly_do  <= 1'b1 ;
      else if ((axi_b_dly_do && (axi_b_dly_cnt == 32'b0)) || axi_b_clr || adc_arm_do) //delayed reached or reset
         axi_b_dly_do  <= 1'b0 ;

      if (adc_dly_do && axi_b_we && adc_dv)
         axi_b_dly_cnt <= axi_b_dly_cnt + {32{1'b1}} ; // -1
      else if (!axi_b_dly_do)
         axi_b_dly_cnt <= set_b_axi_dly ;

      if (axi_b_clr)
         axi_b_dat_sel <= 2'h0 ;
      else if (axi_b_we && adc_dv)
         axi_b_dat_sel <= axi_b_dat_sel + 2'h1 ;

      axi_b_dat_dv <= axi_b_we && (axi_b_dat_sel == 2'b11) && adc_dv ;
   end

   if (axi_b_we && adc_dv) begin
      if (axi_b_dat_sel == 2'b00) axi_b_dat[ 16-1:  0] <= {2'h0,adc_b_dat};
      if (axi_b_dat_sel == 2'b01) axi_b_dat[ 32-1: 16] <= {2'h0,adc_b_dat};
      if (axi_b_dat_sel == 2'b10) axi_b_dat[ 48-1: 32] <= {2'h0,adc_b_dat};
      if (axi_b_dat_sel == 2'b11) axi_b_dat[ 64-1: 48] <= {2'h0,adc_b_dat};
   end

   if (axi_b_clr)
      set_b_axi_trig <= {RSZ{1'b0}};
   else if (adc_trig && !axi_b_dly_do && axi_b_we)
      set_b_axi_trig <= {axi_b_cur_addr[32-1:3],axi_b_dat_sel,1'b0} ; // save write pointer at trigger arrival

   if (axi_b_clr)
      set_b_axi_cur <= set_b_axi_start ;
   else if (axi1_wvalid_o)
      set_b_axi_cur <= axi_b_cur_addr ;
end

axi_wr_fifo #(
  .DW  (  64    ), // data width (8,16,...,1024)
  .AW  (  32    ), // address width
  .FW  (   8    )  // address width of FIFO pointers
)
i_wr1
(
   // global signals
  .axi_clk_i          (  axi1_clk_o        ), // global clock
  .axi_rstn_i         (  axi1_rstn_o       ), // global reset

   // Connection to AXI master
  .axi_waddr_o        (  axi1_waddr_o      ), // write address
  .axi_wdata_o        (  axi1_wdata_o      ), // write data
  .axi_wsel_o         (  axi1_wsel_o       ), // write byte select
  .axi_wvalid_o       (  axi1_wvalid_o     ), // write data valid
  .axi_wlen_o         (  axi1_wlen_o       ), // write burst length
  .axi_wfixed_o       (  axi1_wfixed_o     ), // write burst type (fixed / incremental)
  .axi_werr_i         (  axi1_werr_i       ), // write error
  .axi_wrdy_i         (  axi1_wrdy_i       ), // write ready

   // data and configuration
  .wr_data_i          (  axi_b_dat         ), // write data
  .wr_val_i           (  axi_b_dat_dv      ), // write data valid
  .ctrl_start_addr_i  (  set_b_axi_start   ), // range start address
  .ctrl_stop_addr_i   (  set_b_axi_stop    ), // range stop address
  .ctrl_trig_size_i   (  4'hF              ), // trigger level
  .ctrl_wrap_i        (  1'b1              ), // start from begining when reached stop
  .ctrl_clr_i         (  axi_b_clr         ), // clear / flush
  .stat_overflow_o    (                    ), // overflow indicator
  .stat_cur_addr_o    (  axi_b_cur_addr    ), // current write address
  .stat_write_data_o  (                    )  // write data indicator
);

assign axi1_clk_o  = adc_clk_i ;
assign axi1_rstn_o = (adc_rstn_i != 1'b0) && axi1_rstn_i ;














//---------------------------------------------------------------------------------
// averaging accumulator module instances

reg            acu_ctl_run;
wire           acu_sts_end;

reg  [RSZ-1:0] acu_len_cnt;
wire           acu_len_end;
wire           acu_valid;

reg            acu_rd_dv;

wire           acu_a_mem_ren, acu_b_mem_ren;
wire [RSZ-1:0] acu_a_mem_adr, acu_b_mem_adr;
wire  [46-1:0] acu_a_mem_tmp, acu_b_mem_tmp; // 14+32 = 46
wire  [32-1:0] acu_a_mem_rdt, acu_b_mem_rdt;

// run control signal is triggered by a write into the arm register
always @(posedge adc_clk_i)
if (adc_rstn_i == 1'b0)  acu_ctl_run <= 1'b0;
else if (set_acu_ena) begin
  if (wen & (addr[19:0]==20'h94) & wdata[1])  acu_ctl_run <= 1'b1;
  else if (acu_sts_end)                       acu_ctl_run <= 1'b0;
end

assign acu_valid = acu_ctl_run & (adc_trig | (|acu_len_cnt));

always @(posedge adc_clk_i)
if (adc_rstn_i == 1'b0)  acu_len_cnt <= 0;
else if (acu_valid)      acu_len_cnt <= acu_len_end ? 0 : acu_len_cnt + 1;

assign acu_len_end = acu_len_cnt == set_acu_len;

// memory read data valid
always @(posedge adc_clk_i)
if (adc_rstn_i == 1'b0)  acu_rd_dv <= 1'b0;
else                     acu_rd_dv <= acu_a_mem_ren | acu_b_mem_ren;

red_pitaya_acum acum_a (
  // system signals
  .clk        (adc_clk_i),
  .rst        (~adc_rstn_i),
  .clr        (adc_rst_do),
  // configuration
  .cfg_cnt    (set_acu_cnt),
  // control/status
  .ctl_run    (acu_ctl_run),
  .sts_end    (acu_sts_end),
  .sts_cnt    (set_sts_cnt),
  // input data stream
  .sti_tlast  (acu_len_end),
  .sti_tdata  (adc_a_dat),
  .sti_tvalid (acu_valid),
  .sti_tready (),
  // output data stream
  .sto_tlast  (),
  .sto_tdata  (),
  .sto_tvalid (),
  .sto_tready (1'b1),
  // memmory interface (read only)
  .bus_ren    (acu_a_mem_ren),
  .bus_adr    (acu_a_mem_adr),
  .bus_rdt    (acu_a_mem_tmp)
);

assign acu_a_mem_ren = ren & (addr[17:16] == 2'h3);
assign acu_a_mem_adr =        addr[RSZ+1:2];
assign acu_a_mem_rdt = acu_a_mem_tmp << set_acu_shf;

red_pitaya_acum acum_b (
  // system signals
  .clk        (adc_clk_i),
  .rst        (~adc_rstn_i),
  .clr        (adc_rst_do),
  // configuration
  .cfg_cnt    (set_acu_cnt),
  // control/status
  .ctl_run    (acu_ctl_run),
  .sts_end    (           ),
  .sts_cnt    (           ),
  // input data stream
  .sti_tlast  (acu_len_end),
  .sti_tdata  (adc_b_dat),
  .sti_tvalid (acu_valid),
  .sti_tready (),
  // output data stream
  .sto_tlast  (),
  .sto_tdata  (),
  .sto_tvalid (),
  .sto_tready (1'b1),
  // memmory interface (read only)
  .bus_ren    (acu_b_mem_ren),
  .bus_adr    (acu_b_mem_adr),
  .bus_rdt    (acu_b_mem_tmp)
);

assign acu_b_mem_ren = ren & (addr[17:16] == 2'h3);
assign acu_b_mem_adr =        addr[RSZ+1:2];
assign acu_b_mem_rdt = acu_b_mem_tmp << set_acu_shf;

//---------------------------------------------------------------------------------
//  Trigger source selector

reg               adc_trig_ap      ;
reg               adc_trig_an      ;
reg               adc_trig_bp      ;
reg               adc_trig_bn      ;
reg               adc_trig_sw      ;
reg   [   4-1: 0] set_trig_src     ;
wire              ext_trig_p       ;
wire              ext_trig_n       ;
wire              asg_trig_p       ;
wire              asg_trig_n       ;

always @(posedge adc_clk_i)
if (adc_rstn_i == 1'b0) begin
   adc_arm_do    <= 1'b0 ;
   adc_rst_do    <= 1'b0 ;
   adc_trig_sw   <= 1'b0 ;
   set_trig_src  <= 4'h0 ;
   adc_trig      <= 1'b0 ;
end else begin
   adc_arm_do  <= wen && (addr[19:0]==20'h0) && wdata[0] ; // SW ARM
   adc_rst_do  <= wen && (addr[19:0]==20'h0) && wdata[1] ;
   adc_trig_sw <= wen && (addr[19:0]==20'h4) ; // SW trigger

   if (wen && (addr[19:0]==20'h4))
      set_trig_src <= wdata[3:0] ;
   else if (((adc_dly_do || adc_trig) && (adc_dly_cnt == 32'h0)) || adc_rst_do) //delayed reached or reset
      set_trig_src <= 4'h0 ;

   case (set_trig_src)
       4'd1 : adc_trig <= adc_trig_sw   ; // manual
       4'd2 : adc_trig <= adc_trig_ap   ; // A ch rising edge
       4'd3 : adc_trig <= adc_trig_an   ; // A ch falling edge
       4'd4 : adc_trig <= adc_trig_bp   ; // B ch rising edge
       4'd5 : adc_trig <= adc_trig_bn   ; // B ch falling edge
       4'd6 : adc_trig <= ext_trig_p    ; // external - rising edge
       4'd7 : adc_trig <= ext_trig_n    ; // external - falling edge
       4'd8 : adc_trig <= asg_trig_p    ; // ASG - rising edge
       4'd9 : adc_trig <= asg_trig_n    ; // ASG - falling edge
    default : adc_trig <= 1'b0          ;
   endcase
end

//---------------------------------------------------------------------------------
//  Trigger created from input signal

reg  [  2-1: 0] adc_scht_ap  ;
reg  [  2-1: 0] adc_scht_an  ;
reg  [  2-1: 0] adc_scht_bp  ;
reg  [  2-1: 0] adc_scht_bn  ;
reg  [ 14-1: 0] set_a_tresh  ;
reg  [ 14-1: 0] set_a_treshp ;
reg  [ 14-1: 0] set_a_treshm ;
reg  [ 14-1: 0] set_b_tresh  ;
reg  [ 14-1: 0] set_b_treshp ;
reg  [ 14-1: 0] set_b_treshm ;
reg  [ 14-1: 0] set_a_hyst   ;
reg  [ 14-1: 0] set_b_hyst   ;

always @(posedge adc_clk_i)
if (adc_rstn_i == 1'b0) begin
   adc_scht_ap  <=  2'h0 ;
   adc_scht_an  <=  2'h0 ;
   adc_scht_bp  <=  2'h0 ;
   adc_scht_bn  <=  2'h0 ;
   adc_trig_ap  <=  1'b0 ;
   adc_trig_an  <=  1'b0 ;
   adc_trig_bp  <=  1'b0 ;
   adc_trig_bn  <=  1'b0 ;
end else begin
   set_a_treshp <= set_a_tresh + set_a_hyst ; // calculate positive
   set_a_treshm <= set_a_tresh - set_a_hyst ; // and negative treshold
   set_b_treshp <= set_b_tresh + set_b_hyst ;
   set_b_treshm <= set_b_tresh - set_b_hyst ;

   if (adc_dv) begin
           if ($signed(adc_a_dat) >= $signed(set_a_tresh ))      adc_scht_ap[0] <= 1'b1 ;  // treshold reached
      else if ($signed(adc_a_dat) <  $signed(set_a_treshm))      adc_scht_ap[0] <= 1'b0 ;  // wait until it goes under hysteresis
           if ($signed(adc_a_dat) <= $signed(set_a_tresh ))      adc_scht_an[0] <= 1'b1 ;  // treshold reached
      else if ($signed(adc_a_dat) >  $signed(set_a_treshp))      adc_scht_an[0] <= 1'b0 ;  // wait until it goes over hysteresis

           if ($signed(adc_b_dat) >= $signed(set_b_tresh ))      adc_scht_bp[0] <= 1'b1 ;
      else if ($signed(adc_b_dat) <  $signed(set_b_treshm))      adc_scht_bp[0] <= 1'b0 ;
           if ($signed(adc_b_dat) <= $signed(set_b_tresh ))      adc_scht_bn[0] <= 1'b1 ;
      else if ($signed(adc_b_dat) >  $signed(set_b_treshp))      adc_scht_bn[0] <= 1'b0 ;
   end

   adc_scht_ap[1] <= adc_scht_ap[0] ;
   adc_scht_an[1] <= adc_scht_an[0] ;
   adc_scht_bp[1] <= adc_scht_bp[0] ;
   adc_scht_bn[1] <= adc_scht_bn[0] ;

   adc_trig_ap <= adc_scht_ap[0] && !adc_scht_ap[1] ; // make 1 cyc pulse 
   adc_trig_an <= adc_scht_an[0] && !adc_scht_an[1] ;
   adc_trig_bp <= adc_scht_bp[0] && !adc_scht_bp[1] ;
   adc_trig_bn <= adc_scht_bn[0] && !adc_scht_bn[1] ;
end

//---------------------------------------------------------------------------------
//  External trigger

reg  [  3-1: 0] ext_trig_in    ;
reg  [  2-1: 0] ext_trig_dp    ;
reg  [  2-1: 0] ext_trig_dn    ;
reg  [ 20-1: 0] ext_trig_debp  ;
reg  [ 20-1: 0] ext_trig_debn  ;
reg  [  3-1: 0] asg_trig_in    ;
reg  [  2-1: 0] asg_trig_dp    ;
reg  [  2-1: 0] asg_trig_dn    ;
reg  [ 20-1: 0] asg_trig_debp  ;
reg  [ 20-1: 0] asg_trig_debn  ;

always @(posedge adc_clk_i)
if (adc_rstn_i == 1'b0) begin
   ext_trig_in   <=  3'h0 ;
   ext_trig_dp   <=  2'h0 ;
   ext_trig_dn   <=  2'h0 ;
   ext_trig_debp <= 20'h0 ;
   ext_trig_debn <= 20'h0 ;
   asg_trig_in   <=  3'h0 ;
   asg_trig_dp   <=  2'h0 ;
   asg_trig_dn   <=  2'h0 ;
   asg_trig_debp <= 20'h0 ;
   asg_trig_debn <= 20'h0 ;
end else begin
   //----------- External trigger
   // synchronize FFs
   ext_trig_in <= {ext_trig_in[1:0],trig_ext_i} ;

   // look for input changes
   if ((ext_trig_debp == 20'h0) && (ext_trig_in[1] && !ext_trig_in[2]))
      ext_trig_debp <= set_deb_len ; // ~0.5ms
   else if (ext_trig_debp != 20'h0)
      ext_trig_debp <= ext_trig_debp - 20'd1 ;

   if ((ext_trig_debn == 20'h0) && (!ext_trig_in[1] && ext_trig_in[2]))
      ext_trig_debn <= set_deb_len ; // ~0.5ms
   else if (ext_trig_debn != 20'h0)
      ext_trig_debn <= ext_trig_debn - 20'd1 ;

   // update output values
   ext_trig_dp[1] <= ext_trig_dp[0] ;
   if (ext_trig_debp == 20'h0)
      ext_trig_dp[0] <= ext_trig_in[1] ;

   ext_trig_dn[1] <= ext_trig_dn[0] ;
   if (ext_trig_debn == 20'h0)
      ext_trig_dn[0] <= ext_trig_in[1] ;

   //----------- ASG trigger
   // synchronize FFs
   asg_trig_in <= {asg_trig_in[1:0],trig_asg_i} ;

   // look for input changes
   if ((asg_trig_debp == 20'h0) && (asg_trig_in[1] && !asg_trig_in[2]))
      asg_trig_debp <= set_deb_len ; // ~0.5ms
   else if (asg_trig_debp != 20'h0)
      asg_trig_debp <= asg_trig_debp - 20'd1 ;

   if ((asg_trig_debn == 20'h0) && (!asg_trig_in[1] && asg_trig_in[2]))
      asg_trig_debn <= set_deb_len ; // ~0.5ms
   else if (asg_trig_debn != 20'h0)
      asg_trig_debn <= asg_trig_debn - 20'd1 ;

   // update output values
   asg_trig_dp[1] <= asg_trig_dp[0] ;
   if (asg_trig_debp == 20'h0)
      asg_trig_dp[0] <= asg_trig_in[1] ;

   asg_trig_dn[1] <= asg_trig_dn[0] ;
   if (asg_trig_debn == 20'h0)
      asg_trig_dn[0] <= asg_trig_in[1] ;
end

assign ext_trig_p = (ext_trig_dp == 2'b01) ;
assign ext_trig_n = (ext_trig_dn == 2'b10) ;
assign asg_trig_p = (asg_trig_dp == 2'b01) ;
assign asg_trig_n = (asg_trig_dn == 2'b10) ;

//---------------------------------------------------------------------------------
//  System bus connection

always @(posedge adc_clk_i)
if (adc_rstn_i == 1'b0) begin
   set_a_tresh   <=  14'd5000   ;
   set_b_tresh   <= -14'd5000   ;
   set_dly       <=  32'd0      ;
   set_dec       <=  17'd1      ;
   set_a_hyst    <=  14'd20     ;
   set_b_hyst    <=  14'd20     ;
   set_avg_en    <=   1'b1      ;
   set_a_filt_aa <=  18'h0      ;
   set_a_filt_bb <=  25'h0      ;
   set_a_filt_kk <=  25'hFFFFFF ;
   set_a_filt_pp <=  25'h0      ;
   set_b_filt_aa <=  18'h0      ;
   set_b_filt_bb <=  25'h0      ;
   set_b_filt_kk <=  25'hFFFFFF ;
   set_b_filt_pp <=  25'h0      ;

   set_deb_len   <=  20'd62500  ;
   set_acu_ena   <=   1'd0      ;
   set_acu_cnt   <=  32'd0      ;
   set_acu_shf   <=   5'd0      ;

   set_a_axi_en  <=   1'b0      ;
   set_b_axi_en  <=   1'b0      ;
end else begin
   if (wen) begin
      if (addr[19:0]==20'h8)    set_a_tresh <= wdata[14-1:0] ;
      if (addr[19:0]==20'hC)    set_b_tresh <= wdata[14-1:0] ;
      if (addr[19:0]==20'h10)   set_dly     <= wdata[32-1:0] ;
      if (addr[19:0]==20'h14)   set_dec     <= wdata[17-1:0] ;
      if (addr[19:0]==20'h20)   set_a_hyst  <= wdata[14-1:0] ;
      if (addr[19:0]==20'h24)   set_b_hyst  <= wdata[14-1:0] ;
      if (addr[19:0]==20'h28)   set_avg_en  <= wdata[     0] ;

      if (addr[19:0]==20'h30)   set_a_filt_aa <= wdata[18-1:0] ;
      if (addr[19:0]==20'h34)   set_a_filt_bb <= wdata[25-1:0] ;
      if (addr[19:0]==20'h38)   set_a_filt_kk <= wdata[25-1:0] ;
      if (addr[19:0]==20'h3C)   set_a_filt_pp <= wdata[25-1:0] ;
      if (addr[19:0]==20'h40)   set_b_filt_aa <= wdata[18-1:0] ;
      if (addr[19:0]==20'h44)   set_b_filt_bb <= wdata[25-1:0] ;
      if (addr[19:0]==20'h48)   set_b_filt_kk <= wdata[25-1:0] ;
      if (addr[19:0]==20'h4C)   set_b_filt_pp <= wdata[25-1:0] ;

      if (addr[19:0]==20'h90)   set_deb_len <= wdata[20-1:0] ;
      if (addr[19:0]==20'h94)   set_acu_ena <= wdata[     0] ;
      if (addr[19:0]==20'h98)   set_acu_cnt <= wdata[32-1:0] ;
      if (addr[19:0]==20'h9c)   set_acu_shf <= wdata[ 5-1:0] ;
      if (addr[19:0]==20'ha0)   set_acu_len <= wdata[14-1:0] ;

      if (addr[19:0]==20'h50)   set_a_axi_start <= wdata[32-1:0] ;
      if (addr[19:0]==20'h54)   set_a_axi_stop  <= wdata[32-1:0] ;
      if (addr[19:0]==20'h58)   set_a_axi_dly   <= wdata[32-1:0] ;
      if (addr[19:0]==20'h5C)   set_a_axi_en    <= wdata[     0] ;

      if (addr[19:0]==20'h70)   set_b_axi_start <= wdata[32-1:0] ;
      if (addr[19:0]==20'h74)   set_b_axi_stop  <= wdata[32-1:0] ;
      if (addr[19:0]==20'h78)   set_b_axi_dly   <= wdata[32-1:0] ;
      if (addr[19:0]==20'h7C)   set_b_axi_en    <= wdata[     0] ;
   end
end

always @(*) begin
   err <= 1'b0 ;

   casez (addr[19:0])
     20'h00004 : begin ack <= 1'b1;          rdata <= {{32- 4{1'b0}}, set_trig_src}       ; end 

     20'h00008 : begin ack <= 1'b1;          rdata <= {{32-14{1'b0}}, set_a_tresh}        ; end
     20'h0000C : begin ack <= 1'b1;          rdata <= {{32-14{1'b0}}, set_b_tresh}        ; end
     20'h00010 : begin ack <= 1'b1;          rdata <= {               set_dly}            ; end
     20'h00014 : begin ack <= 1'b1;          rdata <= {{32-17{1'b0}}, set_dec}            ; end

     20'h00018 : begin ack <= 1'b1;          rdata <= {{32-RSZ{1'b0}}, adc_wp_cur}        ; end
     20'h0001C : begin ack <= 1'b1;          rdata <= {{32-RSZ{1'b0}}, adc_wp_trig}       ; end

     20'h00020 : begin ack <= 1'b1;          rdata <= {{32-14{1'b0}}, set_a_hyst}         ; end
     20'h00024 : begin ack <= 1'b1;          rdata <= {{32-14{1'b0}}, set_b_hyst}         ; end

     20'h00028 : begin ack <= 1'b1;          rdata <= {{32- 1{1'b0}}, set_avg_en}         ; end

     20'h00030 : begin ack <= 1'b1;          rdata <= {{32-18{1'b0}}, set_a_filt_aa}      ; end
     20'h00034 : begin ack <= 1'b1;          rdata <= {{32-25{1'b0}}, set_a_filt_bb}      ; end
     20'h00038 : begin ack <= 1'b1;          rdata <= {{32-25{1'b0}}, set_a_filt_kk}      ; end
     20'h0003C : begin ack <= 1'b1;          rdata <= {{32-25{1'b0}}, set_a_filt_pp}      ; end
     20'h00040 : begin ack <= 1'b1;          rdata <= {{32-18{1'b0}}, set_b_filt_aa}      ; end
     20'h00044 : begin ack <= 1'b1;          rdata <= {{32-25{1'b0}}, set_b_filt_bb}      ; end
     20'h00048 : begin ack <= 1'b1;          rdata <= {{32-25{1'b0}}, set_b_filt_kk}      ; end
     20'h0004C : begin ack <= 1'b1;          rdata <= {{32-25{1'b0}}, set_b_filt_pp}      ; end

     20'h00090 : begin ack <= 1'b1;          rdata <= {{32-20{1'b0}}, set_deb_len}        ; end
     20'h00094 : begin ack <= 1'b1;          rdata <= {{32- 2{1'b0}}, acu_ctl_run,
                                                                      set_acu_ena}        ; end
     20'h00098 : begin ack <= 1'b1;          rdata <= {               set_sts_cnt}        ; end
     20'h0009c : begin ack <= 1'b1;          rdata <= {{32- 5{1'b0}}, set_acu_shf}        ; end
     20'h000a0 : begin ack <= 1'b1;          rdata <= {{32-14{1'b0}}, set_acu_len}        ; end

     20'h00050 : begin ack <= 1'b1;          rdata <=                 set_a_axi_start     ; end
     20'h00054 : begin ack <= 1'b1;          rdata <=                 set_a_axi_stop      ; end
     20'h00058 : begin ack <= 1'b1;          rdata <=                 set_a_axi_dly       ; end
     20'h0005C : begin ack <= 1'b1;          rdata <= {{32- 1{1'b0}}, set_a_axi_en}       ; end
     20'h00060 : begin ack <= 1'b1;          rdata <=                 set_a_axi_trig      ; end
     20'h00064 : begin ack <= 1'b1;          rdata <=                 set_a_axi_cur       ; end

     20'h00070 : begin ack <= 1'b1;          rdata <=                 set_b_axi_start     ; end
     20'h00074 : begin ack <= 1'b1;          rdata <=                 set_b_axi_stop      ; end
     20'h00078 : begin ack <= 1'b1;          rdata <=                 set_b_axi_dly       ; end
     20'h0007C : begin ack <= 1'b1;          rdata <= {{32- 1{1'b0}}, set_b_axi_en}       ; end
     20'h00080 : begin ack <= 1'b1;          rdata <=                 set_b_axi_trig      ; end
     20'h00084 : begin ack <= 1'b1;          rdata <=                 set_b_axi_cur       ; end

     20'h1???? : begin ack <= adc_rd_dv;     rdata <= {16'h0, 2'h0,adc_a_rd}              ; end
     20'h2???? : begin ack <= adc_rd_dv;     rdata <= {16'h0, 2'h0,adc_b_rd}              ; end

     20'h3???? : begin ack <= acu_rd_dv;     rdata <= acu_a_mem_rdt                       ; end
     20'h4???? : begin ack <= acu_rd_dv;     rdata <= acu_b_mem_rdt                       ; end

       default : begin ack <= 1'b1;          rdata <=  32'h0                              ; end
   endcase
end

// bridge between ADC and sys clock
bus_clk_bridge i_bridge (
   .sys_clk_i     (  sys_clk_i      ),
   .sys_rstn_i    (  sys_rstn_i     ),
   .sys_addr_i    (  sys_addr_i     ),
   .sys_wdata_i   (  sys_wdata_i    ),
   .sys_sel_i     (  sys_sel_i      ),
   .sys_wen_i     (  sys_wen_i      ),
   .sys_ren_i     (  sys_ren_i      ),
   .sys_rdata_o   (  sys_rdata_o    ),
   .sys_err_o     (  sys_err_o      ),
   .sys_ack_o     (  sys_ack_o      ),

   .clk_i         (  adc_clk_i      ),
   .rstn_i        (  adc_rstn_i     ),
   .addr_o        (  addr           ),
   .wdata_o       (  wdata          ),
   .wen_o         (  wen            ),
   .ren_o         (  ren            ),
   .rdata_i       (  rdata          ),
   .err_i         (  err            ),
   .ack_i         (  ack            )
);

endmodule
