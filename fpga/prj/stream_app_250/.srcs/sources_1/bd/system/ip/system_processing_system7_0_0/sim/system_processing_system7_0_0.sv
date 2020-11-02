`timescale 1ns/1ps

//PORTS

      bit  CAN0_PHY_TX;
      bit  CAN0_PHY_RX;
      bit  CAN1_PHY_TX;
      bit  CAN1_PHY_RX;
      bit  [0 : 0] ENET0_GMII_TX_EN;
      bit  [0 : 0] ENET0_GMII_TX_ER;
      bit  ENET0_MDIO_MDC;
      bit  ENET0_MDIO_O;
      bit  ENET0_MDIO_T;
      bit  ENET0_PTP_DELAY_REQ_RX;
      bit  ENET0_PTP_DELAY_REQ_TX;
      bit  ENET0_PTP_PDELAY_REQ_RX;
      bit  ENET0_PTP_PDELAY_REQ_TX;
      bit  ENET0_PTP_PDELAY_RESP_RX;
      bit  ENET0_PTP_PDELAY_RESP_TX;
      bit  ENET0_PTP_SYNC_FRAME_RX;
      bit  ENET0_PTP_SYNC_FRAME_TX;
      bit  ENET0_SOF_RX;
      bit  ENET0_SOF_TX;
      bit  [7 : 0] ENET0_GMII_TXD;
      bit  ENET0_GMII_COL;
      bit  ENET0_GMII_CRS;
      bit  ENET0_GMII_RX_CLK;
      bit  ENET0_GMII_RX_DV;
      bit  ENET0_GMII_RX_ER;
      bit  ENET0_GMII_TX_CLK;
      bit  ENET0_MDIO_I;
      bit  ENET0_EXT_INTIN;
      bit  [7 : 0] ENET0_GMII_RXD;
      bit  [0 : 0] ENET1_GMII_TX_EN;
      bit  [0 : 0] ENET1_GMII_TX_ER;
      bit  ENET1_MDIO_MDC;
      bit  ENET1_MDIO_O;
      bit  ENET1_MDIO_T;
      bit  ENET1_PTP_DELAY_REQ_RX;
      bit  ENET1_PTP_DELAY_REQ_TX;
      bit  ENET1_PTP_PDELAY_REQ_RX;
      bit  ENET1_PTP_PDELAY_REQ_TX;
      bit  ENET1_PTP_PDELAY_RESP_RX;
      bit  ENET1_PTP_PDELAY_RESP_TX;
      bit  ENET1_PTP_SYNC_FRAME_RX;
      bit  ENET1_PTP_SYNC_FRAME_TX;
      bit  ENET1_SOF_RX;
      bit  ENET1_SOF_TX;
      bit  [7 : 0] ENET1_GMII_TXD;
      bit  ENET1_GMII_COL;
      bit  ENET1_GMII_CRS;
      bit  ENET1_GMII_RX_CLK;
      bit  ENET1_GMII_RX_DV;
      bit  ENET1_GMII_RX_ER;
      bit  ENET1_GMII_TX_CLK;
      bit  ENET1_MDIO_I;
      bit  ENET1_EXT_INTIN;
      bit  [7 : 0] ENET1_GMII_RXD;
      bit  [23 : 0] GPIO_I;
      bit  [23 : 0] GPIO_O;
      bit  [23 : 0] GPIO_T;
      bit  I2C0_SDA_I;
      bit  I2C0_SDA_O;
      bit  I2C0_SDA_T;
      bit  I2C0_SCL_I;
      bit  I2C0_SCL_O;
      bit  I2C0_SCL_T;
      bit  I2C1_SDA_I;
      bit  I2C1_SDA_O;
      bit  I2C1_SDA_T;
      bit  I2C1_SCL_I;
      bit  I2C1_SCL_O;
      bit  I2C1_SCL_T;
      bit  PJTAG_TCK;
      bit  PJTAG_TMS;
      bit  PJTAG_TDI;
      bit  PJTAG_TDO;
      bit  SDIO0_CLK;
      bit  SDIO0_CLK_FB;
      bit  SDIO0_CMD_O;
      bit  SDIO0_CMD_I;
      bit  SDIO0_CMD_T;
      bit  [3 : 0] SDIO0_DATA_I;
      bit  [3 : 0] SDIO0_DATA_O;
      bit  [3 : 0] SDIO0_DATA_T;
      bit  SDIO0_LED;
      bit  SDIO0_CDN;
      bit  SDIO0_WP;
      bit  SDIO0_BUSPOW;
      bit  [2 : 0] SDIO0_BUSVOLT;
      bit  SDIO1_CLK;
      bit  SDIO1_CLK_FB;
      bit  SDIO1_CMD_O;
      bit  SDIO1_CMD_I;
      bit  SDIO1_CMD_T;
      bit  [3 : 0] SDIO1_DATA_I;
      bit  [3 : 0] SDIO1_DATA_O;
      bit  [3 : 0] SDIO1_DATA_T;
      bit  SDIO1_LED;
      bit  SDIO1_CDN;
      bit  SDIO1_WP;
      bit  SDIO1_BUSPOW;
      bit  [2 : 0] SDIO1_BUSVOLT;
      bit  SPI0_SCLK_I;
      bit  SPI0_SCLK_O;
      bit  SPI0_SCLK_T;
      bit  SPI0_MOSI_I;
      bit  SPI0_MOSI_O;
      bit  SPI0_MOSI_T;
      bit  SPI0_MISO_I;
      bit  SPI0_MISO_O;
      bit  SPI0_MISO_T;
      bit  SPI0_SS_I;
      bit  SPI0_SS_O;
      bit  SPI0_SS1_O;
      bit  SPI0_SS2_O;
      bit  SPI0_SS_T;
      bit  SPI1_SCLK_I;
      bit  SPI1_SCLK_O;
      bit  SPI1_SCLK_T;
      bit  SPI1_MOSI_I;
      bit  SPI1_MOSI_O;
      bit  SPI1_MOSI_T;
      bit  SPI1_MISO_I;
      bit  SPI1_MISO_O;
      bit  SPI1_MISO_T;
      bit  SPI1_SS_I;
      bit  SPI1_SS_O;
      bit  SPI1_SS1_O;
      bit  SPI1_SS2_O;
      bit  SPI1_SS_T;
      bit  UART0_DTRN;
      bit  UART0_RTSN;
      bit  UART0_TX;
      bit  UART0_CTSN;
      bit  UART0_DCDN;
      bit  UART0_DSRN;
      bit  UART0_RIN;
      bit  UART0_RX;
      bit  UART1_DTRN;
      bit  UART1_RTSN;
      bit  UART1_TX;
      bit  UART1_CTSN;
      bit  UART1_DCDN;
      bit  UART1_DSRN;
      bit  UART1_RIN;
      bit  UART1_RX;
      bit  TTC0_WAVE0_OUT;
      bit  TTC0_WAVE1_OUT;
      bit  TTC0_WAVE2_OUT;
      bit  TTC0_CLK0_IN;
      bit  TTC0_CLK1_IN;
      bit  TTC0_CLK2_IN;
      bit  TTC1_WAVE0_OUT;
      bit  TTC1_WAVE1_OUT;
      bit  TTC1_WAVE2_OUT;
      bit  TTC1_CLK0_IN;
      bit  TTC1_CLK1_IN;
      bit  TTC1_CLK2_IN;
      bit  WDT_CLK_IN;
      bit  WDT_RST_OUT;
      bit  TRACE_CLK;
      bit  TRACE_CLK_OUT;
      bit  TRACE_CTL;
      bit  [1 : 0] TRACE_DATA;
      bit  [1 : 0] USB0_PORT_INDCTL;
      bit  USB0_VBUS_PWRSELECT;
      bit  USB0_VBUS_PWRFAULT;
      bit  [1 : 0] USB1_PORT_INDCTL;
      bit  USB1_VBUS_PWRSELECT;
      bit  USB1_VBUS_PWRFAULT;
      bit  SRAM_INTIN;
      bit  M_AXI_GP0_ARVALID;
      bit  M_AXI_GP0_AWVALID;
      bit  M_AXI_GP0_BREADY;
      bit  M_AXI_GP0_RREADY;
      bit  M_AXI_GP0_WLAST;
      bit  M_AXI_GP0_WVALID;
      bit  [11 : 0] M_AXI_GP0_ARID;
      bit  [11 : 0] M_AXI_GP0_AWID;
      bit  [11 : 0] M_AXI_GP0_WID;
      bit  [1 : 0] M_AXI_GP0_ARBURST;
      bit  [1 : 0] M_AXI_GP0_ARLOCK;
      bit  [2 : 0] M_AXI_GP0_ARSIZE;
      bit  [1 : 0] M_AXI_GP0_AWBURST;
      bit  [1 : 0] M_AXI_GP0_AWLOCK;
      bit  [2 : 0] M_AXI_GP0_AWSIZE;
      bit  [2 : 0] M_AXI_GP0_ARPROT;
      bit  [2 : 0] M_AXI_GP0_AWPROT;
      bit  [31 : 0] M_AXI_GP0_ARADDR;
      bit  [31 : 0] M_AXI_GP0_AWADDR;
      bit  [31 : 0] M_AXI_GP0_WDATA;
      bit  [3 : 0] M_AXI_GP0_ARCACHE;
      bit  [3 : 0] M_AXI_GP0_ARLEN;
      bit  [3 : 0] M_AXI_GP0_ARQOS;
      bit  [3 : 0] M_AXI_GP0_AWCACHE;
      bit  [3 : 0] M_AXI_GP0_AWLEN;
      bit  [3 : 0] M_AXI_GP0_AWQOS;
      bit  [3 : 0] M_AXI_GP0_WSTRB;
      bit  M_AXI_GP0_ACLK;
      bit  M_AXI_GP0_ARREADY;
      bit  M_AXI_GP0_AWREADY;
      bit  M_AXI_GP0_BVALID;
      bit  M_AXI_GP0_RLAST;
      bit  M_AXI_GP0_RVALID;
      bit  M_AXI_GP0_WREADY;
      bit  [11 : 0] M_AXI_GP0_BID;
      bit  [11 : 0] M_AXI_GP0_RID;
      bit  [1 : 0] M_AXI_GP0_BRESP;
      bit  [1 : 0] M_AXI_GP0_RRESP;
      bit  [31 : 0] M_AXI_GP0_RDATA;
      bit  M_AXI_GP1_ARVALID;
      bit  M_AXI_GP1_AWVALID;
      bit  M_AXI_GP1_BREADY;
      bit  M_AXI_GP1_RREADY;
      bit  M_AXI_GP1_WLAST;
      bit  M_AXI_GP1_WVALID;
      bit  [11 : 0] M_AXI_GP1_ARID;
      bit  [11 : 0] M_AXI_GP1_AWID;
      bit  [11 : 0] M_AXI_GP1_WID;
      bit  [1 : 0] M_AXI_GP1_ARBURST;
      bit  [1 : 0] M_AXI_GP1_ARLOCK;
      bit  [2 : 0] M_AXI_GP1_ARSIZE;
      bit  [1 : 0] M_AXI_GP1_AWBURST;
      bit  [1 : 0] M_AXI_GP1_AWLOCK;
      bit  [2 : 0] M_AXI_GP1_AWSIZE;
      bit  [2 : 0] M_AXI_GP1_ARPROT;
      bit  [2 : 0] M_AXI_GP1_AWPROT;
      bit  [31 : 0] M_AXI_GP1_ARADDR;
      bit  [31 : 0] M_AXI_GP1_AWADDR;
      bit  [31 : 0] M_AXI_GP1_WDATA;
      bit  [3 : 0] M_AXI_GP1_ARCACHE;
      bit  [3 : 0] M_AXI_GP1_ARLEN;
      bit  [3 : 0] M_AXI_GP1_ARQOS;
      bit  [3 : 0] M_AXI_GP1_AWCACHE;
      bit  [3 : 0] M_AXI_GP1_AWLEN;
      bit  [3 : 0] M_AXI_GP1_AWQOS;
      bit  [3 : 0] M_AXI_GP1_WSTRB;
      bit  M_AXI_GP1_ACLK;
      bit  M_AXI_GP1_ARREADY;
      bit  M_AXI_GP1_AWREADY;
      bit  M_AXI_GP1_BVALID;
      bit  M_AXI_GP1_RLAST;
      bit  M_AXI_GP1_RVALID;
      bit  M_AXI_GP1_WREADY;
      bit  [11 : 0] M_AXI_GP1_BID;
      bit  [11 : 0] M_AXI_GP1_RID;
      bit  [1 : 0] M_AXI_GP1_BRESP;
      bit  [1 : 0] M_AXI_GP1_RRESP;
      bit  [31 : 0] M_AXI_GP1_RDATA;
      bit  S_AXI_GP0_ARREADY;
      bit  S_AXI_GP0_AWREADY;
      bit  S_AXI_GP0_BVALID;
      bit  S_AXI_GP0_RLAST;
      bit  S_AXI_GP0_RVALID;
      bit  S_AXI_GP0_WREADY;
      bit  [1 : 0] S_AXI_GP0_BRESP;
      bit  [1 : 0] S_AXI_GP0_RRESP;
      bit  [31 : 0] S_AXI_GP0_RDATA;
      bit  [5 : 0] S_AXI_GP0_BID;
      bit  [5 : 0] S_AXI_GP0_RID;
      bit  S_AXI_GP0_ACLK;
      bit  S_AXI_GP0_ARVALID;
      bit  S_AXI_GP0_AWVALID;
      bit  S_AXI_GP0_BREADY;
      bit  S_AXI_GP0_RREADY;
      bit  S_AXI_GP0_WLAST;
      bit  S_AXI_GP0_WVALID;
      bit  [1 : 0] S_AXI_GP0_ARBURST;
      bit  [1 : 0] S_AXI_GP0_ARLOCK;
      bit  [2 : 0] S_AXI_GP0_ARSIZE;
      bit  [1 : 0] S_AXI_GP0_AWBURST;
      bit  [1 : 0] S_AXI_GP0_AWLOCK;
      bit  [2 : 0] S_AXI_GP0_AWSIZE;
      bit  [2 : 0] S_AXI_GP0_ARPROT;
      bit  [2 : 0] S_AXI_GP0_AWPROT;
      bit  [31 : 0] S_AXI_GP0_ARADDR;
      bit  [31 : 0] S_AXI_GP0_AWADDR;
      bit  [31 : 0] S_AXI_GP0_WDATA;
      bit  [3 : 0] S_AXI_GP0_ARCACHE;
      bit  [3 : 0] S_AXI_GP0_ARLEN;
      bit  [3 : 0] S_AXI_GP0_ARQOS;
      bit  [3 : 0] S_AXI_GP0_AWCACHE;
      bit  [3 : 0] S_AXI_GP0_AWLEN;
      bit  [3 : 0] S_AXI_GP0_AWQOS;
      bit  [3 : 0] S_AXI_GP0_WSTRB;
      bit  [5 : 0] S_AXI_GP0_ARID;
      bit  [5 : 0] S_AXI_GP0_AWID;
      bit  [5 : 0] S_AXI_GP0_WID;
      bit  S_AXI_GP1_ARREADY;
      bit  S_AXI_GP1_AWREADY;
      bit  S_AXI_GP1_BVALID;
      bit  S_AXI_GP1_RLAST;
      bit  S_AXI_GP1_RVALID;
      bit  S_AXI_GP1_WREADY;
      bit  [1 : 0] S_AXI_GP1_BRESP;
      bit  [1 : 0] S_AXI_GP1_RRESP;
      bit  [31 : 0] S_AXI_GP1_RDATA;
      bit  [5 : 0] S_AXI_GP1_BID;
      bit  [5 : 0] S_AXI_GP1_RID;
      bit  S_AXI_GP1_ACLK;
      bit  S_AXI_GP1_ARVALID;
      bit  S_AXI_GP1_AWVALID;
      bit  S_AXI_GP1_BREADY;
      bit  S_AXI_GP1_RREADY;
      bit  S_AXI_GP1_WLAST;
      bit  S_AXI_GP1_WVALID;
      bit  [1 : 0] S_AXI_GP1_ARBURST;
      bit  [1 : 0] S_AXI_GP1_ARLOCK;
      bit  [2 : 0] S_AXI_GP1_ARSIZE;
      bit  [1 : 0] S_AXI_GP1_AWBURST;
      bit  [1 : 0] S_AXI_GP1_AWLOCK;
      bit  [2 : 0] S_AXI_GP1_AWSIZE;
      bit  [2 : 0] S_AXI_GP1_ARPROT;
      bit  [2 : 0] S_AXI_GP1_AWPROT;
      bit  [31 : 0] S_AXI_GP1_ARADDR;
      bit  [31 : 0] S_AXI_GP1_AWADDR;
      bit  [31 : 0] S_AXI_GP1_WDATA;
      bit  [3 : 0] S_AXI_GP1_ARCACHE;
      bit  [3 : 0] S_AXI_GP1_ARLEN;
      bit  [3 : 0] S_AXI_GP1_ARQOS;
      bit  [3 : 0] S_AXI_GP1_AWCACHE;
      bit  [3 : 0] S_AXI_GP1_AWLEN;
      bit  [3 : 0] S_AXI_GP1_AWQOS;
      bit  [3 : 0] S_AXI_GP1_WSTRB;
      bit  [5 : 0] S_AXI_GP1_ARID;
      bit  [5 : 0] S_AXI_GP1_AWID;
      bit  [5 : 0] S_AXI_GP1_WID;
      bit  S_AXI_ACP_ARREADY;
      bit  S_AXI_ACP_AWREADY;
      bit  S_AXI_ACP_BVALID;
      bit  S_AXI_ACP_RLAST;
      bit  S_AXI_ACP_RVALID;
      bit  S_AXI_ACP_WREADY;
      bit  [1 : 0] S_AXI_ACP_BRESP;
      bit  [1 : 0] S_AXI_ACP_RRESP;
      bit  [2 : 0] S_AXI_ACP_BID;
      bit  [2 : 0] S_AXI_ACP_RID;
      bit  [63 : 0] S_AXI_ACP_RDATA;
      bit  S_AXI_ACP_ACLK;
      bit  S_AXI_ACP_ARVALID;
      bit  S_AXI_ACP_AWVALID;
      bit  S_AXI_ACP_BREADY;
      bit  S_AXI_ACP_RREADY;
      bit  S_AXI_ACP_WLAST;
      bit  S_AXI_ACP_WVALID;
      bit  [2 : 0] S_AXI_ACP_ARID;
      bit  [2 : 0] S_AXI_ACP_ARPROT;
      bit  [2 : 0] S_AXI_ACP_AWID;
      bit  [2 : 0] S_AXI_ACP_AWPROT;
      bit  [2 : 0] S_AXI_ACP_WID;
      bit  [31 : 0] S_AXI_ACP_ARADDR;
      bit  [31 : 0] S_AXI_ACP_AWADDR;
      bit  [3 : 0] S_AXI_ACP_ARCACHE;
      bit  [3 : 0] S_AXI_ACP_ARLEN;
      bit  [3 : 0] S_AXI_ACP_ARQOS;
      bit  [3 : 0] S_AXI_ACP_AWCACHE;
      bit  [3 : 0] S_AXI_ACP_AWLEN;
      bit  [3 : 0] S_AXI_ACP_AWQOS;
      bit  [1 : 0] S_AXI_ACP_ARBURST;
      bit  [1 : 0] S_AXI_ACP_ARLOCK;
      bit  [2 : 0] S_AXI_ACP_ARSIZE;
      bit  [1 : 0] S_AXI_ACP_AWBURST;
      bit  [1 : 0] S_AXI_ACP_AWLOCK;
      bit  [2 : 0] S_AXI_ACP_AWSIZE;
      bit  [4 : 0] S_AXI_ACP_ARUSER;
      bit  [4 : 0] S_AXI_ACP_AWUSER;
      bit  [63 : 0] S_AXI_ACP_WDATA;
      bit  [7 : 0] S_AXI_ACP_WSTRB;
      bit  S_AXI_HP0_ARREADY;
      bit  S_AXI_HP0_AWREADY;
      bit  S_AXI_HP0_BVALID;
      bit  S_AXI_HP0_RLAST;
      bit  S_AXI_HP0_RVALID;
      bit  S_AXI_HP0_WREADY;
      bit  [1 : 0] S_AXI_HP0_BRESP;
      bit  [1 : 0] S_AXI_HP0_RRESP;
      bit  [5 : 0] S_AXI_HP0_BID;
      bit  [5 : 0] S_AXI_HP0_RID;
      bit  [63 : 0] S_AXI_HP0_RDATA;
      bit  [7 : 0] S_AXI_HP0_RCOUNT;
      bit  [7 : 0] S_AXI_HP0_WCOUNT;
      bit  [2 : 0] S_AXI_HP0_RACOUNT;
      bit  [5 : 0] S_AXI_HP0_WACOUNT;
      bit  S_AXI_HP0_ACLK;
      bit  S_AXI_HP0_ARVALID;
      bit  S_AXI_HP0_AWVALID;
      bit  S_AXI_HP0_BREADY;
      bit  S_AXI_HP0_RDISSUECAP1_EN;
      bit  S_AXI_HP0_RREADY;
      bit  S_AXI_HP0_WLAST;
      bit  S_AXI_HP0_WRISSUECAP1_EN;
      bit  S_AXI_HP0_WVALID;
      bit  [1 : 0] S_AXI_HP0_ARBURST;
      bit  [1 : 0] S_AXI_HP0_ARLOCK;
      bit  [2 : 0] S_AXI_HP0_ARSIZE;
      bit  [1 : 0] S_AXI_HP0_AWBURST;
      bit  [1 : 0] S_AXI_HP0_AWLOCK;
      bit  [2 : 0] S_AXI_HP0_AWSIZE;
      bit  [2 : 0] S_AXI_HP0_ARPROT;
      bit  [2 : 0] S_AXI_HP0_AWPROT;
      bit  [31 : 0] S_AXI_HP0_ARADDR;
      bit  [31 : 0] S_AXI_HP0_AWADDR;
      bit  [3 : 0] S_AXI_HP0_ARCACHE;
      bit  [3 : 0] S_AXI_HP0_ARLEN;
      bit  [3 : 0] S_AXI_HP0_ARQOS;
      bit  [3 : 0] S_AXI_HP0_AWCACHE;
      bit  [3 : 0] S_AXI_HP0_AWLEN;
      bit  [3 : 0] S_AXI_HP0_AWQOS;
      bit  [5 : 0] S_AXI_HP0_ARID;
      bit  [5 : 0] S_AXI_HP0_AWID;
      bit  [5 : 0] S_AXI_HP0_WID;
      bit  [63 : 0] S_AXI_HP0_WDATA;
      bit  [7 : 0] S_AXI_HP0_WSTRB;
      bit  S_AXI_HP1_ARREADY;
      bit  S_AXI_HP1_AWREADY;
      bit  S_AXI_HP1_BVALID;
      bit  S_AXI_HP1_RLAST;
      bit  S_AXI_HP1_RVALID;
      bit  S_AXI_HP1_WREADY;
      bit  [1 : 0] S_AXI_HP1_BRESP;
      bit  [1 : 0] S_AXI_HP1_RRESP;
      bit  [5 : 0] S_AXI_HP1_BID;
      bit  [5 : 0] S_AXI_HP1_RID;
      bit  [63 : 0] S_AXI_HP1_RDATA;
      bit  [7 : 0] S_AXI_HP1_RCOUNT;
      bit  [7 : 0] S_AXI_HP1_WCOUNT;
      bit  [2 : 0] S_AXI_HP1_RACOUNT;
      bit  [5 : 0] S_AXI_HP1_WACOUNT;
      bit  S_AXI_HP1_ACLK;
      bit  S_AXI_HP1_ARVALID;
      bit  S_AXI_HP1_AWVALID;
      bit  S_AXI_HP1_BREADY;
      bit  S_AXI_HP1_RDISSUECAP1_EN;
      bit  S_AXI_HP1_RREADY;
      bit  S_AXI_HP1_WLAST;
      bit  S_AXI_HP1_WRISSUECAP1_EN;
      bit  S_AXI_HP1_WVALID;
      bit  [1 : 0] S_AXI_HP1_ARBURST;
      bit  [1 : 0] S_AXI_HP1_ARLOCK;
      bit  [2 : 0] S_AXI_HP1_ARSIZE;
      bit  [1 : 0] S_AXI_HP1_AWBURST;
      bit  [1 : 0] S_AXI_HP1_AWLOCK;
      bit  [2 : 0] S_AXI_HP1_AWSIZE;
      bit  [2 : 0] S_AXI_HP1_ARPROT;
      bit  [2 : 0] S_AXI_HP1_AWPROT;
      bit  [31 : 0] S_AXI_HP1_ARADDR;
      bit  [31 : 0] S_AXI_HP1_AWADDR;
      bit  [3 : 0] S_AXI_HP1_ARCACHE;
      bit  [3 : 0] S_AXI_HP1_ARLEN;
      bit  [3 : 0] S_AXI_HP1_ARQOS;
      bit  [3 : 0] S_AXI_HP1_AWCACHE;
      bit  [3 : 0] S_AXI_HP1_AWLEN;
      bit  [3 : 0] S_AXI_HP1_AWQOS;
      bit  [5 : 0] S_AXI_HP1_ARID;
      bit  [5 : 0] S_AXI_HP1_AWID;
      bit  [5 : 0] S_AXI_HP1_WID;
      bit  [63 : 0] S_AXI_HP1_WDATA;
      bit  [7 : 0] S_AXI_HP1_WSTRB;
      bit  S_AXI_HP2_ARREADY;
      bit  S_AXI_HP2_AWREADY;
      bit  S_AXI_HP2_BVALID;
      bit  S_AXI_HP2_RLAST;
      bit  S_AXI_HP2_RVALID;
      bit  S_AXI_HP2_WREADY;
      bit  [1 : 0] S_AXI_HP2_BRESP;
      bit  [1 : 0] S_AXI_HP2_RRESP;
      bit  [5 : 0] S_AXI_HP2_BID;
      bit  [5 : 0] S_AXI_HP2_RID;
      bit  [63 : 0] S_AXI_HP2_RDATA;
      bit  [7 : 0] S_AXI_HP2_RCOUNT;
      bit  [7 : 0] S_AXI_HP2_WCOUNT;
      bit  [2 : 0] S_AXI_HP2_RACOUNT;
      bit  [5 : 0] S_AXI_HP2_WACOUNT;
      bit  S_AXI_HP2_ACLK;
      bit  S_AXI_HP2_ARVALID;
      bit  S_AXI_HP2_AWVALID;
      bit  S_AXI_HP2_BREADY;
      bit  S_AXI_HP2_RDISSUECAP1_EN;
      bit  S_AXI_HP2_RREADY;
      bit  S_AXI_HP2_WLAST;
      bit  S_AXI_HP2_WRISSUECAP1_EN;
      bit  S_AXI_HP2_WVALID;
      bit  [1 : 0] S_AXI_HP2_ARBURST;
      bit  [1 : 0] S_AXI_HP2_ARLOCK;
      bit  [2 : 0] S_AXI_HP2_ARSIZE;
      bit  [1 : 0] S_AXI_HP2_AWBURST;
      bit  [1 : 0] S_AXI_HP2_AWLOCK;
      bit  [2 : 0] S_AXI_HP2_AWSIZE;
      bit  [2 : 0] S_AXI_HP2_ARPROT;
      bit  [2 : 0] S_AXI_HP2_AWPROT;
      bit  [31 : 0] S_AXI_HP2_ARADDR;
      bit  [31 : 0] S_AXI_HP2_AWADDR;
      bit  [3 : 0] S_AXI_HP2_ARCACHE;
      bit  [3 : 0] S_AXI_HP2_ARLEN;
      bit  [3 : 0] S_AXI_HP2_ARQOS;
      bit  [3 : 0] S_AXI_HP2_AWCACHE;
      bit  [3 : 0] S_AXI_HP2_AWLEN;
      bit  [3 : 0] S_AXI_HP2_AWQOS;
      bit  [5 : 0] S_AXI_HP2_ARID;
      bit  [5 : 0] S_AXI_HP2_AWID;
      bit  [5 : 0] S_AXI_HP2_WID;
      bit  [63 : 0] S_AXI_HP2_WDATA;
      bit  [7 : 0] S_AXI_HP2_WSTRB;
      bit  S_AXI_HP3_ARREADY;
      bit  S_AXI_HP3_AWREADY;
      bit  S_AXI_HP3_BVALID;
      bit  S_AXI_HP3_RLAST;
      bit  S_AXI_HP3_RVALID;
      bit  S_AXI_HP3_WREADY;
      bit  [1 : 0] S_AXI_HP3_BRESP;
      bit  [1 : 0] S_AXI_HP3_RRESP;
      bit  [5 : 0] S_AXI_HP3_BID;
      bit  [5 : 0] S_AXI_HP3_RID;
      bit  [63 : 0] S_AXI_HP3_RDATA;
      bit  [7 : 0] S_AXI_HP3_RCOUNT;
      bit  [7 : 0] S_AXI_HP3_WCOUNT;
      bit  [2 : 0] S_AXI_HP3_RACOUNT;
      bit  [5 : 0] S_AXI_HP3_WACOUNT;
      bit  S_AXI_HP3_ACLK;
      bit  S_AXI_HP3_ARVALID;
      bit  S_AXI_HP3_AWVALID;
      bit  S_AXI_HP3_BREADY;
      bit  S_AXI_HP3_RDISSUECAP1_EN;
      bit  S_AXI_HP3_RREADY;
      bit  S_AXI_HP3_WLAST;
      bit  S_AXI_HP3_WRISSUECAP1_EN;
      bit  S_AXI_HP3_WVALID;
      bit  [1 : 0] S_AXI_HP3_ARBURST;
      bit  [1 : 0] S_AXI_HP3_ARLOCK;
      bit  [2 : 0] S_AXI_HP3_ARSIZE;
      bit  [1 : 0] S_AXI_HP3_AWBURST;
      bit  [1 : 0] S_AXI_HP3_AWLOCK;
      bit  [2 : 0] S_AXI_HP3_AWSIZE;
      bit  [2 : 0] S_AXI_HP3_ARPROT;
      bit  [2 : 0] S_AXI_HP3_AWPROT;
      bit  [31 : 0] S_AXI_HP3_ARADDR;
      bit  [31 : 0] S_AXI_HP3_AWADDR;
      bit  [3 : 0] S_AXI_HP3_ARCACHE;
      bit  [3 : 0] S_AXI_HP3_ARLEN;
      bit  [3 : 0] S_AXI_HP3_ARQOS;
      bit  [3 : 0] S_AXI_HP3_AWCACHE;
      bit  [3 : 0] S_AXI_HP3_AWLEN;
      bit  [3 : 0] S_AXI_HP3_AWQOS;
      bit  [5 : 0] S_AXI_HP3_ARID;
      bit  [5 : 0] S_AXI_HP3_AWID;
      bit  [5 : 0] S_AXI_HP3_WID;
      bit  [63 : 0] S_AXI_HP3_WDATA;
      bit  [7 : 0] S_AXI_HP3_WSTRB;
      bit  IRQ_P2F_DMAC_ABORT;
      bit  IRQ_P2F_DMAC0;
      bit  IRQ_P2F_DMAC1;
      bit  IRQ_P2F_DMAC2;
      bit  IRQ_P2F_DMAC3;
      bit  IRQ_P2F_DMAC4;
      bit  IRQ_P2F_DMAC5;
      bit  IRQ_P2F_DMAC6;
      bit  IRQ_P2F_DMAC7;
      bit  IRQ_P2F_SMC;
      bit  IRQ_P2F_QSPI;
      bit  IRQ_P2F_CTI;
      bit  IRQ_P2F_GPIO;
      bit  IRQ_P2F_USB0;
      bit  IRQ_P2F_ENET0;
      bit  IRQ_P2F_ENET_WAKE0;
      bit  IRQ_P2F_SDIO0;
      bit  IRQ_P2F_I2C0;
      bit  IRQ_P2F_SPI0;
      bit  IRQ_P2F_UART0;
      bit  IRQ_P2F_CAN0;
      bit  IRQ_P2F_USB1;
      bit  IRQ_P2F_ENET1;
      bit  IRQ_P2F_ENET_WAKE1;
      bit  IRQ_P2F_SDIO1;
      bit  IRQ_P2F_I2C1;
      bit  IRQ_P2F_SPI1;
      bit  IRQ_P2F_UART1;
      bit  IRQ_P2F_CAN1;
      bit  [1 : 0] IRQ_F2P;
      bit  Core0_nFIQ;
      bit  Core0_nIRQ;
      bit  Core1_nFIQ;
      bit  Core1_nIRQ;
      bit  [1 : 0] DMA0_DATYPE;
      bit  DMA0_DAVALID;
      bit  DMA0_DRREADY;
      bit  [1 : 0] DMA1_DATYPE;
      bit  DMA1_DAVALID;
      bit  DMA1_DRREADY;
      bit  [1 : 0] DMA2_DATYPE;
      bit  DMA2_DAVALID;
      bit  DMA2_DRREADY;
      bit  [1 : 0] DMA3_DATYPE;
      bit  DMA3_DAVALID;
      bit  DMA3_DRREADY;
      bit  DMA0_ACLK;
      bit  DMA0_DAREADY;
      bit  DMA0_DRLAST;
      bit  DMA0_DRVALID;
      bit  DMA1_ACLK;
      bit  DMA1_DAREADY;
      bit  DMA1_DRLAST;
      bit  DMA1_DRVALID;
      bit  DMA2_ACLK;
      bit  DMA2_DAREADY;
      bit  DMA2_DRLAST;
      bit  DMA2_DRVALID;
      bit  DMA3_ACLK;
      bit  DMA3_DAREADY;
      bit  DMA3_DRLAST;
      bit  DMA3_DRVALID;
      bit  [1 : 0] DMA0_DRTYPE;
      bit  [1 : 0] DMA1_DRTYPE;
      bit  [1 : 0] DMA2_DRTYPE;
      bit  [1 : 0] DMA3_DRTYPE;
      bit  FCLK_CLK0;
      bit  FCLK_CLK1;
      bit  FCLK_CLK2;
      bit  FCLK_CLK3;
      bit  FCLK_CLKTRIG0_N;
      bit  FCLK_CLKTRIG1_N;
      bit  FCLK_CLKTRIG2_N;
      bit  FCLK_CLKTRIG3_N;
      bit  FCLK_RESET0_N;
      bit  FCLK_RESET1_N;
      bit  FCLK_RESET2_N;
      bit  FCLK_RESET3_N;
      bit  [31 : 0] FTMD_TRACEIN_DATA;
      bit  FTMD_TRACEIN_VALID;
      bit  FTMD_TRACEIN_CLK;
      bit  [3 : 0] FTMD_TRACEIN_ATID;
      bit  FTMT_F2P_TRIG_0;
      bit  FTMT_F2P_TRIGACK_0;
      bit  FTMT_F2P_TRIG_1;
      bit  FTMT_F2P_TRIGACK_1;
      bit  FTMT_F2P_TRIG_2;
      bit  FTMT_F2P_TRIGACK_2;
      bit  FTMT_F2P_TRIG_3;
      bit  FTMT_F2P_TRIGACK_3;
      bit  [31 : 0] FTMT_F2P_DEBUG;
      bit  FTMT_P2F_TRIGACK_0;
      bit  FTMT_P2F_TRIG_0;
      bit  FTMT_P2F_TRIGACK_1;
      bit  FTMT_P2F_TRIG_1;
      bit  FTMT_P2F_TRIGACK_2;
      bit  FTMT_P2F_TRIG_2;
      bit  FTMT_P2F_TRIGACK_3;
      bit  FTMT_P2F_TRIG_3;
      bit  [31 : 0] FTMT_P2F_DEBUG;
      bit  FPGA_IDLE_N;
      bit  EVENT_EVENTO;
      bit  [1 : 0] EVENT_STANDBYWFE;
      bit  [1 : 0] EVENT_STANDBYWFI;
      bit  EVENT_EVENTI;
      bit  [3 : 0] DDR_ARB;
      bit  [53 : 0] MIO;
      bit  DDR_CAS_n;
      bit  DDR_CKE;
      bit  DDR_Clk_n;
      bit  DDR_Clk;
      bit  DDR_CS_n;
      bit  DDR_DRSTB;
      bit  DDR_ODT;
      bit  DDR_RAS_n;
      bit  DDR_WEB;
      bit  [2 : 0] DDR_BankAddr;
      bit  [14 : 0] DDR_Addr;
      bit  DDR_VRN;
      bit  DDR_VRP;
      bit  [3 : 0] DDR_DM;
      bit  [31 : 0] DDR_DQ;
      bit  [3 : 0] DDR_DQS_n;
      bit  [3 : 0] DDR_DQS;
      bit  PS_SRSTB;
      bit  PS_CLK;
      bit  PS_PORB;

//MODULE DECLARATION
 module system_processing_system7_0_0 (
  GPIO_I,
  GPIO_O,
  GPIO_T,
  TTC0_WAVE0_OUT,
  TTC0_WAVE1_OUT,
  TTC0_WAVE2_OUT,
  USB0_PORT_INDCTL,
  USB0_VBUS_PWRSELECT,
  USB0_VBUS_PWRFAULT,
  M_AXI_GP0_ARVALID,
  M_AXI_GP0_AWVALID,
  M_AXI_GP0_BREADY,
  M_AXI_GP0_RREADY,
  M_AXI_GP0_WLAST,
  M_AXI_GP0_WVALID,
  M_AXI_GP0_ARID,
  M_AXI_GP0_AWID,
  M_AXI_GP0_WID,
  M_AXI_GP0_ARBURST,
  M_AXI_GP0_ARLOCK,
  M_AXI_GP0_ARSIZE,
  M_AXI_GP0_AWBURST,
  M_AXI_GP0_AWLOCK,
  M_AXI_GP0_AWSIZE,
  M_AXI_GP0_ARPROT,
  M_AXI_GP0_AWPROT,
  M_AXI_GP0_ARADDR,
  M_AXI_GP0_AWADDR,
  M_AXI_GP0_WDATA,
  M_AXI_GP0_ARCACHE,
  M_AXI_GP0_ARLEN,
  M_AXI_GP0_ARQOS,
  M_AXI_GP0_AWCACHE,
  M_AXI_GP0_AWLEN,
  M_AXI_GP0_AWQOS,
  M_AXI_GP0_WSTRB,
  M_AXI_GP0_ACLK,
  M_AXI_GP0_ARREADY,
  M_AXI_GP0_AWREADY,
  M_AXI_GP0_BVALID,
  M_AXI_GP0_RLAST,
  M_AXI_GP0_RVALID,
  M_AXI_GP0_WREADY,
  M_AXI_GP0_BID,
  M_AXI_GP0_RID,
  M_AXI_GP0_BRESP,
  M_AXI_GP0_RRESP,
  M_AXI_GP0_RDATA,
  S_AXI_HP0_ARREADY,
  S_AXI_HP0_AWREADY,
  S_AXI_HP0_BVALID,
  S_AXI_HP0_RLAST,
  S_AXI_HP0_RVALID,
  S_AXI_HP0_WREADY,
  S_AXI_HP0_BRESP,
  S_AXI_HP0_RRESP,
  S_AXI_HP0_BID,
  S_AXI_HP0_RID,
  S_AXI_HP0_RDATA,
  S_AXI_HP0_RCOUNT,
  S_AXI_HP0_WCOUNT,
  S_AXI_HP0_RACOUNT,
  S_AXI_HP0_WACOUNT,
  S_AXI_HP0_ACLK,
  S_AXI_HP0_ARVALID,
  S_AXI_HP0_AWVALID,
  S_AXI_HP0_BREADY,
  S_AXI_HP0_RDISSUECAP1_EN,
  S_AXI_HP0_RREADY,
  S_AXI_HP0_WLAST,
  S_AXI_HP0_WRISSUECAP1_EN,
  S_AXI_HP0_WVALID,
  S_AXI_HP0_ARBURST,
  S_AXI_HP0_ARLOCK,
  S_AXI_HP0_ARSIZE,
  S_AXI_HP0_AWBURST,
  S_AXI_HP0_AWLOCK,
  S_AXI_HP0_AWSIZE,
  S_AXI_HP0_ARPROT,
  S_AXI_HP0_AWPROT,
  S_AXI_HP0_ARADDR,
  S_AXI_HP0_AWADDR,
  S_AXI_HP0_ARCACHE,
  S_AXI_HP0_ARLEN,
  S_AXI_HP0_ARQOS,
  S_AXI_HP0_AWCACHE,
  S_AXI_HP0_AWLEN,
  S_AXI_HP0_AWQOS,
  S_AXI_HP0_ARID,
  S_AXI_HP0_AWID,
  S_AXI_HP0_WID,
  S_AXI_HP0_WDATA,
  S_AXI_HP0_WSTRB,
  IRQ_F2P,
  FCLK_CLK0,
  FCLK_CLK1,
  FCLK_RESET0_N,
  MIO,
  DDR_CAS_n,
  DDR_CKE,
  DDR_Clk_n,
  DDR_Clk,
  DDR_CS_n,
  DDR_DRSTB,
  DDR_ODT,
  DDR_RAS_n,
  DDR_WEB,
  DDR_BankAddr,
  DDR_Addr,
  DDR_VRN,
  DDR_VRP,
  DDR_DM,
  DDR_DQ,
  DDR_DQS_n,
  DDR_DQS,
  PS_SRSTB,
  PS_CLK,
  PS_PORB
 );

//PARAMETERS

      parameter C_EN_EMIO_PJTAG = 0;
      parameter C_EN_EMIO_ENET0 = 0;
      parameter C_EN_EMIO_ENET1 = 0;
      parameter C_EN_EMIO_TRACE = 0;
      parameter C_INCLUDE_TRACE_BUFFER = 0;
      parameter C_TRACE_BUFFER_FIFO_SIZE = 128;
      parameter USE_TRACE_DATA_EDGE_DETECTOR = 0;
      parameter C_TRACE_PIPELINE_WIDTH = 8;
      parameter C_TRACE_BUFFER_CLOCK_DELAY = 12;
      parameter C_EMIO_GPIO_WIDTH = 24;
      parameter C_INCLUDE_ACP_TRANS_CHECK = 0;
      parameter C_USE_DEFAULT_ACP_USER_VAL = 0;
      parameter C_S_AXI_ACP_ARUSER_VAL = 31;
      parameter C_S_AXI_ACP_AWUSER_VAL = 31;
      parameter C_M_AXI_GP0_ID_WIDTH = 12;
      parameter C_M_AXI_GP0_ENABLE_STATIC_REMAP = 0;
      parameter C_M_AXI_GP1_ID_WIDTH = 12;
      parameter C_M_AXI_GP1_ENABLE_STATIC_REMAP = 0;
      parameter C_S_AXI_GP0_ID_WIDTH = 6;
      parameter C_S_AXI_GP1_ID_WIDTH = 6;
      parameter C_S_AXI_ACP_ID_WIDTH = 3;
      parameter C_S_AXI_HP0_ID_WIDTH = 6;
      parameter C_S_AXI_HP0_DATA_WIDTH = 64;
      parameter C_S_AXI_HP1_ID_WIDTH = 6;
      parameter C_S_AXI_HP1_DATA_WIDTH = 64;
      parameter C_S_AXI_HP2_ID_WIDTH = 6;
      parameter C_S_AXI_HP2_DATA_WIDTH = 64;
      parameter C_S_AXI_HP3_ID_WIDTH = 6;
      parameter C_S_AXI_HP3_DATA_WIDTH = 64;
      parameter C_M_AXI_GP0_THREAD_ID_WIDTH = 12;
      parameter C_M_AXI_GP1_THREAD_ID_WIDTH = 12;
      parameter C_NUM_F2P_INTR_INPUTS = 2;
      parameter C_IRQ_F2P_MODE = "DIRECT";
      parameter C_DQ_WIDTH = 32;
      parameter C_DQS_WIDTH = 4;
      parameter C_DM_WIDTH = 4;
      parameter C_MIO_PRIMITIVE = 54;
      parameter C_TRACE_INTERNAL_WIDTH = 2;
      parameter C_USE_AXI_NONSECURE = 0;
      parameter C_USE_M_AXI_GP0 = 1;
      parameter C_USE_M_AXI_GP1 = 0;
      parameter C_USE_S_AXI_GP0 = 0;
      parameter C_USE_S_AXI_GP1 = 0;
      parameter C_USE_S_AXI_HP0 = 1;
      parameter C_USE_S_AXI_HP1 = 0;
      parameter C_USE_S_AXI_HP2 = 0;
      parameter C_USE_S_AXI_HP3 = 0;
      parameter C_USE_S_AXI_ACP = 0;
      parameter C_PS7_SI_REV = "PRODUCTION";
      parameter C_FCLK_CLK0_BUF = "TRUE";
      parameter C_FCLK_CLK1_BUF = "TRUE";
      parameter C_FCLK_CLK2_BUF = "FALSE";
      parameter C_FCLK_CLK3_BUF = "FALSE";
      parameter C_PACKAGE_NAME = "clg400";
      parameter C_GP0_EN_MODIFIABLE_TXN = "1";
      parameter C_GP1_EN_MODIFIABLE_TXN = "1";

//INPUT AND OUTPUT PORTS

      input  [23 : 0] GPIO_I;
      output  [23 : 0] GPIO_O;
      output  [23 : 0] GPIO_T;
      output  TTC0_WAVE0_OUT;
      output  TTC0_WAVE1_OUT;
      output  TTC0_WAVE2_OUT;
      output  [1 : 0] USB0_PORT_INDCTL;
      output  USB0_VBUS_PWRSELECT;
      input  USB0_VBUS_PWRFAULT;
      output  M_AXI_GP0_ARVALID;
      output  M_AXI_GP0_AWVALID;
      output  M_AXI_GP0_BREADY;
      output  M_AXI_GP0_RREADY;
      output  M_AXI_GP0_WLAST;
      output  M_AXI_GP0_WVALID;
      output  [11 : 0] M_AXI_GP0_ARID;
      output  [11 : 0] M_AXI_GP0_AWID;
      output  [11 : 0] M_AXI_GP0_WID;
      output  [1 : 0] M_AXI_GP0_ARBURST;
      output  [1 : 0] M_AXI_GP0_ARLOCK;
      output  [2 : 0] M_AXI_GP0_ARSIZE;
      output  [1 : 0] M_AXI_GP0_AWBURST;
      output  [1 : 0] M_AXI_GP0_AWLOCK;
      output  [2 : 0] M_AXI_GP0_AWSIZE;
      output  [2 : 0] M_AXI_GP0_ARPROT;
      output  [2 : 0] M_AXI_GP0_AWPROT;
      output  [31 : 0] M_AXI_GP0_ARADDR;
      output  [31 : 0] M_AXI_GP0_AWADDR;
      output  [31 : 0] M_AXI_GP0_WDATA;
      output  [3 : 0] M_AXI_GP0_ARCACHE;
      output  [3 : 0] M_AXI_GP0_ARLEN;
      output  [3 : 0] M_AXI_GP0_ARQOS;
      output  [3 : 0] M_AXI_GP0_AWCACHE;
      output  [3 : 0] M_AXI_GP0_AWLEN;
      output  [3 : 0] M_AXI_GP0_AWQOS;
      output  [3 : 0] M_AXI_GP0_WSTRB;
      input  M_AXI_GP0_ACLK;
      input  M_AXI_GP0_ARREADY;
      input  M_AXI_GP0_AWREADY;
      input  M_AXI_GP0_BVALID;
      input  M_AXI_GP0_RLAST;
      input  M_AXI_GP0_RVALID;
      input  M_AXI_GP0_WREADY;
      input  [11 : 0] M_AXI_GP0_BID;
      input  [11 : 0] M_AXI_GP0_RID;
      input  [1 : 0] M_AXI_GP0_BRESP;
      input  [1 : 0] M_AXI_GP0_RRESP;
      input  [31 : 0] M_AXI_GP0_RDATA;
      output  S_AXI_HP0_ARREADY;
      output  S_AXI_HP0_AWREADY;
      output  S_AXI_HP0_BVALID;
      output  S_AXI_HP0_RLAST;
      output  S_AXI_HP0_RVALID;
      output  S_AXI_HP0_WREADY;
      output  [1 : 0] S_AXI_HP0_BRESP;
      output  [1 : 0] S_AXI_HP0_RRESP;
      output  [5 : 0] S_AXI_HP0_BID;
      output  [5 : 0] S_AXI_HP0_RID;
      output  [63 : 0] S_AXI_HP0_RDATA;
      output  [7 : 0] S_AXI_HP0_RCOUNT;
      output  [7 : 0] S_AXI_HP0_WCOUNT;
      output  [2 : 0] S_AXI_HP0_RACOUNT;
      output  [5 : 0] S_AXI_HP0_WACOUNT;
      input  S_AXI_HP0_ACLK;
      input  S_AXI_HP0_ARVALID;
      input  S_AXI_HP0_AWVALID;
      input  S_AXI_HP0_BREADY;
      input  S_AXI_HP0_RDISSUECAP1_EN;
      input  S_AXI_HP0_RREADY;
      input  S_AXI_HP0_WLAST;
      input  S_AXI_HP0_WRISSUECAP1_EN;
      input  S_AXI_HP0_WVALID;
      input  [1 : 0] S_AXI_HP0_ARBURST;
      input  [1 : 0] S_AXI_HP0_ARLOCK;
      input  [2 : 0] S_AXI_HP0_ARSIZE;
      input  [1 : 0] S_AXI_HP0_AWBURST;
      input  [1 : 0] S_AXI_HP0_AWLOCK;
      input  [2 : 0] S_AXI_HP0_AWSIZE;
      input  [2 : 0] S_AXI_HP0_ARPROT;
      input  [2 : 0] S_AXI_HP0_AWPROT;
      input  [31 : 0] S_AXI_HP0_ARADDR;
      input  [31 : 0] S_AXI_HP0_AWADDR;
      input  [3 : 0] S_AXI_HP0_ARCACHE;
      input  [3 : 0] S_AXI_HP0_ARLEN;
      input  [3 : 0] S_AXI_HP0_ARQOS;
      input  [3 : 0] S_AXI_HP0_AWCACHE;
      input  [3 : 0] S_AXI_HP0_AWLEN;
      input  [3 : 0] S_AXI_HP0_AWQOS;
      input  [5 : 0] S_AXI_HP0_ARID;
      input  [5 : 0] S_AXI_HP0_AWID;
      input  [5 : 0] S_AXI_HP0_WID;
      input  [63 : 0] S_AXI_HP0_WDATA;
      input  [7 : 0] S_AXI_HP0_WSTRB;
      input  [1 : 0] IRQ_F2P;
      output  FCLK_CLK0;
      output  FCLK_CLK1;
      output  FCLK_RESET0_N;
      inout  [53 : 0] MIO;
      inout  DDR_CAS_n;
      inout  DDR_CKE;
      inout  DDR_Clk_n;
      inout  DDR_Clk;
      inout  DDR_CS_n;
      inout  DDR_DRSTB;
      inout  DDR_ODT;
      inout  DDR_RAS_n;
      inout  DDR_WEB;
      inout  [2 : 0] DDR_BankAddr;
      inout  [14 : 0] DDR_Addr;
      inout  DDR_VRN;
      inout  DDR_VRP;
      inout  [3 : 0] DDR_DM;
      inout  [31 : 0] DDR_DQ;
      inout  [3 : 0] DDR_DQS_n;
      inout  [3 : 0] DDR_DQS;
      inout  PS_SRSTB;
      inout  PS_CLK;
      inout  PS_PORB;

//REG DECLARATIONS

      reg [23 : 0] GPIO_O;
      reg [23 : 0] GPIO_T;
      reg TTC0_WAVE0_OUT;
      reg TTC0_WAVE1_OUT;
      reg TTC0_WAVE2_OUT;
      reg [1 : 0] USB0_PORT_INDCTL;
      reg USB0_VBUS_PWRSELECT;
      reg M_AXI_GP0_ARVALID;
      reg M_AXI_GP0_AWVALID;
      reg M_AXI_GP0_BREADY;
      reg M_AXI_GP0_RREADY;
      reg M_AXI_GP0_WLAST;
      reg M_AXI_GP0_WVALID;
      reg [11 : 0] M_AXI_GP0_ARID;
      reg [11 : 0] M_AXI_GP0_AWID;
      reg [11 : 0] M_AXI_GP0_WID;
      reg [1 : 0] M_AXI_GP0_ARBURST;
      reg [1 : 0] M_AXI_GP0_ARLOCK;
      reg [2 : 0] M_AXI_GP0_ARSIZE;
      reg [1 : 0] M_AXI_GP0_AWBURST;
      reg [1 : 0] M_AXI_GP0_AWLOCK;
      reg [2 : 0] M_AXI_GP0_AWSIZE;
      reg [2 : 0] M_AXI_GP0_ARPROT;
      reg [2 : 0] M_AXI_GP0_AWPROT;
      reg [31 : 0] M_AXI_GP0_ARADDR;
      reg [31 : 0] M_AXI_GP0_AWADDR;
      reg [31 : 0] M_AXI_GP0_WDATA;
      reg [3 : 0] M_AXI_GP0_ARCACHE;
      reg [3 : 0] M_AXI_GP0_ARLEN;
      reg [3 : 0] M_AXI_GP0_ARQOS;
      reg [3 : 0] M_AXI_GP0_AWCACHE;
      reg [3 : 0] M_AXI_GP0_AWLEN;
      reg [3 : 0] M_AXI_GP0_AWQOS;
      reg [3 : 0] M_AXI_GP0_WSTRB;
      reg S_AXI_HP0_ARREADY;
      reg S_AXI_HP0_AWREADY;
      reg S_AXI_HP0_BVALID;
      reg S_AXI_HP0_RLAST;
      reg S_AXI_HP0_RVALID;
      reg S_AXI_HP0_WREADY;
      reg [1 : 0] S_AXI_HP0_BRESP;
      reg [1 : 0] S_AXI_HP0_RRESP;
      reg [5 : 0] S_AXI_HP0_BID;
      reg [5 : 0] S_AXI_HP0_RID;
      reg [63 : 0] S_AXI_HP0_RDATA;
      reg [7 : 0] S_AXI_HP0_RCOUNT;
      reg [7 : 0] S_AXI_HP0_WCOUNT;
      reg [2 : 0] S_AXI_HP0_RACOUNT;
      reg [5 : 0] S_AXI_HP0_WACOUNT;
      reg FCLK_CLK0;
      reg FCLK_CLK1;
      reg FCLK_RESET0_N;
      string ip_name;
      reg disable_port;

//DPI DECLARATIONS
import "DPI-C" function void ps7_set_ip_context(input string ip_name);
import "DPI-C" function void ps7_set_str_param(input string name,input string val);
import "DPI-C" function void ps7_set_int_param(input string name,input longint val);
import "DPI-C" function void ps7_init_c_model();
import "DPI-C" function void ps7_set_input_IRQ_F2P(input int pinIdex, input int pinValue);
import "DPI-C" function void ps7_init_m_axi_gp0(input int M_AXI_GP0_AWID_size,input int M_AXI_GP0_AWADDR_size,input int M_AXI_GP0_AWLEN_size,input int M_AXI_GP0_AWSIZE_size,input int M_AXI_GP0_AWBURST_size,input int M_AXI_GP0_AWLOCK_size,input int M_AXI_GP0_AWCACHE_size,input int M_AXI_GP0_AWPROT_size,input int M_AXI_GP0_AWQOS_size,input int M_AXI_GP0_AWVALID_size,input int M_AXI_GP0_AWREADY_size,input int M_AXI_GP0_WID_size,input int M_AXI_GP0_WDATA_size,input int M_AXI_GP0_WSTRB_size,input int M_AXI_GP0_WLAST_size,input int M_AXI_GP0_WVALID_size,input int M_AXI_GP0_WREADY_size,input int M_AXI_GP0_BID_size,input int M_AXI_GP0_BRESP_size,input int M_AXI_GP0_BVALID_size,input int M_AXI_GP0_BREADY_size,input int M_AXI_GP0_ARID_size,input int M_AXI_GP0_ARADDR_size,input int M_AXI_GP0_ARLEN_size,input int M_AXI_GP0_ARSIZE_size,input int M_AXI_GP0_ARBURST_size,input int M_AXI_GP0_ARLOCK_size,input int M_AXI_GP0_ARCACHE_size,input int M_AXI_GP0_ARPROT_size,input int M_AXI_GP0_ARQOS_size,input int M_AXI_GP0_ARVALID_size,input int M_AXI_GP0_ARREADY_size,input int M_AXI_GP0_RID_size,input int M_AXI_GP0_RDATA_size,input int M_AXI_GP0_RRESP_size,input int M_AXI_GP0_RLAST_size,input int M_AXI_GP0_RVALID_size,input int M_AXI_GP0_RREADY_size);
import "DPI-C" function void ps7_init_s_axi_hp0(input int S_AXI_HP0_AWID_size,input int S_AXI_HP0_AWADDR_size,input int S_AXI_HP0_AWLEN_size,input int S_AXI_HP0_AWSIZE_size,input int S_AXI_HP0_AWBURST_size,input int S_AXI_HP0_AWLOCK_size,input int S_AXI_HP0_AWCACHE_size,input int S_AXI_HP0_AWPROT_size,input int S_AXI_HP0_AWQOS_size,input int S_AXI_HP0_AWVALID_size,input int S_AXI_HP0_AWREADY_size,input int S_AXI_HP0_WID_size,input int S_AXI_HP0_WDATA_size,input int S_AXI_HP0_WSTRB_size,input int S_AXI_HP0_WLAST_size,input int S_AXI_HP0_WVALID_size,input int S_AXI_HP0_WREADY_size,input int S_AXI_HP0_BID_size,input int S_AXI_HP0_BRESP_size,input int S_AXI_HP0_BVALID_size,input int S_AXI_HP0_BREADY_size,input int S_AXI_HP0_ARID_size,input int S_AXI_HP0_ARADDR_size,input int S_AXI_HP0_ARLEN_size,input int S_AXI_HP0_ARSIZE_size,input int S_AXI_HP0_ARBURST_size,input int S_AXI_HP0_ARLOCK_size,input int S_AXI_HP0_ARCACHE_size,input int S_AXI_HP0_ARPROT_size,input int S_AXI_HP0_ARQOS_size,input int S_AXI_HP0_ARVALID_size,input int S_AXI_HP0_ARREADY_size,input int S_AXI_HP0_RID_size,input int S_AXI_HP0_RDATA_size,input int S_AXI_HP0_RRESP_size,input int S_AXI_HP0_RLAST_size,input int S_AXI_HP0_RVALID_size,input int S_AXI_HP0_RREADY_size);
import "DPI-C" function void ps7_simulate_single_cycle_FCLK_CLK0();
import "DPI-C" function void ps7_simulate_single_cycle_FCLK_CLK1();
import "DPI-C" function void ps7_simulate_single_cycle_M_AXI_GP0_ACLK();
import "DPI-C" function void ps7_set_inputs_m_axi_gp0_M_AXI_GP0_ACLK(
input bit M_AXI_GP0_AWREADY,
input bit M_AXI_GP0_WREADY,
input bit [11 : 0] M_AXI_GP0_BID,
input bit [1 : 0] M_AXI_GP0_BRESP,
input bit M_AXI_GP0_BVALID,
input bit M_AXI_GP0_ARREADY,
input bit [11 : 0] M_AXI_GP0_RID,
input bit [31 : 0] M_AXI_GP0_RDATA,
input bit [1 : 0] M_AXI_GP0_RRESP,
input bit M_AXI_GP0_RLAST,
input bit M_AXI_GP0_RVALID
);
import "DPI-C" function void ps7_get_outputs_m_axi_gp0_M_AXI_GP0_ACLK(
output bit [11 : 0] M_AXI_GP0_AWID,
output bit [31 : 0] M_AXI_GP0_AWADDR,
output bit [3 : 0] M_AXI_GP0_AWLEN,
output bit [2 : 0] M_AXI_GP0_AWSIZE,
output bit [1 : 0] M_AXI_GP0_AWBURST,
output bit [1 : 0] M_AXI_GP0_AWLOCK,
output bit [3 : 0] M_AXI_GP0_AWCACHE,
output bit [2 : 0] M_AXI_GP0_AWPROT,
output bit [3 : 0] M_AXI_GP0_AWQOS,
output bit M_AXI_GP0_AWVALID,
output bit [11 : 0] M_AXI_GP0_WID,
output bit [31 : 0] M_AXI_GP0_WDATA,
output bit [3 : 0] M_AXI_GP0_WSTRB,
output bit M_AXI_GP0_WLAST,
output bit M_AXI_GP0_WVALID,
output bit M_AXI_GP0_BREADY,
output bit [11 : 0] M_AXI_GP0_ARID,
output bit [31 : 0] M_AXI_GP0_ARADDR,
output bit [3 : 0] M_AXI_GP0_ARLEN,
output bit [2 : 0] M_AXI_GP0_ARSIZE,
output bit [1 : 0] M_AXI_GP0_ARBURST,
output bit [1 : 0] M_AXI_GP0_ARLOCK,
output bit [3 : 0] M_AXI_GP0_ARCACHE,
output bit [2 : 0] M_AXI_GP0_ARPROT,
output bit [3 : 0] M_AXI_GP0_ARQOS,
output bit M_AXI_GP0_ARVALID,
output bit M_AXI_GP0_RREADY
);

import "DPI-C" function void ps7_simulate_single_cycle_S_AXI_HP0_ACLK();
import "DPI-C" function void ps7_set_inputs_s_axi_hp0_S_AXI_HP0_ACLK(
input bit [5 : 0] S_AXI_HP0_AWID,
input bit [31 : 0] S_AXI_HP0_AWADDR,
input bit [3 : 0] S_AXI_HP0_AWLEN,
input bit [2 : 0] S_AXI_HP0_AWSIZE,
input bit [1 : 0] S_AXI_HP0_AWBURST,
input bit [1 : 0] S_AXI_HP0_AWLOCK,
input bit [3 : 0] S_AXI_HP0_AWCACHE,
input bit [2 : 0] S_AXI_HP0_AWPROT,
input bit [3 : 0] S_AXI_HP0_AWQOS,
input bit S_AXI_HP0_AWVALID,
input bit [5 : 0] S_AXI_HP0_WID,
input bit [63 : 0] S_AXI_HP0_WDATA,
input bit [7 : 0] S_AXI_HP0_WSTRB,
input bit S_AXI_HP0_WLAST,
input bit S_AXI_HP0_WVALID,
input bit S_AXI_HP0_BREADY,
input bit [5 : 0] S_AXI_HP0_ARID,
input bit [31 : 0] S_AXI_HP0_ARADDR,
input bit [3 : 0] S_AXI_HP0_ARLEN,
input bit [2 : 0] S_AXI_HP0_ARSIZE,
input bit [1 : 0] S_AXI_HP0_ARBURST,
input bit [1 : 0] S_AXI_HP0_ARLOCK,
input bit [3 : 0] S_AXI_HP0_ARCACHE,
input bit [2 : 0] S_AXI_HP0_ARPROT,
input bit [3 : 0] S_AXI_HP0_ARQOS,
input bit S_AXI_HP0_ARVALID,
input bit S_AXI_HP0_RREADY
);
import "DPI-C" function void ps7_get_outputs_s_axi_hp0_S_AXI_HP0_ACLK(
output bit S_AXI_HP0_AWREADY,
output bit S_AXI_HP0_WREADY,
output bit [5 : 0] S_AXI_HP0_BID,
output bit [1 : 0] S_AXI_HP0_BRESP,
output bit S_AXI_HP0_BVALID,
output bit S_AXI_HP0_ARREADY,
output bit [5 : 0] S_AXI_HP0_RID,
output bit [63 : 0] S_AXI_HP0_RDATA,
output bit [1 : 0] S_AXI_HP0_RRESP,
output bit S_AXI_HP0_RLAST,
output bit S_AXI_HP0_RVALID
);

   export "DPI-C" function ps7_stop_sim;
   function void ps7_stop_sim();
        $display("End of simulation");
        $finish(0);
   endfunction
   export "DPI-C" function ps7_get_time;
   function real ps7_get_time();
       ps7_get_time = $time;
   endfunction

   export "DPI-C" function ps7_set_output_pins_FCLK_RESET0_N;
   function void ps7_set_output_pins_FCLK_RESET0_N(int value);
       FCLK_RESET0_N = value; 
   endfunction

   export "DPI-C" function ps7_set_output_pins_FCLK_RESET1_N;
   function void ps7_set_output_pins_FCLK_RESET1_N(int value);
       FCLK_RESET1_N = value; 
   endfunction

   export "DPI-C" function ps7_set_output_pins_FCLK_RESET2_N;
   function void ps7_set_output_pins_FCLK_RESET2_N(int value);
       FCLK_RESET2_N = value; 
   endfunction

   export "DPI-C" function ps7_set_output_pins_FCLK_RESET3_N;
   function void ps7_set_output_pins_FCLK_RESET3_N(int value);
       FCLK_RESET3_N = value; 
   endfunction


//INITIAL BLOCK

   initial
   begin
  $sformat(ip_name,"%m");
      ps7_set_ip_context(ip_name);
      ps7_set_int_param ( "C_EN_EMIO_PJTAG",C_EN_EMIO_PJTAG );
      ps7_set_int_param ( "C_EN_EMIO_ENET0",C_EN_EMIO_ENET0 );
      ps7_set_int_param ( "C_EN_EMIO_ENET1",C_EN_EMIO_ENET1 );
      ps7_set_int_param ( "C_EN_EMIO_TRACE",C_EN_EMIO_TRACE );
      ps7_set_int_param ( "C_INCLUDE_TRACE_BUFFER",C_INCLUDE_TRACE_BUFFER );
      ps7_set_int_param ( "C_TRACE_BUFFER_FIFO_SIZE",C_TRACE_BUFFER_FIFO_SIZE );
      ps7_set_int_param ( "USE_TRACE_DATA_EDGE_DETECTOR",USE_TRACE_DATA_EDGE_DETECTOR );
      ps7_set_int_param ( "C_TRACE_PIPELINE_WIDTH",C_TRACE_PIPELINE_WIDTH );
      ps7_set_int_param ( "C_TRACE_BUFFER_CLOCK_DELAY",C_TRACE_BUFFER_CLOCK_DELAY );
      ps7_set_int_param ( "C_EMIO_GPIO_WIDTH",C_EMIO_GPIO_WIDTH );
      ps7_set_int_param ( "C_INCLUDE_ACP_TRANS_CHECK",C_INCLUDE_ACP_TRANS_CHECK );
      ps7_set_int_param ( "C_USE_DEFAULT_ACP_USER_VAL",C_USE_DEFAULT_ACP_USER_VAL );
      ps7_set_int_param ( "C_S_AXI_ACP_ARUSER_VAL",C_S_AXI_ACP_ARUSER_VAL );
      ps7_set_int_param ( "C_S_AXI_ACP_AWUSER_VAL",C_S_AXI_ACP_AWUSER_VAL );
      ps7_set_int_param ( "C_M_AXI_GP0_ID_WIDTH",C_M_AXI_GP0_ID_WIDTH );
      ps7_set_int_param ( "C_M_AXI_GP0_ENABLE_STATIC_REMAP",C_M_AXI_GP0_ENABLE_STATIC_REMAP );
      ps7_set_int_param ( "C_M_AXI_GP1_ID_WIDTH",C_M_AXI_GP1_ID_WIDTH );
      ps7_set_int_param ( "C_M_AXI_GP1_ENABLE_STATIC_REMAP",C_M_AXI_GP1_ENABLE_STATIC_REMAP );
      ps7_set_int_param ( "C_S_AXI_GP0_ID_WIDTH",C_S_AXI_GP0_ID_WIDTH );
      ps7_set_int_param ( "C_S_AXI_GP1_ID_WIDTH",C_S_AXI_GP1_ID_WIDTH );
      ps7_set_int_param ( "C_S_AXI_ACP_ID_WIDTH",C_S_AXI_ACP_ID_WIDTH );
      ps7_set_int_param ( "C_S_AXI_HP0_ID_WIDTH",C_S_AXI_HP0_ID_WIDTH );
      ps7_set_int_param ( "C_S_AXI_HP0_DATA_WIDTH",C_S_AXI_HP0_DATA_WIDTH );
      ps7_set_int_param ( "C_S_AXI_HP1_ID_WIDTH",C_S_AXI_HP1_ID_WIDTH );
      ps7_set_int_param ( "C_S_AXI_HP1_DATA_WIDTH",C_S_AXI_HP1_DATA_WIDTH );
      ps7_set_int_param ( "C_S_AXI_HP2_ID_WIDTH",C_S_AXI_HP2_ID_WIDTH );
      ps7_set_int_param ( "C_S_AXI_HP2_DATA_WIDTH",C_S_AXI_HP2_DATA_WIDTH );
      ps7_set_int_param ( "C_S_AXI_HP3_ID_WIDTH",C_S_AXI_HP3_ID_WIDTH );
      ps7_set_int_param ( "C_S_AXI_HP3_DATA_WIDTH",C_S_AXI_HP3_DATA_WIDTH );
      ps7_set_int_param ( "C_M_AXI_GP0_THREAD_ID_WIDTH",C_M_AXI_GP0_THREAD_ID_WIDTH );
      ps7_set_int_param ( "C_M_AXI_GP1_THREAD_ID_WIDTH",C_M_AXI_GP1_THREAD_ID_WIDTH );
      ps7_set_int_param ( "C_NUM_F2P_INTR_INPUTS",C_NUM_F2P_INTR_INPUTS );
      ps7_set_str_param ( "C_IRQ_F2P_MODE",C_IRQ_F2P_MODE );
      ps7_set_int_param ( "C_DQ_WIDTH",C_DQ_WIDTH );
      ps7_set_int_param ( "C_DQS_WIDTH",C_DQS_WIDTH );
      ps7_set_int_param ( "C_DM_WIDTH",C_DM_WIDTH );
      ps7_set_int_param ( "C_MIO_PRIMITIVE",C_MIO_PRIMITIVE );
      ps7_set_int_param ( "C_TRACE_INTERNAL_WIDTH",C_TRACE_INTERNAL_WIDTH );
      ps7_set_int_param ( "C_USE_AXI_NONSECURE",C_USE_AXI_NONSECURE );
      ps7_set_int_param ( "C_USE_M_AXI_GP0",C_USE_M_AXI_GP0 );
      ps7_set_int_param ( "C_USE_M_AXI_GP1",C_USE_M_AXI_GP1 );
      ps7_set_int_param ( "C_USE_S_AXI_GP0",C_USE_S_AXI_GP0 );
      ps7_set_int_param ( "C_USE_S_AXI_GP1",C_USE_S_AXI_GP1 );
      ps7_set_int_param ( "C_USE_S_AXI_HP0",C_USE_S_AXI_HP0 );
      ps7_set_int_param ( "C_USE_S_AXI_HP1",C_USE_S_AXI_HP1 );
      ps7_set_int_param ( "C_USE_S_AXI_HP2",C_USE_S_AXI_HP2 );
      ps7_set_int_param ( "C_USE_S_AXI_HP3",C_USE_S_AXI_HP3 );
      ps7_set_int_param ( "C_USE_S_AXI_ACP",C_USE_S_AXI_ACP );
      ps7_set_str_param ( "C_PS7_SI_REV",C_PS7_SI_REV );
      ps7_set_str_param ( "C_FCLK_CLK0_BUF",C_FCLK_CLK0_BUF );
      ps7_set_str_param ( "C_FCLK_CLK1_BUF",C_FCLK_CLK1_BUF );
      ps7_set_str_param ( "C_FCLK_CLK2_BUF",C_FCLK_CLK2_BUF );
      ps7_set_str_param ( "C_FCLK_CLK3_BUF",C_FCLK_CLK3_BUF );
      ps7_set_str_param ( "C_PACKAGE_NAME",C_PACKAGE_NAME );
      ps7_set_str_param ( "C_GP0_EN_MODIFIABLE_TXN",C_GP0_EN_MODIFIABLE_TXN );
      ps7_set_str_param ( "C_GP1_EN_MODIFIABLE_TXN",C_GP1_EN_MODIFIABLE_TXN );

  ps7_init_m_axi_gp0($bits(M_AXI_GP0_AWID),$bits(M_AXI_GP0_AWADDR),$bits(M_AXI_GP0_AWLEN),$bits(M_AXI_GP0_AWSIZE),$bits(M_AXI_GP0_AWBURST),$bits(M_AXI_GP0_AWLOCK),$bits(M_AXI_GP0_AWCACHE),$bits(M_AXI_GP0_AWPROT),$bits(M_AXI_GP0_AWQOS),$bits(M_AXI_GP0_AWVALID),$bits(M_AXI_GP0_AWREADY),$bits(M_AXI_GP0_WID),$bits(M_AXI_GP0_WDATA),$bits(M_AXI_GP0_WSTRB),$bits(M_AXI_GP0_WLAST),$bits(M_AXI_GP0_WVALID),$bits(M_AXI_GP0_WREADY),$bits(M_AXI_GP0_BID),$bits(M_AXI_GP0_BRESP),$bits(M_AXI_GP0_BVALID),$bits(M_AXI_GP0_BREADY),$bits(M_AXI_GP0_ARID),$bits(M_AXI_GP0_ARADDR),$bits(M_AXI_GP0_ARLEN),$bits(M_AXI_GP0_ARSIZE),$bits(M_AXI_GP0_ARBURST),$bits(M_AXI_GP0_ARLOCK),$bits(M_AXI_GP0_ARCACHE),$bits(M_AXI_GP0_ARPROT),$bits(M_AXI_GP0_ARQOS),$bits(M_AXI_GP0_ARVALID),$bits(M_AXI_GP0_ARREADY),$bits(M_AXI_GP0_RID),$bits(M_AXI_GP0_RDATA),$bits(M_AXI_GP0_RRESP),$bits(M_AXI_GP0_RLAST),$bits(M_AXI_GP0_RVALID),$bits(M_AXI_GP0_RREADY));

  ps7_init_s_axi_hp0($bits(S_AXI_HP0_AWID),$bits(S_AXI_HP0_AWADDR),$bits(S_AXI_HP0_AWLEN),$bits(S_AXI_HP0_AWSIZE),$bits(S_AXI_HP0_AWBURST),$bits(S_AXI_HP0_AWLOCK),$bits(S_AXI_HP0_AWCACHE),$bits(S_AXI_HP0_AWPROT),$bits(S_AXI_HP0_AWQOS),$bits(S_AXI_HP0_AWVALID),$bits(S_AXI_HP0_AWREADY),$bits(S_AXI_HP0_WID),$bits(S_AXI_HP0_WDATA),$bits(S_AXI_HP0_WSTRB),$bits(S_AXI_HP0_WLAST),$bits(S_AXI_HP0_WVALID),$bits(S_AXI_HP0_WREADY),$bits(S_AXI_HP0_BID),$bits(S_AXI_HP0_BRESP),$bits(S_AXI_HP0_BVALID),$bits(S_AXI_HP0_BREADY),$bits(S_AXI_HP0_ARID),$bits(S_AXI_HP0_ARADDR),$bits(S_AXI_HP0_ARLEN),$bits(S_AXI_HP0_ARSIZE),$bits(S_AXI_HP0_ARBURST),$bits(S_AXI_HP0_ARLOCK),$bits(S_AXI_HP0_ARCACHE),$bits(S_AXI_HP0_ARPROT),$bits(S_AXI_HP0_ARQOS),$bits(S_AXI_HP0_ARVALID),$bits(S_AXI_HP0_ARREADY),$bits(S_AXI_HP0_RID),$bits(S_AXI_HP0_RDATA),$bits(S_AXI_HP0_RRESP),$bits(S_AXI_HP0_RLAST),$bits(S_AXI_HP0_RVALID),$bits(S_AXI_HP0_RREADY));
  ps7_init_c_model();
  end
  initial
  begin
     FCLK_CLK0 = 1'b0;
  end

  always #(10.0) FCLK_CLK0 <= ~FCLK_CLK0;

  always@(posedge FCLK_CLK0)
  begin
   ps7_set_ip_context(ip_name);
   ps7_simulate_single_cycle_FCLK_CLK0();
  end

  initial
  begin
     FCLK_CLK1 = 1'b0;
  end

  always #(20.0) FCLK_CLK1 <= ~FCLK_CLK1;

  always@(posedge FCLK_CLK1)
  begin
   ps7_set_ip_context(ip_name);
   ps7_simulate_single_cycle_FCLK_CLK1();
  end

always@(posedge IRQ_F2P[0])
begin
    ps7_set_input_IRQ_F2P(0,1);
end
always@(negedge IRQ_F2P[0])
begin
    ps7_set_input_IRQ_F2P(0,0);
end
always@(posedge IRQ_F2P[1])
begin
    ps7_set_input_IRQ_F2P(1,1);
end
always@(negedge IRQ_F2P[1])
begin
    ps7_set_input_IRQ_F2P(1,0);
end

always@(posedge M_AXI_GP0_ACLK)
  begin

   ps7_set_ip_context(ip_name);

   ps7_set_inputs_m_axi_gp0_M_AXI_GP0_ACLK(
    M_AXI_GP0_AWREADY,
    M_AXI_GP0_WREADY,
    M_AXI_GP0_BID,
    M_AXI_GP0_BRESP,
    M_AXI_GP0_BVALID,
    M_AXI_GP0_ARREADY,
    M_AXI_GP0_RID,
    M_AXI_GP0_RDATA,
    M_AXI_GP0_RRESP,
    M_AXI_GP0_RLAST,
    M_AXI_GP0_RVALID
  );

   ps7_simulate_single_cycle_M_AXI_GP0_ACLK();

   ps7_get_outputs_m_axi_gp0_M_AXI_GP0_ACLK(
    M_AXI_GP0_AWID,
    M_AXI_GP0_AWADDR,
    M_AXI_GP0_AWLEN,
    M_AXI_GP0_AWSIZE,
    M_AXI_GP0_AWBURST,
    M_AXI_GP0_AWLOCK,
    M_AXI_GP0_AWCACHE,
    M_AXI_GP0_AWPROT,
    M_AXI_GP0_AWQOS,
    M_AXI_GP0_AWVALID,
    M_AXI_GP0_WID,
    M_AXI_GP0_WDATA,
    M_AXI_GP0_WSTRB,
    M_AXI_GP0_WLAST,
    M_AXI_GP0_WVALID,
    M_AXI_GP0_BREADY,
    M_AXI_GP0_ARID,
    M_AXI_GP0_ARADDR,
    M_AXI_GP0_ARLEN,
    M_AXI_GP0_ARSIZE,
    M_AXI_GP0_ARBURST,
    M_AXI_GP0_ARLOCK,
    M_AXI_GP0_ARCACHE,
    M_AXI_GP0_ARPROT,
    M_AXI_GP0_ARQOS,
    M_AXI_GP0_ARVALID,
    M_AXI_GP0_RREADY
  );
   end


always@(posedge S_AXI_HP0_ACLK)
  begin

   ps7_set_ip_context(ip_name);

   ps7_set_inputs_s_axi_hp0_S_AXI_HP0_ACLK(
    S_AXI_HP0_AWID,
    S_AXI_HP0_AWADDR,
    S_AXI_HP0_AWLEN,
    S_AXI_HP0_AWSIZE,
    S_AXI_HP0_AWBURST,
    S_AXI_HP0_AWLOCK,
    S_AXI_HP0_AWCACHE,
    S_AXI_HP0_AWPROT,
    S_AXI_HP0_AWQOS,
    S_AXI_HP0_AWVALID,
    S_AXI_HP0_WID,
    S_AXI_HP0_WDATA,
    S_AXI_HP0_WSTRB,
    S_AXI_HP0_WLAST,
    S_AXI_HP0_WVALID,
    S_AXI_HP0_BREADY,
    S_AXI_HP0_ARID,
    S_AXI_HP0_ARADDR,
    S_AXI_HP0_ARLEN,
    S_AXI_HP0_ARSIZE,
    S_AXI_HP0_ARBURST,
    S_AXI_HP0_ARLOCK,
    S_AXI_HP0_ARCACHE,
    S_AXI_HP0_ARPROT,
    S_AXI_HP0_ARQOS,
    S_AXI_HP0_ARVALID,
    S_AXI_HP0_RREADY
  );

   ps7_simulate_single_cycle_S_AXI_HP0_ACLK();

   ps7_get_outputs_s_axi_hp0_S_AXI_HP0_ACLK(
    S_AXI_HP0_AWREADY,
    S_AXI_HP0_WREADY,
    S_AXI_HP0_BID,
    S_AXI_HP0_BRESP,
    S_AXI_HP0_BVALID,
    S_AXI_HP0_ARREADY,
    S_AXI_HP0_RID,
    S_AXI_HP0_RDATA,
    S_AXI_HP0_RRESP,
    S_AXI_HP0_RLAST,
    S_AXI_HP0_RVALID
  );
   end

endmodule

