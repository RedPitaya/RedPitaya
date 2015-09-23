/**
 * $Id: red_pitaya_asg_ch.v 1271 2014-02-25 12:32:34Z matej.oblak $
 *
 * @brief Red Pitaya ASG submodule. Holds table and FSM for one channel.
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
 * Arbitrary signal generator takes data stored in buffer and sends them to DAC.
 *
 *
 *                /-----\         /--------\
 *   SW --------> | BUF | ------> | kx + o | ---> DAC DAT
 *          |     \-----/         \--------/
 *          |        ^
 *          |        |
 *          |     /-----\
 *          ----> |     |
 *                | FSM | ------> trigger notification
 *   trigger ---> |     |
 *                \-----/
 *
 *
 * Submodule for ASG which hold buffer data and control registers for one channel.
 * 
 */

module asg_ch #(
  int unsigned DWO = 14,  // data width for output
  int unsigned RSZ = 14   // RAM size 2^RSZ
)(
  // system signals
  input  logic            clk             ,  // dac clock
  input  logic            rstn            ,  // dac reset - active low
  // DAC
  output logic [DWO-1: 0] dac_o           ,  // dac data output
  // trigger
  input  logic            trig_i          ,  // trigger input
  output logic            trig_done_o     ,  // trigger event
  // buffer ctrl
  input  logic            buf_we_i        ,  // buffer write enable
  input  logic [RSZ-1: 0] buf_addr_i      ,  // buffer address
  input  logic [DWO-1: 0] buf_wdata_i     ,  // buffer write data
  output logic [DWO-1: 0] buf_rdata_o     ,  // buffer read data
  output logic [RSZ-1: 0] buf_rpnt_o      ,  // buffer current read pointer
  // configuration
  input  logic [RSZ+15:0] set_size_i      ,  // set table data size
  input  logic [RSZ+15:0] set_step_i      ,  // set pointer step
  input  logic [RSZ+15:0] set_ofs_i       ,  // set reset offset
  input  logic            set_rst_i       ,  // set FSM to reset
  input  logic            set_once_i      ,  // set only once  -- not used
  input  logic            set_wrap_i      ,  // set wrap enable
  input  logic [ DWO-1:0] set_amp_i       ,  // set amplitude scale
  input  logic [ DWO-1:0] set_dc_i        ,  // set output offset
  input  logic            set_zero_i      ,  // set output to zero
  input  logic [  16-1:0] set_ncyc_i      ,  // set number of cycle
  input  logic [  16-1:0] set_rnum_i      ,  // set number of repetitions
  input  logic [  32-1:0] set_rdly_i      ,  // set delay between repetitions
  input  logic            set_rgate_i        // set external gated repetition
);

//---------------------------------------------------------------------------------
//
//  DAC buffer RAM

reg   [ DWO-1: 0] dac_buf [0:(1<<RSZ)-1] ;
reg   [  14-1: 0] dac_rd    ;
reg   [  14-1: 0] dac_rdat  ;
reg   [ RSZ-1: 0] dac_rp    ;
reg   [RSZ+15: 0] dac_pnt   ; // read pointer
reg   [RSZ+15: 0] dac_pntp  ; // previour read pointer
wire  [RSZ+16: 0] dac_npnt  ; // next read pointer
wire  [RSZ+16: 0] dac_npnt_sub ;
wire              dac_npnt_sub_neg;

reg   [  28-1: 0] dac_mult  ;
reg   [  15-1: 0] dac_sum   ;

// read
always @(posedge clk)
begin
   buf_rpnt_o <= dac_pnt[RSZ+15:16];
   dac_rp     <= dac_pnt[RSZ+15:16];
   dac_rd     <= dac_buf[dac_rp] ;
   dac_rdat   <= dac_rd ;  // improve timing
end

// write
always @(posedge clk)
if (buf_we_i)  dac_buf[buf_addr_i] <= buf_wdata_i[14-1:0] ;

// read-back
always @(posedge clk)
buf_rdata_o <= dac_buf[buf_addr_i] ;

// scale and offset
always @(posedge clk)
begin
  dac_mult <= $signed(dac_rdat) * $signed({1'b0,set_amp_i}) ;
  dac_sum  <= $signed(dac_mult[28-1:13]) + $signed(set_dc_i) ;
  // saturation
  if (set_zero_i)  dac_o <= 14'h0;
  else             dac_o <= ^dac_sum[15-1:15-2] ? {dac_sum[15-1], {13{~dac_sum[15-1]}}} : dac_sum[13:0];
end

//---------------------------------------------------------------------------------
//
//  read pointer & state machine

reg  [  16-1: 0] cyc_cnt      ;
reg  [  16-1: 0] rep_cnt      ;
reg  [  32-1: 0] dly_cnt      ;
reg  [   8-1: 0] dly_tick     ;

reg              dac_do       ;
reg              dac_rep      ;
wire             dac_trig     ;
reg              dac_trigr    ;

// state machine
always @(posedge clk)
if (rstn == 1'b0) begin
  cyc_cnt   <= 16'h0 ;
  rep_cnt   <= 16'h0 ;
  dly_cnt   <= 32'h0 ;
  dly_tick  <=  8'h0 ;
  dac_do    <=  1'b0 ;
  dac_rep   <=  1'b0 ;
  dac_pntp  <= {RSZ+16{1'b0}} ;
  dac_trigr <=  1'b0 ;
end else begin
  // make 1us tick
  if (dac_do || (dly_tick == 8'd124))
    dly_tick <= 8'h0 ;
  else
    dly_tick <= dly_tick + 8'h1 ;

  // delay between repetitions 
  if (set_rst_i || dac_do)
    dly_cnt <= set_rdly_i ;
  else if (|dly_cnt && (dly_tick == 8'd124))
    dly_cnt <= dly_cnt - 32'h1 ;

  // repetitions counter
  if (trig_i && !dac_do)
    rep_cnt <= set_rnum_i ;
  else if (!set_rgate_i && (|rep_cnt && dac_rep && (dac_trig && !dac_do)))
    rep_cnt <= rep_cnt - 16'h1 ;
  else if (set_rgate_i)
    rep_cnt <= 16'h0 ;

  // count number of table read cycles
  dac_pntp  <= dac_pnt;
  dac_trigr <= dac_trig; // ignore trigger when count
  if (dac_trig)
    cyc_cnt <= set_ncyc_i ;
  else if (!dac_trigr && |cyc_cnt && ({1'b0,dac_pntp} > {1'b0,dac_pnt}))
    cyc_cnt <= cyc_cnt - 16'h1 ;

  // in cycle mode
  if (dac_trig && !set_rst_i)
     dac_do <= 1'b1 ;
  else if (set_rst_i || ((cyc_cnt==16'h1) && ~dac_npnt_sub_neg) )
     dac_do <= 1'b0 ;

  // in repetition mode
  if (dac_trig && !set_rst_i)
     dac_rep <= 1'b1 ;
  else if (set_rst_i || (rep_cnt==16'h0))
     dac_rep <= 1'b0 ;
end

assign dac_trig = (!dac_rep && trig_i) || (dac_rep && |rep_cnt && (dly_cnt == 32'h0)) ;

assign dac_npnt_sub = dac_npnt - {1'b0,set_size_i} - 1;
assign dac_npnt_sub_neg = dac_npnt_sub[RSZ+16];

// read pointer logic
always @(posedge clk)
if (rstn == 1'b0) begin
  dac_pnt  <= {RSZ+16{1'b0}};
end else begin
  if (set_rst_i || (dac_trig && !dac_do)) // manual reset or start
    dac_pnt <= set_ofs_i;
  else if (dac_do) begin
    if (~dac_npnt_sub_neg)  dac_pnt <= set_wrap_i ? dac_npnt_sub : set_ofs_i; // wrap or go to start
    else                    dac_pnt <= dac_npnt[RSZ+15:0]; // normal increase
  end
end

assign dac_npnt = dac_pnt + set_step_i;
assign trig_done_o = !dac_rep && trig_i;

endmodule: asg_ch
