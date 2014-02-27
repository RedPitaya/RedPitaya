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
* @file xspips_selftest.c
*
* This component contains the implementation of selftest functions for an SPI
* device.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- ----------------------------------------------
* 1.00  drg/jz 01/25/10 First release
* 1.04a	sg     01/30/13 SetDelays test includes DelayTestNss parameter.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xspips.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/*****************************************************************************/
/**
*
* Runs a self-test on the driver/device. The self-test is destructive in that
* a reset of the device is performed in order to check the reset values of
* the registers and to get the device into a known state.
*
* Upon successful return from the self-test, the device is reset.
*
* @param	InstancePtr is a pointer to the XSpiPs instance.
*
* @return
* 		- XST_SUCCESS if successful
*		- XST_REGISTER_ERROR indicates a register did not read or write
*		correctly.
*
* @note		None.
*
******************************************************************************/
int XSpiPs_SelfTest(XSpiPs *InstancePtr)
{
	int Status;
	u32 Register;
	u8 DelayTestNss;
	u8 DelayTestBtwn;
	u8 DelayTestAfter;
	u8 DelayTestInit;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Reset the SPI device to leave it in a known good state
	 */
	XSpiPs_Reset(InstancePtr);

	/*
	 * All the SPI registers should be in their default state right now.
	 */
	Register = XSpiPs_ReadReg(InstancePtr->Config.BaseAddress,
				 XSPIPS_CR_OFFSET);
	if (Register != XSPIPS_CR_RESET_STATE) {
		return XST_REGISTER_ERROR;
	}

	Register = XSpiPs_ReadReg(InstancePtr->Config.BaseAddress,
				 XSPIPS_SR_OFFSET);
	if (Register != XSPIPS_ISR_RESET_STATE) {
		return XST_REGISTER_ERROR;
	}

	DelayTestNss = 0x5A;
	DelayTestBtwn = 0xA5;
	DelayTestAfter = 0xAA;
	DelayTestInit = 0x55;

	/*
	 * Write and read the delay register, just to be sure there is some
	 * hardware out there.
	 */
	Status = XSpiPs_SetDelays(InstancePtr, DelayTestNss, DelayTestBtwn,
				   DelayTestAfter, DelayTestInit);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	XSpiPs_GetDelays(InstancePtr, &DelayTestNss, &DelayTestBtwn,
			&DelayTestAfter, &DelayTestInit);
	if ((0x5A != DelayTestNss) || (0xA5 != DelayTestBtwn) ||
		(0xAA != DelayTestAfter) || (0x55 != DelayTestInit)) {
		return XST_REGISTER_ERROR;
	}

	Status = XSpiPs_SetDelays(InstancePtr, 0, 0, 0, 0);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Reset the SPI device to leave it in a known good state
	 */
	XSpiPs_Reset(InstancePtr);

	return XST_SUCCESS;
}
