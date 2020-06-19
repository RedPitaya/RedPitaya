////////////////////////////////////////////////////////////////////////////////
// Module: LA trigger detection
// Authors: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

module la_trg #(
  int unsigned DN = 1,
  type DT = logic [8-1:0]  // sti.dat type
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
  // stream input/output
  axi4_stream_if.d  sti,
  axi4_stream_if.s  sto
);

DT    [DN-0:0] dat;

logic [DN-1:0] sts_cmp;
logic [DN-1:0] sts_edg;

generate
for (genvar i=0; i<DN; i++) begin: for_dn

// comparator
assign sts_cmp [i] = (sti.TDATA[i] & cfg_cmp_msk) == (cfg_cmp_val & cfg_cmp_msk);
// edge detection
assign sts_edg [i] = |(cfg_edg_pos & (~dat[i] &  dat[i+1]))
                   | |(cfg_edg_neg & ( dat[i] & ~dat[i+1]));

end: for_dn
endgenerate

// data chain for checking edges
always_ff @(posedge sti.ACLK)
if (sti.transf)  dat [0] <= sti.TDATA [DN-1];

assign dat [DN:1] = sti.TDATA;

always_ff @(posedge sti.ACLK)
if (~sti.ARESETn) begin
  sts_trg <= '0;
end else begin
  if (ctl_rst) begin
    sts_trg <= '0;
  end if (sti.transf) begin
    sts_trg <= sti.TKEEP & sts_cmp & sts_edg;
  end
end

// align data with trigger edge
axi4_stream_reg align_reg(.sti (sti), .sto (sto));

endmodule: la_trg
