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
//
// PERIODIC MODE (FREQUENCY/PHASE AND RESOLUTION
// 
// The frequency is specified by the cfg_stp value, the phase by the cfg_off
// value and both also depend on the buffer length cfg_siz. The cfg_stp (step
// length) and cfg_off (initial position) values are fixed point with a
// magnitude of CWM bits and a fraction of CWF bits.
//
// The buffer is usually programmed to contain a full period of the desired
// waveform, and the whole available buffer is usually used since it provides
// the best resolution. The buffer length is defined as (cfg_siz+1) and can be
// at most 2**CWM locations when:
// cfg_off = 2**CWF - 1
//
// The frequency and phase resolutions are defined by the smaller values of
// control variables.
//
// Frequency:
// f = Fs/(cfg_siz+1) * (cfg_stp+1)/(2**CWF)
//
// Frequency (max bufer size):
// f = Fs/(2**(CWM+CWF)) * (cfg_stp+1)
//
// Phase:
// Φ = 360°/(cfg_siz+1) * (cfg_off+1)/(2**CWF)
//
// Phase (max bufer size):
// Φ = 360°/(2**(CWM+CWF)) * (cfg_off+1)
//
// Resolution:
// Δf = Fs  /2**(CWM+CWF)
// ΔΦ = 360°/2**(CWM+CWF)
//
// Example values:
// The default fixed point format for cfg_stp and cfg_off is u14.16 and the
// default buffer size is 2**14=16384 locations.
// Fs = 125MHz
// Δf = 125MHz/2**(14+16) = 0.116Hz
// ΔΦ = 360°  /2**(14+16) = 0.000000335°
//
////////////////////////////////////////////////////////////////////////////////
//
// BURST MODE
//
// Burst mode is enabled using the cfg_ben signal.
// In the next diagram 'D' is date read from the buffer, while I is idle data
// (repetition of last value read from the buffer). The D*I* sequence can be
// repeated.
//
// DDDDDDDDIIIIIIIIDDDDDDDDIIIIIIIIDDDDDDDDIIIIIIII...
// |<---->|        |<---->|        |<---->|            cfg_bdl+1 data length
// |<------------>||<------------>||<------------>|    cfg_bln+1 leriod length
//                                                     cfg_bnm+1 repetitions 
//
////////////////////////////////////////////////////////////////////////////////

module old_asg #(
  // trigger
  int unsigned TN = 1,
  // data bus
  int unsigned DN = 1,
  type DT = logic [8-1:0],
  // buffer parameters
  int unsigned CWM = 14,  // counter width magnitude (fixed point integer)
  int unsigned CWF = 16,  // counter width fraction  (fixed point fraction)
  // burst counter parameters
  int unsigned CWL = 32,  // counter width length
  int unsigned CWN = 16   // counter width number
)(
  // stream output
  axi4_stream_if.s           sto    ,
  // control
  input  logic               ctl_rst,  // set FSM to reset
  // trigger
  input  logic      [TN-1:0] trg_i  ,  // input
  output logic               trg_o  ,  // output event
  // interrupts
  output logic               irq_trg,  // trigger
  output logic               irq_stp,  // stop
  // configuration (periodic mode)
  input  logic      [TN-1:0] cfg_trg,  // trigger mask
  input  logic [CWM+CWF-1:0] cfg_siz,  // data table size
  input  logic [CWM+CWF-1:0] cfg_stp,  // pointer step    size
  input  logic [CWM+CWF-1:0] cfg_off,  // pointer initial offset (used to define phase)
  // configuration (burst mode)
  input  logic               cfg_ben,  // burst enable
  input  logic               cfg_inf,  // infinite
  input  logic     [CWM-1:0] cfg_bdl,  // burst data length
  input  logic     [CWL-1:0] cfg_bln,  // burst      length
  input  logic     [CWN-1:0] cfg_bnm,  // burst number of repetitions
  // status
  output logic     [CWL-1:0] sts_bln,  // burst length counter
  output logic     [CWN-1:0] sts_bnm,  // burst number counter
  output logic               sts_run,  // running status
  // System bus
  sys_bus_if.s               bus
);

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

// buffer
DT                  buf_mem [0:2**CWM-1];
DT                  buf_rdata;  // read data
logic [CWM    -1:0] buf_raddr;  // read address

// pointers
logic [CWM+CWF-1:0] ptr_cur; // current
logic [CWM+CWF-0:0] ptr_nxt; // next
logic [CWM+CWF-0:0] ptr_nxt_sub ;
logic               ptr_nxt_sub_neg;
// counter end status
logic               end_bdl;  // burst data length
logic               end_bln;  // burst      length
logic               end_bnm;  // burst repetitions
// status and events
logic               sts_vld;  // valid
logic               sts_lst;  // last
logic               sts_trg;  // trigger event
logic               sts_rpt;  // repeat  event
logic               sts_aen;  // address enable
logic               sts_ren;  // read    enable
logic               sts_rdy;  // ready

////////////////////////////////////////////////////////////////////////////////
//  DAC buffer RAM
////////////////////////////////////////////////////////////////////////////////

// CPU write access
always @(posedge bus.clk)
if (bus.wen)  buf_mem[bus.addr] <= bus.wdata;

// CPU read-back access
always @(posedge bus.clk)
if (bus.ren)  bus.rdata <= buf_mem[bus.addr];

// CPU control signals
always_ff @(posedge bus.clk)
if (~bus.rstn)  bus.ack <= 1'b0;
else            bus.ack <= bus.ren | bus.wen;

assign bus.err = 1'b0;

// stream read
always @(posedge sto.ACLK)
begin 
  if (sts_aen)  buf_raddr <= ptr_cur[CWF+:CWM];
  if (sts_ren)  buf_rdata <= buf_mem[buf_raddr];
end

// valid signal used to enable memory read access
always @(posedge sto.ACLK)
if (~sto.ARESETn) begin
  sts_vld <= 1'b0;
  sts_ren <= 1'b0;
  sts_lst <= 1'b0;
end else begin
  if (ctl_rst) begin
    sts_vld <= 1'b0;
    sts_ren <= 1'b0;
    sts_lst <= 1'b0;
  end else if (sts_rdy) begin
    sts_vld <= sts_run; 
    sts_ren <= sts_aen;
    sts_lst <= sts_rpt & end_bnm;
  end
end

////////////////////////////////////////////////////////////////////////////////
//  read pointer & state machine
////////////////////////////////////////////////////////////////////////////////

// state machine
always_ff @(posedge sto.ACLK)
if (~sto.ARESETn) begin
  sts_aen <= 1'b0;
  sts_run <= 1'b0;
  sts_bln <= '0;
  sts_bnm <= '0;
end else begin
  // synchronous clear
  if (ctl_rst) begin
    sts_aen <= 1'b0;
    sts_run <= 1'b0;
    sts_bln <= '0;
    sts_bnm <= '0;
  end else if (sts_rdy) begin
    if (cfg_ben) begin
      // burst mode
      if (sts_trg) begin
        sts_aen <= sts_run ? ~end_bnm : 1'b1;
        sts_run <= sts_run ? ~end_bnm : 1'b1;
        sts_bnm <= sts_run ? sts_bnm + !cfg_inf : '0;
        sts_bln <= '0;
      end else begin
        if (end_bdl) sts_aen <= 1'b0;
        if (sts_run) sts_bln <= sts_bln + 1; 
      end
    end else begin
      // periodic mode
      if (sts_trg) begin
        sts_aen <= 1'b1;
        sts_run <= 1'b1;
      end
    end
  end
end

// counter end status
assign end_bnm = (sts_bnm == cfg_bnm) & ~cfg_inf;
assign end_bln = (sts_bln == cfg_bln);
assign end_bdl = (sts_bln == cfg_bdl);

// next value of burst period counter

logic trg;
assign trg = |(trg_i & cfg_trg);

assign sts_trg = sts_run ? sts_rpt : trg;
assign sts_rpt = sts_run & end_bln & cfg_ben;

// interrupts
assign irq_trg = sts_trg;
assign irq_stp = sts_lst;

////////////////////////////////////////////////////////////////////////////////
// read pointer logic
////////////////////////////////////////////////////////////////////////////////

always_ff @(posedge sto.ACLK)
if (~sto.ARESETn) begin
  ptr_cur <= '0;
end else begin
  // synchronous clear
  if (ctl_rst) begin
    ptr_cur <= '0;
  end else if (sts_rdy) begin
    // start on trigger, new triggers are ignored while ASG is running
    if (sts_trg) begin
      ptr_cur <= cfg_off;
    // modulo (proper wrapping) increment pointer
    end else if (sts_aen) begin
      ptr_cur <= ~ptr_nxt_sub_neg ? ptr_nxt_sub : ptr_nxt;
    end
  end
end

// next pointer value and overflow
assign ptr_nxt     = ptr_cur + (cfg_stp + 1);
assign ptr_nxt_sub = ptr_nxt - (cfg_siz + 1);
assign ptr_nxt_sub_neg = ptr_nxt_sub[CWM+CWF];

////////////////////////////////////////////////////////////////////////////////
// output stream
////////////////////////////////////////////////////////////////////////////////

// trigger output
always_ff @(posedge sto.ACLK)
if (~sto.ARESETn)  trg_o <= 1'b0;
else               trg_o <= sts_trg;

// output data
assign sto.TDATA = buf_rdata;

// output keep/last
always_ff @(posedge sto.ACLK)
if (sts_rdy) begin
  sto.TKEEP <= '1;
  sto.TLAST <= sts_lst;
end

// output valid
always_ff @(posedge sto.ACLK)
if (~sto.ARESETn) begin
  sto.TVALID <= 1'b0;
end else begin
  // synchronous clear
  if (ctl_rst) begin
    sto.TVALID <= 1'b0;
  end else if (sts_rdy) begin
    sto.TVALID <= sts_vld;
  end
end

assign sts_rdy = sto.TREADY | ~sto.TVALID;

endmodule: old_asg
