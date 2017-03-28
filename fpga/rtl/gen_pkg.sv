////////////////////////////////////////////////////////////////////////////////
// Module: Red Pitaya generator.
// Authors: Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

package gen_pkg;

// generator events
typedef struct packed {
  logic lst;  // last
  logic per;  // period
  logic trg;  // software trigger
  logic stp;  // software stop
  logic str;  // software start
  logic rst;  // software reset
} evn_t;

endpackage: gen_pkg
