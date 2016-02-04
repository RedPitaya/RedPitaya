module str_src #(
  type DAT_T = logic [8-1:0],
  logic IV = 1'b0  // idle data bus value
)(
  str_bus_if.s str
);

logic str_ena;

DAT_T        buf_dat [$];
logic        buf_lst [$];
int unsigned buf_tmg [$];
int unsigned buf_siz;

////////////////////////////////////////////////////////////////////////////////
// stream
////////////////////////////////////////////////////////////////////////////////

// stream enable
assign str_ena = str.trn | ~str.vld;

//int str_tmg;
//
//// transfer delay counter
//always @ (posedge str.clk, posedge rstn)
//if (!str.rstn)     str_tmg <= 0;
//else if (str.vld)  str_tmg <= str.rdy ? 0 : str_tmg + 1;

// data, last
always @ (posedge str.clk, negedge str.rstn)
if (!str.rstn) begin
  str.vld <= 1'b0;
  str.dat <= '0;
  str.lst <= '0;
end else if (str_ena) begin
  if (buf_siz) begin
    str.vld <= 1'b1;
    str.dat <= buf_dat.pop_front();
    str.lst <= buf_lst.pop_front();
  end else begin
    str.vld <= 1'b0;
    str.dat <= {$bits(DAT_T){IV}};
    str.lst <= {$bits(DAT_T){IV}};
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
  input  DAT_T        dat,
  input  logic        lst,
  input  int unsigned tmg = 0
);
  buf_dat.push_back(dat);
  buf_lst.push_back(lst);
  buf_tmg.push_back(tmg);
endtask: put

endmodule: str_src
