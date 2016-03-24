////////////////////////////////////////////////////////////////////////////////
// Module: LA trigger detection
// Authors: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

module la_trigger #(
  int unsigned DN = 1,
  type DT = logic [8-1:0]  // str.dat type
)(
  // control
  input  logic          ctl_rst,  // synchronous reset
  // configuration
  input  DT             cfg_cmp_msk,  // comparator mask
  input  DT             cfg_cmp_val,  // comparator value
  input  DT             cfg_edg_pos,  // edge positive
  input  DT             cfg_edg_neg,  // edge negative
  // output triggers
  output logic [DN-1:0] sts_trg,
  // stream monitor
  axi4_stream_if.m      str
);

DT    [DN-0:0] dat;

logic [DN-1:0] sts_cmp;
logic [DN-1:0] sts_edg;

generate
for (genvar i=0; i<DN; i++) begin: for_dn

// comparator
assign sts_cmp [i] = (str.TDATA[i] & cfg_cmp_msk) == (cfg_cmp_val & cfg_cmp_msk);
// edge detection
assign sts_edg = |(cfg_edg_pos & (~dat[i] &  dat[i+1]))
               | |(cfg_edg_neg & ( dat[i] & ~dat[i+1]));

end: for_dn
endgenerate

// data chain for checking edges
always_ff @(posedge str.ACLK)
if (str.transf)  dat [0] <= str.TDATA [DN-1];

assign dat [DN:1] = str.TDATA;

always_ff @(posedge str.ACLK)
if (~str.ARESETn) begin
  sts_trg <= '0;
end else begin
  if (ctl_rst) begin
    sts_trg <= '0;
  end if (str.transf) begin
    sts_trg <= str.TKEEP & sts_cmp & sts_edg;
  end
end

endmodule: la_trigger
