////////////////////////////////////////////////////////////////////////////////
//
// Author: Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

module str_to_ram #(
  // data bus
  int unsigned DN = 1,
  type DAT_T = logic [8-1:0],
  int unsigned AW = 14  // counter width magnitude (fixed point integer)
)(
  // control
  input  logic    ctl_rst,  // set FSM to reset
  // stream input
  str_bus_if.d    str,
  // System bus
  sys_bus_if.s    bus
);

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

// buffer
DAT_T          buf_mem [0:2**AW-1];
logic          buf_wen  , buf_ren;
DAT_T          buf_wdata, buf_rdata;
logic [AW-1:0] buf_waddr, buf_raddr;

////////////////////////////////////////////////////////////////////////////////
// stream write
////////////////////////////////////////////////////////////////////////////////

assign buf_wdata = str.dat;
assign buf_wen   = str.trn;

always @(posedge str.clk)
if (str.rstn) begin
  buf_waddr <= '0;
end else begin
  if (ctl_rst) begin
    buf_waddr <= '0;
  end else begin
    buf_waddr <= buf_waddr + buf_wen;
  end
end

always @(posedge str.clk)
if (buf_wen)  buf_mem[buf_waddr] <= buf_wdata;

////////////////////////////////////////////////////////////////////////////////
// read pointer logic
////////////////////////////////////////////////////////////////////////////////

assign buf_raddr = bus.addr;
assign buf_ren   = bus.ren;

// CPU read access
always @(posedge bus.clk)
if (buf_ren)  buf_rdata <= buf_mem[buf_raddr];

// CPU control signals
always_ff @(posedge bus.clk)
if (~bus.rstn)  bus.ack <= 1'b0;
else            bus.ack <= bus.ren | bus.wen;

assign bus.err = 1'b0;

//// stream read
//always @(posedge sto.clk)
//begin 
//  if (sts_aen)  buf_raddr <= ptr_cur[CWF+:CWM];
//  if (sts_ren)  buf_rdata <= buf_mem[buf_raddr];
//end

endmodule: str_to_ram
