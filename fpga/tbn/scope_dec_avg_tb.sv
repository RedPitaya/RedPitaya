////////////////////////////////////////////////////////////////////////////////
// Module: Linear transformation (gain, offset and saturation)
// Author: Iztok Jeras
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

`timescale 1ns / 1ps

module scope_dec_avg_tb #(
  // clock time periods
  realtime  TP = 8.0ns,  // 125MHz
  // stream parameters
  int unsigned DN = 1,
  type DTI = logic signed [16-1:0],  // data width for input
  type DTO = logic signed [16-1:0],  // data width for output
  // decimation parameters
  int unsigned DCW = 17,  // data width for counter
  int unsigned DSW =  4   // data width for shifter
);

typedef DTI DTI_A []; 
typedef DTO DTO_A []; 

// system signals
logic                  clk ;  // clock
logic                  rstn;  // reset - active low

// configuration
logic                  ctl_rst;
logic                  cfg_avg;  // averaging enable
logic        [DCW-1:0] cfg_dec;  // decimation factor
logic        [DSW-1:0] cfg_shr;  // shift right

// stream input/output
axi4_stream_if #(.DT (DTI)) sti (.ACLK (clk), .ARESETn (rstn));
axi4_stream_if #(.DT (DTO)) sto (.ACLK (clk), .ARESETn (rstn));

////////////////////////////////////////////////////////////////////////////////
// clock
////////////////////////////////////////////////////////////////////////////////

initial        clk = 1'h0;
always #(TP/2) clk = ~clk;

// clocking 
default clocking cb @ (posedge clk);
  input  rstn;
  input  ctl_rst;
  input  cfg_avg;
  input  cfg_dec;
  input  cfg_shr;
endclocking: cb

////////////////////////////////////////////////////////////////////////////////
// test sequence
////////////////////////////////////////////////////////////////////////////////

// timeout
initial begin
  repeat(10000) @(posedge clk);
  $finish();
end

initial begin
  // for now initialize configuration to an idle value
  ctl_rst <= 1'b0;
  cfg_avg <= 1'b0;
  cfg_dec <= 0;
  cfg_shr <= 0;
  // initialization
  rstn <= 1'b0;
  ##4;
  rstn <= 1'b1;
  ##4;

  test_block (2);

  // end simulation
  ##4;
  $finish();
end

// calculate filter
// TODO: implement actual calculation
function automatic DTO_A calc (
  ref DTI_A dat,
  input int unsigned dec
);
  int unsigned j = 0;
  calc = new [dat.size()/dec];
  for (int unsigned i=0; i<dat.size(); i+=dec) begin
    calc [j] = dat [i];
    j++;
  end
endfunction: calc

////////////////////////////////////////////////////////////////////////////////
// tests
////////////////////////////////////////////////////////////////////////////////

task test_block (
  input int unsigned dec = 1
);
  DTI dti [];
  DTO dto [];
  axi4_stream_pkg::axi4_stream_class #(.DT (DTI)) cli;
  axi4_stream_pkg::axi4_stream_class #(.DT (DTI)) clo;
  // prepare data
  cli = new;
  clo = new;
  dti = cli.range (-8, 8);
  dto = calc(dti, dec);
  // send data into stream
  cli.add_pkt (dti);
  clo.add_pkt (dto);
  cfg_dec <= dec-1;
  fork
    str_src.run (cli);
    str_drn.run (clo);
  join
  // check received data
//  error += clo.check (dto);
endtask: test_block

////////////////////////////////////////////////////////////////////////////////
// module instance
////////////////////////////////////////////////////////////////////////////////

axi4_stream_src #(.DN (DN), .DT (DTI)) str_src (.str (sti));

scope_dec_avg #(
  .DTI (DTI),
  .DTO (DTO),
  // decimation parameters
  .DCW (DCW),
  .DSW (DSW)
) scope_dec_avg (
  // stream input/output
  .sti      (sti),
  .sto      (sto),
  // configuration
  .cfg_avg  (cfg_avg),
  .cfg_dec  (cfg_dec),
  .cfg_shr  (cfg_shr),
  // control
  .ctl_rst  (ctl_rst)
);

axi4_stream_drn #(.DN (DN), .DT (DTO)) str_drn (.str (sto));

////////////////////////////////////////////////////////////////////////////////
// waveforms
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("scope_dec_avg_tb.vcd");
  $dumpvars(0, scope_dec_avg_tb);
end

endmodule: scope_dec_avg_tb
