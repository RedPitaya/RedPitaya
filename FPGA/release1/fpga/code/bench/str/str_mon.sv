////////////////////////////////////////////////////////////////////////////////
// AXI4-Stream monitor (also source/drain)
// (C) 2015 Iztok Jeras
////////////////////////////////////////////////////////////////////////////////

module str_mon #(
  int unsigned DW = 8,
  // enable source/drain/monitor functionality
  bit SRC = 1'b0,
  bit DRN = 1'b0,
  bit MON = 1'b1
)(
  str_if str
);

clocking str_src @(posedge str.clk);
  default input #1step output #1step;
  input  str.tready;
  output str.tvalid;
  output str.tdata ;
  output str.tlast ;
  output str.tkeep ;
endclocking: str_src

clocking clk_drn @(posedge clk);
  default input #1step output #1step;
  output str.tready;
  input  str.tvalid;
  input  str.tdata ;
  input  str.tlast ;
  input  str.tkeep ;
endclocking

typedef struct {
  real fix;
  real rnd;
} tmg_t;

str_pkg.tmg_t tmg_vld;
str_pkg.tmg_t tmg_rdy;

// source valid signal
task src (
  input  int unsigned len,    // stream length
  input  bit          lst,    // stream last enable
  input  tmg_t        tmg,    // valid signal timing
  input  [DW-1:0]     dat []  // data
);
  for (int unsigned i=0; i<len; i++) begin
    str_src.tvalid = 1'b1;
    str_src.tdata  = dat [i];
    str_src.tlast  = lst & (i==len-1);
    str_src.tkeep  = '1;
    ##1;
  end
  str_src.tvalid = 1'b0;
  str_src.tdata  = 'x;
  str_src.tlast  = 'x;
  str_src.tkeep  = 'x;
endtask: src

endmodule: str_mon
