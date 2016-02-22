////////////////////////////////////////////////////////////////////////////////
// Module: RLE (Run Length Encoding) compression
// Author: Iztok Jeras
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

`timescale 1ns / 1ps

module rle_tb #(
  // clock time periods
  realtime  TP = 4.0ns,  // 250MHz
  // counter properties
  int unsigned CW = 8,
  // stream parameters
  int unsigned DN = 1,
  int unsigned DW = 8,
  type DTI = logic [   DW-1:0], // data type for input
  type DTO = logic [CW+DW-1:0]  // data type for output
);

////////////////////////////////////////////////////////////////////////////////
// RLE compression/decompression
////////////////////////////////////////////////////////////////////////////////

// compressed data type
typedef struct packed {
  logic [CW-1:0] cnt;
  logic [DW-1:0] dat;
} DTC;

// dynamic array data types
typedef DTI DTI_A [];
typedef DTC DTC_A [];

// compression counter
function automatic int unsigned rle_compress_cnt (ref DTI_A dat);
  DTI tmp;
  bit [CW-1:0] cnt;
  int unsigned j;
  j=0;
  tmp = dat[0];
  for (int unsigned i=1; i<dat.size(); i++) begin
    if ((dat[i] == tmp) && (~&cnt)) begin
      cnt++;
    end else begin
      j++;
    end
    tmp=dat[i];
  end
  return j+1;
endfunction: rle_compress_cnt

// compression
function automatic DTC_A rle_compress (ref DTI_A dat);
  int unsigned j;
  // output array memory allocation
  DTC_A dtc;
  dtc = new [rle_compress_cnt(dat)];
  // output array populate
  j=0;
  dtc[j] = '{cnt: 0, dat: dat[0]};
  for (int unsigned i=1; i<dat.size(); i++) begin
    if ((dat[i] == dtc[j].dat) && (~&dtc[j].cnt)) begin
      dtc[j].cnt++;
    end else begin
      j++;
      dtc[j] = '{cnt: 0, dat: dat[i]};
    end
  end
  return dtc;
endfunction: rle_compress

// decompression counter
function automatic int unsigned rle_decompress_cnt (ref DTC_A dtc);
  int unsigned j;
  j=0;
  for (int unsigned i=0; i<dtc.size(); i++) begin
    j += dtc[i].cnt + 1;
  end
  return j;
endfunction: rle_decompress_cnt

// decompression
function automatic DTI_A rle_decompress (ref DTC_A dtc);
  int unsigned j;
  // output array memory allocation
  DTI_A dat;
  dat = new [rle_decompress_cnt(dtc)];
  // output array populate
  j=0;
  for (int unsigned i=0; i<dtc.size(); i++) begin
    for (int unsigned cnt=0; cnt<=dtc[i].cnt; cnt++) begin
      dat[j] = dtc[i].dat;
      j++;
    end
  end
  return dat;
endfunction: rle_decompress

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

// system signals
logic clk ;  // clock
logic rstn;  // reset - active low

// configuration
logic ctl_rst;  // reset
logic cfg_ena;  // enable

// stream input/output
axi4_stream_if #(.DAT_T (DTI)) sti (.ACLK (clk), .ARESETn (rstn));
axi4_stream_if #(.DAT_T (DTO)) sto (.ACLK (clk), .ARESETn (rstn));

// error counter
int unsigned error = 0;

////////////////////////////////////////////////////////////////////////////////
// clock and test sequence
////////////////////////////////////////////////////////////////////////////////

initial        clk = 1'h0;
always #(TP/2) clk = ~clk;

initial begin
  // for now initialize configuration to an idle value
  ctl_rst = 1'b0;
  cfg_ena = 1'b1;

  // function tests
  test_compress();
  test_decompress();

  // initialization
  rstn = 1'b0;
  repeat(4) @(posedge clk);
  // start
  rstn = 1'b1;
  repeat(4) @(posedge clk);

  // RTL tests
  test_rle();
  test_bypass();

  // end simulation
  repeat(4) @(posedge clk);
  $finish();
end

task test_compress ();
  DTI_A dat;
  DTC_A dtc;
  $display ("TEST: compression function");
  dat = new [8];
  dat = '{0,0,1,2,2,3,3,3};
  dtc = rle_compress (dat);
  $display ("dat [%d] = %p", dat.size(), dat);
  $display ("dtc [%d] = %p", dtc.size(), dtc);
endtask: test_compress

task test_decompress ();
  DTC_A dtc;
  DTI_A dat;
  $display ("TEST: decompression function");
  dtc = new [4];
  dtc = '{'{0,4},'{1,5},'{2,6},'{3,7}};
  dat = rle_decompress (dtc);
  $display ("dtc [%d] = %p", dtc.size(), dtc);
  $display ("dat [%d] = %p", dat.size(), dat);
endtask: test_decompress

task test_rle ();
  DTI_A dat;
  DTC_A dtc;
  $display ("TEST: rle RTL");
  dat = new [8];
  dat = '{0,0,1,2,2,3,3,3};
  dtc = rle_compress (dat);
  $display ("dat [%d] = %p", dat.size(), dat);
  $display ("dtc [%d] = %p", dtc.size(), dtc);
  // send data into stream
  for (int i=0; i<dat.size(); i++) begin
    str_src.put(dat[i], '1, 1'b0);
  end
  repeat(dat.size()+4) @(posedge clk);

  // check received data
  for (int i=0; i<dtc.size(); i++) begin
    DTC   [DN-1:0] dto;
    logic [DN-1:0] kep;
    logic          lst;
    int unsigned   tmg;

    str_drn.get(dto, kep, lst, tmg);
    if (dtc[i] != dto) begin
      $display ("Error: i=%d: (out=%p) != (ref=%p)", i, dtc[i], dto);
      error++;
    end
  end

  repeat(4) @(posedge clk);
endtask: test_rle

task test_bypass ();
  DTI_A dat;
  $display ("TEST: rle bypass");
  dat = new [8];
  dat = '{0,0,1,2,2,3,3,3};
  $display ("dat [%d] = %p", dat.size(), dat);
  // disable RLE (enable bypass
  cfg_ena = 1'b0;
  repeat(4) @(posedge clk);
  // send data into stream
  for (int i=0; i<dat.size(); i++) begin
    str_src.put(dat[i], '1, 1'b0);
  end
  repeat(dat.size()+4) @(posedge clk);

  // check received data
  for (int i=0; i<dat.size(); i++) begin
    DTC   [DN-1:0] dto;
    logic [DN-1:0] kep;
    logic          lst;
    int unsigned   tmg;

    str_drn.get(dto, kep, lst, tmg);
    if (DTC'{CW'(0), dat[i]} != dto) begin
      $display ("Error: i=%d: (out=%p) != (ref=%p)", i, dat[i], dto);
      error++;
    end
  end

  repeat(4) @(posedge clk);
endtask: test_bypass

////////////////////////////////////////////////////////////////////////////////
// module instance
////////////////////////////////////////////////////////////////////////////////

axi4_stream_src #(.DN (DN), .DAT_T (DTI)) str_src (.str (sti));

rle #(
  .DN  (DN),
  .DTI (DTI),
  .DTO (DTO),
  .CW  (CW)
) rle (
  // stream input/output
  .sti      (sti    ),
  .sto      (sto    ),
  // configuration/control
  .ctl_rst  (ctl_rst),
  .cfg_ena  (cfg_ena)
);

axi4_stream_drn #(.DN (DN), .DAT_T (DTO)) str_drn (.str (sto));

////////////////////////////////////////////////////////////////////////////////
// waveforms
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("rle_tb.vcd");
  $dumpvars(0, rle_tb);
end

endmodule: rle_tb
