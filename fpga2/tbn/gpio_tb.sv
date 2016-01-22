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

logic [32-1:0] tmp;

// timeout
initial begin
  repeat(100) @(posedge clk);
  $finish();
end

initial begin
  repeat(8) @(posedge clk);
  axi_write ('h00, 'h67);
  axi_write ('h04, 'ha5);
  axi_read  ('h00, tmp);
  axi_read  ('h04, tmp);
  repeat(16) @(posedge clk);
  $finish();
end

task axi_write (
  input  logic [32-1:0] adr,
  input  logic [32-1:0] dat
);
  int b;
  bus_master.WriteTransaction (
    .AWDelay (1),  .aw ('{
                          addr: adr,
                          prot: 0
                         }),
     .WDelay (0),   .w ('{
                          data: dat,
                          strb: '1
                         }),
     .BDelay (0),   .b (b)
  );
endtask: axi_write

task axi_read (
  input  logic [32-1:0] adr,
  output logic [32-1:0] dat
);
  logic [32+2-1:0] r;
  bus_master.ReadTransaction (
    .ARDelay (0),  .ar ('{
                          addr: adr,
                          prot: 0
                         }),
     .RDelay (0),   .r (r)
//     .RDelay (0),   .r ('{
//                          data: dat,
//                          resp: rsp
//                         })
  );
  dat = r >> 2;
endtask: axi_read

////////////////////////////////////////////////////////////////////////////////
// module instances
////////////////////////////////////////////////////////////////////////////////

axi4_lite_if axi4_lite (.ACLK (clk), .ARESETn (rstn));

axi4_lite_master bus_master (
  .intf (axi4_lite)
);

gpio #(.DW (DW)) gpio (
  .gpio_e    (gpio_e),
  .gpio_o    (gpio_o),
  .gpio_i    (gpio_io),
   // System bus
  .bus       (axi4_lite)
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
