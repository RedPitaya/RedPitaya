////////////////////////////////////////////////////////////////////////////////
// Module: Linear transformation (gain, offset and saturation)
// Author: Iztok Jeras
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

`timescale 1ns / 1ps

module str_dec_tb #(
  // clock time periods
  realtime  TP = 4.0ns,  // 250MHz
  // stream parameters
  type DT = logic [8-1:0], // data type for input
  int unsigned CW = 17  // counter width
);

typedef DT DT_A [];

// system signals
logic clk ;  // clock
logic rstn;  // reset - active low

// control
logic          ctl_rst;  // synchronous reset
// configuration
logic [CW-1:0] cfg_dec;  // decimation factor

// stream input/output
axi4_stream_if #(.DT (DT)) sti (.ACLK (clk), .ARESETn (rstn));
axi4_stream_if #(.DT (DT)) sto (.ACLK (clk), .ARESETn (rstn));

int unsigned error = 0;

////////////////////////////////////////////////////////////////////////////////
// clock and test sequence
////////////////////////////////////////////////////////////////////////////////

initial        clk = 1'h0;
always #(TP/2) clk = ~clk;

initial begin
  DT dti [];
  DT dto [];
  axi4_stream_pkg::axi4_stream_class #(.DT (DT)) cli;
  axi4_stream_pkg::axi4_stream_class #(.DT (DT)) clo;

  // for now initialize configuration to an idle value
  ctl_rst = 1'b0;
  cfg_dec = 0;

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
function automatic DT_A calc (ref DT_A dat);
  calc = new [dat.size()] (dat);
endfunction: calc

////////////////////////////////////////////////////////////////////////////////
// module instance
////////////////////////////////////////////////////////////////////////////////

axi4_stream_src #(.DT (DT)) str_src (.str (sti));

str_dec #(
  .CW (CW)
) str_dec (
  // control
  .ctl_rst  (ctl_rst),
  // configuration
  .cfg_dec  (cfg_dec),
  // stream input/output
  .sti      (sti    ),
  .sto      (sto    )
);

axi4_stream_drn #(.DT (DT)) str_drn (.str (sto));

////////////////////////////////////////////////////////////////////////////////
// waveforms
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("str_dec_tb.vcd");
  $dumpvars(0, str_dec_tb);
end

endmodule: str_dec_tb
