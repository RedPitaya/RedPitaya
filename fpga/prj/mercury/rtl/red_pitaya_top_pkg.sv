////////////////////////////////////////////////////////////////////////////////
// Module: Red Pitaya TOP package.
// Authors: Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

package top_pkg;

// module numbers
localparam int unsigned MNO = 2;              // number of oscilloscope modules
localparam int unsigned MNG = 2;              // number of generator    modules
localparam int unsigned MNS = MNG+MNO+1+1+1;  // number of event source modules

// software events
typedef struct packed {
  evn_pkg::evn_t           ctrg; // complex trigger
  evn_pkg::evn_t           la;   // logic analyzer
  evn_pkg::evn_t           lg;   // logic generator
  evn_pkg::evn_t [MNO-1:0] osc;  // oscilloscope
  evn_pkg::evn_t [MNG-1:0] gen;  // generator
} evn_t;

// trigger events
typedef struct packed {
  logic           ctrg; // complex trigger
  logic           la;   // logic analyzer
  logic           lg;   // logic generator
  logic [MNO-1:0] osc;  // oscilloscope
  logic [MNG-1:0] gen;  // generator
} trg_t;

// interrupts
typedef struct packed {
  logic           la;   // logic analyzer
  logic           lg;   // logic generator
  logic [MNO-1:0] osc;  // oscilloscope
  logic [MNG-1:0] gen;  // generator
} irq_t;

endpackage: top_pkg
