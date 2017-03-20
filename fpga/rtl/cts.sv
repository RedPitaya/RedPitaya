////////////////////////////////////////////////////////////////////////////////
// Module: Current time stamp
// Authors: Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

module cts #(
  // timestamp parameters
  int unsigned TW = 64   // timestamp width
)(
  // system signals
  input  logic           clk,
  input  logic           rstn,
  // current time stamp
  output logic  [TW-1:0] cts
);

// TODO for not this value is limited to 32-bits
// to achieve 64 or at least 48 bits, pipelining is needed
always_ff @(posedge clk)
if (~rstn)  cts <= '0;
else        cts <= 32'(cts + 1);

endmodule: cts
