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
  str_bus_if.d                  sti,      // input
  str_bus_if.s                  sto       // output
);

logic signed [DCW+DWI-1:0] sum;

logic        [DCW    -1:0] cnt;


logic sti_trn;

assign sti.rdy = sto.rdy | ~sto.vld;
assign sti_trn = sti.vld & sti.rdy;

always_ff @(posedge sti.clk)
if (~sti.rstn) begin
  sum     <= '0;
  cnt     <= '0;
  sto.vld <= 1'b0;
end else begin
  if (ctl_rst) begin
    sum <= '0;
    cnt <= '0;
  end else begin
    if (cfg_avg) begin
      if (cnt == cfg_dec) begin
         cnt <= '0;
         sum <= sti.dat;
      end else begin
         cnt <= cnt + 'd1;
         sum <= sum + sti.dat;
      end
    end else begin
    end
  end
end

always_ff @(posedge sti.clk)
begin
  if (cfg_avg) sto.dat <= sti.dat;
  else         sto.dat <= sum >>> cfg_shr;
end

endmodule: scope_dec_avg
