////////////////////////////////////////////////////////////////////////////////
//
// Author: Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

module str_to_ram #(
  // data bus
  int unsigned DN = 1,
  type DT = logic [16-1:0],
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
logic [32-1:0] buf_mem [0:2**(AW-1)-1];
logic          buf_wen  ;
logic [32-1:0] buf_wdata;
logic [AW-1:0] buf_waddr;

////////////////////////////////////////////////////////////////////////////////
// stream write
////////////////////////////////////////////////////////////////////////////////

assign buf_wen = str.transf & (buf_waddr[0] | str.TLAST);

assign buf_wdata[1*16+:16] = str.TDATA;

always @(posedge bus.clk)
if (str.transf) buf_wdata[0*16+:16] <= str.TDATA;

always_ff @(posedge str.ACLK)
if (~str.ARESETn) begin
  buf_waddr <= '0;
end else begin
  if (ctl_rst) begin
    buf_waddr <= '0;
  end else begin
    buf_waddr <= buf_waddr + str.transf;
  end
end

always_ff @(posedge str.ACLK)
if (buf_wen)  buf_mem[buf_waddr>>1] <= buf_wdata;

////////////////////////////////////////////////////////////////////////////////
// read pointer logic
////////////////////////////////////////////////////////////////////////////////

// CPU read access
always_ff @(posedge bus.clk)
if (bus.ren)  bus.rdata <= buf_mem[bus.addr >> 2];

// CPU control signals
always_ff @(posedge bus.clk)
if (~bus.rstn) begin
  bus.ack <= 1'b0;
end else begin
  bus.ack <= bus.ren | bus.wen;
end

assign bus.err = 1'b0;

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
