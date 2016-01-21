////////////////////////////////////////////////////////////////////////////////
// Module: GPIO testbench
// Authors: Iztok Jeras <iztok.jeras@redpitaya.com>
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

`timescale 1ns / 1ps

module gpio_tb #(
  // time periods
  realtime  TP = 8.0ns,  // 125MHz
  // RTL config
  parameter int unsigned DW = 8 // data width
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

wire  [DW-1:0] gpio_io; // tristate
logic [DW-1:0] gpio_o;  // output
logic [DW-1:0] gpio_e;  // enable

////////////////////////////////////////////////////////////////////////////////
// test sequence
////////////////////////////////////////////////////////////////////////////////

initial begin
  repeat(8) @(posedge clk);
  axi_write (0, 'h67);
  axi_write (1, 'ha5);
  repeat(16) @(posedge clk);
  $finish();
end

task axi_write (
  input  logic [32-1:0] adr,
  input  logic [32-1:0] dat
);
  int b;
  bus_master.WriteTransaction (
    .AWDelay (0),  .aw ('{
                          addr  : adr,
                          prot  : 0
                         }),
     .WDelay (0),   .w ('{
                          data  : dat,
                          strb  : '1
                         }),
     .BDelay (0),   .b (b)
  );
endtask: axi_write


////////////////////////////////////////////////////////////////////////////////
// module instances
////////////////////////////////////////////////////////////////////////////////

axi4_lite_if bus (.ACLK (clk), .ARESETn (rstn));

axi4_lite_master bus_master (
  .intf (bus)
);

gpio #(.DW (DW)) gpio (
  .gpio_e    (gpio_e),
  .gpio_o    (gpio_o),
  .gpio_i    (gpio_io),
   // System bus
  .bus       (bus)
);

//                              tristate output  enable
bufif0 bufif0_gpio_io [DW-1:0] (gpio_io, gpio_o, gpio_e);

pullup pullup_gpio_io [DW-1:0] (gpio_io);

////////////////////////////////////////////////////////////////////////////////
// waveforms
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("gpio_tb.vcd");
  $dumpvars(0, gpio_tb);
end

endmodule: gpio_tb
