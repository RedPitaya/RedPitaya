////////////////////////////////////////////////////////////////////////////////
// Module: Red Pitaya TOP package.
// Authors: Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

package evn_pkg;

// source events
typedef struct packed {
  // signal and hardware events
  logic lst;  // last
  logic trg;  // trigger
  // software events
  logic swt;  // software trigger
  logic stp;  // software stop
  logic str;  // software start
  logic rst;  // software reset
} evs_t;

// trigger events
typedef struct packed {
  // other events
  logic ext;  // external trigger
  // module events
  logic lst;  // module last
  logic trg;  // module trigger
  // software events
  logic swt;  // software trigger
} evt_t;

// drain events
typedef struct packed {
  evt_t trg;  // triggers
  logic stp;  // software stop
  logic str;  // software start
  logic rst;  // software reset
} evd_t;

// event mapping function
function evd_t evn_f (
  input  evs_t evs,
  input  logic ext
);
  evd_t evd;
  // other events
  evd.trg.ext = ext;
  // module events
  evd.trg.lst = evs.lst;
  evd.trg.trg = evs.trg;
  // software events
  evd.trg.swt = evs.swt;
  evd.stp     = evs.stp;
  evd.str     = evs.str;
  evd.rst     = evs.rst;
  return evd;
endfunction: evn_f

endpackage: evn_pkg
