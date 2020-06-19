////////////////////////////////////////////////////////////////////////////////
// Stream edge (positive/negative) detection
// Authors: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

module scope_dec_avg #(
  // stream parameters
  int unsigned DN = 1,
  type DTI = logic signed [16-1:0],  // data width for input
  type DTO = logic signed [16-1:0],  // data width for output
  // decimation parameters
  int unsigned DCW = 17,  // data width for counter
  int unsigned DSW =  4   // data width for shifter
)(
  // control
  input  logic                  ctl_rst,  // synchronous reset
  // configuration
  input  logic                  cfg_avg,  // averaging enable
  input  logic        [DCW-1:0] cfg_dec,  // decimation factor
  input  logic        [DSW-1:0] cfg_shr,  // shift right
  // streams
  axi4_stream_if.d              sti,      // input
  axi4_stream_if.s              sto       // output
);

logic signed [$bits(DTI)+DCW-1:0] sum;

logic                   [DCW-1:0] cnt;
logic                             vld;

assign sti.TREADY = sto.TREADY | ~sto.TVALID;

always_ff @(posedge sti.ACLK)
if (~sti.ARESETn) begin
  sum        <= '0;
  cnt        <= '0;
  sto.TVALID <= 1'b0;
end else begin
  if (ctl_rst) begin
    sum <= '0;
    cnt <= '0;
  end else if (sti.transf) begin
    // decimation counter
    cnt <= vld ? 0 : cnt + 'd1;
    if (cfg_avg) begin
      sum <= vld ? sti.TDATA[0] : sum + sti.TDATA[0];
    end
  end
  sto.TVALID <= vld;
end

assign vld = cnt == cfg_dec;

always_ff @(posedge sti.ACLK)
if (vld & sti.transf) begin
  if (cfg_avg) sto.TDATA[0] <= sum >>> cfg_shr;
  else         sto.TDATA[0] <= sti.TDATA[0];
end

// TODO: last signal should not be lost due to decimation
always_ff @(posedge sti.ACLK)
sto.TLAST <= sti.TLAST;

// TODO properly handle keep signal
always_ff @(posedge sti.ACLK)
sto.TKEEP <= sti.TKEEP;

endmodule: scope_dec_avg
