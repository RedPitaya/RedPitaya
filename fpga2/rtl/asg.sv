////////////////////////////////////////////////////////////////////////////////
// Arbitrary signal generator. Holds table and FSM for one channel.
// Author: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
// GENERAL DESCRIPTION:
//
// Arbitrary signal generator takes data stored in buffer and sends them to DAC.
//
//
//                /-----\
//   SW --------> | BUF | ---> output
//          |     \-----/
//          |        ^
//          |        |
//          |     /-----\
//          ----> |     |
//                | FSM | ---> trigger notification
//   trigger ---> |     |
//                \-----/
//
//
// Submodule for ASG which hold buffer data and control registers for one channel.
// 
////////////////////////////////////////////////////////////////////////////////

module asg #(
  // data parameters
  int unsigned DWO = 14,  // data width for output
  // buffer parameters
  int unsigned CWM = 14,  // counter width magnitude (fixed point integer)
  int unsigned CWF = 16   // counter width fraction  (fixed point fraction)
)(
  // system signals
  input  logic                  clk      ,  // clock
  input  logic                  rstn     ,  // reset - active low
  // DAC
  output logic signed [DWO-1:0] sto_dat  ,  // data
  output logic                  sto_vld  ,  // valid
  input  logic                  sto_rdy  ,  // ready
  // trigger
  input  logic                  trg_i   ,  // input
  output logic                  trg_o   ,  // output event
  // CPU buffer access
  input  logic                  bus_ena  ,  // enable
  input  logic                  bus_wen  ,  // write enable
  input  logic        [CWM-1:0] bus_addr ,  // address
  input  logic signed [DWO-1:0] bus_wdata,  // write data
  output logic signed [DWO-1:0] bus_rdata,  // read  data
  // configuration
  input  logic    [CWM+CWF-1:0] cfg_size ,  // data tablesize
  input  logic    [CWM+CWF-1:0] cfg_step ,  // pointer step    size
  input  logic    [CWM+CWF-1:0] cfg_offs ,  // pointer initial offset (used to define phase)
  input  logic       [  16-1:0] cfg_ncyc ,  // set number of cycle
  input  logic       [  16-1:0] cfg_rnum ,  // set number of repetitions
  input  logic       [  32-1:0] cfg_rdly ,  // set delay between repetitions
  input  logic                  cfg_wrap ,  // set wrap enable
  // control
  input  logic                  ctl_rst  ,  // set FSM to reset
);

////////////////////////////////////////////////////////////////////////////////
//  DAC buffer RAM
////////////////////////////////////////////////////////////////////////////////

logic signed [    DWO-1:0] buf_mem [0:2**CWM-1];
logic signed [    DWO-1:0] buf_rdata;  // read data
logic        [CWM    -1:0] buf_raddr;  // read address
logic        [CWM+CWF-1:0] dac_pnt   ; // read pointer
logic        [CWM+CWF-1:0] dac_pntp  ; // previour read pointer
logic        [CWM+CWF-0:0] dac_npnt  ; // next read pointer
logic        [CWM+CWF-0:0] dac_npnt_sub ;
logic                      dac_npnt_sub_neg;

// stream read
always @(posedge clk)
begin
  buf_raddr <= dac_pnt[CWF+:CWM];
  buf_rdata <= buf_mem[buf_raddr];
end

// CPU write access
always @(posedge clk)
if (bus_ena &  bus_wen)  buf_mem[bus_addr] <= bus_wdata;

// CPU read-back access
always @(posedge clk)
if (bus_ena & ~bus_wen)  bus_rdata <= buf_mem[bus_addr];

////////////////////////////////////////////////////////////////////////////////
//  read pointer & state machine
////////////////////////////////////////////////////////////////////////////////

logic [  16-1: 0] cyc_cnt;
logic [  16-1: 0] rep_cnt;

logic             dac_do  ;
logic             dac_rep ;
logic             dac_trg;

// state machine
always_ff @(posedge clk)
if (~rstn) begin
  cyc_cnt   <= '0;
  rep_cnt   <= '0;
  dly_cnt   <= '0;
  dac_do    <= '0;
  dac_rep   <= '0;
  dac_pntp  <= '0;
end else begin
  // delay between repetitions 
  if (ctl_rst || dac_do)   dly_cnt <= cfg_rdly;
  else if (|dly_cnt)       dly_cnt <= dly_cnt - 32'h1;
  // repetitions counter
  if (trg_i && !dac_do)                                    rep_cnt <= cfg_rnum;
  else if (|rep_cnt && dac_rep && (dac_trg && !dac_do))    rep_cnt <= rep_cnt - 16'h1;
  // count number of table read cycles
  dac_pntp  <= dac_pnt;

  else if (!dac_trg && |cyc_cnt && ({1'b0,dac_pntp} > {1'b0,dac_pnt}))    cyc_cnt <= cyc_cnt - 16'h1 ;
  // in cycle mode
  if (dac_trg && !ctl_rst)                                           dac_do <= 1'b1 ;
  else if (ctl_rst || ((cyc_cnt==16'h1) && ~dac_npnt_sub_neg) )       dac_do <= 1'b0 ;
  // in repetition mode
  if (dac_trg && !ctl_rst)                  dac_rep <= 1'b1 ;
  else if (ctl_rst || (rep_cnt==16'h0))      dac_rep <= 1'b0 ;
end

assign dac_trg = (!dac_rep && trg_i) || (dac_rep && |rep_cnt && (dly_cnt == 32'h0)) ;

assign dac_npnt_sub = dac_npnt - {1'b0,cfg_size} - 1;
assign dac_npnt_sub_neg = dac_npnt_sub[CWM+16];

// read pointer logic
always_ff @(posedge clk)
if (~rstn) begin                    dac_pnt <= '0;
end else begin
  // manual reset or start
  if (ctl_rst || (dac_trg && !dac_do))  dac_pnt <= cfg_offs;
  else if (dac_do) begin
    if (~dac_npnt_sub_neg)               dac_pnt <= cfg_wrap ? dac_npnt_sub : cfg_offs; // wrap or go to start
    else                                 dac_pnt <= dac_npnt; // normal increase
  end
end

assign dac_npnt = dac_pnt + cfg_step;
assign trg_o = !dac_rep && trg_i;

endmodule: asg
