package axi4_stream_pkg;

class axi4_stream_class #(
  int unsigned DN = 1,
  type DT = logic [8-1:0]
);

  typedef struct {
    DT    [DN-1:0] dat;
    logic [DN-1:0] kep;
    logic          lst;
    int unsigned   vld;
    int unsigned   rdy;
  } mem_t;

  mem_t mem [];
  
  task set_packet (
    ref   DT           dat [],
    input int unsigned vld = 0,
    input int unsigned rdy = 0
  );
    mem = new [dat.size()];
    for (int unsigned i=0; i<dat.size(); i++) begin
      mem[i].dat = dat[i];
      mem[i].kep = '1;
      mem[i].lst = i==(dat.size()-1);
      mem[i].vld = vld;
      mem[i].rdy = rdy;
    end
  endtask: set_packet

endclass: axi4_stream_class

endpackage: axi4_stream_pkg
