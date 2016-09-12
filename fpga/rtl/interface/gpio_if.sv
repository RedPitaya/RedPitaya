interface gpio_if #(
  int unsigned DW = 8  // data width
);

// IP pad signals
logic [DW-1:0] i;
logic [DW-1:0] o;
logic [DW-1:0] t;

// master
modport m (
  input  i,
  output o,
  output t
);

// slave
modport s (
  output i,
  input  o,
  input  t
);

endinterface: gpio_if
