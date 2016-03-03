package axi4_stream_pkg;

class axi4_stream_class #(
  int unsigned DN = 1,
  type DT = logic [8-1:0]
);

  typedef DT DT_A [];

  typedef struct {
    DT    [DN-1:0] dat;
    logic [DN-1:0] kep;
    logic          lst;
    int unsigned   vld;
    int unsigned   rdy;
  } mem_t;

  mem_t mem [];
  
  task set_packet (
    ref   DT_A         dat,
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

  function DT_A range (
    int start,
    int stop,
    int step = 1
  );
    range = new [(stop-start)/step];
    for (int i=0; i<range.size(); i++) begin
      range [i] = start + i * step;
    end
  endfunction: range

  // check data against reference
  function automatic int unsigned check (
    ref DT_A dat
  );
    check = 0;
    // TODO check size
    // TODO check TKEEP
    for (int i=0; i<dat.size(); i++) begin
      if ( (mem[i].dat != dat[i])
         | (mem[i].lst != (i==(dat.size()-1))) ) begin
        $display ("Error: i=%d: (val=%p/%b) != (ref=%p/%b)", i, mem[i].dat, mem[i].lst, dat[i], (i==(dat.size()-1)));
        check++;
      end
    end
  endfunction: check

endclass: axi4_stream_class

endpackage: axi4_stream_pkg
