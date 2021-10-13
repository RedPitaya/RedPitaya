////////////////////////////////////////////////////////////////////////////////
// Module: RLE (Run Length Encoding) decompression
// Author: Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
// GENERAL DESCRIPTION:
//
// RLE encoded signal decompression
//
////////////////////////////////////////////////////////////////////////////////

module rle_rev #(
  // counter properties
  int unsigned CW = 8,
  int unsigned BW = 8,
  // stream properties
  int unsigned DN = 1,
  type DTI = logic [CW+8-1:0], // data type for input
  type DTO = logic [   8-1:0]  // data type for output
)(
  // input stream input/output
  axi4_stream_if.d sti,  // input
  axi4_stream_if.s sto,  // output
  // configuration
  input  logic     ctl_rst,  // reset
  input  logic     cfg_ena,   // enable
  output logic     new_read  // signals for new FIFO read
);

////////////////////////////////////////////////////////////////////////////////
// local variables
////////////////////////////////////////////////////////////////////////////////

// old values
axi4_stream_if #(.DN (1), .DT (DTI)) old (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));

// counter
logic [CW-1:0] cnt;
logic [CW-1:0] nxt;
logic          max;
logic          endtr;
logic [CW+BW-1:0] sti_reg;
wire  [CW-1:0] rle_val = sti_reg[CW+BW-1:BW];

// new compressed value
logic trn;

assign new_read = trn; // when all RLE encoded values are sent.
////////////////////////////////////////////////////////////////////////////////
// store previous value
////////////////////////////////////////////////////////////////////////////////

assign sti.TREADY = old.TREADY | ~old.TVALID;
always @(posedge sti.ACLK) begin
  if (~sti.ARESETn)
    sti_reg <= 'hFFFFFF;
  else if (sti.transf & (sti.TDATA[0][CW+BW-1:BW] > 'h0))
    sti_reg <= sti.TDATA[0];
end

always_ff @(posedge sti.ACLK)
if (sti.transf) begin
  old.TDATA[0]  <= sti.TDATA[0];
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
// counter
////////////////////////////////////////////////////////////////////////////////

assign nxt = cnt + 1;
//assign max = &cnt;
assign endtr = (cnt==rle_val); //modified for custom RLE decoding

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

// transfer due to (bypass) or (all samples were sent or end of stream)
assign trn = ~cfg_ena | (endtr | old.TLAST);

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
  sto.TDATA[0] <= old.TDATA[0][BW-1:0];
end

endmodule: rle_rev
