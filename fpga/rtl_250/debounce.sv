////////////////////////////////////////////////////////////////////////////////
// Module: debounce
// Author: Iztok Jeras <iztok.jeras@redpitaya.com>
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

module debounce #(
  int unsigned CW = 8,  // counter width
  bit          DI = '0  // data idle state
)(
  // system signals
  input  logic          clk ,  // clock
  input  logic          rstn,  // reset (active low)
  // configuration
  input  logic          ena,   // enable
  input  logic [CW-1:0] len,   // range
  // input stream
  input  logic          d_i,   // data input
  output logic          d_o,   // debounced output
  output logic          d_p,   // debounced output posedge
  output logic          d_n    // debounced output negedge
);

logic [CW-1:0] cnt;  // counter
logic    [1:0] d_s;  // input synchronizer

// prevention of metastability problems
always_ff @(posedge clk)
d_s <= {d_s[0], d_i};

// the counter should start running on a change
always_ff @(posedge clk)
if (~rstn)                cnt <= 0;
else begin
  if (ena) begin
    if (|cnt)             cnt <= cnt - 1;
    else if (d_s[1]^d_o)  cnt <= len;
  end else                cnt <= 0;
end

// when the counter is zero the output should follow the input
always_ff @(posedge clk)
if (~rstn) begin
  d_o <= DI;
  d_p <= 1'b0;
  d_n <= 1'b0;
end else begin
  // output folowing the input
  if (~|cnt | ~ena)
    d_o <= d_s[1];
  // edge pulses  
  if (~|cnt) begin
    d_p <=  d_s[1] & ~d_o;
    d_n <= ~d_s[1] &  d_o;
  end else begin
    d_p <= 1'b0;
    d_n <= 1'b0;
  end
end

endmodule: debounce
