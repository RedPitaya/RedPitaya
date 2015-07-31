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


#include <stdio.h>
#include <string.h>

#include <xadc_core_if.h>

#define MAX_CMD_NAME_SIZE 100
#define MAX_UNIT_NAME_SIZE 50

#define VCC_INT_CMD			"xadc_get_value_vccint"
#define VCC_AUX_CMD			"xadc_get_value_vccaux"
#define VCC_BRAM_CMD		"xadc_get_value_vccbram"
#define VCC_TEMP_CMD		"xadc_get_value_temp"
#define VCC_EXT_CH_CMD		"xadc_get_value_ext_ch"

struct command
{
	const enum XADC_Param parameter_id;
	const char cmd_name[MAX_CMD_NAME_SIZE];
	const char unit[MAX_UNIT_NAME_SIZE];
};

struct command command_list[EParamMax] = {
				{EParamVccInt, 	VCC_INT_CMD, "mV"},
				{EParamVccAux, 	VCC_AUX_CMD, "mV"},
				{EParamVccBRam, VCC_BRAM_CMD, "mV"},
				{EParamTemp, 	VCC_TEMP_CMD, "Degree Celsius"},
				{EParamVAux0, 	VCC_EXT_CH_CMD, "mV"}
};

int main(int argc, char *argv[])
{
	int i;

	if(xadc_core_init(EXADC_INIT_READ_ONLY) != 0)
	{
		perror("Coun't Start the XADC Core\nExiting\n");
		goto EXIT;
	}

	for (i=0; i < EParamMax; i++)
	{
		if(strcmp(argv[0],command_list[i].cmd_name) == 0)
		{
			printf("%.2f %s\n",xadc_touch(command_list[i].parameter_id), command_list[i].unit);
			goto EXIT;
		}
	}

	printf("This Program can not be used directly\n" \
			"Please rename (file) it to the commands needed\n" \
			"Following is the list of commands Available:\n");

	for(i = 0; i< EParamMax; i++)
	{
		printf("%s\n",command_list[i].cmd_name);
	}

EXIT:
 	xadc_core_deinit();
	return 0;
}

