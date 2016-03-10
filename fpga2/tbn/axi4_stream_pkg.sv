////////////////////////////////////////////////////////////////////////////////
// Module: AXI4-Stream package
// Author: Iztok Jeras
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

package axi4_stream_pkg;

class axi4_stream_class #(
  int unsigned DN = 1,
  type DT = logic [8-1:0]
);

////////////////////////////////////////////////////////////////////////////////
// type definitions
////////////////////////////////////////////////////////////////////////////////

  // data array
  typedef DT DT_A [];

  // data transfer type
  typedef struct {
    DT    [DN-1:0] dat;
    logic [DN-1:0] kep;
    logic          lst;
    int unsigned   vld;
    int unsigned   rdy;
  } trn_t;

  // data packet type
  typedef trn_t pkt_t [];

  // data queue type
  typedef pkt_t que_t [$];

////////////////////////////////////////////////////////////////////////////////
// generic functions
////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////
// data handlers
////////////////////////////////////////////////////////////////////////////////

  pkt_t pkt;
  que_t que;

  task add_pkt (
    ref   DT_A         dat,
    input bit          lst = 1,
    input int unsigned vld_max = 0,
    input int unsigned vld_rnd = 1,
    input int unsigned rdy_max = 0,
    input int unsigned rdy_rnd = 1,
    input int          seed = 0
  );
    pkt_t pkt;
    pkt = new [dat.size()];
    for (int unsigned i=0; i<dat.size(); i+=DN) begin
      for (int unsigned j=0; j<DN; j++) begin
        pkt[i].dat [j] = dat[DN*i+j];
      end
      pkt[i].kep = ((dat.size()-i)<=DN) ? 1<<(dat.size()-i)-1 : '1; // TODO for now at least check if the last transfer is correct
      pkt[i].lst = ((dat.size()-i)<=DN) & lst;
      // calculate random timing TODO: rethink this part
      pkt[i].vld = $dist_poisson(seed, vld_rnd);
      pkt[i].rdy = $dist_poisson(seed, rdy_rnd);
      // limit to max value
      pkt[i].vld = pkt[i].vld > vld_max ? vld_max : pkt[i].vld;
      pkt[i].rdy = pkt[i].rdy > rdy_max ? vld_max : pkt[i].rdy;
    end
    que.push_back(pkt);
  endtask: add_pkt

  // check data against reference
  function automatic int unsigned check (
    ref DT_A dat
  );
    pkt_t pkt;
    pkt = que.pop_front();
    check = 0;
    // TODO check size
    // TODO check TKEEP
    for (int i=0; i<dat.size(); i+=DN) begin
      for (int unsigned j=0; j<DN; j++) begin
        if ( (pkt[i].dat [j] != dat[i+j]               )
           | (pkt[i].lst     != ((dat.size()-i)<=DN) ) ) begin
          $display ("Error: i=%d: (val=%p/%b) != (ref=%p/%b)", i, pkt[i].dat[j], pkt[i].lst, dat[i+j], ((dat.size()-i)<=DN));
          check++;
        end
      end
    end
  endfunction: check

endclass: axi4_stream_class

endpackage: axi4_stream_pkg
