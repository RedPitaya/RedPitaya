////////////////////////////////////////////////////////////////////////////////
// Module: Linear adder
// Author: Iztok Jeras
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

`timescale 1ns / 1ps

module lin_add_tb #(
  // clock time periods
  realtime  TP = 4.0ns,  // 250MHz
  // stream parameters
  int unsigned DN = 1,
  type DTI = logic signed [8-1:0], // data type for input
  type DTO = logic signed [8-1:0], // data type for output
  type DTS = DTI                   // data type for configuration
);

typedef DTI DTI_A [];
typedef DTO DTO_A [];

// system signals
logic clk ;  // clock
logic rstn;  // reset - active low

// configuration
DTS cfg_sum;

// stream input/output
axi4_stream_if #(.DT (DTI)) sti (.ACLK (clk), .ARESETn (rstn));
axi4_stream_if #(.DT (DTO)) sto (.ACLK (clk), .ARESETn (rstn));

// calibration
real offset = -0.95;

int unsigned error = 0;

////////////////////////////////////////////////////////////////////////////////
// clock
////////////////////////////////////////////////////////////////////////////////

initial        clk = 1'h0;
always #(TP/2) clk = ~clk;

// clocking 
default clocking cb @ (posedge clk);
  input  rstn;
  input  cfg_sum;
endclocking: cb

////////////////////////////////////////////////////////////////////////////////
// test sequence
////////////////////////////////////////////////////////////////////////////////

initial begin
  DTI dti [];
  DTO dto [];
  axi4_stream_pkg::axi4_stream_class #(.DT (DTI)) cli;
  axi4_stream_pkg::axi4_stream_class #(.DT (DTO)) clo;

  // for now initialize configuration to an idle value
  cfg_sum = (offset * 2**($bits(DTS)-1));

  // initialization
  rstn = 1'b0;
  ##4;
  // start
  rstn = 1'b1;
  ##4;

  // send data into stream
  cli = new;
  clo = new;
  dti = cli.range (-8, 8);
  dto = calc(dti);
  // send data into stream
  cli.add_pkt (dti);
  clo.add_pkt (dto);
  fork
    str_src.run (cli);
    str_drn.run (clo);
  join
  // check received data
  error += clo.check (dto);

  // end simulation
  repeat(4) @(posedge clk);
  if (error)  $display("FAILURE");
  else        $display("SUCCESS");
  $finish();
end

// calculate lin_add transformation
// TODO: implement actual calculation
function automatic DTO_A calc (ref DTI_A dat);
  calc = new [dat.size()] (dat);
endfunction: calc

////////////////////////////////////////////////////////////////////////////////
// module instance
////////////////////////////////////////////////////////////////////////////////

axi4_stream_src #(.DT (DTI)) str_src (.str (sti));

lin_add #(
  .DTI (DTI),
  .DTO (DTO),
  .DTS (DTS)
) lin_add (
  // stream input/output
  .sti      (sti    ),
  .sto      (sto    ),
  // configuration
  .cfg_sum  (cfg_sum)
);

axi4_stream_drn #(.DT (DTO)) str_drn (.str (sto));

////////////////////////////////////////////////////////////////////////////////
// waveforms
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("lin_add_tb.vcd");
  $dumpvars(0, lin_add_tb);
end

endmodule: lin_add_tb
