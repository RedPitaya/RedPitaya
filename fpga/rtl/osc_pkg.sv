////////////////////////////////////////////////////////////////////////////////
// Module: Red Pitaya oscilloscope package.
// Authors: Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

package osc_pkg;

// oscilloscope events
typedef struct packed {
  logic lst;  // last
  logic lvl;  // level/edge
  logic trg;  // software trigger
  logic stp;  // software stop
  logic str;  // software start
  logic rst;  // software reset
} evn_t;

endpackage: osc_pkg
