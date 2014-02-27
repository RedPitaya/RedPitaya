/******************************************************************************
*
* (c) Copyright 2012-2013 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
******************************************************************************/
/*****************************************************************************/
/**
* @file sd_hardware.h
*
* Controller register and bit definitions
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who	Date	 Changes
* ----- ---- -------- ---------------------------------------------------
* 1.00a bh	02/01/11 Initial version,
* </pre>
*
* @note	None.
*
******************************************************************************/
#ifndef __SD_HARDWARE_H__
#define __SD_HARDWARE_H__

#define SD_BLOCK_SZ		0x200
#define SD_CLK_400K		400000
#define SD_CLK_50M		50000000
#define SD_CLK_25M		25000000
#define SD_HS_SUPPORT		0x2
#define SD_4BIT_SUPPORT		0x5

#define SD_DMA_ADDR_R		0x00
#define SD_BLOCK_SZ_R		0x04
#define SD_BLOCK_CNT_R		0x06

#define SD_ARG_R		0x08

#define SD_TRNS_MODE_R		0x0C
#define	SD_TRNS_DMA		0x01
#define	SD_TRNS_BLK_CNT_EN	0x02
#define	SD_TRNS_ACMD12		0x04
#define	SD_TRNS_READ		0x10
#define	SD_TRNS_MULTI		0x20

#define SD_CMD_R		0x0E
#define	SD_CMD_RESP_NONE	0x00
#define	SD_CMD_RESP_136	 	0x01
#define	SD_CMD_RESP_48		0x02
#define	SD_CMD_RESP_48_BUSY 	0x03
#define	SD_CMD_CRC		0x08
#define	SD_CMD_INDEX		0x10
#define	SD_CMD_DATA		0x20

#define SD_RSP_R		0x10

#define SD_BUFF_R		0x20

#define SD_PRES_STATE_R		0x24
#define	SD_CMD_INHIBIT		0x00000001
#define	SD_DATA_INHIBIT	 	0x00000002
#define	SD_WRITE_ACTIVE	 	0x00000100
#define	SD_READ_ACTIVE		0x00000200
#define	SD_CARD_INS		0x00010000
#define	SD_CARD_WP		0x00080000

#define SD_HOST_CTRL_R		0x28
#define SD_HOST_4BIT		0x02
#define SD_HOST_HS		0x04
#define SD_HOST_ADMA1		0x08
#define SD_HOST_ADMA2		0x10

#define SD_PWR_CTRL_R	 	0x29
#define	SD_POWER_ON		0x01
#define	SD_POWER_18		0x0A
#define	SD_POWER_30		0x0C
#define	SD_POWER_33		0x0E

#define SD_BLCK_GAP_CTL_R	0x2A

#define SD_WAKE_CTL_R		0x2B

#define SD_CLK_CTL_R		0x2C
#define	SD_DIV_SHIFT		8
#define	SD_CLK_SD_EN		0x0004
#define	SD_CLK_INT_STABLE	0x0002
#define	SD_CLK_INT_EN		0x0001

#define SD_TIMEOUT_CTL_R	0x2E

#define SD_SOFT_RST_R		0x2F
#define	SD_RST_ALL		0x01
#define	SD_RST_CMD		0x02
#define	SD_RST_DATA		0x04

#define SD_INT_STAT_R		0x30
#define SD_INT_ENA_R		0x34
#define SD_SIG_ENA_R		0x38
#define	SD_INT_CMD_CMPL	 	0x00000001
#define	SD_INT_TRNS_CMPL	0x00000002
#define	SD_INT_DMA		0x00000008
#define	SD_INT_ERROR		0x00008000
#define	SD_INT_ERR_CTIMEOUT 	0x00010000
#define	SD_INT_ERR_CCRC	 	0x00020000
#define	SD_INT_ERR_CEB		0x00040000
#define	SD_INT_ERR_IDX		0x00080000
#define	SD_INT_ERR_DTIMEOUT 	0x00100000
#define	SD_INT_ERR_DCRC	 	0x00200000
#define	SD_INT_ERR_DEB		0x00400000
#define	SD_INT_ERR_CLMT	 	0x00800000
#define	SD_INT_ERR_ACMD12	0x01000000
#define	SD_INT_ERR_ADMA	 	0x02000000
#define	SD_INT_ERR_TRESP	0x10000000

#define SD_CAPABILITIES_R	0x40

#define SD_ADMA_ADDR_R		0x58
#define DESC_ATBR_VALID     	(0x1 << 0)
#define DESC_ATBR_END       	(0x1 << 1)
#define DESC_ATBR_INT       	(0x1 << 2)
#define DESC_ATBR_ACT_TRAN  	(0x2 << 4)
#define DESC_ATBR_LEN_SHIFT	16

#endif
