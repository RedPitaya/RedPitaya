interface spi_if #(
  int unsigned DW = 4,  // data width
  int unsigned SSN = 3  // slave select width
);

// serial clock
logic sck_i;
logic sck_o;
logic sck_t;
// quad IO [HOLD#, WP#, MISO, MOSI]
logic [DW-1:0] io_i;
logic [DW-1:0] io_o;
logic [DW-1:0] io_t;
// primary slave select
logic ss_i;
logic ss_o;
logic ss_t;
// slave select
logic ss1_o;
logic ss2_o;

// master
modport m (
  input  sck_i,
  output sck_o,
  output sck_t,
  input  io_i ,
  output io_o ,
  output io_t ,
  input  ss_i ,
  output ss_o ,
  output ss_t ,
  output ss1_o,
  output ss2_o
);

// slave
modport s (
  output sck_i,
  input  sck_o,
  input  sck_t,
  output io_i ,
  input  io_o ,
  input  io_t ,
  output ss_i ,
  input  ss_o ,
  input  ss_t ,
  input  ss1_o,
  input  ss2_o
);

endinterface: spi_if
