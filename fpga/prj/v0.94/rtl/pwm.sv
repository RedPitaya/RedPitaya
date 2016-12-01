////////////////////////////////////////////////////////////////////////////////
// PWM (pulse width modulation) and PDM (pulse density modulation)
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

module pwm #(
  int unsigned  CCW = 8,  // configuration counter width (resolution)
  bit [CCW-1:0] CCE = '1  // 100% value
)(
  // system signals
  input  logic           clk ,  // clock
  input  logic           rstn,  // reset (active low)
  // input stream
  input  logic [CCW-1:0] str_dat,  // data
  output logic           str_rdy,  // ready
  // PWM output
  output logic           pwm
);

// local signals
logic [CCW-1:0] dat;  // registered input
logic [CCW-1:0] cnt;  // counter current value
logic [CCW-1:0] nxt;  // counter next value
logic           lst;  // counter last value

// stream data register
always_ff @(posedge clk)
if (str_rdy) dat <= str_dat;

// stream ready register
always_ff @(posedge clk)
if (~rstn)  str_rdy <= 1'b1;
else        str_rdy <= lst;

// counter
always_ff @(posedge clk)
if (~rstn)  cnt <= '0;
else        cnt <= lst ? '0 : nxt;

assign nxt = cnt + 'd1;
assign lst = nxt == CCE;

// PWM output
always_ff @(posedge clk)
if (~rstn)  pwm <= 1'b0;
else begin
  if      (str_rdy)     pwm <= |str_dat;
  else if (cnt == dat)  pwm <= 1'b0;
end

endmodule: pwm
