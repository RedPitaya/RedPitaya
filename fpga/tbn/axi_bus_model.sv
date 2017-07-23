////////////////////////////////////////////////////////////////////////////////
//
// AXI bus model (master)
// Copyright (C) 2016 Red Pitaya
// Author: Iztok Jeras <iztok.jeras@redpitaya.com>
// 
// Based on:
// https://code.google.com/p/axi-bfm/
// Copyright (C) 2012 Suvan Kundu <sovan.kundu@gmail.com>
//
////////////////////////////////////////////////////////////////////////////////
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
////////////////////////////////////////////////////////////////////////////////

module axi_bus_model #(
  int unsigned AW = 32,    // address width
  int unsigned DW = 32,    // data width
  int unsigned SW = DW/8,  // select width
  int unsigned IW = 1,     // ID width
  int unsigned LW = 4      // length width
)(
  axi4_if intf
);

typedef struct packed {
  bit [IW-1:0] id    ;
  bit [AW-1:0] addr  ;
  bit  [4-1:0] region;
  bit [LW-1:0] len   ;
  bit  [3-1:0] size  ;
  bit  [2-1:0] burst ;
  bit          lock  ;
  bit  [4-1:0] cache ;
  bit  [3-1:0] prot  ;
  bit  [4-1:0] qos   ;
} ABeat;

typedef struct packed {
  bit [IW-1:0] id  ;
  bit  [2-1:0] resp;  
} BBeat;

typedef struct packed {
  bit [IW-1:0] id  ;
  bit [DW-1:0] data;
  bit  [2-1:0] resp;
  bit          last;
} RBeat;

typedef struct packed {
  bit [IW-1:0] id  ;
  bit [DW-1:0] data;
  bit [SW-1:0] strb;
  bit          last;
} WBeat;

////////////////////////////////////////////////////////////////////////////////
// channel transactions
////////////////////////////////////////////////////////////////////////////////

task ARTransfer (
  input  int unsigned delay,
  input  ABeat        ar
);
  for (int unsigned i=0; i<delay; i++) @(posedge intf.ACLK);
  @(posedge intf.ACLK); // TODO
  intf.ARVALID  = 1'b1     ;
  intf.ARID     = ar.id    ;
  intf.ARADDR   = ar.addr  ;
  intf.ARREGION = ar.region;
  intf.ARLEN    = ar.len   ;
  intf.ARSIZE   = ar.size  ;
  intf.ARBURST  = ar.burst ;
  intf.ARLOCK   = ar.lock  ;
  intf.ARCACHE  = ar.cache ;
  intf.ARPROT   = ar.prot  ;
  intf.ARQOS    = ar.qos   ;
  @(posedge intf.ACLK);
  while (!intf.ARREADY) @(posedge intf.ACLK);
  intf.ARVALID  = 1'b0;
endtask: ARTransfer

task RTransfer (
  input  int unsigned delay,
  output RBeat        r
);
  for (int unsigned i=0; i<delay; i++) @(posedge intf.ACLK);
  @(posedge intf.ACLK); // TODO
  intf.RREADY = 1'b1;
  while(!intf.RVALID) @(posedge intf.ACLK);
  r.id   = intf.RID  ;
  r.data = intf.RDATA;
  r.resp = intf.RRESP;
  r.last = intf.RLAST;
  intf.RREADY <= 1'b0;
endtask: RTransfer

task AWTransfer (
  input  int unsigned delay,
  input  ABeat        aw
);
  for(int i=0; i<delay; i++) @(posedge intf.ACLK);
  @(posedge intf.ACLK); // TODO
  intf.AWVALID  = 1'b1     ;
  intf.AWID     = aw.id    ;
  intf.AWADDR   = aw.addr  ;
  intf.AWREGION = aw.region;
  intf.AWLEN    = aw.len   ;
  intf.AWSIZE   = aw.size  ;
  intf.AWBURST  = aw.burst ;
  intf.AWLOCK   = aw.lock  ;
  intf.AWCACHE  = aw.cache ;
  intf.AWPROT   = aw.prot  ;
  intf.AWQOS    = aw.qos   ;
  @(posedge intf.ACLK);
  while (!intf.AWREADY) @(posedge intf.ACLK);
  intf.AWVALID  = 1'b0     ;
endtask: AWTransfer

task WTransfer (
  input  int unsigned delay,
  input  WBeat        w
);
  for (int unsigned i=0; i<delay; i++) @(posedge intf.ACLK);
  @(posedge intf.ACLK); // TODO
  intf.WVALID = 1'b1   ;
  intf.WID    = w.id  ;
  intf.WDATA  = w.data;
  intf.WSTRB  = w.strb;
  intf.WLAST  = w.last;
  @(posedge intf.ACLK);
  while (!intf.WREADY) @(posedge intf.ACLK);
  intf.WVALID = 1'b0   ;
endtask: WTransfer

task BTransfer (
  input  int unsigned delay,
  output BBeat        b
);
  for(int i=0; i<delay; i++) @(posedge intf.ACLK);
  @(posedge intf.ACLK); // TODO
  intf.BREADY = 1'b1;
  while(!intf.BVALID) @(posedge intf.ACLK);
  b.id   = intf.BID;
  b.resp = intf.BRESP;
  intf.BREADY = 1'b0;
endtask: BTransfer

////////////////////////////////////////////////////////////////////////////////
// single transactions
////////////////////////////////////////////////////////////////////////////////

task ReadTransaction (
  input  int unsigned ARDelay,
  input  ABeat        ar,
  input  int unsigned  RDelay,
  output RBeat         r
);
  ARTransfer(ARDelay, ar);
   RTransfer( RDelay,  r);
endtask: ReadTransaction

task WriteTransaction (
  input  int unsigned AWDelay,
  input  ABeat        aw,
  input  int unsigned  WDelay,
  input  WBeat         w,
  input  int unsigned  BDelay,
  output BBeat         b
);
  fork
    AWTransfer(AWDelay, aw);
     WTransfer( WDelay,  w);
  join
  BTransfer(BDelay, b);
endtask: WriteTransaction
  
////////////////////////////////////////////////////////////////////////////////
// burst transactions
////////////////////////////////////////////////////////////////////////////////

task RBurst (
  input bit [IW-1:0] id,
  input int          len,
  inout byte         data[],
  inout bit    [2:0] resp[]
);
  bit          [IW-1:0]        id_t  ;
  bit [256-1:0][SW-1:0][8-1:0] data_t;
  bit                  [2-1:0] resp_t;
  bit                          last_t;
  automatic int unsigned j=0;
  for (int unsigned i=0; i<256; i++) begin
    RBeat b;
    RTransfer(0, b);
    {id_t, data_t[j], resp_t, last_t} = b;
    if (id_t == id) begin
      j++;
      if (last_t)
      break;
    end
  end
endtask: RBurst

task WBurst (
  input bit [IW-1:0] id,
  input int          len,
  input byte         data[],
  input bit          strb[]
);
  bit [SW-1:0][8-1:0] data_t;
  bit [SW-1:0]        strb_t;
  bit                 last_t;
  for(int unsigned i=0; i<len; i++) begin
    for(int unsigned j=0; j<SW; j++) begin
      data_t[j] = data[SW*i+j];
      strb_t[j] = strb[SW*i+j];
    end
    last_t = (i == (len -1));
    WTransfer(0, WBeat'{id, data_t, strb_t, last_t});
  end
endtask: WBurst

////////////////////////////////////////////////////////////////////////////////
// continuous run
////////////////////////////////////////////////////////////////////////////////

int unsigned AWDelay;
int unsigned  WDelay;
int unsigned ARDelay;
int unsigned  RDelay;
int unsigned  BDelay;

ABeat AR_Q [$];
RBeat  R_Q [$];
ABeat AW_Q [$];
WBeat  W_Q [$];
BBeat  B_Q [$];

task Run;
  ABeat ar;
  RBeat  r;
  ABeat aw;
  WBeat  w;
  BBeat  b;
  fork
    forever begin
      ar = AR_Q.pop_back();
      ARTransfer(ARDelay, ar);
    end
    forever begin
      RTransfer(RDelay, r);
      R_Q.push_back(r);
    end
    forever begin
      aw = AW_Q.pop_back();
      AWTransfer(AWDelay, aw);
    end
    forever begin
      w = W_Q.pop_back();
      WTransfer(WDelay, w);
    end
    forever begin
      BTransfer(BDelay, b);
      B_Q.push_back(b);
    end
  join
endtask: Run

////////////////////////////////////////////////////////////////////////////////
// reset
////////////////////////////////////////////////////////////////////////////////

always @(negedge intf.ARESETn, posedge intf.ACLK)
if (!intf.ARESETn) begin
  // address read
  intf.ARID     <= '0;
  intf.ARADDR   <= '0;
  intf.ARREGION <= '0;
  intf.ARLEN    <= '0;
  intf.ARSIZE   <= $clog2(SW);
  intf.ARBURST  <= 2'b01;
  intf.ARLOCK   <= '0;
  intf.ARCACHE  <= '0;
  intf.ARPROT   <= '0;
  intf.ARQOS    <= '0;
  intf.ARVALID  <= '0;
  // data read
  intf.RREADY   <= '0;
  // address write
  intf.AWID     <= '0;
  intf.AWADDR   <= '0;
  intf.AWREGION <= '0;
  intf.AWLEN    <= '0;
  intf.AWSIZE   <= $clog2(SW);
  intf.AWBURST  <= 2'b01;
  intf.AWLOCK   <= '0;
  intf.AWCACHE  <= '0;
  intf.AWPROT   <= '0;
  intf.AWQOS    <= '0;
  intf.AWVALID  <= '0;
  // data write
  intf.WID      <= '0;
  intf.WDATA    <= '0;
  intf.WSTRB    <= '1;
  intf.WLAST    <= '0;
  intf.WVALID   <= '0;
  // response write
  intf.BREADY   <= '0;
end

endmodule: axi_bus_model
