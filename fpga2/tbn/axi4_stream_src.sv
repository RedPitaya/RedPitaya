module axi4_stream_src #(
  int unsigned DN = 1,
  type DAT_T = logic [8-1:0],
  logic IV = 1'b0  // idle data bus value
)(
  axi4_stream_if.s str
);

DAT_T [DN-1:0] buf_dat [$];
logic [DN-1:0] buf_kep [$];
logic          buf_lst [$];
int unsigned   buf_tmg [$];
int unsigned   buf_siz;

// clocking 
default clocking clk @ (posedge str.ACLK);
  input  ARESETn = str.ARESETn;
  output TDATA   = str.TDATA  ;
  output TKEEP   = str.TKEEP  ;
  output TLAST   = str.TLAST  ;
  output TVALID  = str.TVALID ;
  input  TREADY  = str.TREADY ;
  input  transf  = str.transf ;
endclocking: clk

////////////////////////////////////////////////////////////////////////////////
// stream
////////////////////////////////////////////////////////////////////////////////

//int str_tmg;
//
//// transfer delay counter
//always @ (posedge str.ACLK, posedge str.ARESETn)
//if (!str.ARESETn)     str_tmg <= 0;
//else if (str.TVALID)  str_tmg <= str.TREADY ? 0 : str_tmg + 1;

initial begin
  clk.TVALID <= 1'b0;
  clk.TDATA  <= '0;
  clk.TKEEP  <= '0;
  clk.TLAST  <= '0;
  forever begin
    if (buf_siz) begin
      clk.TVALID <= 1'b1;
      clk.TDATA  <= buf_dat.pop_front();
      clk.TKEEP  <= buf_kep.pop_front();
      clk.TLAST  <= buf_lst.pop_front();
    end else begin
      clk.TVALID <= 1'b0;
      clk.TDATA  <= {$bits(DAT_T){IV}};
      clk.TKEEP  <= {$bits(DN   ){IV}};
      clk.TLAST  <=               IV  ;
    end
    ##1;
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
