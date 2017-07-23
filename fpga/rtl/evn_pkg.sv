////////////////////////////////////////////////////////////////////////////////
// Module: Red Pitaya TOP package.
// Authors: Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

package evn_pkg;

// software events
typedef struct packed {
  logic swt;  // software trigger
  logic stp;  // software stop
  logic str;  // software start
  logic rst;  // software reset
} evn_t;

// trigger events
typedef struct packed {
  logic lst;  // module last
  logic trg;  // module trigger
} trg_t;

endpackage: evn_pkg
