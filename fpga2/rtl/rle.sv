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
axi4_stream_if #(.DN (1), .DT (DTI)) old (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));

// comparator
logic cmp;

// counter
logic [CW-1:0] cnt;
logic [CW-1:0] nxt;
logic          max;

// new compressed value
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
end else begin
  if (ctl_rst) begin
    old.TLAST  <= 1'b1;
  end else if (sti.transf) begin
    old.TLAST  <= sti.TLAST;
  end
end

always_ff @(posedge sti.ACLK)
if (~sti.ARESETn) begin
  old.TVALID <= 1'b0;
end else begin
  if (ctl_rst) begin
    old.TVALID <= 1'b0;
  end else if (sti.TREADY & trn) begin
    old.TVALID <= sti.TVALID;
  end
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
if (~sti.ARESETn) begin
  cnt <= '0;
end else begin
  if (ctl_rst) begin
    cnt <= '0;
  end else if (sti.transf) begin
    cnt <= trn ? '0 : nxt;
  end
end

// transfer due to (bypass) or (counter full or end of stream) or (input data change)
assign trn = ~cfg_ena | (max | old.TLAST) | (~cmp & sti.TVALID);

////////////////////////////////////////////////////////////////////////////////
// compression
////////////////////////////////////////////////////////////////////////////////

assign old.TREADY = (sto.TREADY | ~sto.TVALID);

always_ff @(posedge sti.ACLK)
if (~sti.ARESETn) begin
  sto.TVALID <= 1'b0;
end else begin
  if (ctl_rst) begin
    sto.TVALID <= 1'b0;
  end else if (old.TREADY) begin
    sto.TVALID <= old.TVALID & trn;
  end
end

always_ff @(posedge sti.ACLK)
if (old.transf) begin
  sto.TLAST <= old.TLAST;
  sto.TKEEP <= '1;
  sto.TDATA <= {cnt, old.TDATA};
end

endmodule: rle
