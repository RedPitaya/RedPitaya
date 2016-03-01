/**
 * $Id: red_pitaya_hk_tb.v 961 2014-01-21 11:40:39Z matej.oblak $
 *
 * @brief Red Pitaya house keeping testbench.
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
 * Testbench for house keeping module.
 *
 * Simple testbench to test register read and write. Testing expansion connector
 * and DNA macro.
 * 
 */

`timescale 1ns / 1ps

module red_pitaya_hk_tb #(
  // time periods
  realtime  TP = 8.0ns,  // 125MHz
  // RTL config
  parameter DWL = 8, // data width for LED
  parameter DWE = 8, // data width for extension
  parameter [57-1:0] DNA = 57'h0823456789ABCDE
);

glbl glbl ();

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

// tristate signals
wire  [  8-1: 0] led    ;
wire  [  8-1: 0] exp_p_dat;
wire  [  8-1: 0] exp_n_dat;
//               output       input        enable
logic [  8-1: 0] exp_p_dat_o, exp_p_dat_i, exp_p_dat_e;
logic [  8-1: 0] exp_n_dat_o, exp_n_dat_i, exp_n_dat_e;

pullup   pullup_exp_dat   [8-1:0] (exp_p_dat);
pulldown pulldown_exp_dat [8-1:0] (exp_n_dat);

////////////////////////////////////////////////////////////////////////////////
// test sequence
////////////////////////////////////////////////////////////////////////////////

logic [ 32-1: 0] sys_addr ;
logic [ 32-1: 0] sys_wdata;
logic [  4-1: 0] sys_sel  ;
logic            sys_wen  ;
logic            sys_ren  ;
logic [ 32-1: 0] sys_rdata;
logic            sys_err  ;
logic            sys_ack  ;

int unsigned error = 0;

logic [32-1:0] wdata;
logic [32-1:0] rdata;
logic [57-1:0] dna;

initial begin
  wait (rstn)
  repeat(600) @(posedge clk);
    // LED
    wdata = 32'h000000_03;
    bus.write(32'h30, wdata);
    if (led != (wdata[DWL-1:0] & 8'h01))  error++;

    // EXPANSION P
    bus.write(32'h10, 32'h000000_33); // direction
    bus.write(32'h18, 32'h000000_0F); // value

    // read ID value
    bus.read(32'h00, rdata);
    $display ("ID = 0x%08x", rdata);

    // read DNA [57-1:0] value
    bus.read(32'h04, rdata); // lower part
    dna [32-1:00] = rdata [   32-1:0];
    bus.read(32'h08, rdata); // higher part
    dna [57-1:32] = rdata [57-32-1:0];
    $display ("DNA = 0x%016x", dna);

    bus.read(32'h24, rdata);
    bus.read(32'h30, rdata);
  repeat(20000) @(posedge clk);
  $finish();
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

red_pitaya_hk hk (
  .clk_i          (clk      ),
  .rstn_i         (rstn     ),
   // LED
  .led_o          (led      ),
  // Expansion connector
  .exp_p_dat_i    (exp_p_dat_i),
  .exp_p_dat_o    (exp_p_dat_o),
  .exp_p_dir_o    (exp_p_dat_e),
  .exp_n_dat_i    (exp_n_dat_i),
  .exp_n_dat_o    (exp_n_dat_o),
  .exp_n_dir_o    (exp_n_dat_e),
   // System bus
  .sys_addr       (sys_addr ),
  .sys_wdata      (sys_wdata),
  .sys_sel        (sys_sel  ),
  .sys_wen        (sys_wen  ),
  .sys_ren        (sys_ren  ),
  .sys_rdata      (sys_rdata),
  .sys_err        (sys_err  ),
  .sys_ack        (sys_ack  )
);

//                               tristate   output       enable
bufif0 bufif0_exp_p_dat [8-1:0] (exp_p_dat, exp_p_dat_o, exp_p_dat_e);
bufif0 bufif0_exp_n_dat [8-1:0] (exp_n_dat, exp_n_dat_o, exp_n_dat_e);
//     input         tristate
assign exp_p_dat_i = exp_p_dat;
assign exp_n_dat_i = exp_n_dat;

////////////////////////////////////////////////////////////////////////////////
// waveforms
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("red_pitaya_hk_tb.vcd");
  $dumpvars(0, red_pitaya_hk_tb);
end

endmodule: red_pitaya_hk_tb
