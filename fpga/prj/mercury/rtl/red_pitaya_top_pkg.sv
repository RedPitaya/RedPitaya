////////////////////////////////////////////////////////////////////////////////
// Module: Red Pitaya TOP package.
// Authors: Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

package top_pkg;

// module numbers
localparam int unsigned MNO = 2;  // number of oscilloscope modules
localparam int unsigned MNG = 2;  // number of generator    modules

// all events
typedef struct packed {
  osc_pkg::evn_t [MNO-1:0] osc;  // oscilloscope
  gen_pkg::evn_t [MNG-1:0] gen;  // generate
} evn_t;

// interrupts
typedef struct packed {
  logic           lg;   // logic generator
  logic           la;   // logic analyzer
  logic [MNO-1:0] osc;  // oscilloscope
  logic [MNG-1:0] gen;  // generate
} irq_t;

endpackage: top_pkg
