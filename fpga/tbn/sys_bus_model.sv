////////////////////////////////////////////////////////////////////////////////
// Red Pitaya system bus model.
// Author Matej Oblak
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

/**
 * GENERAL DESCRIPTION:
 *
 * Simple model of Red Pitaya system bus.
 *
 * In order to simplify other testbenches this model was written. It has two
 * task, one for write other for read. Both serves as replacement for PS-AXI
 * control of Red Pitaya system bus.
 * 
 */

`timescale 1ns / 1ps

interface sys_bus_model #(
  type DT = logic [32-1:0],  // data type
  type AT = logic [32-1:0]   // address type
)(
  sys_bus_if.m bus
);

// clocking 
default clocking clk @ (posedge bus.clk);
  input  rstn  = bus.rstn ;
  output wen   = bus.wen  ;
  output ren   = bus.ren  ;
  output addr  = bus.addr ;
  output wdata = bus.wdata;
  input  rdata = bus.rdata;
  input  ack   = bus.ack  ;
  input  err   = bus.err  ;
endclocking: clk

initial begin
  bus.wen <= 1'b0;
  bus.ren <= 1'b0;
end

// bus write transfer
task transaction (
  input  logic we,
  input  AT addr,
  input  DT wdata,
  output DT rdata
);
  ##1;
  bus.wen    <=  we;
  bus.ren    <= ~we;
  bus.addr   <= addr;
  bus.wdata  <= wdata;
  ##1;
  bus.wen    <= 1'b0;
  bus.ren    <= 1'b0;
  while (~bus.ack & ~bus.err)
  ##1;
  rdata <= bus.rdata;
  ##1;
endtask: transaction

// bus write transfer
task write (
  input  AT addr,
  input  DT wdata
);
  logic [32-1:0] rdata;
  transaction (.we (1'b1), .addr (addr), .wdata (wdata), .rdata (rdata));
endtask: write

// bus read transfer
task read (
  input  AT addr,
  output DT rdata
);
  transaction (.we (1'b0), .addr (addr), .wdata ('x), .rdata (rdata));
endtask: read

endinterface: sys_bus_model
