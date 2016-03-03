////////////////////////////////////////////////////////////////////////////////
// stream
////////////////////////////////////////////////////////////////////////////////

module axi4_stream_src #(
  int unsigned DN = 1,
  type DT = logic [8-1:0],
  logic IV = 1'b0  // idle data bus value
)(
  axi4_stream_if.s str
);

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

task run (axi4_stream_pkg::axi4_stream_class #(.DT (DT)) cls);
  @(clk);
  foreach (cls.mem[i]) begin
    if (cls.mem[i].vld) begin
      idle();
      ##(cls.mem[i].vld);
    end
    clk.TVALID <= 1'b1;
    clk.TDATA  <= cls.mem[i].dat;
    clk.TKEEP  <= cls.mem[i].kep;
    clk.TLAST  <= cls.mem[i].lst;
    do begin
      ##1;
    end while (~clk.TREADY);
  end
  idle();
endtask: run

// set idle state
task idle;
  clk.TVALID <= 1'b0;
  clk.TDATA  <= {$bits(DT){IV}};
  clk.TKEEP  <= {$bits(DN){IV}};
  clk.TLAST  <=            IV  ;
endtask: idle

initial idle();

endmodule: axi4_stream_src
