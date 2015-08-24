/*****************************************************************************
 *
 *     Author: Xilinx, Inc. (c) Copyright 2012 Xilinx Inc.
 *     All rights reserved.
 *
 *     This file may be used under the terms of the GNU General Public
 *     License version 3.0 as published by the Free Software Foundation
 *     and appearing in the file LICENSE.GPL included in the packaging of
 *     this file.  Please review the following information to ensure the
 *     GNU General Public License version 3.0 requirements will be met.
 *     http://www.gnu.org/copyleft/gpl.html.
 *
 *     With respect to any license that requires Xilinx to make available to
 *     recipients of object code distributed by Xilinx pursuant to such
 *     license the corresponding source code, and if you desire to receive
 *     such source code from Xilinx and cannot access the internet to obtain
 *     a copy thereof, then Xilinx hereby offers (which offer is valid for as
 *     long as required by the applicable license; and we may charge you the
 *     cost thereof unless prohibited by the license) to provide you with a
 *     copy of such source code; and to accept such offer send a letter
 *     requesting such source code (please be specific by identifying the
 *     particular Xilinx Software you are inquiring about (name and version
 *     number), to:  Xilinx, Inc., Legal Department, Attention: Software
 *     Compliance Officer, 2100 Logic Drive, San Jose, CA U.S.A. 95124.
 *
 *     XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS"
 *     AS A COURTESY TO YOU, SOLELY FOR USE IN DEVELOPING PROGRAMS AND
 *     SOLUTIONS FOR XILINX DEVICES.  BY PROVIDING THIS DESIGN, CODE,
 *     OR INFORMATION AS ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE,
 *     APPLICATION OR STANDARD, XILINX IS MAKING NO REPRESENTATION
 *     THAT THIS IMPLEMENTATION IS FREE FROM ANY CLAIMS OF INFRINGEMENT,
 *     AND YOU ARE RESPONSIBLE FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE
 *     FOR YOUR IMPLEMENTATION.  XILINX EXPRESSLY DISCLAIMS ANY
 *     WARRANTY WHATSOEVER WITH RESPECT TO THE ADEQUACY OF THE
 *     IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OR
 *     REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM CLAIMS OF
 *     INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *     FOR A PARTICULAR PURPOSE.
 *
 *     CRITICAL APPLICATIONS WARRANTIES
 *     Xilinx products are not designed or intended to be fail-safe, or
 *     for use in any application requiring fail-safe performance, such as
 *     life-support or safety devices or systems, Class III medical devices,
 *     nuclear facilities, applications related to the deployment of airbags,
 *     or any other applications that could lead to death, personal injury,
 *     or severe property or environmental damage (individually and
 *     collectively,  "Critical Applications"). Customer assumes the sole
 *     risk and liability  of any use of Xilinx products in Critical
 *     Applications, subject only to applicable laws and regulations
 *     governing limitations on product liability.
 *
 *     THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF
 *     THIS FILE AT ALL TIMES.
 *
 *     This file is a part of sobel_qt application, which is based in part
 *     on the work of the Qwt project (http://qwt.sf.net).
 *
 *****************************************************************************/

#ifndef XADC_CORE_H_
#define XADC_CORE_H_

#define MAX_PATH_SIZE	200
#define MAX_NAME_SIZE	50
#define MAX_VALUE_SIZE  100

#define SYS_PATH_IIO	"/sys/bus/iio/devices"
#define DEVICE_NAME 	"xadc"

enum EErrorCode
{
	RET_SUCCESS,
	RET_ERR_DEV_NOT_FOUND,
	RET_CANNOT_OPEN_FILE,
	RET_FILE_READ_FAILED,
	RET_FILE_WRITE_FAILED
};

enum EConvType
{
	EConvType_None,
	EConvType_Raw_to_Scale,
	EConvType_Scale_to_Raw,
	EConvType_Max
};

//todo: Provide mutex lock for the structure
struct XadcParameter
{
	const char name[MAX_NAME_SIZE];
	float value;
	float (* const conv_fn)(float,enum EConvType);
	// mutex *lock;
};

struct Xadc_callback;

struct XadcThreshold
{
	const enum XADC_Alarm id;
	const char name[MAX_NAME_SIZE];
	float thres_low;
	float thres_high;
	float (* const conv_fn)(float,enum EConvType);
	bool isActive;
	struct Xadc_callback callback;
	pthread_mutex_t *lock;
};

#endif /* XADC_CORE_H_ */
