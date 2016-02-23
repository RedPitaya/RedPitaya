////////////////////////////////////////////////////////////////////////////////
// Stream edge (positive/negative) detection
// Authors: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

module scope_dec_avg #(
  // stream parameters
  int unsigned DWI = 14,  // data width for input
  int unsigned DWO = 14,  // data width for output
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

logic signed [DCW+DWI-1:0] sum;

logic        [DCW    -1:0] cnt;

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
  end else begin
    if (cfg_avg) begin
      if (cnt == cfg_dec) begin
         cnt <= '0;
         sum <= sti.TDATA;
      end else begin
         cnt <= cnt + 'd1;
         sum <= sum + sti.TDATA;
      end
    end else begin
    end
  end
end

always_ff @(posedge sti.ACLK)
begin
  if (cfg_avg) sto.TDATA <= sti.TDATA;
  else         sto.TDATA <= sum >>> cfg_shr;
end

// TODO: last signal should not be lost due to decimation
always_ff @(posedge sti.ACLK)
sto.TLAST <= sti.TLAST;

// TODO properly handle keep signal
always_ff @(posedge sti.ACLK)
sto.TKEEP <= sti.TKEEP;

endmodule: scope_dec_avg
