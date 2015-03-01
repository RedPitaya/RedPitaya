////////////////////////////////////////////////////////////////////////////////
// IIR filter using AXI4-Stream interface
// (C) 2015 Iztok Jeras
////////////////////////////////////////////////////////////////////////////////

interface iir #(
  int unsigned DN = 1,  // data number
  int unsigned DW = 8,  // data width
  // coeficients
  int unsigned ACN = 1,
  int unsigned BCN = 1
  
  int unsigned CW = 8   // coeficient width
)(
  str_if.drn sti,
  str_if.src sto,
);



endmodule: iir
