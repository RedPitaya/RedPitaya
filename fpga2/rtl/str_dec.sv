////////////////////////////////////////////////////////////////////////////////
// Stream decimation without filtering
// Authors: Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

module str_dec #(
  int unsigned DN = 1,  //
  int unsigned CW = 17  // counter width
)(
  // control
  input  logic          ctl_rst,  // synchronous reset
  // configuration
  input  logic [CW-1:0] cfg_dec,  // decimation factor
  // streams
  axi4_stream_if.d      sti,      // input
  axi4_stream_if.s      sto       // output
);

logic [CW-1:0] cnt;
logic          nul;

assign sti.TREADY = sto.TREADY | ~sto.TVALID | ~nul;

// counter
always_ff @(posedge sti.ACLK)
if (~sti.ARESETn) begin
  cnt <= '0;
end else begin
  if (ctl_rst)  cnt <= '0;
  else          cnt <= nul ? cfg_dec : cnt - 'd1;
end

assign nul = ~|cnt;

// output valid signal
always_ff @(posedge sti.ACLK)
if (~sti.ARESETn) begin
  sto.TVALID <= 1'b0;
end else begin
  if (ctl_rst)  sto.TVALID <= sti.TVALID;
  else          sto.TVALID <= sti.TVALID & nul;
end

always_ff @(posedge sti.ACLK)
if (sti.transf & (ctl_rst | nul)) begin
  sto.TDATA <= sti.TDATA;
  sto.TKEEP <= sti.TKEEP; // TODO
  sto.TLAST <= sti.TLAST;
end

endmodule: str_dec
