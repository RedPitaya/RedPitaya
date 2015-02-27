////////////////////////////////////////////////////////////////////////////////
// AXI4-Stream monitor (also source/drain)
// (C) 2015 Iztok Jeras
////////////////////////////////////////////////////////////////////////////////

module str_mon #(
  // data stream
  int unsigned DN = 1,
  int unsigned DW = 8,
  // enable source/drain/monitor functionality
  string       SM = "" // "SRC"/"DRN"/"MON"
)(
  str_if str
);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

generate
if (SM=="SRC") begin: gen_src

  default clocking str_src @(posedge str.clk);
    default input #1step output #1step;
    input  tready = str.tready;
    output tvalid = str.tvalid;
    output tdata  = str.tdata ;
    output tlast  = str.tlast ;
    output tkeep  = str.tkeep ;
  endclocking: str_src

  initial str.tvalid = 1'b0;

  // source valid signal
  task src (
    input  int unsigned   len,    // stream length
    input  bit            lst,    // stream last enable
    input  str_pkg::tmg_t tmg,    // valid signal timing
    input  [DW-1:0]       dat []  // data
  );
    ##1;
    for (int unsigned i=0; i<len; i++) begin
      str_src.tvalid <= 1'b1;
      str_src.tdata  <= dat [i];
      str_src.tlast  <= lst & (i==len-1);
      str_src.tkeep  <= '1;
      ##1;
    end
    str_src.tvalid <= 1'b0;
    str_src.tdata  <= 'x;
    str_src.tlast  <= 'x;
    str_src.tkeep  <= 'x;
  endtask: src

end: gen_src
else if (SM=="DRN") begin: gen_drn

  clocking str_drn @(posedge str.clk);
    default input #1step output #1step;
    output tready = str.tready;
    input  tvalid = str.tvalid;
    input  tdata  = str.tdata ;
    input  tlast  = str.tlast ;
    input  tkeep  = str.tkeep ;
  endclocking: str_drn

end
endgenerate

endmodule: str_mon
