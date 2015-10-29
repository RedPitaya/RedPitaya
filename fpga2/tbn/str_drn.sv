module str_drn #(
  int unsigned DW = 1                    // data width
)(
  // system signals
  input  logic                 clk,      // clock
  input  logic                 rstn,     // reset - active low
  // stream
  input  logic                 str_vld,  // transfer valid
  input  logic signed [DW-1:0] str_dat,  // grouped bus signals
  output logic                 str_rdy   // transfer ready
);

logic str_trn;
logic str_ena;

logic signed [DW-1:0] buf_dat [$];
int unsigned          buf_tmg [$];

////////////////////////////////////////////////////////////////////////////////
// stream
////////////////////////////////////////////////////////////////////////////////

// stream transfer event
assign str_trn = str_vld & str_rdy;

// stream enable
assign str_ena = str_trn | ~str_vld;

//int str_tmg;
//
//// transfer delay counter
//always @ (posedge clk, posedge rst)
//if (!rstn)         str_tmg <= 0;
//else if (str_vld)  str_tmg <= str_rdy ? 0 : str_tmg + 1;

// on transfer store data in the queue
always @ (posedge clk)
if (str_trn)   buf_dat.push_back(str_dat);

assign str_rdy = 1'b1;

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////


endmodule
