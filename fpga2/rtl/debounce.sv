////////////////////////////////////////////////////////////////////////////////
// Module: debounce
// Author: Iztok Jeras <iztok.jeras@redpitaya.com>
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

module debounce #(
  int unsigned CW = 8,  // counter width
  int unsigned DW = 1,  // data width
  bit [PW-1:0] DI = '0  // data idle state
)(
  // system signals
  input  logic          clk ,  // clock
  input  logic          rstn,  // reset (active low)
  // configuration
  input  logic          ena,   // enable
  input  logic [CW-1:0] len,   // range
  // input stream
  input  logic [PW-1:0] d_i,   // data input
  output logic [PW-1:0] d_o,   // debounced output
  output logic [PW-1:0] d_p,   // debounced output posedge
  output logic [PW-1:0] d_n    // debounced output negedge
);

generate
for (genvar i=0; i<PW; i++) begin: for_i

logic [CW-1:0] cnt;  // counter
logic    [1:0] d_s;  // input synchronizer

// prevention of metastability problems
always_ff @(posedge clk)
d_s <= {d_s[0], d_i[i]};

// the counter should start running on a change
always_ff @(posedge clk)
if (~rstn)              cnt <= 0;
else if (ena) begin
  if (|cnt)             cnt <= cnt - 1;
  else if (d_s[1]^d_o)  cnt <= CN;
end

// when the counter is zero the output should follow the input
always_ff @(posedge clk)
if (~rstn)    d_o <= DI[i];
else if (ena) begin
  if (~|cnt)  d_o <= d_s[1];
end

end: for_i
endgenerate

endmodule: debounce
