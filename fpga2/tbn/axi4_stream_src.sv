////////////////////////////////////////////////////////////////////////////////
// Module: AXI4-Stream source
// Author: Iztok Jeras
// (c) Red Pitaya  (redpitaya.com)
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

task run (axi4_stream_pkg::axi4_stream_class #(.DN (DN), .DT (DT)) cls);
  @(clk);
  while (cls.que.size()) begin
    idle();
    cls.pkt = cls.que.pop_front();
    foreach (cls.pkt[i]) begin
      if (cls.pkt[i].vld) begin
        idle();
        ##(cls.pkt[i].vld);
      end
      clk.TVALID <= 1'b1;
      clk.TDATA  <= cls.pkt[i].dat;
      clk.TKEEP  <= cls.pkt[i].kep;
      clk.TLAST  <= cls.pkt[i].lst;
      do begin
        ##1;
      end while (~clk.TREADY);
    end
    cls.pkt.delete();
    idle();
  end
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
