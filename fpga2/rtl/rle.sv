////////////////////////////////////////////////////////////////////////////////
// Module: RLE (Run Length Encoding) compression
// Author: Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
// GENERAL DESCRIPTION:
//
// A RLE compression is applied to the signal.
//
////////////////////////////////////////////////////////////////////////////////

module rle #(
  // counter properties
  int unsigned CW = 8,
  // stream properties
  int unsigned DN = 1,
  type DTI = logic [   8-1:0], // data type for input
  type DTO = logic [CW+8-1:0]  // data type for output
)(
  // input stream input/output
  axi4_stream_if.d sti,  // input
  axi4_stream_if.s sto,  // output
  // configuration
  input  logic     ctl_rst,  // reset
  input  logic     cfg_ena   // enable
);

////////////////////////////////////////////////////////////////////////////////
// local variables
////////////////////////////////////////////////////////////////////////////////

// old values
axi4_stream_if #(.DN (1), .DAT_T (DTI)) old (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));

// comparator
logic cmp;

// counter
logic [CW-1:0] cnt;
logic [CW-1:0] nxt;
logic          max;

// compression
logic trn;

////////////////////////////////////////////////////////////////////////////////
// store previous value
////////////////////////////////////////////////////////////////////////////////

assign sti.TREADY = old.TREADY | ~old.TVALID;

always_ff @(posedge sti.ACLK)
if (sti.transf) begin
  old.TDATA  <= sti.TDATA;
  old.TKEEP  <= sti.TKEEP;
end

always_ff @(posedge sti.ACLK)
if (~sti.ARESETn) begin
  old.TLAST  <= 1'b1;
end else if (sti.transf) begin
  old.TLAST  <= sti.TLAST;
end

always_ff @(posedge sti.ACLK)
if (~sti.ARESETn) begin
  old.TVALID <= 1'b0;
end else if (sti.TREADY) begin
  old.TVALID <= sti.TVALID;
end

////////////////////////////////////////////////////////////////////////////////
// comparator
////////////////////////////////////////////////////////////////////////////////

assign cmp = (old.TDATA == sti.TDATA);

////////////////////////////////////////////////////////////////////////////////
// counter
////////////////////////////////////////////////////////////////////////////////

assign nxt = cnt + 1;
assign max = &cnt;

always_ff @(posedge sti.ACLK)
if (~sti.ARESETn)     cnt <= '0;
else if (sti.transf)  cnt <= trn ? '0 : nxt;

assign trn = (old.TVALID & (~cmp | max)) | old.TLAST; 

////////////////////////////////////////////////////////////////////////////////
// compression
////////////////////////////////////////////////////////////////////////////////

assign old.TREADY = sto.TREADY | ~sto.TVALID;

always_ff @(posedge sti.ACLK)
if (~sti.ARESETn)     sto.TVALID <= 1'b0;
else if (old.transf)  sto.TVALID <= trn;

always_ff @(posedge sti.ACLK)
if (old.transf) begin
  sto.TLAST <= old.TLAST;
  sto.TKEEP <= '1;
  sto.TDATA <= {cnt, old.TDATA};
end

endmodule: rle
