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
  int unsigned DWC = 17,  // data width for counter
  int unsigned DWS =  4,  // data width for shifter
)(
  // system signals
  input  logic                  clk ,  // clock
  input  logic                  rstn,  // reset - active low
  // control
  input  logic                  ctl_clr,  // synchronous clear
  // configuration
  input  logic                  cfg_avg,  // averaging enable
  input  logic        [DWC-1:0] cfg_dec,  // decimation factor
  input  logic        [DWS-1:0] cfg_shf,  // shift right
  // stream input
  input  logic signed [DWI-1:0] sti_dat,  // data
  input  logic                  sti_vld,  // valid
  output logic                  sti_rdy,  // ready
  // stream output
  output logic signed [DWO-1:0] sto_dat,  // data
  output logic                  sto_vld,  // valid
  input  logic                  sto_rdy,  // ready
  // triggers
  input  logic                  trg_ext,  // external input
  output logic                  trg_out,  // output
);

logic signed [DWC+DWI-1:0] sum;

logic        [DWC    -1:0] cnt;


logic sti_trn;

assign sti_rdy = sto_rdy | ~sto_vld;
assign sti_trn = sti_vld & sti_rdy;

always_ff @(posedge clk)
if (~rstn) begin
  sum     <= '0;
  cnt     <= '0;
  sto_vld <= 
end else begin
  if (ctl_clr) begin
    sum <= '0;
    cnt <= '0;
  end else begin
    if (cfg_avg) begin
      if (cnt == cfg_dec) begin
         cnt <= '0;
         sum <= sti_dat;
      end else begin
         cnt <= cnt + 'd1;
         sum <= sum + sti_dat;
      end
    end else begin
    end
  end
end

always_ff @(posedge clk)
begin
  if (cfg_avg) sto_dat <= sti_dat;
  else         sto_dat <= sum >>> cfg_shr;
end

endmodule: scope_dec_avg
