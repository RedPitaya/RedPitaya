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
 * Simple testbench to test register read and write. Testing DNA macro.
 * 
 */

`timescale 1ns / 1ps

module red_pitaya_hk_tb #(
  // time periods
  realtime  TP = 8.0ns,  // 125MHz
  // RTL config
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

////////////////////////////////////////////////////////////////////////////////
// test sequence
////////////////////////////////////////////////////////////////////////////////

logic [ 32-1: 0] sys_addr ;
logic [ 32-1: 0] sys_wdata;
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
    // read ID value
    bus.read(32'h00, rdata);
    $display ("ID = 0x%08x", rdata);

    // read DNA [57-1:0] value
    bus.read(32'h04, rdata); // lower part
    dna [32-1:00] = rdata [   32-1:0];
    bus.read(32'h08, rdata); // higher part
    dna [57-1:32] = rdata [57-32-1:0];
    $display ("DNA = 0x%016x", dna);
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
  .sys_wen      (sys_wen  ),
  .sys_ren      (sys_ren  ),
  .sys_rdata    (sys_rdata),
  .sys_err      (sys_err  ),
  .sys_ack      (sys_ack  ) 
);

red_pitaya_hk hk (
  .clk_i          (clk      ),
  .rstn_i         (rstn     ),
  // global configuration
  .digital_loop   (),
  // System bus
  .sys_addr       (sys_addr ),
  .sys_wdata      (sys_wdata),
  .sys_wen        (sys_wen  ),
  .sys_ren        (sys_ren  ),
  .sys_rdata      (sys_rdata),
  .sys_err        (sys_err  ),
  .sys_ack        (sys_ack  )
);

////////////////////////////////////////////////////////////////////////////////
// waveforms
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("red_pitaya_hk_tb.vcd");
  $dumpvars(0, red_pitaya_hk_tb);
end

endmodule: red_pitaya_hk_tb
