////////////////////////////////////////////////////////////////////////////////
// AXI4-Stream pass through
// Authors: Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

module axi4_stream_pas #(
  int unsigned DNI = 1,  // data number for input
  int unsigned DNO = 1   // data number for output
)(
  // control
  input  logic     ena,  // enable
  // streams
  axi4_stream_if.d sti,  // input
  axi4_stream_if.s sto   // output
);

// hanshaking
assign sti.TREADY = sto.TREADY & ena;
assign sto.TVALID = sti.TVALID & ena;

// data number dependant signals
generate
if (DNI == DNO) begin: pas_eq

  assign sto.TDATA = sti.TDATA;
  assign sto.TKEEP = sti.TKEEP;

end: pas_eq
else if (DNO > DNI) begin: pas_split

  // TODO only DNO==2 DNI==1 is supported for now
  for (genvar i=0; i<DNO; i++) begin: for_dno
    assign sto.TDATA[i+:2] =    sti.TDATA[i]  ;
    assign sto.TKEEP[i+:2] = {2{sti.TKEEP[i]}};
  end: for_dno

end: pas_split
else if (DNO < DNI) begin: pas_join

  // TODO only DNO==1 DNI==2 is supported for now
  assign sto.TDATA = sti.TDATA[0+:2];
  assign sto.TKEEP = sti.TKEEP[0+:2];

end: pas_join
endgenerate

// simple control signal
assign sto.TLAST = sti.TLAST;

endmodule: axi4_stream_pas
