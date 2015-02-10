/******************************************************************************
*
* (c) Copyright 2010-2013 Xilinx, Inc. All rights reserved.
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
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xspips_hw.h
*
* This header file contains the identifiers and basic driver functions (or
* macros) that can be used to access the device. Other driver functions
* are defined in xspips.h.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- -----------------------------------------------
* 1.00   drg/jz 01/25/10 First release
* 1.02a  sg     05/31/12 Updated XSPIPS_FIFO_DEPTH to 128 from 32 to match HW
*			 for CR 658289
* 1.04a	 sg     01/30/13 Created XSPIPS_CR_MODF_GEN_EN_MASK macro and added it
*			 to XSPIPS_CR_RESET_STATE. Created
* 			 XSPIPS_IXR_WR_TO_CLR_MASK for interrupts which need
*			 write-to-clear. Added shift and mask macros for d_nss
*			 parameter. Added Rx Watermark mask.
*
* </pre>
*
******************************************************************************/

#ifndef XSPIPS_HW_H		/* prevent circular inclusions */
#define XSPIPS_HW_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xil_io.h"

/************************** Constant Definitions *****************************/

/** @name Register Map
 *
 * Register offsets from the base address of an SPI device.
 * @{
 */
#define XSPIPS_CR_OFFSET	0x00  /**< Configuration */
#define XSPIPS_SR_OFFSET	0x04  /**< Interrupt Status */
#define XSPIPS_IER_OFFSET	0x08  /**< Interrupt Enable */
#define XSPIPS_IDR_OFFSET	0x0c  /**< Interrupt Disable */
#define XSPIPS_IMR_OFFSET	0x10  /**< Interrupt Enabled Mask */
#define XSPIPS_ER_OFFSET	0x14  /**< Enable/Disable Register */
#define XSPIPS_DR_OFFSET	0x18  /**< Delay Register */
#define XSPIPS_TXD_OFFSET	0x1C  /**< Data Transmit Register */
#define XSPIPS_RXD_OFFSET	0x20  /**< Data Receive Register */
#define XSPIPS_SICR_OFFSET	0x24  /**< Slave Idle Count */
#define XSPIPS_TXWR_OFFSET	0x28  /**< Transmit FIFO Watermark */
#define XSPIPS_RXWR_OFFSET	0x2C  /**< Receive FIFO Watermark */
/* @} */

/** @name Configuration Register
 *
 * This register contains various control bits that
 * affects the operation of an SPI device. Read/Write.
 * @{
 */
#define XSPIPS_CR_MODF_GEN_EN_MASK 0x00020000 /**< Modefail Generation
						 Enable */
#define XSPIPS_CR_MANSTRT_MASK   0x00010000 /**< Manual Transmission Start */
#define XSPIPS_CR_MANSTRTEN_MASK 0x00008000 /**< Manual Transmission Start
						 Enable */
#define XSPIPS_CR_SSFORCE_MASK   0x00004000 /**< Force Slave Select */
#define XSPIPS_CR_SSCTRL_MASK    0x00003C00 /**< Slave Select Decode */
#define XSPIPS_CR_SSCTRL_SHIFT   10	    /**< Slave Select Decode shift */
#define XSPIPS_CR_SSCTRL_MAXIMUM 0xF	    /**< Slave Select maximum value */
#define XSPIPS_CR_SSDECEN_MASK   0x00000200 /**< Slave Select Decode Enable */

#define XSPIPS_CR_PRESC_MASK     0x00000038 /**< Prescaler Setting */
#define XSPIPS_CR_PRESC_SHIFT    3	    /**< Prescaler shift */
#define XSPIPS_CR_PRESC_MAXIMUM  0x07	    /**< Prescaler maximum value */

#define XSPIPS_CR_CPHA_MASK      0x00000004 /**< Phase Configuration */
#define XSPIPS_CR_CPOL_MASK      0x00000002 /**< Polarity Configuration */

#define XSPIPS_CR_MSTREN_MASK    0x00000001 /**< Master Mode Enable */
#define XSPIPS_CR_RESET_STATE    0x00020000 /**< Mode Fail Generation Enable */
/* @} */


/** @name SPI Interrupt Registers
 *
 * <b>SPI Status Register</b>
 *
 * This register holds the interrupt status flags for an SPI device. Some
 * of the flags are level triggered, which means that they are set as long
 * as the interrupt condition exists. Other flags are edge triggered,
 * which means they are set once the interrupt condition occurs and remain
 * set until they are cleared by software. The interrupts are cleared by
 * writing a '1' to the interrupt bit position in the Status Register.
 * Read/Write.
 *
 * <b>SPI Interrupt Enable Register</b>
 *
 * This register is used to enable chosen interrupts for an SPI device.
 * Writing a '1' to a bit in this register sets the corresponding bit in the
 * SPI Interrupt Mask register.  Write only.
 *
 * <b>SPI Interrupt Disable Register </b>
 *
 * This register is used to disable chosen interrupts for an SPI device.
 * Writing a '1' to a bit in this register clears the corresponding bit in the
 * SPI Interrupt Mask register. Write only.
 *
 * <b>SPI Interrupt Mask Register</b>
 *
 * This register shows the enabled/disabled interrupts of an SPI device.
 * Read only.
 *
 * All four registers have the same bit definitions. They are only defined once
 * for each of the Interrupt Enable Register, Interrupt Disable Register,
 * Interrupt Mask Register, and Channel Interrupt Status Register
 * @{
 */

#define XSPIPS_IXR_TXUF_MASK		0x00000040  /**< Tx FIFO Underflow */
#define XSPIPS_IXR_RXFULL_MASK		0x00000020  /**< Rx FIFO Full */
#define XSPIPS_IXR_RXNEMPTY_MASK	0x00000010  /**< Rx FIFO Not Empty */
#define XSPIPS_IXR_TXFULL_MASK		0x00000008  /**< Tx FIFO Full */
#define XSPIPS_IXR_TXOW_MASK		0x00000004  /**< Tx FIFO Overwater */
#define XSPIPS_IXR_MODF_MASK		0x00000002  /**< Mode Fault */
#define XSPIPS_IXR_RXOVR_MASK		0x00000001  /**< Rx FIFO Overrun */
#define XSPIPS_IXR_DFLT_MASK		0x00000027  /**< Default interrupts
							 mask */
#define XSPIPS_IXR_WR_TO_CLR_MASK	0x00000043  /**< Interrupts which
							 need write to clear */
#define XSPIPS_ISR_RESET_STATE		0x04	    /**< Default to tx/rx
						       * reg empty */
/* @} */


/** @name Enable Register
 *
 * This register is used to enable or disable an SPI device.
 * Read/Write
 * @{
 */
#define XSPIPS_ER_ENABLE_MASK	0x00000001 /**< SPI Enable Bit Mask */
/* @} */


/** @name Delay Register
 *
 * This register is used to program timing delays in
 * slave mode. Read/Write
 * @{
 */
#define XSPIPS_DR_NSS_MASK	0xFF000000 /**< Delay for slave select
					      * de-assertion between
					      * word transfers mask */
#define XSPIPS_DR_NSS_SHIFT	24	   /**< Delay for slave select
					      * de-assertion between
					      * word transfers shift */
#define XSPIPS_DR_BTWN_MASK	0x00FF0000 /**< Delay Between Transfers	mask */
#define XSPIPS_DR_BTWN_SHIFT	16	   /**< Delay Between Transfers shift */
#define XSPIPS_DR_AFTER_MASK	0x0000FF00 /**< Delay After Transfers mask */
#define XSPIPS_DR_AFTER_SHIFT	8 	   /**< Delay After Transfers shift */
#define XSPIPS_DR_INIT_MASK	0x000000FF /**< Delay Initially mask */
/* @} */


/** @name Slave Idle Count Registers
 *
 * This register defines the number of pclk cycles the slave waits for a the
 * SPI clock to become stable in quiescent state before it can detect the start
 * of the next transfer in CPHA = 1 mode.
 * Read/Write
 *
 * @{
 */
#define XSPIPS_SICR_MASK	0x000000FF /**< Slave Idle Count Mask */
/* @} */



/** @name Transmit FIFO Watermark Register
 *
 * This register defines the watermark setting for the Transmit FIFO. The
 * transmit FIFO is 128 bytes deep, so the register is 7 bits. Valid values
 * are 1 to 128.
 *
 * @{
 */
#define XSPIPS_TXWR_MASK	0x0000007F /**< Transmit Watermark Mask */
/* @} */

/** @name Receive FIFO Watermark Register
 *
 * This register defines the watermark setting for the Receive FIFO. The
 * receive FIFO is 128 bytes deep, so the register is 7 bits. Valid values
 * are 1 to 128.
 *
 * @{
 */
#define XSPIPS_RXWR_MASK	0x0000007F /**< Receive Watermark Mask */
/* @} */

/** @name FIFO Depth
 *
 * This macro provides the depth of transmit FIFO and receive FIFO.
 *
 * @{
 */
#define XSPIPS_FIFO_DEPTH	128 /**< FIFO depth of Tx and Rx */
/* @} */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#define XSpiPs_In32 Xil_In32
#define XSpiPs_Out32 Xil_Out32

/****************************************************************************/
/**
* Read a register.
*
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the 1st register of the
*		device to the target register.
*
* @return	The value read from the register.
*
* @note		C-Style signature:
*		u32 XSpiPs_ReadReg(u32 BaseAddress. int RegOffset)
*
******************************************************************************/
#define XSpiPs_ReadReg(BaseAddress, RegOffset) \
	XSpiPs_In32((BaseAddress) + (RegOffset))

/***************************************************************************/
/**
* Write to a register.
*
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the 1st register of the
*		device to target register.
* @param	RegisterValue is the value to be written to the register.
*
* @return	None.
*
* @note		C-Style signature:
*		void XSpiPs_WriteReg(u32 BaseAddress, int RegOffset,
*		u32 RegisterValue)
*
******************************************************************************/
#define XSpiPs_WriteReg(BaseAddress, RegOffset, RegisterValue) \
    XSpiPs_Out32((BaseAddress) + (RegOffset), (RegisterValue))

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
