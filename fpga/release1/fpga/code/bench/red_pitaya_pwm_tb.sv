/**
 * @brief Red Pitaya PWM module testbench.
 *
 * @Author Iztok Jeras
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

`timescale 1ns / 1ps

module red_pitaya_pwm_tb #(
  // clock time periods
  realtime  TP = 4.0ns,  // 250MHz
  // PWM parameters
  int unsigned CCW = 24
);

// system signals
logic clk ;
logic rstn;

// configuration
logic [CCW-1:0] cfg;

// signals to PWM IO
logic pwm_o;
logic pwm_s;

////////////////////////////////////////////////////////////////////////////////
// test sequence
////////////////////////////////////////////////////////////////////////////////

initial        clk = 1'h0;
always #(TP/2) clk = ~clk;

initial begin
  cfg  = 0;
  rstn = 1'b0;
  repeat(4) @(posedge clk);
  rstn = 1'b1;
  for (bit [8-1:0] i=0; i<=156; i+=16) begin
    cfg = {i, 16'h0000};
    repeat (8) begin  wait (pwm_s) @(posedge clk);  end
  end

  cfg  = 0;
  rstn = 1'b0;
  repeat(4) @(posedge clk);
  rstn = 1'b1;
  cfg = {8'd33  , 16'h0000};
  repeat (4) begin  wait (pwm_s) @(posedge clk);  end    cfg = {8'd33, 16'h0001};
  repeat (4) begin  wait (pwm_s) @(posedge clk);  end    cfg = {8'd33, 16'h0010};
  repeat (4) begin  wait (pwm_s) @(posedge clk);  end    cfg = {8'd33, 16'h0100};
  repeat (4) begin  wait (pwm_s) @(posedge clk);  end    cfg = {8'd33, 16'h1000};
  repeat (4) begin  wait (pwm_s) @(posedge clk);  end    cfg = {8'd33, 16'h1010};
  repeat (4) begin  wait (pwm_s) @(posedge clk);  end    cfg = {8'd33, 16'h1111};
  repeat (4) begin  wait (pwm_s) @(posedge clk);  end    cfg = {8'd33, 16'h5555};
  repeat (4) begin  wait (pwm_s) @(posedge clk);  end    cfg = {8'd33, 16'hFFFF};

  cfg  = 0;
  rstn = 1'b0;
  repeat(4) @(posedge clk);
  rstn = 1'b1;
  repeat (4) begin  wait (pwm_s) @(posedge clk);  end    cfg = {8'd0  , 16'h8000};
  repeat (4) begin  wait (pwm_s) @(posedge clk);  end    cfg = {8'd0  , 16'h8001};
  repeat (4) begin  wait (pwm_s) @(posedge clk);  end    cfg = {8'd0  , 16'h8181};
  repeat (4) begin  wait (pwm_s) @(posedge clk);  end    cfg = {8'd0  , 16'hFFFF};
  repeat (4) begin  wait (pwm_s) @(posedge clk);  end    cfg = {8'd1  , 16'h0000};
  repeat (4) begin  wait (pwm_s) @(posedge clk);  end    cfg = {8'd1  , 16'h8001};
  repeat (4) begin  wait (pwm_s) @(posedge clk);  end    cfg = {8'd156, 16'h0000};
  repeat (4) begin  wait (pwm_s) @(posedge clk);  end    cfg = {8'd155, 16'hFFFF};
  repeat (4) begin  wait (pwm_s) @(posedge clk);  end 

  $finish();
end

////////////////////////////////////////////////////////////////////////////////
// module instance
////////////////////////////////////////////////////////////////////////////////

red_pitaya_pwm #(
  .CCW (CCW)
) pwm (
  // system signals
  .clk   (clk ),
  .rstn  (rstn),
  // configuration
  .cfg   (cfg), 
  // PWM outputs
  .pwm_o (pwm_o),
  .pwm_s (pwm_s)
);

////////////////////////////////////////////////////////////////////////////////
// waveforms
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("red_pitaya_pwm_tb.vcd");
  $dumpvars(0, red_pitaya_pwm_tb);
end

endmodule: red_pitaya_pwm_tb
