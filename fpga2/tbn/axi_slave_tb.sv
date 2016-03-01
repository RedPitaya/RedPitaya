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

module axi_slave_tb ();

wire             axi_arvalid     ;
wire             axi_awvalid     ;
wire             axi_bready      ;
wire             axi_rready      ;
wire             axi_wlast       ;
wire             axi_wvalid      ;
wire  [ 12-1: 0] axi_arid        ;
wire  [ 12-1: 0] axi_awid        ;
wire  [ 12-1: 0] axi_wid         ;
wire  [  2-1: 0] axi_arburst     ;
wire  [  2-1: 0] axi_arlock      ;
wire  [  3-1: 0] axi_arsize      ;
wire  [  2-1: 0] axi_awburst     ;
wire  [  2-1: 0] axi_awlock      ;
wire  [  3-1: 0] axi_awsize      ;
wire  [  3-1: 0] axi_arprot      ;
wire  [  3-1: 0] axi_awprot      ;
wire  [ 32-1: 0] axi_araddr      ;
wire  [ 32-1: 0] axi_awaddr      ;
wire  [ 32-1: 0] axi_wdata       ;
wire  [  4-1: 0] axi_arcache     ;
wire  [  4-1: 0] axi_arlen       ;
wire  [  4-1: 0] axi_arqos       ;
wire  [  4-1: 0] axi_awcache     ;
wire  [  4-1: 0] axi_awlen       ;
wire  [  4-1: 0] axi_awqos       ;
wire  [  4-1: 0] axi_wstrb       ;
reg              axi_aclk        ;
wire             axi_arready     ;
wire             axi_awready     ;
wire             axi_bvalid      ;
wire             axi_rlast       ;
wire             axi_rvalid      ;
wire             axi_wready      ;
wire  [ 12-1: 0] axi_bid         ;
wire  [ 12-1: 0] axi_rid         ;
wire  [  2-1: 0] axi_bresp       ;
wire  [  2-1: 0] axi_rresp       ;
wire  [ 32-1: 0] axi_rdata       ;
reg              axi_arstn       ;

wire             sys_clk         ;
wire             sys_rstn        ;
wire  [ 32-1: 0] sys_addr        ;
wire  [ 32-1: 0] sys_wdata       ;
wire  [  4-1: 0] sys_sel         ;
wire             sys_wen         ;
wire             sys_ren         ;
reg   [ 32-1: 0] sys_rdata       ;
reg              sys_err         ;
reg              sys_ack         ;

axi_master_model #(
  .AW    ( 32    ),
  .DW    ( 32    ),
  .IW    ( 12    ),
  .LW    (  4    )
) master (
  // axi signals
  .aclk_i         (  axi_aclk        ),
  .arstn_i        (  axi_arstn       ),
  // axi Write Address Channel Signals
  .awid_o         (  axi_awid        ),
  .awlen_o        (  axi_awlen       ),
  .awsize_o       (  axi_awsize      ),
  .awburst_o      (  axi_awburst     ),
  .awcache_o      (  axi_awcache     ),
  .awaddr_o       (  axi_awaddr      ),
  .awprot_o       (  axi_awprot      ),
  .awvalid_o      (  axi_awvalid     ),
  .awready_i      (  axi_awready     ),
  .awlock_o       (  axi_awlock      ),
  // axi Write Data Channel Signals
  .wdata_o        (  axi_wdata       ),
  .wstrb_o        (  axi_wstrb       ),
  .wlast_o        (  axi_wlast       ),
  .wvalid_o       (  axi_wvalid      ),
  .wready_i       (  axi_wready      ),  
  // axi Write Response Channel Signals
  .bid_i          (  axi_bid         ),
  .bresp_i        (  axi_bresp       ),
  .bvalid_i       (  axi_bvalid      ),
  .bready_o       (  axi_bready      ),
  // axi Read Address Channel Signals
  .arid_o         (  axi_arid        ),
  .arlen_o        (  axi_arlen       ),
  .arsize_o       (  axi_arsize      ),
  .arburst_o      (  axi_arburst     ),
  .arprot_o       (  axi_arprot      ),
  .arcache_o      (  axi_arcache     ),
  .arvalid_o      (  axi_arvalid     ),
  .araddr_o       (  axi_araddr      ),
  .arlock_o       (  axi_arlock      ),
  .arready_i      (  axi_arready     ),
  // axi Read Data Channel Sigals
  .rid_i          (  axi_rid         ),
  .rdata_i        (  axi_rdata       ),
  .rresp_i        (  axi_rresp       ),
  .rvalid_i       (  axi_rvalid      ),
  .rlast_i        (  axi_rlast       ),
  .rready_o       (  axi_rready      ) 
);

axi_slave #(
  .AXI_DW     (  32     ), // data width (8,16,...,1024)
  .AXI_AW     (  32     ), // address width
  .AXI_IW     (  12     )  // ID width
) slave (
  // global signals
  .axi_clk_i        (  axi_aclk           ),  // global clock
  .axi_rstn_i       (  axi_arstn          ),  // global reset
  // axi write address channel
  .axi_awid_i       (  axi_awid           ),  // write address ID
  .axi_awaddr_i     (  axi_awaddr         ),  // write address
  .axi_awlen_i      (  axi_awlen          ),  // write burst length
  .axi_awsize_i     (  axi_awsize         ),  // write burst size
  .axi_awburst_i    (  axi_awburst        ),  // write burst type
  .axi_awlock_i     (  axi_awlock         ),  // write lock type
  .axi_awcache_i    (  axi_awcache        ),  // write cache type
  .axi_awprot_i     (  axi_awprot         ),  // write protection type
  .axi_awvalid_i    (  axi_awvalid        ),  // write address valid
  .axi_awready_o    (  axi_awready        ),  // write ready
  // axi write data channel
  .axi_wid_i        (  axi_wid            ),  // write data ID
  .axi_wdata_i      (  axi_wdata          ),  // write data
  .axi_wstrb_i      (  axi_wstrb          ),  // write strobes
  .axi_wlast_i      (  axi_wlast          ),  // write last
  .axi_wvalid_i     (  axi_wvalid         ),  // write valid
  .axi_wready_o     (  axi_wready         ),  // write ready
  // axi write response channel
  .axi_bid_o        (  axi_bid            ),  // write response ID
  .axi_bresp_o      (  axi_bresp          ),  // write response
  .axi_bvalid_o     (  axi_bvalid         ),  // write response valid
  .axi_bready_i     (  axi_bready         ),  // write response ready
  // axi read address channel
  .axi_arid_i       (  axi_arid           ),  // read address ID
  .axi_araddr_i     (  axi_araddr         ),  // read address
  .axi_arlen_i      (  axi_arlen          ),  // read burst length
  .axi_arsize_i     (  axi_arsize         ),  // read burst size
  .axi_arburst_i    (  axi_arburst        ),  // read burst type
  .axi_arlock_i     (  axi_arlock         ),  // read lock type
  .axi_arcache_i    (  axi_arcache        ),  // read cache type
  .axi_arprot_i     (  axi_arprot         ),  // read protection type
  .axi_arvalid_i    (  axi_arvalid        ),  // read address valid
  .axi_arready_o    (  axi_arready        ),  // read address ready
  // axi read data channel
  .axi_rid_o        (  axi_rid            ),  // read response ID
  .axi_rdata_o      (  axi_rdata          ),  // read data
  .axi_rresp_o      (  axi_rresp          ),  // read response
  .axi_rlast_o      (  axi_rlast          ),  // read last
  .axi_rvalid_o     (  axi_rvalid         ),  // read response valid
  .axi_rready_i     (  axi_rready         ),  // read response ready
  // system read/write channel
  .sys_addr_o       (  sys_addr           ),  // system read/write address
  .sys_wdata_o      (  sys_wdata          ),  // system write data
  .sys_sel_o        (  sys_sel            ),  // system write byte select
  .sys_wen_o        (  sys_wen            ),  // system write enable
  .sys_ren_o        (  sys_ren            ),  // system read enable
  .sys_rdata_i      (  sys_rdata          ),  // system read data
  .sys_err_i        (  sys_err            ),  // system error indicator
  .sys_ack_i        (  sys_ack            )   // system acknowledge signal
);

//---------------------------------------------------------------------------------
//
// Registers connected to system bus

assign sys_clk  = axi_aclk    ;
assign sys_rstn = axi_arstn   ;

logic [  4-1: 0] rd_ack   ;
logic [ 32-1: 0] reg_a    ;
logic [ 32-1: 0] reg_b    ;
logic [ 32-1: 0] reg_c    ;

always @(posedge sys_clk)
   if (sys_rstn == 1'b0) begin
      rd_ack <= 4'h0 ;
      reg_a  <= 32'h0         ;
      reg_b  <= 32'h12345678  ;
      reg_c  <= 32'h505       ;
   end else begin
      rd_ack <= {rd_ack[2:0], (sys_ren || sys_wen)};

      if (sys_wen && (sys_addr[9:0]==10'h0))    reg_a <= sys_wdata ;
      if (sys_wen && (sys_addr[9:0]==10'h4))    reg_b <= sys_wdata ;
      if (sys_wen && (sys_addr[9:0]==10'h8))    reg_c <= sys_wdata ;
   end

always_comb
begin
   sys_err = 1'b0 ;
   casez (sys_addr[9:0])
         10'h0 : begin sys_ack = 1'b1;          sys_rdata = reg_a      ; end 
         10'h4 : begin sys_ack = rd_ack[3];     sys_rdata = reg_b      ; end
         10'h8 : begin sys_ack = 1'b1;          sys_rdata = reg_c      ; end 
       default : begin sys_ack = 1'b0;          sys_rdata = 32'h0      ; end
   endcase
end

//---------------------------------------------------------------------------------
//
// Read/write commands

initial
begin
   axi_arstn = 1'b0;
   repeat(10) @(posedge axi_aclk);
   axi_arstn = 1'b1;
end

initial   axi_aclk = 1'b0;
always #5 axi_aclk = !axi_aclk ;

reg [32-1: 0] rdat ;
reg [ 2-1: 0] resp ;

initial
begin
   wait (axi_arstn)
   //                                                addr  , wdat        , id   , size, lock, prot  , rdat, resp
   repeat(10) @(posedge axi_aclk);  master.wr_single(32'h20, 32'h33445566, 12'h0, 3'h2, 2'h0, 3'b010,       resp);  // no register behind
   repeat(10) @(posedge axi_aclk);  master.wr_single(32'h00, 32'h66666666, 12'h0, 3'h2, 2'h0, 3'b010,       resp);
   repeat(10) @(posedge axi_aclk);  master.rd_single(32'h04,               12'h0, 3'h2, 2'h0, 3'b010, rdat, resp);
   repeat(10) @(posedge axi_aclk);  master.rd_single(32'h00,               12'h0, 3'h1, 2'h0, 3'b010, rdat, resp);  // unsupported size
   repeat(10) @(posedge axi_aclk);  master.rd_single(32'h14,               12'h0, 3'h2, 2'h0, 3'b010, rdat, resp);  // no register behind

   repeat(10) @(posedge axi_aclk);  master.rd_single(32'h04,               12'h0, 3'h2, 2'h0, 3'b010, rdat, resp);
   repeat(10) @(posedge axi_aclk);  master.rd_single(32'h00,               12'h0, 3'h2, 2'h0, 3'b010, rdat, resp);
   repeat(10) @(posedge axi_aclk);  master.wr_single(32'h04, 32'h00000000, 12'h0, 3'h1, 2'h0, 3'b010,       resp);  // unsupported size
   repeat(10) @(posedge axi_aclk);  master.wr_single(32'h04, 32'h00000444, 12'h0, 3'h2, 2'h0, 3'b010,       resp);
   repeat(10) @(posedge axi_aclk);  master.rd_single(32'h04,               12'h0, 3'h2, 2'h0, 3'b010, rdat, resp);

   repeat(10) @(posedge axi_aclk);  master.rd_single(32'h08,               12'h0, 3'h2, 2'h0, 3'b010, rdat, resp);
   repeat(10) @(posedge axi_aclk);  master.wr_single(32'h00, 32'h33445566, 12'h0, 3'h2, 2'h0, 3'b010,       resp);
   repeat(10) @(posedge axi_aclk);  master.wr_single(32'h08, 32'h00000606, 12'h0, 3'h2, 2'h0, 3'b010,       resp);

   repeat(20000) @(posedge axi_aclk);
end

endmodule: axi_slave_tb
