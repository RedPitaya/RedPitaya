module axi4_sync #(
)(
  axi4_if.s    axi_i,
  axi4_if.m    axi_o
);

always @(posedge axi_i.ACLK) begin
    axi_o.AWID <= axi_i.AWID;
    axi_o.AWADDR <= axi_i.AWADDR;
    axi_o.AWREGION <= axi_i.AWREGION;
    axi_o.AWLEN <= axi_i.AWLEN;
    axi_o.AWSIZE <= axi_i.AWSIZE;
    axi_o.AWBURST <= axi_i.AWBURST;
    axi_o.AWLOCK <= axi_i.AWLOCK;
    axi_o.AWCACHE <= axi_i.AWCACHE;
    axi_o.AWPROT <= axi_i.AWPROT;
    axi_o.AWQOS <= axi_i.AWQOS;
    axi_o.AWVALID <= axi_i.AWVALID;
    axi_i.AWREADY <= axi_o.AWREADY;
    axi_o.WID <= axi_i.WID;
    axi_o.WDATA <= axi_i.WDATA;
    axi_o.WSTRB <= axi_i.WSTRB;
    axi_o.WLAST <= axi_i.WLAST;
    axi_o.WVALID <= axi_i.WVALID;
    axi_i.WREADY <= axi_o.WREADY;
    axi_i.BID <= axi_o.BID;
    axi_i.BRESP <= axi_o.BRESP;
    axi_i.BVALID <= axi_o.BVALID;
    axi_o.BREADY <= axi_i.BREADY;
    axi_o.ARID <= axi_i.ARID;
    axi_o.ARADDR <= axi_i.ARADDR;
    axi_o.ARREGION <= axi_i.ARREGION;
    axi_o.ARLEN <= axi_i.ARLEN;
    axi_o.ARSIZE <= axi_i.ARSIZE;
    axi_o.ARBURST <= axi_i.ARBURST;
    axi_o.ARLOCK <= axi_i.ARLOCK;
    axi_o.ARCACHE <= axi_i.ARCACHE;
    axi_o.ARPROT <= axi_i.ARPROT;
    axi_o.ARQOS <= axi_i.ARQOS;
    axi_o.ARVALID <= axi_i.ARVALID;
    axi_i.ARREADY <= axi_o.ARREADY;
    axi_o.RID <= axi_i.RID;
    axi_o.RDATA <= axi_i.RDATA;
    axi_o.RRESP <= axi_i.RRESP;
    axi_o.RLAST <= axi_i.RLAST;
    axi_o.RVALID <= axi_i.RVALID;
    axi_i.RREADY <= axi_o.RREADY;
end

endmodule: axi4_sync