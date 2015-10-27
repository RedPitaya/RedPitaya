////////////////////////////////////////////////////////////////////////////////
// Module: clock divider
// Author: Iztok Jeras <iztok.jeras@redpitaya.com>
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

module clkdiv #(
  int unsigned CW = 8  // counter width (resolution)
)(
  // system signals
  input  logic                  clk ,  // clock
  input  logic                  rstn,  // reset (active low)
  // configuration
  input  logic                  ena,   // enable
  input  logic         [CW-1:0] div,   // divider (division ratio - 1)
  // output
  output logic                  cke    // clock enable (synchronous)
);

// local signals
logic [CW-1:0] cnt;  // counter current value
logic [CW-1:0] nxt;  // counter next value
logic          clr;  // counter clear

// counter current value
always_ff @(posedge clk)
if (~rstn)  cnt <= '0;
else begin
  if (ena)  cnt <= clr ? '0 : nxt;
  else      cnt <= '0;
end

// counter next value
assign nxt = cnt + 'd1;

// counter cycle end
assign clr = nxt == div;

// output
always_ff @(posedge clk)
cke <= clr;

endmodule: clkdiv
