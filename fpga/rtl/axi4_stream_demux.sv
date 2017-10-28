////////////////////////////////////////////////////////////////////////////////
// Module: AXI4-Stream demux
// Authors: Iztok Jeras, Miha Cankar
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

module axi4_stream_demux #(
  // select parameters
  int unsigned SN = 2,          // select number of ports
  int unsigned SW = $clog2(SN), // select signal width
  // data stream parameters
  int unsigned DN = 1,
  type DT = logic [8-1:0]
)(
  // control
  input  logic [SW-1:0] sel,  // select
  // streams
  axi4_stream_if.d sti,          // input
  axi4_stream_if.s sto [SN-1:0]  // output
);

logic [SN-1:0] tready;

generate
for (genvar i=0; i<SN; i++) begin: for_str

assign sto[i].TVALID = (i==sel)             ; // data from ADC is always valid
assign sto[i].TDATA  =            sti.TDATA ;
assign sto[i].TKEEP  =            sti.TKEEP ;
assign sto[i].TLAST  =            sti.TLAST ;

assign tready[i] = sto[i].TREADY;

end: for_str
endgenerate

assign sti.TREADY = 1'b1;// oscilloscope must handle all data

endmodule: axi4_stream_demux
