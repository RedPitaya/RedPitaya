////////////////////////////////////////////////////////////////////////////////
// Module: Stream to MM
// Author: Iztok Jeras
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

`timescale 1ns / 1ps

module str2mm_tb #(
  // clock time periods
  realtime  TP = 4.0ns,  // 250MHz
  // parameters
  int unsigned TN = 1,   // trigger number
  int unsigned TW = 64,  // time width
  int unsigned CW = 32,  // counter width
  // data bus type
  int unsigned DN = 1,
  type DT = logic signed [14-1:0]
);

typedef DT DT_A [];

// system signals
logic          clk ;  // clock
logic          rstn;  // reset - active low
logic          clr ;  // synchronous clear

// stream input/output
axi4_stream_if #(.DN (DN), .DT (DT)) str (.ACLK (clk), .ARESETn (rstn));
// system bus
sys_bus_if bus (.clk (clk), .rstn (rstn));

int unsigned error = 0;

////////////////////////////////////////////////////////////////////////////////
// clock and time stamp
////////////////////////////////////////////////////////////////////////////////

initial        clk = 1'h0;
always #(TP/2) clk = ~clk;

// clocking 
default clocking cb @ (posedge clk);
  input  rstn;
  output clr;
endclocking: cb

////////////////////////////////////////////////////////////////////////////////
// test sequence
////////////////////////////////////////////////////////////////////////////////

initial begin
  // initialization
  rstn <= 1'b0;
  ##4;
  rstn <= 1'b1;
  ##4;

  // tests
  test_block;
  test_pass;

  // end simulation
  ##4;
  if (error)  $display("FAILURE");
  else        $display("SUCCESS");
  $finish();
end

////////////////////////////////////////////////////////////////////////////////
// tests
////////////////////////////////////////////////////////////////////////////////

task test_block;
  DT dti [];
  axi4_stream_pkg::axi4_stream_class #(.DT (DT)) cli;
  // prepare data
  cli = new;
  dti = cli.range (-8, 8);
  // add packet into queue
  cli.add_pkt (dti);
  str_src.run (cli);
endtask: test_block

task test_pass;
  DT dti [];
  axi4_stream_pkg::axi4_stream_class #(.DT (DT)) cli;
  // prepare data
  cli = new;
  dti = cli.range (-8, 8);
  // add packet into queue
  cli.add_pkt (dti);
  str_src.run (cli);
endtask: test_pass

////////////////////////////////////////////////////////////////////////////////
// helper tasks
////////////////////////////////////////////////////////////////////////////////

// generate reset pulse
task clr_pls ();
  cb.clr <= 1'b1;
  ##1;
  cb.clr <= 1'b0;
endtask: clr_pls

////////////////////////////////////////////////////////////////////////////////
// module instance
////////////////////////////////////////////////////////////////////////////////

axi4_stream_src #(.DN (DN), .DT (DT)) str_src (.str (str));

sys_bus_model busm (.bus (bus));

str2mm #(
  .DT (DT),
  .DN (DN),
  .DL (1<<8)
) str2mm (
  // stream input/output
  .str      (str),
  .bus      (bus),
  .clr      (clr)
);

////////////////////////////////////////////////////////////////////////////////
// waveforms
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("str2mm_tb.vcd");
  $dumpvars(0, str2mm_tb);
end

endmodule: str2mm_tb
