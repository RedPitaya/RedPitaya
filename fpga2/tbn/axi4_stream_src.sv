module axi4_stream_src #(
  int unsigned DN = 1,
  type DAT_T = logic [8-1:0],
  logic IV = 1'b0  // idle data bus value
)(
  axi4_stream_if.s str
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
//if (!str.ARESETn)     str_tmg <= 0;
//else if (str.TVALID)  str_tmg <= str.TREADY ? 0 : str_tmg + 1;

// data, last
always @ (posedge str.ACLK, negedge str.ARESETn)
if (!str.ARESETn) begin
  str.TVALID <= 1'b0;
  str.TDATA  <= '0;
  str.TKEEP  <= '0;
  str.TLAST  <= '0;
end else if (str_ena) begin
  if (buf_siz) begin
    str.TVALID <= 1'b1;
    str.TDATA  <= buf_dat.pop_front();
    str.TKEEP  <= buf_kep.pop_front();
    str.TLAST  <= buf_lst.pop_front();
  end else begin
    str.TVALID <= 1'b0;
    str.TDATA  <= {$bits(DAT_T){IV}};
    str.TKEEP  <= {$bits(DN   ){IV}};
    str.TLAST  <=               IV  ;
  end
end

// transfer delay counter
// TODO

////////////////////////////////////////////////////////////////////////////////
// stream data queue
////////////////////////////////////////////////////////////////////////////////

// queue size
assign buf_siz = buf_dat.size();

// put data into queue
task put (
  input  DAT_T [DN-1:0] dat,
  input  logic [DN-1:0] kep,
  input  logic          lst,
  input  int unsigned   tmg = 0
);
  buf_dat.push_back(dat);
  buf_kep.push_back(kep);
  buf_lst.push_back(lst);
  buf_tmg.push_back(tmg);
endtask: put

endmodule: axi4_stream_src
