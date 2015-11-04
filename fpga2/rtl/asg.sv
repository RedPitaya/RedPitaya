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
  input  logic                  trg_i    ,  // input
  output logic                  trg_o    ,  // output event
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
  // configuration (burst mode)
  input  logic                  cfg_bena ,  // burst enable
  input  logic                  cfg_binf ,  // infinite
  input  logic       [  16-1:0] cfg_bcyc ,  // number of data cycle
  input  logic       [  32-1:0] cfg_bdly ,  // number of delay cycles
  input  logic       [  16-1:0] cfg_bnum ,  // number of repetitions
  // control
  input  logic                  ctl_rst     // set FSM to reset
);

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

typedef enum logic [2-1:0] {
  STS_IDL = 2'b00,
  STS_DAT = 2'b01,
  STS_DLY = 2'b10
} status_t;

// buffer
logic signed [    DWO-1:0] buf_mem [0:2**CWM-1];
logic signed [    DWO-1:0] buf_rdata;  // read data
logic        [CWM    -1:0] buf_raddr;  // read address

// pointers
logic [CWM+CWF-1:0] ptr_cur; // current
logic [CWM+CWF-0:0] ptr_nxt; // next
logic [CWM+CWF-0:0] ptr_nxt_sub ;
logic               ptr_nxt_sub_neg;
// counters
logic [16-1:0] cnt_cyc;
logic [32-1:0] cnt_dly;
logic [16-1:0] cnt_rep;
// status and events
status_t       sts_run;
status_t       sts_vld;
logic          sts_trg;
logic          sts_end;
logic          sts_lst;

////////////////////////////////////////////////////////////////////////////////
//  DAC buffer RAM
////////////////////////////////////////////////////////////////////////////////

// CPU write access
always @(posedge clk)
if (bus_ena &  bus_wen)  buf_mem[bus_addr] <= bus_wdata;

// CPU read-back access
always @(posedge clk)
if (bus_ena & ~bus_wen)  bus_rdata <= buf_mem[bus_addr];

// stream read
always @(posedge clk)
begin 
  if (sts_run==STS_DAT)  buf_raddr <= ptr_cur[CWF+:CWM];
  if (sts_vld==STS_DAT)  buf_rdata <= buf_mem[buf_raddr];
end

// valid signal used to enable memory read access
always @(posedge clk)
begin
  if (ctl_rst) sts_vld <= STS_IDL;
  else         sts_vld <= sts_run; 
end

////////////////////////////////////////////////////////////////////////////////
//  read pointer & state machine
////////////////////////////////////////////////////////////////////////////////

// state machine
always_ff @(posedge clk)
if (~rstn) begin
  sts_run <= STS_IDL;
  cnt_cyc <= '0;
  cnt_dly <= '0;
  cnt_rep <= '0;
end else begin
  // synchronous clear
  if (ctl_rst) begin
    sts_run <= STS_IDL;
    cnt_cyc <= '0;
    cnt_dly <= '0;
    cnt_rep <= '0;
  // start on trigger, new triggers are ignored while ASG is running
  end else if (sts_trg) begin
    if (sts_lst) begin
      sts_run <= STS_IDL;
      cnt_rep <= '0;
    end else begin
      sts_run <= STS_DAT;
      if (cfg_bena) begin
        cnt_cyc <= cfg_bcyc; 
        cnt_dly <= cfg_bdly;
        cnt_rep <= sts_end ? cnt_rep-1 : cfg_bnum;
      end
    end
  // decrement counters
  end else begin
    if (cfg_bena) begin
      if (sts_run == STS_DAT & ~|cnt_cyc & |cnt_dly)  sts_run <=  STS_DLY;
      if (sts_run == STS_DAT &  |cnt_cyc           )  cnt_cyc <= cnt_cyc-1;
      if (sts_run == STS_DLY &             |cnt_dly)  cnt_dly <= cnt_dly-1;
    end
  end
end

assign sts_trg = (trg_i & (sts_run==STS_IDL))
               | (sts_end & (|cnt_rep | cfg_binf));

assign sts_end = cfg_bena & (sts_run!=STS_IDL) & ~|cnt_cyc & ~|cnt_dly;
assign sts_lst = cnt_rep == 1;

// read pointer logic
always_ff @(posedge clk)
if (~rstn) begin
  ptr_cur <= '0;
end else begin
  // synchronous clear
  if (ctl_rst) begin
    ptr_cur <= '0;
  // start on trigger, new triggers are ignored while ASG is running
  end else if (sts_trg) begin
    ptr_cur <= cfg_offs;
  // modulo (proper wrapping) increment pointer
  end else if (sts_run==STS_DAT) begin
    ptr_cur <= ~ptr_nxt_sub_neg ? ptr_nxt_sub : ptr_nxt;
  end
end

// next pointer value and overflow
assign ptr_nxt = ptr_cur + cfg_step;
assign ptr_nxt_sub = ptr_nxt - cfg_size - 1;
assign ptr_nxt_sub_neg = ptr_nxt_sub[CWM+16];

////////////////////////////////////////////////////////////////////////////////
// output stream
////////////////////////////////////////////////////////////////////////////////

// trigger output
assign trg_o = sts_trg;

// output data
assign sto_dat = sto_vld ? buf_rdata : '0;

// output valid
always_ff @(posedge clk)
if (~rstn) begin
  sto_vld <= 1'b0;
end else begin
  // synchronous clear
  if (ctl_rst) begin
    sto_vld <= 1'b0;
  end else begin
    sto_vld <= |sts_vld;
  end
end

endmodule: asg
