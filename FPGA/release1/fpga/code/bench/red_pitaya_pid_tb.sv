/**
 * $Id: red_pitaya_pid_tb.v 961 2014-01-21 11:40:39Z matej.oblak $
 *
 * @brief Red Pitaya MIMO PID testbench.
 *
 * @Author Matej Oblak
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in Verilog hardware description language (HDL).
 * Please visit http://en.wikipedia.org/wiki/Verilog
 * for more details on the language used herein.
 */

/**
 * GENERAL DESCRIPTION:
 *
 * Testbench for MIMO PID controller.
 *
 * Testing red_pitaya_pid module using only PID11 section. For system which it
 * controls simple model is used (acts as system of first order). 
 * 
 */

`timescale 1ns / 1ps

module red_pitaya_pid_tb #(
  // time periods
  realtime  TP = 8.0ns  // 125MHz
);

////////////////////////////////////////////////////////////////////////////////
// signal generation
////////////////////////////////////////////////////////////////////////////////

logic              clk ;
logic              rstn;

// ADC clock
initial        clk = 1'b0;
always #(TP/2) clk = ~clk;

// ADC reset
initial begin
  rstn = 1'b0;
  repeat(4) @(posedge clk);
  rstn = 1'b1;
end

////////////////////////////////////////////////////////////////////////////////
// test sequence
// Simple module - first order system
////////////////////////////////////////////////////////////////////////////////

logic [ 14-1: 0] dat_a_in        ;
logic [ 14-1: 0] dat_b_in        ;
logic [ 14-1: 0] dat_a_out       ;
logic [ 14-1: 0] dat_b_out       ;

logic [ 32-1: 0] sys_addr        ;
logic [ 32-1: 0] sys_wdata       ;
logic [  4-1: 0] sys_sel         ;
logic            sys_wen         ;
logic            sys_ren         ;
logic [ 32-1: 0] sys_rdata       ;
logic            sys_err         ;
logic            sys_ack         ;

genvar k;
reg  [14-1: 0] uut_reg [0:21];
reg  [20-1: 0] uut_sum ;
integer        uut_scl ;
reg  [14-1: 0] uut_aaa ;
reg  [14-1: 0] uut_bbb ;
reg  [14-1: 0] uut_out ;
reg            uut_en  ;

generate
for (k=0; k<21; k=k+1) begin : delay
   always @(posedge clk)
      uut_reg[k+1] <= uut_reg[k] ;
end
endgenerate

always @(posedge clk) begin
   uut_reg[0] <= dat_a_out ;
   uut_sum <= $signed(dat_a_out)   + $signed(uut_reg[ 0]) + $signed(uut_reg[ 1]) + $signed(uut_reg[ 2]) + $signed(uut_reg[ 3])
            + $signed(uut_reg[ 4]) + $signed(uut_reg[ 5]) + $signed(uut_reg[ 6]) + $signed(uut_reg[ 7]) + $signed(uut_reg[ 8])
            + $signed(uut_reg[ 9]) + $signed(uut_reg[10]) + $signed(uut_reg[11]) + $signed(uut_reg[12]) + $signed(uut_reg[13])
            + $signed(uut_reg[14]) + $signed(uut_reg[15]) + $signed(uut_reg[16]) + $signed(uut_reg[17]) + $signed(uut_reg[18]) ;

   uut_scl <= $signed(uut_sum) / 20 ; // make simple avarage
   uut_aaa <= uut_scl ;
   uut_bbb <= uut_aaa ;
   uut_out <= uut_scl ;
   
   dat_a_in <= uut_en ? uut_out : 14'h0 ;
end

reg [8-1: 0] ch0_set;
reg [8-1: 0] ch1_set;

initial begin
   dat_b_in <= 14'h0 ;
   uut_en   <=  1'b0 ;

   wait (rstn)
   repeat(10) @(posedge clk);
   //PID settings
   bus.write(32'h10, 32'd7000  );  // set point
   bus.write(32'h14,-32'd3000  );  // Kp
   bus.write(32'h18, 32'd1000  );  // Ki
   bus.write(32'h1C, 32'd1000  );  // Kd

   repeat(100) @(posedge clk);
   uut_en <= 1'b1 ;

   //Int reset
   repeat(20) @(posedge clk);
   bus.write(32'h00, 32'b1110  );  // int reset

   repeat(2000000) @(posedge clk);
end

////////////////////////////////////////////////////////////////////////////////
// module instances
////////////////////////////////////////////////////////////////////////////////

sys_bus_model bus (
  // system signals
  .clk          (clk      ),
  .rstn         (rstn     ),
  // bus protocol signals
  .sys_addr     (sys_addr ),
  .sys_wdata    (sys_wdata),
  .sys_sel      (sys_sel  ),
  .sys_wen      (sys_wen  ),
  .sys_ren      (sys_ren  ),
  .sys_rdata    (sys_rdata),
  .sys_err      (sys_err  ),
  .sys_ack      (sys_ack  ) 
);

red_pitaya_pid pid (
   // signals
  .clk_i        (clk      ),  // clock
  .rstn_i       (rstn     ),  // reset - active low
  .dat_a_i      (dat_a_in ),  // in 1
  .dat_b_i      (dat_b_in ),  // in 2
  .dat_a_o      (dat_a_out),  // out 1
  .dat_b_o      (dat_b_out),  // out 2
   // System bus
  .sys_addr     (sys_addr ),
  .sys_wdata    (sys_wdata),
  .sys_sel      (sys_sel  ),
  .sys_wen      (sys_wen  ),
  .sys_ren      (sys_ren  ),
  .sys_rdata    (sys_rdata),
  .sys_err      (sys_err  ),
  .sys_ack      (sys_ack  )
);

////////////////////////////////////////////////////////////////////////////////
// waveforms
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("red_pitaya_pid_tb.vcd");
  $dumpvars(0, red_pitaya_pid_tb);
end

endmodule: red_pitaya_pid_tb

