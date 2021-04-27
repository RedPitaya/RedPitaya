
`timescale 1 ns / 1 ps

module axi_cfg_register #
(
  parameter integer CFG_DATA_WIDTH = 1024,
  parameter integer AXI_DATA_WIDTH = 32,
  parameter integer AXI_ADDR_WIDTH = 32
)
(
  // System signals
  input  wire                        aclk,
  input  wire                        aresetn,

  // Configuration bits
  output wire [CFG_DATA_WIDTH-1:0]   cfg_data,

  // Slave side
  input  wire [AXI_ADDR_WIDTH-1:0]   s_axi_awaddr,  // AXI4-Lite slave: Write address
  input  wire                        s_axi_awvalid, // AXI4-Lite slave: Write address valid
  output wire                        s_axi_awready, // AXI4-Lite slave: Write address ready
  input  wire [AXI_DATA_WIDTH-1:0]   s_axi_wdata,   // AXI4-Lite slave: Write data
  input  wire [AXI_DATA_WIDTH/8-1:0] s_axi_wstrb,   // AXI4-Lite slave: Write strobe
  input  wire                        s_axi_wvalid,  // AXI4-Lite slave: Write data valid
  output wire                        s_axi_wready,  // AXI4-Lite slave: Write data ready
  output wire [1:0]                  s_axi_bresp,   // AXI4-Lite slave: Write response
  output wire                        s_axi_bvalid,  // AXI4-Lite slave: Write response valid
  input  wire                        s_axi_bready,  // AXI4-Lite slave: Write response ready
  input  wire [AXI_ADDR_WIDTH-1:0]   s_axi_araddr,  // AXI4-Lite slave: Read address
  input  wire                        s_axi_arvalid, // AXI4-Lite slave: Read address valid
  output wire                        s_axi_arready, // AXI4-Lite slave: Read address ready
  output wire [AXI_DATA_WIDTH-1:0]   s_axi_rdata,   // AXI4-Lite slave: Read data
  output wire [1:0]                  s_axi_rresp,   // AXI4-Lite slave: Read data response
  output wire                        s_axi_rvalid,  // AXI4-Lite slave: Read data valid
  input  wire                        s_axi_rready   // AXI4-Lite slave: Read data ready
);

  function integer clogb2 (input integer value);
    for(clogb2 = 0; value > 0; clogb2 = clogb2 + 1) value = value >> 1;
  endfunction

  localparam integer ADDR_LSB = clogb2(AXI_DATA_WIDTH/8 - 1);
  localparam integer CFG_SIZE = CFG_DATA_WIDTH/AXI_DATA_WIDTH;
  localparam integer CFG_WIDTH = CFG_SIZE > 1 ? clogb2(CFG_SIZE-1) : 1;

  reg int_bvalid_reg, int_bvalid_next;

  reg int_rvalid_reg, int_rvalid_next;
  reg [AXI_DATA_WIDTH-1:0] int_rdata_reg, int_rdata_next;

  wire [AXI_DATA_WIDTH-1:0] int_data_mux [CFG_SIZE-1:0];
  wire [CFG_DATA_WIDTH-1:0] int_data_wire;
  wire [CFG_SIZE-1:0] int_ce_wire;
  wire int_wvalid_wire;

  genvar j, k;

  assign int_wvalid_wire = s_axi_awvalid & s_axi_wvalid;

  generate
    for(j = 0; j < CFG_SIZE; j = j + 1)
    begin : WORDS
      assign int_data_mux[j] = int_data_wire[j*AXI_DATA_WIDTH+AXI_DATA_WIDTH-1:j*AXI_DATA_WIDTH];
      assign int_ce_wire[j] = int_wvalid_wire & (s_axi_awaddr[ADDR_LSB+CFG_WIDTH-1:ADDR_LSB] == j);
      for(k = 0; k < AXI_DATA_WIDTH; k = k + 1)
      begin : BITS
        FDRE #(
          .INIT(1'b0)
        ) FDRE_inst (
          .CE(int_ce_wire[j] & s_axi_wstrb[k/8]),
          .C(aclk),
          .R(~aresetn),
          .D(s_axi_wdata[k]),
          .Q(int_data_wire[j*AXI_DATA_WIDTH + k])
        );
      end
    end
  endgenerate

  always @(posedge aclk)
  begin
    if(~aresetn)
    begin
      int_bvalid_reg <= 1'b0;
      int_rvalid_reg <= 1'b0;
      int_rdata_reg <= {(AXI_DATA_WIDTH){1'b0}};
    end
    else
    begin
      int_bvalid_reg <= int_bvalid_next;
      int_rvalid_reg <= int_rvalid_next;
      int_rdata_reg <= int_rdata_next;
    end
  end

  always @*
  begin
    int_bvalid_next = int_bvalid_reg;

    if(int_wvalid_wire)
    begin
      int_bvalid_next = 1'b1;
    end

    if(s_axi_bready & int_bvalid_reg)
    begin
      int_bvalid_next = 1'b0;
    end
  end

  always @*
  begin
    int_rvalid_next = int_rvalid_reg;
    int_rdata_next = int_rdata_reg;

    if(s_axi_arvalid)
    begin
      int_rvalid_next = 1'b1;
      int_rdata_next = int_data_mux[s_axi_araddr[ADDR_LSB+CFG_WIDTH-1:ADDR_LSB]];
    end

    if(s_axi_rready & int_rvalid_reg)
    begin
      int_rvalid_next = 1'b0;
    end
  end

  assign cfg_data = int_data_wire;

  assign s_axi_bresp = 2'd0;
  assign s_axi_rresp = 2'd0;

  assign s_axi_awready = int_wvalid_wire;
  assign s_axi_wready = int_wvalid_wire;
  assign s_axi_bvalid = int_bvalid_reg;
  assign s_axi_arready = 1'b1;
  assign s_axi_rdata = int_rdata_reg;
  assign s_axi_rvalid = int_rvalid_reg;

endmodule
