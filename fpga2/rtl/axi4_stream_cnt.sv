////////////////////////////////////////////////////////////////////////////////
// Module: AXI4-Stream packet size counter
// Authors: Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

module axi4_stream_cnt #(
  int unsigned DN = 1,   // data number
  int unsigned CW = 32   // counter width
)(
  // control
  input  logic          ctl_rst,  // synchronous reset
  // counter staus
  output logic [CW-1:0] sts_cur,  // current     counter status
  output logic [CW-1:0] sts_lst,  // last packet counter status
  // stream monitor
  axi4_stream_if.m      str
);

// increment width
localparam int unsigned IW = $clog2(DN);

// local variables
logic [IW-1:0] sts_inc;  // counter increment
logic [CW-1:0] sts_nxt;  // next counter value

// counting active TKEEP bits
always_comb
begin
  sts_inc = 0;
  for (int unsigned i=0; i<DN; i++) begin
    sts_inc = sts_inc + str.TKEEP[i];
  end
end

// increment of current counter
assign sts_nxt = sts_cur + sts_inc;

// counters
always_ff @(posedge str.ACLK)
if (~str.ARESETn) begin
  sts_cur <= '0;
  sts_lst <= '0;
end else begin
  if (ctl_rst) begin
    sts_cur <= '0;
    sts_lst <= '0;
  end if (str.transf) begin
    if (str.TLAST) begin
      sts_cur <= '0;
      sts_lst <= sts_nxt;
    end else begin
      sts_cur <= sts_nxt;
    end
  end
end

endmodule: axi4_stream_cnt
