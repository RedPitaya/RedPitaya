////////////////////////////////////////////////////////////////////////////////
// Module: Linear transformation (gain, offset and saturation)
// Author: Iztok Jeras
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

`timescale 1ns / 1ps

module scope_filter_tb #(
  // clock time periods
  realtime  TP = 8.0ns,  // 125MHz
  // stream parameters
  int unsigned DN = 1,
  type DTI = logic signed [16-1:0],   // data width for input
  type DTO = logic signed [16-1:0]    // data width for output
);

typedef DTI DTI_A []; 
typedef DTO DTO_A []; 

// system signals
logic                  clk ;  // clock
logic                  rstn;  // reset - active low

// configuration
logic signed [ 18-1:0] cfg_aa;   // config AA coefficient
logic signed [ 25-1:0] cfg_bb;   // config BB coefficient
logic signed [ 25-1:0] cfg_pp;   // config PP coefficient
logic signed [ 25-1:0] cfg_kk;   // config KK coefficient

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
  input  cfg_aa;
  input  cfg_bb;
  input  cfg_pp;
  input  cfg_kk;
endclocking: cb

////////////////////////////////////////////////////////////////////////////////
// test sequence
////////////////////////////////////////////////////////////////////////////////

initial begin
  // for now initialize configuration to an idle value
  cfg_aa <= 'h7D93;
  cfg_bb <= 'h437C7;
  cfg_pp <= 'h2666;
  cfg_kk <= 'hd9999a;
  // initialization
  rstn <= 1'b0;
  ##4;
  rstn <= 1'b1;
  ##4;

  test_block;

  // end simulation
  ##4;
  $finish();
end

// calculate filter
// TODO: implement actual calculation
function automatic DTO_A calc (ref DTI_A dat);
  calc = new [dat.size()] (dat);
endfunction: calc

////////////////////////////////////////////////////////////////////////////////
// tests
////////////////////////////////////////////////////////////////////////////////

task test_block;
  DTI dti [];
  DTO dto [];
  axi4_stream_pkg::axi4_stream_class #(.DT (DTI)) cli;
  axi4_stream_pkg::axi4_stream_class #(.DT (DTI)) clo;
  // prepare data
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
//  error += clo.check (dto);
endtask: test_block

////////////////////////////////////////////////////////////////////////////////
// module instance
////////////////////////////////////////////////////////////////////////////////

axi4_stream_src #(.DN (DN), .DT (DTI)) str_src (.str (sti));

scope_filter #(
  .DTI (DTI),
  .DTO (DTO)
) scope_filter (
  // stream input/output
  .sti      (sti),
  .sto      (sto),
  // configuration
  .cfg_aa   (cfg_aa),
  .cfg_bb   (cfg_bb),
  .cfg_kk   (cfg_kk),
  .cfg_pp   (cfg_pp),
  // control
  .ctl_rst  ()
);

red_pitaya_dfilt1 dfilt1 (
   // ADC
   .adc_clk_i  (clk ),
   .adc_rstn_i (rstn),
   .adc_dat_i  (sti.TDATA[0][14-1:0]),
   .adc_dat_o  (),
   // configuration
   .cfg_aa_i   (cfg_aa),
   .cfg_bb_i   (cfg_bb),
   .cfg_kk_i   (cfg_kk),
   .cfg_pp_i   (cfg_pp)
);

axi4_stream_drn #(.DN (DN), .DT (DTO)) str_drn (.str (sto));

////////////////////////////////////////////////////////////////////////////////
// waveforms
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("scope_filter_tb.vcd");
  $dumpvars(0, scope_filter_tb);
end

endmodule: scope_filter_tb
