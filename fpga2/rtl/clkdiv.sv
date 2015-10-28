////////////////////////////////////////////////////////////////////////////////
// Module: clock divider
// Author: Iztok Jeras <iztok.jeras@redpitaya.com>
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

module clkdiv #(
  int unsigned DWC = 8  // data width counter (resolution)
)(
  // system signals
  input  logic           clk ,  // clock
  input  logic           rstn,  // reset (active low)
  // configuration
  input  logic           ena,   // enable
  input  logic [DWC-1:0] div,   // divider (division ratio - 1)
  // output
  output logic           cke    // clock enable (synchronous)
);

// local signals
logic [DWC-1:0] cnt;  // counter current value
logic           clr;  // counter clear

// counter current value
always_ff @(posedge clk)
if (~rstn)  cnt <= '0;
else begin
  if (ena)  cnt <= clr ? '0 : cnt + 'd1;
  else      cnt <= '0;
end

// counter cycle end
assign clr = cnt == div;

// output
always_ff @(posedge clk)
cke <= clr;

endmodule: clkdiv
