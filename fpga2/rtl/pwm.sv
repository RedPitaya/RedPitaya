////////////////////////////////////////////////////////////////////////////////
// Module: PWM (pulse width modulation)
// Author: Iztok Jeras <iztok.jeras@redpitaya.com>
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

module pwm #(
  int unsigned  CCW = 8  // configuration counter width (resolution)
)(
  // system signals
  input  logic           clk ,  // clock
  input  logic           rstn,  // reset (active low)
  // configuration
  input  logic           ena,   // enable
  input  logic [CCW-1:0] rng,   // range
  // input stream
  input  logic [CCW-1:0] str_dat,  // data
  input  logic           str_vld,  // valid
  output logic           str_rdy,  // ready
  // PWM output
  output logic           pwm
);

// local signals
logic [CCW-1:0] cnt;  // counter current value
logic [CCW-1:0] nxt;  // counter next value

// counter current value
always_ff @(posedge clk)
if (~rstn)  cnt <= '0;
else begin
  if (ena)  cnt <= str_rdy ? '0 : nxt;
  else      cnt <= '0;
end

// counter next value
assign nxt = cnt + 'd1;

// counter cycle end
assign str_rdy = nxt == rng;

// PWM output
always_ff @(posedge clk)
if (~rstn)  pwm <= 1'b0;
else begin
  if (ena) begin
    if      (cnt == 0      )  pwm <= |str_dat;
    else if (cnt == str_dat)  pwm <= 1'b0;
  end else begin
    pwm <= 1'b0;
  end
end

endmodule: pwm
