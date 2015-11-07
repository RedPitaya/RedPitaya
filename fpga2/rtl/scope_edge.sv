////////////////////////////////////////////////////////////////////////////////
// Stream edge (positive/negative) detection
// Authors: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

module scope_edge #(
  // stream parameters
  parameter DWI = 14   // data width for input
)(
  // system signals
  input  logic                  clk ,  // clock
  input  logic                  rstn,  // reset - active low
  // stream monitor
  input  logic signed [DWI-1:0] sti_dat,  // data
  input  logic                  sti_vld,  // valid
  input  logic                  sti_rdy,  // ready
  // configuration
  input  logic signed [DWI-1:0] cfg_lvl,  // level
  input  logic        [DWI-1:0] cfg_hst,  // hystheresis
  // output triggers
  output logic                  trg_pdg,  // positive edge
  output logic                  trg_ndg   // negative edge
);

// level +/- hystheresys
logic signed [DWI-0:0] cfg_lvp;
logic signed [DWI-0:0] cfg_lvn;

always @(posedge clk)
begin
  cfg_lvp <= cfg_lvl + $signed({1'b0, cfg_hst});
  cfg_lvn <= cfg_lvl - $signed({1'b0, cfg_hst});
end

// edge status signals
logic [2-1:0] sts_pdg;
logic [2-1:0] sts_ndg;

// stream transfer
assign sti_trn = sti_vld & sti_rdy;

always @(posedge clk)
if (rstn == 1'b0) begin
  sts_pdg <= '0;
  sts_ndg <= '0;
  trg_pdg <= '0;
  trg_ndg <= '0;
end else begin
  if (sti_trn) begin
         if (sti_dat >= cfg_lvl)  sts_pdg[0] <= 1'b1;  // level reached
    else if (sti_dat <  cfg_lvn)  sts_pdg[0] <= 1'b0;  // signal goes under hysteresis
         if (sti_dat <= cfg_lvl)  sts_ndg[0] <= 1'b1;  // level reached
    else if (sti_dat >  cfg_lvp)  sts_ndg[0] <= 1'b0;  // signal goes over hysteresis

    sts_pdg[1] <= sts_pdg[0];
    sts_ndg[1] <= sts_ndg[0];
  end

  trg_pdg <= sts_pdg[0] & ~sts_pdg[1]; // make 1 cyc pulse 
  trg_ndg <= sts_ndg[0] & ~sts_ndg[1];
end

endmodule: scope_edge
