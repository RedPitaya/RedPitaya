module str_src #(
  int unsigned DW = 1,                   // data width
  logic        IV = 1'b0                 // idle data bus value
)(
  // system signals
  input  logic                 clk,      // clock
  input  logic                 rstn,     // reset - active low
  // stream
  output logic                 str_vld,  // transfer valid
  output logic signed [DW-1:0] str_dat,  // grouped bus signals
  input  logic                 str_rdy   // transfer ready
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

// valid is active if there is data in the queue
always @ (posedge clk, posedge rstn)
if (!rstn)         str_vld <= 1'b0;
else if (str_ena)  str_vld <= buf_dat.size() > 0;

// transfer delay counter
always @ (posedge clk, posedge rstn)
if (!rstn)         str_dat <= '0;
else if (str_ena)  str_dat <= buf_dat.size() > 0 ? buf_dat.pop_front() : {DW{IV}};

////////////////////////////////////////////////////////////////////////////////
// stream data queue
////////////////////////////////////////////////////////////////////////////////

// 
task put (
  input  logic [DW-1:0] dat,
  input  int unsigned   tmg = 0
);
  buf_dat.push_back(dat);
  buf_tmg.push_back(tmg);
endtask: put

endmodule: str_src
