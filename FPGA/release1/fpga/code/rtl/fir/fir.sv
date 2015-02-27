////////////////////////////////////////////////////////////////////////////////
// FIR filter using AXI4-Stream interface
// (C) 2015 Iztok Jeras
////////////////////////////////////////////////////////////////////////////////

interface fir #(
  int unsigned DN = 1,  // data number
  int unsigned DW = 8,  // data width
  // coeficients
  int unsigned CN = 1,
  int unsigned CW = 8   // coeficient width
)(
  // input/output streams
  str_if.drn sti,
  str_if.src sto,
  // weights
  logic signed [CN-1:0] [CW-1:0] wgh
);

logic signed [DN-1:0] [CN-1:0] [DW   -1:0] dat;
logic signed [DN-1:0] [CN-1:0] [DW+CW-1:0] mul;

function [DW+CW+$clog2(CN)-1:0] add (
  input  logic signed [CN-1:0] [DW+CW-1:0] mul       , // per tap multiplied values
  input  int unsigned                      tpb =    0, // tap begin
  input  int unsigned                      tpe = CN-1, // tap end
);
  add = '0;
  for (int unsigned i=tpb; i<=tpe; i++) begin
    add = add + mul [i];
  end
endfunction: add

generate
for (genvar i=0; i<CN; i++) begin: for_cn
  // delay chain
  if (i==0) begin
    always_comb
    dat[i]  = sti.tdata;
  end else begin
    always_ff @ (posedge sti.clk)
    dat[i] <= dat[i-1];
  end
  // weight multiplier
  always_comb
  mul[i] = dat[i] * wgh[i];
end: for_cn
endgenerate

// accumulator
always_ff @ (posedge sti.clk)
begin
  sto.tlast <= sti.tlast;
  sto.tdata <= add(mul);
  sto.tkeep <= '1;
end

// forward control signal
always_ff @ (posedge sti.clk)
if (sti.rst) begin
  sto.tvalid <= 1'b0;
end else begin
  sto.tvalid <= sti.tvalid;
end

// backpressure control signal
assign sti.tready = sto.tready | ~sto.tvalid;

endmodule: fir
