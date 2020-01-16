////////////////////////////////////////////////////////////////////////////////
// Arbitrary signal generator. Holds table and FSM for one channel.
// Author: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
// BURST MODE
//
// Burst mode is enabled using the cfg_ben signal.
// In the next diagram 'D' is date read from the buffer, while I is idle data
// (repetition of last value read from the buffer). The D*I* sequence can be
// repeated. Each sample from the table is repeated (cfg_bdr+1) times.
// (cfg_bdl+1) samples are read from the table.
//
// DDDDDDDDIIIIIIIIDDDDDDDDIIIIIIIIDDDDDDDDIIIIIIII...
//                                                     cfg_bdr+1 data repetitions
// |<---->|        |<---->|        |<---->|            cfg_bdl+1 data length
// |<------------>||<------------>||<------------>|    cfg_bpl+1 period length
//                                                     cfg_bpn+1 period number
//
////////////////////////////////////////////////////////////////////////////////

module asg_bst #(
  // data bus
  int unsigned AN = 1,
  type AT = logic [8-1:0],
  // burst counter parameters
  int unsigned CWR = 14,  // counter width repetitions
  int unsigned CWM = 14,  // counter width
  int unsigned CWL = 32,  // counter width length
  int unsigned CWN = 16   // counter width number
)(
  // stream output
  axi4_stream_if.s       sto    ,
  // events input/output
  input  evn_pkg::evn_t  evn    ,  // input
  output evn_pkg::evn_t  evs    ,  // output
  // trigger
  input  logic           ctl_trg,
  input  logic           cfg_tre,
  // events
  output logic           evn_per,  // period
  // generator mode
  input  logic           cfg_inf,  // infinite
  // burst configuration (burst mode)
  input  logic [CWR-1:0] cfg_bdr,  // burst data   repetitions
  input  logic [CWM-1:0] cfg_bdl,  // burst data   length
  input  logic [CWL-1:0] cfg_bpl,  // burst period length
  input  logic [CWN-1:0] cfg_bpn,  // burst period number
  // status
  output logic [CWL-1:0] sts_bpl,  // burst period length counter
  output logic [CWN-1:0] sts_bpn   // burst period number counter
);

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

// trigger control
logic               trg_msk;  // trigger mask

// burst counters/status
logic     [CWR-1:0] sts_bdr;  // burst data   repetitions
logic     [CWM-1:0] sts_bdl;  // burst data   length
logic               end_bdr;  // burst data   repetitions
logic               end_bdl;  // burst data   length
logic               end_bpl;  // burst period length
logic               end_bpn;  // burst period number
// address enable
logic               sts_adr;      // address enable
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
// burst mode state machine
////////////////////////////////////////////////////////////////////////////////

// control end event
assign ctl_end = evn.stp | (end_bpl & end_bpn); 

// state machine
always_ff @(posedge sto.ACLK)
if (~sto.ARESETn) begin
  sts_adr <= 1'b0;
end else begin
  // synchronous clear
  if (evn.rst) begin
    sts_adr <= 1'b0;
  end else begin
    // address enable status
    if      (ctl_end)  sts_adr <= 1'b0;
    else if (ctl_run)  sts_adr <= 1'b1;
    else if (evs.swt & sts_rdy) begin
      if      (end_bpl)  sts_adr <= 1'b1;
      else if (end_bdl)  sts_adr <= 1'b0;
    end
  end
end

// address pointer counter
always_ff @(posedge sto.ACLK)
if (~sto.ARESETn) begin
  sts_bdr <= '0;
  sts_bdl <= '0;
  sts_bpl <= '0;
  sts_bpn <= '0;
end else begin
  // synchronous clear
  if (evn.rst) begin
    sts_bdr <= '0;
    sts_bdl <= '0;
    sts_bpl <= '0;
    sts_bpn <= '0;
  end else begin
    if (ctl_run) begin
      sts_bdr <= '0;
      sts_bdl <= '0;
      sts_bpl <= '0;
      sts_bpn <= '0;
    end else if (evs.swt & sts_rdy) begin
      sts_bdr <= end_bdr ? 0 : sts_bdr + !end_bdl;
      sts_bdl <= end_bpl ? 0 : sts_bdl +  end_bdr;
      sts_bpl <= end_bpl ? 0 : sts_bpl +        1; 
      sts_bpn <=               sts_bpn +  end_bpl;
    end
  end
end

// counter end status
assign end_bdr = (sts_bdr == cfg_bdr) | end_bpl;
assign end_bdl = (sts_bdl == cfg_bdl) & end_bdr;
assign end_bpl = (sts_bpl == cfg_bpl);
assign end_bpn = (sts_bpn == cfg_bpn) & ~cfg_inf;

// events
always_ff @(posedge sto.ACLK)
if (~sto.ARESETn)  evn_per <= 1'b0;
else               evn_per <= evs.swt & end_bpl;

////////////////////////////////////////////////////////////////////////////////
// output stream
////////////////////////////////////////////////////////////////////////////////

assign sto.TDATA  = sts_bdl;
// TODO: it might not be necessary to read data all the time
// optimizing it would slightly reduce power consumption
assign sto.TKEEP  = sts_adr;
assign sto.TLAST  = ctl_end;
assign sto.TVALID = evs.swt;

assign sts_rdy = sto.TREADY | ~sto.TVALID;

endmodule: asg_bst
