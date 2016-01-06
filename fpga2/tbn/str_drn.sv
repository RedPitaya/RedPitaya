module str_drn #(
  type DAT_T = logic signed [8-1:0]
)(
  str_bus_if.d  str
);

logic str_trn;
logic str_ena;

DAT_T        buf_dat [$];
logic        buf_lst [$];
int unsigned buf_tmg [$];
int unsigned buf_siz;

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
always @ (posedge str.clk, negedge str.rstn)
if (!str.rstn) begin
  buf_dat = {};
  buf_lst = {};
end else if (str_trn) begin
  buf_dat.push_back(str.dat);
  buf_lst.push_back(str.lst);
end

assign str.rdy = 1'b1;

////////////////////////////////////////////////////////////////////////////////
// stream data queue
////////////////////////////////////////////////////////////////////////////////

// queue size
assign buf_siz = buf_dat.size();

// put data into queue
task get (
  output  DAT_T        dat,
  output  logic        lst,
  output  int unsigned tmg
);
  dat = buf_dat.pop_front();
  lst = buf_lst.pop_front();
  tmg = buf_tmg.pop_front();
endtask: get

endmodule
