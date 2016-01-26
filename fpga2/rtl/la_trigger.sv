////////////////////////////////////////////////////////////////////////////////
// Module: LA trigger detection
// Authors: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

module la_trigger #(
  int unsigned DN = 1,
  type DAT_T = logic [8-1:0]  // str.dat type
)(
  // control
  input  logic ctl_rst,  // synchronous reset
  // configuration
  input  DAT_T cfg_cmp_msk,  // comparator mask
  input  DAT_T cfg_cmp_val,  // comparator value
  input  DAT_T cfg_edg_pos,  // edge positive
  input  DAT_T cfg_edg_neg,  // edge negative
  // output triggers
  output logic sts_trg,  // TODO: should have DN width
  // stream monitor
  str_bus_if.m str
);

logic str_trn;
DAT_T str_old;

logic sts_cmp;
logic sts_edg;

// stream transfer
assign str_trn = str.vld & str.rdy;

assign sts_cmp = (str.dat & cfg_cmp_msk) == (cfg_cmp_val & cfg_cmp_msk);
assign sts_edg = |(cfg_edg_pos & (~str_old &  str.dat))
               | |(cfg_edg_neg & ( str_old & ~str.dat));

always @(posedge str.clk)
if (str_trn)  str_old <= str.dat;

always @(posedge str.clk)
if (~str.rstn) begin
  sts_trg <= '0;
end else begin
  if (ctl_rst) begin
    sts_trg <= '0;
  end if (str_trn) begin
    sts_trg <= sts_cmp & sts_edg;
  end
end

endmodule: la_trigger
