
`timescale 1 ns / 1 ps

	module BlockImage_v1_0 #
	(
		// Users to add parameters here
		parameter integer SCREEN_HEIGHT	= 600,
		parameter integer SCREEN_WIDTH	= 800,
		
		parameter integer RESET_POSX = 10,
		parameter integer RESET_POSY = 10,
		parameter integer RESET_SIZEX = 10,
		parameter integer RESET_SIZEY = 10,
		parameter integer RESET_COLOR = 1,
		// User parameters ends
		// Do not modify the parameters beyond this line

		// Parameters of Axi Slave Bus Interface S00_AXI
		parameter integer C_S00_AXI_DATA_WIDTH	= 32,
		parameter integer C_S00_AXI_ADDR_WIDTH	= 5
	)
	(
		// Users to add ports here
		input wire [10 : 0] hst,
		input wire [9 : 0] vst,
        input wire [2 : 0] rgb_i,
		output wire [2 : 0] rgb_o,
		// User ports ends
		// Do not modify the ports beyond this line


		// Ports of Axi Slave Bus Interface S00_AXI
		input wire  s00_axi_aclk,
		input wire  s00_axi_aresetn,
		input wire [C_S00_AXI_ADDR_WIDTH-1 : 0] s00_axi_awaddr,
		input wire [2 : 0] s00_axi_awprot,
		input wire  s00_axi_awvalid,
		output wire  s00_axi_awready,
		input wire [C_S00_AXI_DATA_WIDTH-1 : 0] s00_axi_wdata,
		input wire [(C_S00_AXI_DATA_WIDTH/8)-1 : 0] s00_axi_wstrb,
		input wire  s00_axi_wvalid,
		output wire  s00_axi_wready,
		output wire [1 : 0] s00_axi_bresp,
		output wire  s00_axi_bvalid,
		input wire  s00_axi_bready,
		input wire [C_S00_AXI_ADDR_WIDTH-1 : 0] s00_axi_araddr,
		input wire [2 : 0] s00_axi_arprot,
		input wire  s00_axi_arvalid,
		output wire  s00_axi_arready,
		output wire [C_S00_AXI_DATA_WIDTH-1 : 0] s00_axi_rdata,
		output wire [1 : 0] s00_axi_rresp,
		output wire  s00_axi_rvalid,
		input wire  s00_axi_rready
	);
// Instantiation of Axi Bus Interface S00_AXI
	BlockImage_v1_0_S00_AXI # ( 
		.SCREEN_HEIGHT(SCREEN_HEIGHT),
		.SCREEN_WIDTH(SCREEN_WIDTH),
		.RESET_POSX(RESET_POSX),
		.RESET_POSY(RESET_POSY),
		.RESET_SIZEX(RESET_SIZEX),
		.RESET_SIZEY(RESET_SIZEY),
		.RESET_COLOR(RESET_COLOR),
		
		.C_S_AXI_DATA_WIDTH(C_S00_AXI_DATA_WIDTH),
		.C_S_AXI_ADDR_WIDTH(C_S00_AXI_ADDR_WIDTH)
	) BlockImage_v1_0_S00_AXI_inst (
	    .hst(hst),
	    .vst(vst),
	    .rgb_i(rgb_i),
	    .rgb_o(rgb_o),
		.S_AXI_ACLK(s00_axi_aclk),
		.S_AXI_ARESETN(s00_axi_aresetn),
		.S_AXI_AWADDR(s00_axi_awaddr),
		.S_AXI_AWPROT(s00_axi_awprot),
		.S_AXI_AWVALID(s00_axi_awvalid),
		.S_AXI_AWREADY(s00_axi_awready),
		.S_AXI_WDATA(s00_axi_wdata),
		.S_AXI_WSTRB(s00_axi_wstrb),
		.S_AXI_WVALID(s00_axi_wvalid),
		.S_AXI_WREADY(s00_axi_wready),
		.S_AXI_BRESP(s00_axi_bresp),
		.S_AXI_BVALID(s00_axi_bvalid),
		.S_AXI_BREADY(s00_axi_bready),
		.S_AXI_ARADDR(s00_axi_araddr),
		.S_AXI_ARPROT(s00_axi_arprot),
		.S_AXI_ARVALID(s00_axi_arvalid),
		.S_AXI_ARREADY(s00_axi_arready),
		.S_AXI_RDATA(s00_axi_rdata),
		.S_AXI_RRESP(s00_axi_rresp),
		.S_AXI_RVALID(s00_axi_rvalid),
		.S_AXI_RREADY(s00_axi_rready)
	);

	// Add user logic here

//    reg [15 : 0] posx;
//    reg [15 : 0] posy;
//    reg [15 : 0] sizex;
//    reg [15 : 0] sizey;
//    reg [2 : 0] draw_color;
//		reg [C_S00_AXI_DATA_WIDTH-1 : 0] readReg;
		 
//		assign s00_axi_rdata = readReg;
		
//    always @(posedge s00_axi_aclk)
//    begin
//    	if (s00_axi_aresetn == 0) begin
//        posx = RESET_POSX;
//        posy = RESET_POSY;
//        sizex = RESET_SIZEX;
//        sizey = RESET_SIZEY;
//        draw_color = RESET_COLOR;
//      end else begin
//      	if (s00_axi_arready & s00_axi_arvalid & ~s00_axi_rvalid) begin
//      		// reading
//					case (s00_axi_araddr[C_S00_AXI_ADDR_WIDTH-1 : 0])
//						5'h0: readReg <= posx;
//						5'h4: readReg <= posy;
//						5'h8: readReg <= sizex;
//						5'hc: readReg <= sizey;
//						5'h10: readReg <= draw_color;
//	      	endcase
//      	end
//      	if (s00_axi_awready && s00_axi_wready && s00_axi_wvalid && s00_axi_awvalid) begin
//      		// writing
//					case (s00_axi_awaddr[C_S00_AXI_ADDR_WIDTH-1 : 0])
//						5'h0: posx <= s00_axi_wdata;
//						5'h4: posy <= s00_axi_wdata;
//						5'h8: sizex <= s00_axi_wdata;
//						5'hc: sizey <= s00_axi_wdata;
//						5'h10: draw_color <= s00_axi_wdata;
//	          default : begin
//							posx <= posx;
//							posy <= posy;
//							sizex <= sizex;
//							sizey <= sizey;
//							draw_color <= draw_color;
//						end
//	      	endcase
//      	end
//			end
//    end
	 
//    RectPic # 
//    (
//    	SCREEN_HEIGHT,
//    	SCREEN_WIDTH	
//    ) pic_inst (
//        .clk50(s00_axi_aclk),
//        .hst(hst),
//        .vst(vst),
//        .block_posx(posx),
//        .block_posy(posy),
//        .block_sizex(sizex),
//        .block_sizey(sizey),
//        .draw_color(draw_color),
//        .rgb_i(rgb_i),
//        .rgb_o(rgb_o)
//    );
	// User logic ends

	endmodule
