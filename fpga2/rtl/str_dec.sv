////////////////////////////////////////////////////////////////////////////////
// Stream decimation without filtering
// Authors: Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

module str_dec #(
  int unsigned CW = 17  // counter width
)(
  // control
  input  logic          ctl_rst,  // synchronous reset
  // configuration
  input  logic [CW-1:0] cfg_dec,  // decimation factor
  // streams
  str_bus_if.d          sti,      // input
  str_bus_if.s          sto       // output
);

logic [CW-1:0] cnt;
logic          nul;

logic sti_trn;

assign sti.rdy = sto.rdy | ~sto.vld | ~nul;
assign sti_trn = sti.vld & sti.rdy;

// counter
always_ff @(posedge sti.clk)
if (~sti.rstn) begin
  cnt <= '0;
end else begin
  if (ctl_rst)  cnt <= '0;
  else          cnt <= nul ? cfg_dec : cnt - 'd1;
end

assign nul = ~|cnt;

// output valid signal
always_ff @(posedge sti.clk)
if (~sti.rstn) begin
  sto.vld <= 1'b0;
end else begin
  if (ctl_rst)  sto.vld <= sti.vld;
  else          sto.vld <= sti.vld & nul;
end

always_ff @(posedge sti.clk)
if (sti_trn & (ctl_rst | nul))  sto.dat <= sti.dat;

endmodule: str_dec
