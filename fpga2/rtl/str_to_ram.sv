////////////////////////////////////////////////////////////////////////////////
//
// Author: Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

module str_to_ram #(
  // data bus
  int unsigned DN = 1,
  type DT = logic [8-1:0],
  int unsigned AW = 14  // counter width magnitude (fixed point integer)
)(
  // stream input
  axi4_stream_if.m  str,
  // System bus
  sys_bus_if.s      bus
);

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

// control
logic          ctl_rst;

// buffer
DT     [2-1:0] buf_mem [0:2**(AW-1)-1];
logic          buf_wen  , buf_ren;
DT             buf_wdata           ;
logic [32-1:0]            buf_rdata;
logic [AW-1:0] buf_waddr, buf_raddr;

////////////////////////////////////////////////////////////////////////////////
// stream write
////////////////////////////////////////////////////////////////////////////////

assign buf_wdata = str.TDATA;
assign buf_wen   = str.transf;

always_ff @(posedge str.ACLK)
if (~str.ARESETn) begin
  buf_waddr <= '0;
end else begin
  if (ctl_rst) begin
    buf_waddr <= '0;
  end else begin
    buf_waddr <= buf_waddr + buf_wen;
  end
end

always_ff @(posedge str.ACLK)
if (buf_wen)  buf_mem[buf_waddr[AW-1:1]][buf_waddr[0]] <= buf_wdata;

////////////////////////////////////////////////////////////////////////////////
// read pointer logic
////////////////////////////////////////////////////////////////////////////////

// CPU read access
always @(posedge bus.clk)
begin
  if (bus.ren)  buf_raddr <= bus.addr >> 2;
  if (buf_ren)  buf_rdata <= buf_mem[buf_raddr];
end

// CPU control signals
always_ff @(posedge bus.clk)
if (~bus.rstn) begin
  buf_ren <= 1'b0;
  bus.ack <= 1'b0;
end else begin
  buf_ren <= bus.ren | bus.wen;
  bus.ack <= buf_ren;
end

assign bus.err = 1'b0;
assign bus.rdata = buf_rdata;

////////////////////////////////////////////////////////////////////////////////
// CPU write access
////////////////////////////////////////////////////////////////////////////////

// SW reset
always_ff @(posedge bus.clk)
if (~bus.rstn) begin
  ctl_rst <= 1'b0;
end else begin
  ctl_rst <= bus.wen;
end

endmodule: str_to_ram
