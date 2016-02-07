module axi4_stream_drn #(
  int unsigned DN = 1,
  type DAT_T = logic [8-1:0]
)(
  axi4_stream_if.d str
);

logic str_ena;

DAT_T [DN-1:0] buf_dat [$];
logic [DN-1:0] buf_kep [$];
logic          buf_lst [$];
int unsigned   buf_tmg [$];
int unsigned   buf_siz;

////////////////////////////////////////////////////////////////////////////////
// stream
////////////////////////////////////////////////////////////////////////////////

// stream enable
assign str_ena = str.transf | ~str.TVALID;

//int str_tmg;
//
//// transfer delay counter
//always @ (posedge str.ACLK, posedge str.ARESETn)
//if (~str.ARESETn)     str_tmg <= 0;
//else if (str.TVALID)  str_tmg <= str.TREADY ? 0 : str_tmg + 1;

// on transfer store data in the queue
always @ (posedge str.ACLK, negedge str.ARESETn)
if (!str.ARESETn) begin
  buf_dat = {};
  buf_kep = {};
  buf_lst = {};
end else if (str.transf) begin
  buf_dat.push_back(str.TDATA);
  buf_kep.push_back(str.TKEEP);
  buf_lst.push_back(str.TLAST);
end

assign str.TREADY = 1'b1;

////////////////////////////////////////////////////////////////////////////////
// stream data queue
////////////////////////////////////////////////////////////////////////////////

// queue size
assign buf_siz = buf_dat.size();

// put data into queue
task get (
  output  DAT_T [DN-1:0] dat,
  output  logic [DN-1:0] kep,
  output  logic          lst,
  output  int unsigned   tmg
);
  dat = buf_dat.pop_front();
  kep = buf_kep.pop_front();
  lst = buf_lst.pop_front();
  tmg = buf_tmg.pop_front();
endtask: get

endmodule: axi4_stream_drn
