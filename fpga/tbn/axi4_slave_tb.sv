/**
 * $Id: axi_slave_tb.v 961 2014-01-21 11:40:39Z matej.oblak $
 *
 * @brief Red Pitaya AXI slave testbench.
 *
 * @Author Matej Oblak
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in Verilog hardware description language (HDL).
 * Please visit http://en.wikipedia.org/wiki/Verilog
 * for more details on the language used herein.
 */

/**
 * GENERAL DESCRIPTION:
 *
 * AXI slave testbench.
 *
 * This is testbench to test AXI slave module, which is used to communicate
 * with PS.
 * On one side axi_master_model is used to generate requests on AXI bus, while
 * on second side they are some registers which acts as Red Pitaya bus slave. 
 * 
 */

`timescale 1ns / 1ps

module axi4_slave_tb ();

logic sys_clk ;
logic sys_rstn;

axi4_if #(.DW (32), .AW (32), .IW (12), .LW (4)) axi (.ACLK (sys_clk), .ARESETn (sys_rstn));
sys_bus_if                                       bus (.clk  (sys_clk), .rstn    (sys_rstn));

axi4_slave #(
  .DW (32),
  .AW (32),
  .IW (12)
) axi_slave_gp0 (
  // AXI bus
  .axi       (axi),
  // system read/write channel
  .bus       (bus)
);

axi_master_model #(
  .AW    ( 32    ),
  .DW    ( 32    ),
  .IW    ( 12    ),
  .LW    (  4    )
) master (
  // axi signals
  .aclk_i    (axi.ACLK   ),
  .arstn_i   (axi.ARESETn),
  // axi Write Address Channel Signals
  .awid_o    (axi.AWID   ),
  .awlen_o   (axi.AWLEN  ),
  .awsize_o  (axi.AWSIZE ),
  .awburst_o (axi.AWBURST),
  .awcache_o (axi.AWCACHE),
  .awaddr_o  (axi.AWADDR ),
  .awprot_o  (axi.AWPROT ),
  .awvalid_o (axi.AWVALID),
  .awready_i (axi.AWREADY),
  .awlock_o  (axi.AWLOCK ),
  // axi Write Data Channel Signals
  .wdata_o   (axi.WDATA  ),
  .wstrb_o   (axi.WSTRB  ),
  .wlast_o   (axi.WLAST  ),
  .wvalid_o  (axi.WVALID ),
  .wready_i  (axi.WREADY ),  
  // axi Write Response Channel Signals
  .bid_i     (axi.BID    ),
  .bresp_i   (axi.BRESP  ),
  .bvalid_i  (axi.BVALID ),
  .bready_o  (axi.BREADY ),
  // axi Read Address Channel Signals
  .arid_o    (axi.ARID   ),
  .arlen_o   (axi.ARLEN  ),
  .arsize_o  (axi.ARSIZE ),
  .arburst_o (axi.ARBURST),
  .arprot_o  (axi.ARPROT ),
  .arcache_o (axi.ARCACHE),
  .arvalid_o (axi.ARVALID),
  .araddr_o  (axi.ARADDR ),
  .arlock_o  (axi.ARLOCK ),
  .arready_i (axi.ARREADY),
  // axi Read Data Channel Sigals
  .rid_i     (axi.RID    ),
  .rdata_i   (axi.RDATA  ),
  .rresp_i   (axi.RRESP  ),
  .rvalid_i  (axi.RVALID ),
  .rlast_i   (axi.RLAST  ),
  .rready_o  (axi.RREADY ) 
);

// since the PS GP0 port is AXI3 and the local bus is AXI4
assign axi.AWREGION = '0;
assign axi.ARREGION = '0;

////////////////////////////////////////////////////////////////////////////////
// system clock and reset
////////////////////////////////////////////////////////////////////////////////

initial
begin
   sys_rstn = 1'b0;
   repeat(10) @(posedge sys_clk);
   sys_rstn = 1'b1;
end

initial   sys_clk = 1'b0;
always #5 sys_clk = ~sys_clk;

////////////////////////////////////////////////////////////////////////////////
// Registers connected to system bus
////////////////////////////////////////////////////////////////////////////////

logic [ 4-1:0] rd_ack;
logic [32-1:0] reg_a ;
logic [32-1:0] reg_b ;
logic [32-1:0] reg_c ;

always_ff @(posedge bus.clk)
if (bus.rstn == 1'b0) begin
  rd_ack <=  4'h0       ;
  reg_a  <= 32'h0       ;
  reg_b  <= 32'h12345678;
  reg_c  <= 32'h505     ;
end else begin
  rd_ack <= {rd_ack[2:0], (bus.ren || bus.wen)};
  if (bus.wen) begin
    if (bus.addr[9:0]==10'h0)  reg_a <= bus.wdata;
    if (bus.addr[9:0]==10'h4)  reg_b <= bus.wdata;
    if (bus.addr[9:0]==10'h8)  reg_c <= bus.wdata;
  end
end

always_comb
begin
  bus.err = 1'b0 ;
  casez (bus.addr[9:0])
      10'h0 : begin bus.ack = 1'b1;      bus.rdata = reg_a; end 
      10'h4 : begin bus.ack = rd_ack[3]; bus.rdata = reg_b; end
      10'h8 : begin bus.ack = 1'b1;      bus.rdata = reg_c; end 
    default : begin bus.ack = 1'b0;      bus.rdata = 32'h0; end
  endcase
end

////////////////////////////////////////////////////////////////////////////////
// Read/write commands
////////////////////////////////////////////////////////////////////////////////

logic [32-1: 0] rdat;
logic [ 2-1: 0] resp;

initial
begin
  wait (sys_rstn)
  //                                               addr  , wdat        , id   , size, lock, prot  , rdat, resp
  repeat(10) @(posedge sys_clk);  master.wr_single(32'h20, 32'h33445566, 12'h0, 3'h2, 2'h0, 3'b010,       resp);  // no register behind
  repeat(10) @(posedge sys_clk);  master.wr_single(32'h00, 32'h66666666, 12'h0, 3'h2, 2'h0, 3'b010,       resp);
  repeat(10) @(posedge sys_clk);  master.rd_single(32'h04,               12'h0, 3'h2, 2'h0, 3'b010, rdat, resp);
  repeat(10) @(posedge sys_clk);  master.rd_single(32'h00,               12'h0, 3'h1, 2'h0, 3'b010, rdat, resp);  // unsupported size
  repeat(10) @(posedge sys_clk);  master.rd_single(32'h14,               12'h0, 3'h2, 2'h0, 3'b010, rdat, resp);  // no register behind

  repeat(10) @(posedge sys_clk);  master.rd_single(32'h04,               12'h0, 3'h2, 2'h0, 3'b010, rdat, resp);
  repeat(10) @(posedge sys_clk);  master.rd_single(32'h00,               12'h0, 3'h2, 2'h0, 3'b010, rdat, resp);
  repeat(10) @(posedge sys_clk);  master.wr_single(32'h04, 32'h00000000, 12'h0, 3'h1, 2'h0, 3'b010,       resp);  // unsupported size
  repeat(10) @(posedge sys_clk);  master.wr_single(32'h04, 32'h00000444, 12'h0, 3'h2, 2'h0, 3'b010,       resp);
  repeat(10) @(posedge sys_clk);  master.rd_single(32'h04,               12'h0, 3'h2, 2'h0, 3'b010, rdat, resp);

  repeat(10) @(posedge sys_clk);  master.rd_single(32'h08,               12'h0, 3'h2, 2'h0, 3'b010, rdat, resp);
  repeat(10) @(posedge sys_clk);  master.wr_single(32'h00, 32'h33445566, 12'h0, 3'h2, 2'h0, 3'b010,       resp);
  repeat(10) @(posedge sys_clk);  master.wr_single(32'h08, 32'h00000606, 12'h0, 3'h2, 2'h0, 3'b010,       resp);

  repeat(20000) @(posedge sys_clk);
  $finish();
end

endmodule: axi4_slave_tb
