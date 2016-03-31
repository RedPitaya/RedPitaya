////////////////////////////////////////////////////////////////////////////////
// Module: Red Pitaya arbitrary signal generator testbench.
// Authors: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

`timescale 1ns / 1ps

module str_to_ram_tb #(
  // time period
  realtime  TP = 8.0ns,  // 125MHz
  // types
  type S16 = logic signed [16-1:0],
  type S14 = logic signed [14-1:0],
  int unsigned DN = 1,
  type DT = S14,
  // buffer parameters
  int unsigned AW = 14
);

////////////////////////////////////////////////////////////////////////////////
// DAC signal generation
////////////////////////////////////////////////////////////////////////////////

// syste signals
logic clk ;
logic rstn;

// control
logic ctl_rst;  // set FSM to reset

// stream
axi4_stream_if #(.DN (DN), .DT (DT)) str (.ACLK (clk), .ARESETn (rstn));

// system bus
sys_bus_if    bus  (.clk (clk), .rstn (rstn));

// DAC clock
initial        clk = 1'b0;
always #(TP/2) clk = ~clk;

// DAC reset
initial begin
  rstn = 1'b0;
  repeat(4) @(posedge clk);
  rstn = 1'b1;
end

////////////////////////////////////////////////////////////////////////////////
// signal generation
////////////////////////////////////////////////////////////////////////////////

int unsigned buf_len = 1<<AW;

initial begin
  DT dat [];
  DT rdata_blk [];
  axi4_stream_pkg::axi4_stream_class #(.DT (DT)) cli;

  ctl_rst = 1'b0;
  repeat(10) @(posedge clk);

  cli = new;
  dat = new [8];
  dat = '{0,0,1,2,2,3,3,3};
  // send data into stream
  cli.add_pkt (dat);
  fork
    str_src.run (cli);
  join

  // read table
  rdata_blk = new [buf_len];
  for (int i=0; i<buf_len; i++) begin
    busm.read(i*4, rdata_blk [i]);  // read table
  end

//  // check received data
//  error += clo.check (dtc);
//  repeat(4) @(posedge clk);

  // end simulation
  repeat(20) @(posedge clk);
  $stop();
  //$finish();
end

////////////////////////////////////////////////////////////////////////////////
// module instances
////////////////////////////////////////////////////////////////////////////////

axi4_stream_src #(.DN (DN), .DT (DT), .IV (1'bx)) str_src (.str (str));

sys_bus_model busm (.bus (bus));

str_to_ram #(
  .DN  (DN),
  .DT  (DT),
  .AW  (AW)
) str_to_ram (
  // control
  .ctl_rst  (ctl_rst),
  // stream input
  .str      (str),
  // System bus
  .bus      (bus)
);

assign str.TREADY = 1'b1;

////////////////////////////////////////////////////////////////////////////////
// waveforms
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("str_to_ram_tb.vcd");
  $dumpvars(0, str_to_ram_tb);
end

endmodule: str_to_ram_tb
