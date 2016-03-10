////////////////////////////////////////////////////////////////////////////////
// Module: AXI4-Stream drain
// Author: Iztok Jeras
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

module axi4_stream_drn #(
  int unsigned DN = 1,
  type DT = logic [8-1:0]
)(
  axi4_stream_if.d str
);

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

// on transfer store data in the queue
task automatic run (ref axi4_stream_pkg::axi4_stream_class #(.DN (DN), .DT (DT)) cls);
  @(clk);
  while (cls.que.size()) begin
    idle();
    cls.pkt = cls.que.pop_front();
    foreach (cls.pkt[i]) begin
      if (cls.pkt[i].rdy) begin
        idle();
        ##(cls.pkt[i].rdy);
      end
      clk.TREADY <= 1'b1;
      do begin
        ##1;
      end while (~clk.TVALID);
      // TODO: with the proper operator ModelSim reports an error
      cls.pkt[i].dat = clk.TDATA;
      cls.pkt[i].kep = clk.TKEEP;
      cls.pkt[i].lst <= clk.TLAST;
    end
    cls.pkt.delete();
    idle();
  end
endtask: run

// set idle state
task idle;
  clk.TREADY <= 1'b0;
endtask: idle

initial idle();

endmodule: axi4_stream_drn
