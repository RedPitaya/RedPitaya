module axi4_stream_drn #(
  int unsigned DN = 1,
  type DAT_T = logic [8-1:0]
)(
  axi4_stream_if.d str
);

DAT_T [DN-1:0] buf_dat [$];
logic [DN-1:0] buf_kep [$];
logic          buf_lst [$];
int unsigned   buf_tmg [$];
int unsigned   buf_siz;

// clocking 
default clocking clk @ (posedge str.ACLK);
  default input #1step output #1step;
  input  ARESETn = str.ARESETn;
  input  TDATA   = str.TDATA  ;
  input  TKEEP   = str.TKEEP  ;
  input  TLAST   = str.TLAST  ;
  input  TVALID  = str.TVALID ;
  output TREADY  = str.TREADY ;
  input  transf  = str.transf ;
endclocking: clk

////////////////////////////////////////////////////////////////////////////////
// stream
////////////////////////////////////////////////////////////////////////////////

//int str_tmg;
//
//// transfer delay counter
//always @ (posedge str.ACLK, posedge str.ARESETn)
//if (~str.ARESETn)     str_tmg <= 0;
//else if (str.TVALID)  str_tmg <= str.TREADY ? 0 : str_tmg + 1;

// on transfer store data in the queue
initial
begin
  buf_dat = {};
  buf_kep = {};
  buf_lst = {};
  forever begin
    ##1;
    if (clk.transf) begin
      buf_dat.push_back(clk.TDATA);
      buf_kep.push_back(clk.TKEEP);
      buf_lst.push_back(clk.TLAST);
    end
  end
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

task clr ();
  buf_dat = {};
  buf_kep = {};
  buf_lst = {};
  buf_tmg = {};
endtask: clr

endmodule: axi4_stream_drn
