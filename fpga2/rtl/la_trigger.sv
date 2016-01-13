////////////////////////////////////////////////////////////////////////////////
// Module: LA trigger detection
// Authors: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

module la_trigger #(
  parameter DW = 16   // data width
)(
  // control
  input  logic          ctl_rst,  // synchronous reset
  // configuration
  input  logic [DW-1:0] cfg_msk_cur,  // current mask
  input  logic [DW-1:0] cfg_msk_old,  // old     mask
  input  logic [DW-1:0] cfg_val_cur,  // current value
  input  logic [DW-1:0] cfg_val_old,  // old     value
  // output triggers
  output logic          sts_trg,
  // stream monitor
  str_bus_if.m          str
);

logic          str_trn;
logic [DW-1:0] str_old;

// stream transfer
assign str_trn = str.vld & str.rdy;

always @(posedge str.clk)
if (str_trn)  str_old <= str.dat;

always @(posedge str.clk)
if (~str.rstn) begin
  sts_trg <= '0;
end else begin
  if (ctl_rst) begin
    sts_trg <= '0;
  end if (str_trn) begin
    sts_trg <= ((str.dat & cfg_msk_cur) == (cfg_val_cur & cfg_msk_cur))
             | ((str_old & cfg_msk_old) == (cfg_val_old & cfg_msk_old));
  end
end

endmodule: la_trigger
