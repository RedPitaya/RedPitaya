////////////////////////////////////////////////////////////////////////////////
// Stream edge (positive/negative) detection
// Authors: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

module scope_edge #(
  // stream parameters
  int unsigned DN = 1, // TODO: for now only value 1 is permitted
  type DT = logic signed [16-1:0]
)(
  // control
  input  logic      ctl_rst,  // synchronous reset
  // configuration
  input  logic      cfg_edg,  // edge select (0-rising, 1-falling)
  input  DT         cfg_pos,  // positive level
  input  DT         cfg_neg,  // negative level
  // output triggers
  output logic      sts_trg,
  // stream
  axi4_stream_if.d  sti,
  axi4_stream_if.s  sto
);

// position of the sign bit
localparam int unsigned SGN = $bits(DT);

// subtraction from pos/neg levels
logic signed [SGN:0] sub_pos;
logic signed [SGN:0] sub_neg;

// level status signal and registered version
logic sts_lvl, sts_reg;

// subtraction stream
axi4_stream_if #(.DN (DN), .DT (DT)) sts (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));  

////////////////////////////////////////////////////////////////////////////////
// subtraction stage
////////////////////////////////////////////////////////////////////////////////

// subtraction
always @(posedge sti.ACLK)
if (sti.transf) begin
  sub_pos <= cfg_pos - sti.TDATA[0];
  sub_neg <= sti.TDATA[0] - cfg_neg;
end

// add to the stream the delay caused by subtraction stage
axi4_stream_reg reg_sub (.sti (sti), .sto (sts));

////////////////////////////////////////////////////////////////////////////////
// trigger stage
////////////////////////////////////////////////////////////////////////////////

// toggle pos/neg edge, if pos/neg level is passed
assign sts_lvl = sts_reg ^ ((cfg_edg ^ sts_reg) ? sub_neg[SGN] : sub_pos[SGN]);

always @(posedge sts.ACLK)
if (~sts.ARESETn) begin
  sts_reg <= 1'b0;
  sts_trg <= 1'b0;
end else begin
  if (ctl_rst) begin
    sts_reg <= 1'b0;
    sts_trg <= 1'b0;
  end else if (sts.transf) begin
    // store previous level status
    sts_reg <= sts_lvl;
    // trigger pulse if level status changes to active
    sts_trg <= sts_lvl & ~sts_reg;
  end
end

// add to the stream the delay caused by trigger stage
axi4_stream_reg reg_trg (.sti (sts), .sto (sto));

endmodule: scope_edge
