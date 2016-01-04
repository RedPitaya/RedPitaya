module str_drn #(
  type DAT_T = logic signed [8-1:0]
)(
  str_bus_if.d  str
);

logic str_trn;
logic str_ena;

DAT_T        buf_dat [$];
int unsigned buf_tmg [$];

////////////////////////////////////////////////////////////////////////////////
// stream
////////////////////////////////////////////////////////////////////////////////

// stream transfer event
assign str_trn = str.vld & str.rdy;

// stream enable
assign str_ena = str_trn | ~str.vld;

//int str_tmg;
//
//// transfer delay counter
//always @ (posedge str.clk, posedge rst)
//if (~str.rstn)     str_tmg <= 0;
//else if (str.vld)  str_tmg <= str.rdy ? 0 : str_tmg + 1;

// on transfer store data in the queue
always @ (posedge str.clk)
if (str_trn)   buf_dat.push_back(str.dat);

assign str.rdy = 1'b1;

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////


endmodule
