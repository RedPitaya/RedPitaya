////////////////////////////////////////////////////////////////////////////////
// Module: Red Pitaya TOP package.
// Authors: Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

package top_pkg;

// module numbers
localparam int unsigned MNO = 2;  // number of oscilloscope modules
localparam int unsigned MNG = 2;  // number of generator    modules
localparam int unsigned MNE = 1;  // number of external events

// output events
typedef struct packed {
  // signal and hardware events
  logic lst;  // last
  logic trg;  // trigger
  // software events
  logic swt;  // software trigger
  logic stp;  // software stop
  logic str;  // software start
  logic rst;  // software reset
} evo_t;

// source events
typedef struct packed {
  evo_t           la;   // logic analyzer
  evo_t           lg;   // logic generator
  evo_t [MNO-1:0] osc;  // oscilloscope
  evo_t [MNG-1:0] gen;  // generator
} evs_t;

// drain events
typedef struct packed {
  logic           la;   // logic analyzer
  logic           lg;   // logic generator
  logic [MNO-1:0] osc;  // oscilloscope
  logic [MNG-1:0] gen;  // generator
} evd_t;

// input events events
typedef struct packed {
  struct packed {
    // other events
    logic [MNE-1:0] ext;  // external trigger
    // module events
    evd_t           lst;  // module last
    evd_t           trg;  // module trigger
    // software events
    evd_t           swt;  // software trigger
  } trg;
  evd_t           stp;  // software stop
  evd_t           str;  // software start
  evd_t           rst;  // software reset
} evi_t;

// event mapping function
function evi_t evn_f (
  evs_t           evs,
  logic [MNE-1:0] ext
);
  evi_t evi;
  // other events
  evi.trg.ext = ext;
  // module events
  evi.trg.lst = '{la: evs.la.lst, lg: evs.lg.lst, osc: '{evs.osc[1].lst, evs.osc[0].lst}, gen: '{evs.gen[1].lst, evs.gen[0].lst}};
  evi.trg.trg = '{la: evs.la.trg, lg: evs.lg.trg, osc: '{evs.osc[1].trg, evs.osc[0].trg}, gen: '{evs.gen[1].trg, evs.gen[0].trg}};
  // software events
  evi.trg.swt = '{la: evs.la.swt, lg: evs.lg.swt, osc: '{evs.osc[1].swt, evs.osc[0].swt}, gen: '{evs.gen[1].swt, evs.gen[0].swt}};
  evi.stp     = '{la: evs.la.stp, lg: evs.lg.stp, osc: '{evs.osc[1].stp, evs.osc[0].stp}, gen: '{evs.gen[1].stp, evs.gen[0].stp}};
  evi.str     = '{la: evs.la.str, lg: evs.lg.str, osc: '{evs.osc[1].str, evs.osc[0].str}, gen: '{evs.gen[1].str, evs.gen[0].str}};
  evi.rst     = '{la: evs.la.rst, lg: evs.lg.rst, osc: '{evs.osc[1].rst, evs.osc[0].rst}, gen: '{evs.gen[1].rst, evs.gen[0].rst}};
  return evi;
endfunction: evn_f

// interrupts
typedef struct packed {
  logic           la;   // logic analyzer
  logic           lg;   // logic generator
  logic [MNO-1:0] osc;  // oscilloscope
  logic [MNG-1:0] gen;  // generator
} irq_t;

endpackage: top_pkg
