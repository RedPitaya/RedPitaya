////////////////////////////////////////////////////////////////////////////////
// Arbitrary signal generator. Holds table and FSM for one channel.
// Author: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
// PERIODIC MODE (FREQUENCY/PHASE AND RESOLUTION)
// 
// The frequency is specified by the cfg_ste value, the phase by the cfg_off
// value and both also depend on the buffer length cfg_siz. The cfg_ste (step
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
// f = Fs/(cfg_siz+1) * (cfg_ste+1)/(2**CWF)
//
// Frequency (max bufer size):
// f = Fs/(2**(CWM+CWF)) * (cfg_ste+1)
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
// The default fixed point format for cfg_ste and cfg_off is u14.16 and the
// default buffer size is 2**14=16384 locations.
// Fs = 125MHz
// Δf = 125MHz/2**(14+16) = 0.116Hz
// ΔΦ = 360°  /2**(14+16) = 0.000000335°
//
////////////////////////////////////////////////////////////////////////////////

module asg_per #(
  // data bus
  int unsigned AN = 1,
  type AT = logic [8-1:0],
  // continuous/periodic buffer parameters
  int unsigned CWM = 14,  // counter width magnitude (fixed point integer)
  int unsigned CWF = 16,  // counter width fraction  (fixed point fraction)
  type CT = logic [CWM-1:-CWF]
)(
  // stream output
  axi4_stream_if.s       sto    ,
  // events input/output
  input  evn_pkg::evn_t  evn    ,  // input
  output evn_pkg::evn_t  evs    ,  // output
  // trigger
  input  logic           ctl_trg,
  input  logic           cfg_tre,
  // continuous/periodic configuration
  input  CT              cfg_siz,  // data table size
  input  CT              cfg_ste,  // pointer step    size
  input  CT              cfg_off   // pointer initial offset (used to define phase)
);

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

// trigger control
logic               trg_msk;  // trigger mask
// continuous/periodic pointers
logic [CWM+CWF-1:0] ptr_cur; // current
logic [CWM+CWF-0:0] ptr_nxt; // next
logic [CWM+CWF-0:0] ptr_nxt_sub ;
logic               ptr_nxt_sub_neg;
// backpressure (TODO: it is not implemented properly)
logic               sts_rdy;      // ready
// events
logic               ctl_run;      // run start event
logic               ctl_end;      // run end event

////////////////////////////////////////////////////////////////////////////////
// start/stop status
////////////////////////////////////////////////////////////////////////////////

// start status
always_ff @(posedge sto.ACLK)
if (~sto.ARESETn) begin
  evs.str <= 1'b0;
  evs.swt <= 1'b0;
end else begin
  // synchronous clear
  if (evn.rst) begin
    evs.str <= 1'b0;
    evs.swt <= 1'b0;
  end else begin
    // start/stop status
    if      (evn.stp)  evs.str <= 1'b0;
    else if (evn.str)  evs.str <= 1'b1;
    // trigger status
    if      (ctl_end)  evs.swt <= 1'b0;
    else if (ctl_run)  evs.swt <= 1'b1;
  end
end

// reset status
assign evs.rst = 1'b0;

// stop status
assign evs.stp = ~evs.str;

// control run (trigger while started or simultaneous trigger and start)
assign ctl_run = ( evn.swt || (ctl_trg & trg_msk)  ) & (evs.str | evn.str) ;

// if trigger repeat enable is not set
// mask trigger during active burst
always_ff @(posedge sto.ACLK)
if (~sto.ARESETn) begin
  trg_msk <= 1'b1;
end else begin
  // synchronous clear
  if (evn.rst) begin
    trg_msk <= 1'b1;
  end else begin
    // address enable status
    if      (ctl_end)  trg_msk <= 1'b1;
    else if (ctl_run)  trg_msk <= cfg_tre;
  end
end

////////////////////////////////////////////////////////////////////////////////
// continuous/periodic mode state machine
////////////////////////////////////////////////////////////////////////////////

// control end event
// the only way to stop continuous/periodic mode is with a stop event
assign ctl_end = evn.stp;

// address pointer counter
always_ff @(posedge sto.ACLK)
if (~sto.ARESETn) begin
  ptr_cur <= '0;
end else begin
  // synchronous clear
  if (evn.rst) begin
    ptr_cur <= '0;
  end else begin
    // start on trigger, new triggers are ignored while ASG is running
    if                (ctl_run)  ptr_cur <= cfg_off;
    // modulo (proper wrapping) increment pointer
    else if (evs.swt & sts_rdy)  ptr_cur <= ~ptr_nxt_sub_neg ? ptr_nxt_sub : ptr_nxt;
  end
end

// next pointer value and overflow
assign ptr_nxt     = ptr_cur + (cfg_ste + 1);
assign ptr_nxt_sub = ptr_nxt - (cfg_siz + 1);
assign ptr_nxt_sub_neg = ptr_nxt_sub[CWM+CWF];

////////////////////////////////////////////////////////////////////////////////
// output stream
////////////////////////////////////////////////////////////////////////////////

assign sto.TDATA  = ptr_cur[CWF+:CWM];
// TODO: it might not be necessary to read data all the time
// optimizing it would slightly reduce power consumption
assign sto.TKEEP  = '1;
assign sto.TLAST  = ctl_end;
assign sto.TVALID = evs.swt;

assign sts_rdy = sto.TREADY | ~sto.TVALID;

endmodule: asg_per
