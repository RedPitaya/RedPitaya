////////////////////////////////////////////////////////////////////////////////
// Stream edge (positive/negative) detection
// Authors: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

module osc_trg #(
  // stream parameters
  int unsigned DN = 1, // TODO: for now only value 1 is permitted
  type DT = logic signed [16-1:0]
)(
  // control
  input  logic          ctl_rst,  // synchronous reset
  // configuration
  input  DT             cfg_low,  // negative level
  input  DT             cfg_upp,  // positive level
  input  logic          cfg_edg,  // edge select (0-rising, 1-falling)
  // output triggers
  output logic          sts_trg,
  // stream
  axi4_stream_if.d      sti,
  axi4_stream_if.s      sto
);

// position of the sign bit
localparam int unsigned SGN = $bits(DT);

// subtraction from lower/upper levels
logic signed [SGN:0] sub_low;
logic signed [SGN:0] sub_upp;

// level status signal and registered version
logic sts_lvl, sts_reg;

// subtraction stream
axi4_stream_if #(.DN (DN), .DT (DT)) sts (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));  

////////////////////////////////////////////////////////////////////////////////
// subtraction stage
////////////////////////////////////////////////////////////////////////////////

// subtraction
always_ff @(posedge sti.ACLK)
if (sti.transf) begin
  sub_low <= sti.TDATA[0] - cfg_low;
  sub_upp <= cfg_upp - sti.TDATA[0];
end

// add to the stream the delay caused by subtraction stage
axi4_stream_reg reg_sub (.sti (sti), .sto (sts));

////////////////////////////////////////////////////////////////////////////////
// trigger stage
////////////////////////////////////////////////////////////////////////////////

// toggle lower/upper edge, if lower/upper level is passed
assign sts_lvl = sts_reg ^ ((cfg_edg ^ sts_reg) ? sub_low[SGN] : sub_upp[SGN]);

always_ff @(posedge sts.ACLK)
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

endmodule: osc_trg
