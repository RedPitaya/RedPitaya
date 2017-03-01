////////////////////////////////////////////////////////////////////////////////
// Stream edge (positive/negative) detection
// Authors: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

module scope_edge #(
  // stream parameters
  type DT = logic signed [16-1:0]
)(
  // control
  input  logic      ctl_rst,  // synchronous reset
  // configuration
  input  logic      cfg_edg,  // edge select (0-rising, 1-falling)
  input  DT         cfg_lvl,  // level
  input  DT         cfg_hst,  // hystheresis
  // output triggers
  output logic      sts_trg,  // positive edge
  // stream monitor
  axi4_stream_if.m  str
);

// level +/- hystheresys
DT cfg_lvn;

always @(posedge str.ACLK)
begin
  cfg_lvn <= cfg_lvl + cfg_hst;
end

// edge status signals
logic [2-1:0] sts_edg;

always @(posedge str.ACLK)
if (~str.ARESETn) begin
  sts_edg <= '0;
  sts_trg <= '0;
end else begin
  if (ctl_rst) begin
    sts_edg <= '0;
    sts_trg <= '0;
  end else begin
    if (str.transf) begin
           if (str.TDATA >= cfg_lvl)  sts_edg[0] <= 1'b1;  // level reached
      else if (str.TDATA <  cfg_lvn)  sts_edg[0] <= 1'b0;  // signal goes under hysteresis
  
      sts_edg[1] <= sts_edg[0];
    end
  
    sts_trg <= sts_edg[0] & ~sts_edg[1]; // make 1 cyc pulse 
  end
end

endmodule: scope_edge
