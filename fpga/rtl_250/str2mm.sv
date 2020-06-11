////////////////////////////////////////////////////////////////////////////////
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

module str2mm #(
  type DT = logic signed [16-1:0],
  int unsigned DN = 1,
  int unsigned DL = 1<<14
)(
  // control
  input  logic     ctl_rst,
  // stream input
  axi4_stream_if.d str,
  // System bus
  sys_bus_if.s     bus
);

localparam int unsigned AW = $clog2(DL);

// memory
DT             buf_dat [0:DL-1];
DT    [ 2-1:0] buf_rdat;
// write port
logic          buf_wen;
logic [AW-1:0] buf_wad;

////////////////////////////////////////////////////////////////////////////////
// write
////////////////////////////////////////////////////////////////////////////////

assign buf_wen = str.TVALID & str.TREADY;

assign str.TREADY = 1;

always_ff @(posedge str.ACLK)
if (~str.ARESETn) begin
  buf_wad <= '0;
end else begin
  if (ctl_rst) begin
    buf_wad <= '0;
  end else if (buf_wen) begin
    if (str.TLAST)  buf_wad <= '0;
    else            buf_wad <= buf_wad + 1;
  end
end

always_ff @(posedge str.ACLK)
if (buf_wen) begin
   buf_dat [buf_wad] <= str.TDATA;
end

////////////////////////////////////////////////////////////////////////////////
// read
////////////////////////////////////////////////////////////////////////////////

logic          buf_ren;

always_ff @(posedge bus.clk)
buf_ren <= bus.ren;

always_ff @(posedge bus.clk)
for (int unsigned i=0; i<2; i++) begin
  buf_rdat [i] <= buf_dat [{bus.addr[2+:AW-1],i[0]}];
end

always_ff @(posedge bus.clk)
for (int unsigned i=0; i<2; i++) begin
  if (buf_ren) begin
    bus.rdata [16*i+:16] <= buf_rdat[i];
  end
end

always_ff @(posedge bus.clk)
if (~bus.rstn) begin
  bus.err <= 1'b0;
  bus.ack <= 1'b0;
end else begin
  bus.ack <= (bus.wen | buf_ren);

end

endmodule: str2mm
