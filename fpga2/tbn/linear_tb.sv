////////////////////////////////////////////////////////////////////////////////
// Module: Linear transformation (gain, offset and saturation)
// Author: Iztok Jeras
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

`timescale 1ns / 1ps

module linear_tb #(
  // clock time periods
  realtime  TP = 4.0ns,  // 250MHz
  // stream parameters
  int unsigned DN = 1,
  type DTI = logic signed [8-1:0], // data type for input
  type DTO = logic signed [8-1:0], // data type for output
  int unsigned DWM = 16,           // data width for multiplier (gain)
  int unsigned DWS = $bits(DTO)    // data width for summation (offset)
);

typedef DTI DTI_A [];
typedef DTO DTO_A [];

// system signals
logic                  clk ;  // clock
logic                  rstn;  // reset - active low

// configuration
logic signed [DWM-1:0] cfg_mul;
logic signed [DWS-1:0] cfg_sum;

// stream input/output
axi4_stream_if #(.DT (DTI)) sti (.ACLK (clk), .ARESETn (rstn));
axi4_stream_if #(.DT (DTO)) sto (.ACLK (clk), .ARESETn (rstn));

// calibration
real gain   = 1.0;
real offset = 0.1;

int unsigned error = 0;

////////////////////////////////////////////////////////////////////////////////
// clock and test sequence
////////////////////////////////////////////////////////////////////////////////

initial        clk = 1'h0;
always #(TP/2) clk = ~clk;

initial begin
  DTI dti [];
  DTO dto [];
  axi4_stream_pkg::axi4_stream_class #(.DT (DTI)) cli;
  axi4_stream_pkg::axi4_stream_class #(.DT (DTO)) clo;

  // for now initialize configuration to an idle value
  cfg_sum = (offset * 2**(DWS-1));
  cfg_mul = (gain   * 2**(DWM-2));

  // initialization
  rstn = 1'b0;
  repeat(4) @(posedge clk);
  // start
  rstn = 1'b1;
  repeat(4) @(posedge clk);

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

// calculate linear transformation
// TODO: implement actual calculation
function automatic DTO_A calc (ref DTI_A dat);
  calc = new [dat.size()] (dat);
endfunction: calc

////////////////////////////////////////////////////////////////////////////////
// module instance
////////////////////////////////////////////////////////////////////////////////

axi4_stream_src #(.DT (DTI)) str_src (.str (sti));

linear #(
  .DTI (DTI),
  .DTO (DTO),
  .DWM (DWM),
  .DWS (DWS)
) linear (
  // stream input/output
  .sti      (sti    ),
  .sto      (sto    ),
  // configuration
  .cfg_mul  (cfg_mul),
  .cfg_sum  (cfg_sum)
);

axi4_stream_drn #(.DT (DTO)) str_drn (.str (sto));

////////////////////////////////////////////////////////////////////////////////
// waveforms
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("linear_tb.vcd");
  $dumpvars(0, linear_tb);
end

endmodule: linear_tb
