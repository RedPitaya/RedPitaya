////////////////////////////////////////////////////////////////////////////////
// Module: Red Pitaya complex trigger.
// Authors: Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

module ctrg #(
  // monitor parameters
  int unsigned CW  = 32,  // counter width
  // event parameters
  int unsigned ER  = 0,   // event reset
  int unsigned EN  = 1,   // event number
  int unsigned EL  = $clog2(EN),
  // trigger parameters
  int unsigned TN  = 1    // trigger number
)(
  // events input/output
  input  evn_pkg::evn_t [EN-1:0] evi,  // input
  output evn_pkg::evn_t          evo,  // output
  // triggers input/output
  input  logic          [TN-1:0] trg,  // input
  output logic                   tro,  // output
  // system bus
  sys_bus_if.s                   bus
);

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

// event select
logic  [EL-1:0] cfg_evn;
// trigger mask
logic  [TN-1:0] cfg_trg;

// software events
evn_pkg::evn_t  evn;  // multiplexed input
evn_pkg::evn_t  evs;  // status

// trigger
logic           ctl_trg;

// trigger event counter
logic           sts_run;
logic  [CW-1:0] sts_cnt;

////////////////////////////////////////////////////////////////////////////////
//  System bus connection
////////////////////////////////////////////////////////////////////////////////

always_ff @(posedge bus.clk)
if (~bus.rstn) begin
  bus.err <= 1'b0;
  bus.ack <= 1'b0;
end else begin
  bus.err <= 1'b0;
  bus.ack <= bus.wen | bus.ren;
end

localparam int unsigned BAW=5;

// write access
always_ff @(posedge bus.clk)
if (~bus.rstn) begin
  // event select
  cfg_evn <= ER;
  // trigger mask
  cfg_trg <= '0;
end else begin
  if (bus.wen) begin
    // event select
    if (bus.addr[BAW-1:0]=='h04)  cfg_evn <= bus.wdata;
    // triger mask
    if (bus.addr[BAW-1:0]=='h08)  cfg_trg <= bus.wdata;
  end
end

// event outputs
always_ff @(posedge bus.clk)
if (~bus.rstn)  evo <= '0;
else            evo <= (bus.wen & (bus.addr[BAW-1:0]=='h00)) ? bus.wdata : '0;

// read access
always_ff @(posedge bus.clk)
casez (bus.addr[BAW-1:0])
  // control
  'h00: bus.rdata <= evs;
  // event select
  'h04: bus.rdata <= cfg_evn;
  // trigger mask
  'h08: bus.rdata <= cfg_trg;
  // trigger pre/post time
  'h10: bus.rdata <= sts_cnt;
  // default is 'x for better optimization
  default: bus.rdata <= 'x;
endcase

////////////////////////////////////////////////////////////////////////////////
// trigger status handler
////////////////////////////////////////////////////////////////////////////////

always_ff @(posedge bus.clk)
if (~bus.rstn)  evn <= '0;
else            evn <= evi[cfg_evn];

assign evs.rst = 1'b0;

assign ctl_trg = evn.swt | |(trg & cfg_trg);

////////////////////////////////////////////////////////////////////////////////
// trigger counter
////////////////////////////////////////////////////////////////////////////////

// trigger status bit is not used
assign evs.swt = 1'b0;

// run status start/stop bits
assign evs.str =  sts_run;
assign evs.stp = ~sts_run;

always_ff @(posedge bus.clk)
if (~bus.rstn) begin
   sts_run <= 1'b0;
   sts_cnt <= 0;
end else begin
   // run status
   if (evn.rst) begin
       sts_run <= 1'b0;
   end else if (evn.str) begin
       sts_run <= 1'b1;
   end else if (evn.stp) begin
       sts_run <= 1'b0;
   end
   // counter status
   if (evn.rst) begin
       sts_cnt <= 0;
   end else begin 
       sts_cnt <= sts_cnt + (sts_run & ctl_trg);
   end
end

////////////////////////////////////////////////////////////////////////////////
// trigger output
////////////////////////////////////////////////////////////////////////////////

// TODO: it is unused for now
assign tro = 1'b0;

endmodule: ctrg
