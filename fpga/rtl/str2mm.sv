////////////////////////////////////////////////////////////////////////////////
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

module str2mm #(
  type DT = logic signed [16-1:0],
  int unsigned DN = 1,
  int unsigned DL = 1<<14
)(
  // stream input
  axi4_stream_if.s str,
  // System bus
  sys_bus_if.s     bus,
  // control (synchronous clear)
  input  logic     clr
);

localparam int unsigned AW = $clog2(DL);

// memory
DT             buf_dat [0:DL-1];
// write port
logic          buf_wen;
logic [AW-1:0] buf_wad;

////////////////////////////////////////////////////////////////////////////////
// write
////////////////////////////////////////////////////////////////////////////////

assign buf_wen = str.TVALID & str.TREADY;

assign str.TREADY = 1;

always @(posedge str.ACLK)
if (~str.ARESETn) begin
  buf_wad <= '0;
end else begin
  buf_wad <= buf_wad + buf_wen;
end

always @(posedge str.ACLK)
if (buf_wen)
   buf_dat [buf_wad] <= str.TDATA;

////////////////////////////////////////////////////////////////////////////////
// read
////////////////////////////////////////////////////////////////////////////////

always @(posedge str.ACLK)
if (bus.ren)
  bus.rdata <= buf_dat [bus.addr];

always @(posedge str.ACLK)
if (~bus.rstn) begin
  bus.err <= 1'b0;
  bus.ack <= 1'b0;
end else if (bus.wen | bus.ren) begin
  bus.ack <= 1'b1;
end

endmodule: str2mm
