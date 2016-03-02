module axi4_stream_src #(
  int unsigned DN = 1,
  type DAT_T = logic [8-1:0],
  logic IV = 1'b0  // idle data bus value
)(
  axi4_stream_if.s str
);

typedef struct {
  DAT_T [DN-1:0] dat;
  logic [DN-1:0] kep;
  logic          lst;
  int unsigned   vld;
//  int unsigned   rdy;
} mem_t;

mem_t        mem [$];
int unsigned mem_siz;

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

int str_tmg;

// transfer delay counter
always @ (posedge str.ACLK, posedge str.ARESETn)
if (!str.ARESETn)     str_tmg <= 0;
else if (str.TVALID)  str_tmg <= str.TREADY ? 0 : str_tmg + 1;

initial begin
  mem_t val;
  idle();
  forever begin
    if (mem_siz) begin
      val = mem.pop_front();
      if (val.vld) begin
        idle();
        ##(val.vld);
      end
      clk.TVALID <= 1'b1;
      clk.TDATA  <= val.dat;
      clk.TKEEP  <= val.kep;
      clk.TLAST  <= val.lst;
    end else begin
      idle();
    end
    ##1;
  end
end

// set idle state
task idle;
  clk.TVALID <= 1'b0;
  clk.TDATA  <= {$bits(DAT_T){IV}};
  clk.TKEEP  <= {$bits(DN   ){IV}};
  clk.TLAST  <=               IV  ;
endtask: idle

////////////////////////////////////////////////////////////////////////////////
// stream data queue
////////////////////////////////////////////////////////////////////////////////

// queue size
assign buf_siz = mem.size();

// put data into queue
task put (
  input  DAT_T [DN-1:0] dat,
  input  logic [DN-1:0] kep,
  input  logic          lst,
  input  int unsigned   vld = 0
);
  mem.push_back('{dat, kep, lst, vld});
endtask: put

endmodule: axi4_stream_src
