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

#ifndef XADC_CORE_IF_H_
#define XADC_CORE_IF_H_

#include <stdbool.h>

enum XADC_Param
{
	EParamVccInt,
  	EParamVccAux,
	EParamVccBRam,
	EParamTemp,
	EParamVAux0,
	EParamMax
};

enum XADC_Alarm
{
	EAlarmVccInt_TH,
	EAlarmVccAux_TH,
	EAlarmVccBRam_TH,
	EAlarmTemp_TH,
	EAlarmMAX
};

struct Xadc_callback
{
	void(*func)(void *arg);
	void *arg;
};

enum XADC_Init_Type
{
	EXADC_INIT_READ_ONLY,
	EXADC_INIT_FULL,
	EXADC_NOT_INITIALIZED
};

/*
 * int xadc_core_init(enum XADC_Init_Type type);
 *
 * This function should be called once in the beginning
 * of the program. It is responsible for finding the
 * XADC device nodes and to initialized global variables.
 * If called with init type FULL, it also spawn the event
 * handling thread which monitors the alarms and handle
 * the particular event.
 *
 * Argument		:
 * 			- 	type: enum value for the type of initialization.
 * 						READ_ONLY type, if event handling is not
 * 						needed. FULL type, if event handling is
 * 						also required.
 *
 * Return Value : [Integer]  0 : For success,
 *                         -1 : Device node not found
 *
 */
int xadc_core_init(enum XADC_Init_Type type);

/*
 * int xadc_core_deinit(void);
 *
 * This function should be called at the end of program.
 * It is responsible for release the resources.
 *
 * Argument: N/A
 * Return Value:[Integer] 0: Success
 */
int xadc_core_deinit(void);

/*
 * void xadc_update_stat(void);
 *
 * This function just updates the global caches for all
 * the statistic parameters. This should be called before
 * xadc_get_value() fucntion, which reads the statistic
 * from the cache.
 *
 * Argument: N/A
 * Return Value: N/A
 */
void xadc_update_stat(void);

/*
 * float xadc_get_value(enum XADC_Param parameter);
 *
 * This function returns the cached statistics value of
 * the given parameter. For voltage, the return value is
 * in mili-volt and for temperature its degree Celcius.
 *
 * Argument:
 * 		- parameter: enum value for the parameter whose
 * 					 value is needed.
 * 	Return Value:[float] cached statistic of the given
 * 	             parameter[ mV / degree C].
 *
 */
float xadc_get_value(enum XADC_Param parameter);	// read the parameter from the cache [not real time update]

/*
 * float xadc_touch(enum XADC_Param parameter);
 *
 * This function updates the global cache statistic for the
 * given parameter. It returns the realtime value of the
 * given parameter unlike xadc_get_value.
 *
 * Argument:
 * 		- parameter: enum value for the parameter whose value
 * 		             is needed.
 *
 * Return Value:[float] realtime value of the given parameter
 *                      [ mV / degree C].
 */
float xadc_touch(enum XADC_Param parameter);	// update the cache and read the value

/*
 * int xadc_set_threshold(enum XADC_Alarm alarm,float threshold_low, float threshold_high, struct Xadc_callback *callback);
 *
 * This function sets the threshold values for the given alarm.
 *
 * Argument:
	-	alarm: enum value for the alarm for which threshold are set.
	-	threshold_low: Low threshold value for the alarm [mV /degreeC]
    -	threshold_high: High threshold value for the alarm [mV /degreeC]
    -	callback: callback information for the event on the given alarm.
                  If NULL is passed in this arg, no call back will be
                  registered otherwise on event occurrence callback->func(arg)
                  will be called.

 * Return Value: [Integer] 0: Success, Non-zero: Error
 */
int xadc_set_threshold(enum XADC_Alarm alarm,float threshold_low, float threshold_high, struct Xadc_callback *callback);

/*
 * bool xadc_get_alarm_status(enum XADC_Alarm alarm);
 *
 * This function returns the current status of the event for the given alarm.
 * Valid only after setting the threshold. It is useful in case, one don't
 * need callback but just get status of the event at some stage.
 *
 * Argument:
 * 	- alarm: enum value for the alarm for which event status is needed.
 *
 * 	Return Value:[bool] true: Event Active , false: Event Inactive.
 *
 */
bool xadc_get_alarm_status(enum XADC_Alarm alarm);

#endif /* XADC_CORE_IF_H_ */
