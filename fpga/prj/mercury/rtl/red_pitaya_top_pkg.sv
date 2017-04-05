////////////////////////////////////////////////////////////////////////////////
// Module: Red Pitaya TOP package.
// Authors: Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

package top_pkg;

// module numbers
localparam int unsigned MNO = 2;            // number of oscilloscope modules
localparam int unsigned MNG = 2;            // number of generator    modules
localparam int unsigned MNS = MNG+MNO+1+1;  // number of event source modules
localparam int unsigned MNE = 1;            // number of external events

// source events
typedef struct packed {
  evn_pkg::evs_t           la;   // logic analyzer
  evn_pkg::evs_t           lg;   // logic generator
  evn_pkg::evs_t [MNO-1:0] osc;  // oscilloscope
  evn_pkg::evs_t [MNG-1:0] gen;  // generator
} evs_t;

// list drain events
typedef struct packed {
  logic           la;   // logic analyzer
  logic           lg;   // logic generator
  logic [MNO-1:0] osc;  // oscilloscope
  logic [MNG-1:0] gen;  // generator
} evl_t;

typedef struct packed {
  // other events
  logic [MNE-1:0] ext;  // external trigger
  // module events
  evl_t           lst;  // module last
  evl_t           trg;  // module trigger
} evt_t;

// input events events
typedef struct packed {
  evt_t           trg;  // hardware triggers
  evl_t           swt;  // software trigger
  evl_t           stp;  // software stop
  evl_t           str;  // software start
  evl_t           rst;  // software reset
} evd_t;

// event mapping function
function evd_t evn_f (
  input  evs_t           evs,
  input  logic [MNE-1:0] ext
);
  evd_t evd;
  // other events
  evd.trg.ext = ext;
  // module events
  evd.trg.lst = '{la: evs.la.lst, lg: evs.lg.lst, osc: '{evs.osc[1].lst, evs.osc[0].lst}, gen: '{evs.gen[1].lst, evs.gen[0].lst}};
  evd.trg.trg = '{la: evs.la.trg, lg: evs.lg.trg, osc: '{evs.osc[1].trg, evs.osc[0].trg}, gen: '{evs.gen[1].trg, evs.gen[0].trg}};
  // software events
  evd.swt     = '{la: evs.la.swt, lg: evs.lg.swt, osc: '{evs.osc[1].swt, evs.osc[0].swt}, gen: '{evs.gen[1].swt, evs.gen[0].swt}};
  evd.stp     = '{la: evs.la.stp, lg: evs.lg.stp, osc: '{evs.osc[1].stp, evs.osc[0].stp}, gen: '{evs.gen[1].stp, evs.gen[0].stp}};
  evd.str     = '{la: evs.la.str, lg: evs.lg.str, osc: '{evs.osc[1].str, evs.osc[0].str}, gen: '{evs.gen[1].str, evs.gen[0].str}};
  evd.rst     = '{la: evs.la.rst, lg: evs.lg.rst, osc: '{evs.osc[1].rst, evs.osc[0].rst}, gen: '{evs.gen[1].rst, evs.gen[0].rst}};
  return evd;
endfunction: evn_f

// interrupts
typedef struct packed {
  logic           la;   // logic analyzer
  logic           lg;   // logic generator
  logic [MNO-1:0] osc;  // oscilloscope
  logic [MNG-1:0] gen;  // generator
} irq_t;

endpackage: top_pkg
