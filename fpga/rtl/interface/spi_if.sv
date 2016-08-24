interface spi_if #(
  int unsigned SSN = 3  // slave select width
);

logic sclk_i;
logic sclk_o;
logic sclk_t;
logic miso_i;
logic miso_o;
logic miso_t;
logic mosi_i;
logic mosi_o;
logic mosi_t;
logic ss1_o ;
logic ss2_o ;
logic ss_i  ;
logic ss_o  ;
logic ss_t  ;

modport m (
  input  sclk_i,
  output sclk_o,
  output sclk_t,
  input  miso_i,
  output miso_o,
  output miso_t,
  input  mosi_i,
  output mosi_o,
  output mosi_t,
  input  ss_i  ,
  output ss_o  ,
  output ss_t  ,
  output ss1_o ,
  output ss2_o
);

modport s (
  output sclk_i,
  input  sclk_o,
  input  sclk_t,
  output miso_i,
  input  miso_o,
  input  miso_t,
  output mosi_i,
  input  mosi_o,
  input  mosi_t,
  output ss_i  ,
  input  ss_o  ,
  input  ss_t  ,
  input  ss1_o ,
  input  ss2_o
);

endinterface: spi_if
