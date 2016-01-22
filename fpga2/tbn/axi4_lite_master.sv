////////////////////////////////////////////////////////////////////////////////
//
// AXI4-Lite master model
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

`timescale 1ns / 1ps

module axi4_lite_master #(
  int unsigned AW = 32,   // address width
  int unsigned DW = 32,   // data width
  int unsigned SW = DW/8  // select width
)(
  axi4_lite_if intf
);

typedef struct packed {
  bit [AW-1:0] addr  ;
  bit  [3-1:0] prot  ;
} ABeat;

typedef struct packed {
  bit  [2-1:0] resp;  
} BBeat;

typedef struct packed {
  bit [DW-1:0] data;
  bit  [2-1:0] resp;
} RBeat;

typedef struct packed {
  bit [DW-1:0] data;
  bit [SW-1:0] strb;
} WBeat;

////////////////////////////////////////////////////////////////////////////////
// channel transactions
////////////////////////////////////////////////////////////////////////////////

task ARTransfer (
  input  int unsigned delay,
  input  ABeat        ar
);
  for (int unsigned i=0; i<delay; i++) @(posedge intf.ACLK);
  #1;
  intf.ARVALID  = 1'b1     ;
  intf.ARADDR   = ar.addr  ;
  intf.ARPROT   = ar.prot  ;
  @(posedge intf.ACLK);
  while (!intf.ARREADY) @(posedge intf.ACLK);
  #1;
  intf.ARVALID  = 1'b0;
endtask: ARTransfer

task automatic RTransfer (
  input  int unsigned delay,
  output RBeat        r
);
  for (int unsigned i=0; i<delay; i++) @(posedge intf.ACLK);
  #1;
  intf.RREADY = 1'b1;
  @(posedge intf.ACLK);
  while(!intf.RVALID) @(posedge intf.ACLK);
  #1;
  r.data = intf.RDATA;
  r.resp = intf.RRESP;
  intf.RREADY <= 1'b0;
endtask: RTransfer

task AWTransfer (
  input  int unsigned delay,
  input  ABeat        aw
);
  for(int i=0; i<delay; i++) @(posedge intf.ACLK);
  #1;
  intf.AWVALID  = 1'b1     ;
  intf.AWADDR   = aw.addr  ;
  intf.AWPROT   = aw.prot  ;
  @(posedge intf.ACLK);
  while (!intf.AWREADY) @(posedge intf.ACLK);
  #1;
  intf.AWVALID  = 1'b0     ;
endtask: AWTransfer

task WTransfer (
  input  int unsigned delay,
  input  WBeat        w
);
  for (int unsigned i=0; i<delay; i++) @(posedge intf.ACLK);
  #1;
  intf.WVALID = 1'b1   ;
  intf.WDATA  = w.data;
  intf.WSTRB  = w.strb;
  @(posedge intf.ACLK);
  while (!intf.WREADY) @(posedge intf.ACLK);
  #1;
  intf.WVALID = 1'b0   ;
endtask: WTransfer

task automatic BTransfer (
  input  int unsigned delay,
  output BBeat        b
);
  for(int i=0; i<delay; i++) @(posedge intf.ACLK);
  #1;
  intf.BREADY = 1'b1;
  @(posedge intf.ACLK);
  while(!intf.BVALID) @(posedge intf.ACLK);
  #1;
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
  fork
    ARTransfer(ARDelay, ar);
     RTransfer( RDelay,  r);
  join
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
     BTransfer( BDelay,  b);
  join
endtask: WriteTransaction
  
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
  intf.ARADDR   <= '0;
  intf.ARPROT   <= '0;
  intf.ARVALID  <= '0;
  // data read
  intf.RREADY   <= '0;
  // address write
  intf.AWADDR   <= '0;
  intf.AWPROT   <= '0;
  intf.AWVALID  <= '0;
  // data write
  intf.WDATA    <= '0;
  intf.WSTRB    <= '1;
  intf.WVALID   <= '0;
  // response write
  intf.BREADY   <= '0;
end

endmodule: axi4_lite_master
