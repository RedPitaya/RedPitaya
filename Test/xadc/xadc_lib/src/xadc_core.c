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
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <ctype.h>
#include <pthread.h>
#include <assert.h>

/*
 * events.h is suppose to be part of cross compile tool's linux includes
 * But as its under development, the file is in ../linux_include
 * once it is in the main, once should include
 * #include <linux/iio/events.h>
 * This file should be always in sync with <kernel>/include/linux/iio/events.h
 *
 */
#include <events.h>

#include <xadc_core_if.h>
#include "xadc_core.h"

//todo: provide isInit assertion for all the functions.

#define MULTIPLIER_12_BIT

#ifdef MULTIPLIER_12_BIT
	static const int multiplier = 1 << 12;
#elif MULTIPLIER_16_BIT
	static const int multiplier = 1 << 16;
#endif
	static const int mV_mul = 1000;

//function prototype
float conv_temperature(float input, enum EConvType conv_direction);
float conv_voltage(float input, enum EConvType conv_direction);
float conv_voltage_ext_ch(float input, enum EConvType conv_direction);

static int line_from_file(char* filename, char* linebuf);
static int line_to_file(char* filename, char* linebuf);

static char gDevicePath[MAX_PATH_SIZE];
static char gNodeName[MAX_NAME_SIZE];
pthread_t gEvent_handler;
static bool gExitNow = false;
static enum XADC_Init_Type gInit_state = EXADC_NOT_INITIALIZED;

struct XadcParameter gXadcData[EParamMax] = {
	[EParamVccInt] = { "in_voltage0_vccint_raw", 	0, conv_voltage},
	[EParamVccAux] = { "in_voltage4_vccpaux_raw", 	0, conv_voltage},			// todo: workaround
	[EParamVccBRam]= { "in_voltage2_vccbram_raw", 	0, conv_voltage},
	[EParamTemp]   = { "in_temp0_raw", 				0, conv_temperature},
	[EParamVAux0]  = { "in_voltage8_raw",			0, conv_voltage_ext_ch}
};

struct XadcThreshold gXadcAlarm[EAlarmMAX] = {
		[EAlarmVccInt_TH] = { EAlarmVccInt_TH, 	"in_voltage0_vccint_thresh",  	0, 	0, conv_voltage, 		false,(struct Xadc_callback){0,0}, NULL},
		[EAlarmVccAux_TH] = { EAlarmVccAux_TH, 	"in_voltage4_vccpaux_thresh", 	0, 	0, conv_voltage, 		false,(struct Xadc_callback){0,0}, NULL},	// todo: workaround
		[EAlarmVccBRam_TH]= { EAlarmVccBRam_TH, "in_voltage2_vccbram_thresh", 	0, 	0, conv_voltage,		false,(struct Xadc_callback){0,0}, NULL},
		[EAlarmTemp_TH]   = { EAlarmTemp_TH, 	"in_temp0_thresh", 				0, 	0, conv_temperature,	false,(struct Xadc_callback){0,0}, NULL},
};

static int read_xadc_param(struct XadcParameter *param)
{
	char filename[MAX_PATH_SIZE];
	char read_value[MAX_VALUE_SIZE];

	memset(filename, 0, sizeof(filename) );

	sprintf(filename, "%s/%s", gDevicePath, param->name );

	if (line_from_file(filename,read_value) == RET_SUCCESS)
	{
		param->value = param->conv_fn(atof(read_value), EConvType_Raw_to_Scale);
	}
	else
	{
		printf("\n***Error: reading file %s\n",filename);
		param->value = 0;
		return RET_FILE_READ_FAILED;
	}

	return RET_SUCCESS;
}

static int write_xadc_threshold(struct XadcThreshold *param)
{
	char filename[MAX_PATH_SIZE];
	char write_value[MAX_VALUE_SIZE];
	float value = 0;

	memset(filename, 0, sizeof(filename));
	memset(write_value, 0, sizeof(write_value));

	// set the high threshold
	sprintf(filename, "%s/events/%s_rising_value", gDevicePath, param->name);
	value = param->conv_fn(param->thres_high, EConvType_Scale_to_Raw);
	sprintf(write_value, "%d", (unsigned int) (value + 0.5));
	if (line_to_file(filename, write_value) != RET_SUCCESS) {
		perror("Error writing high threshold");
	}

	if (param->id != EAlarmTemp_TH) {
		// set the low threshold
		sprintf(filename, "%s/events/%s_falling_value", gDevicePath,
				param->name);
		value = param->conv_fn(param->thres_low, EConvType_Scale_to_Raw);
		sprintf(write_value, "%d", (unsigned int) (value + 0.5));
		if (line_to_file(filename, write_value) != RET_SUCCESS) {
			perror("Error writing low threshold");
		}

	} else {
		// set the Hysteresis for temperature threshold
		sprintf(filename, "%s/events/%s_rising_hysteresis", gDevicePath,
				param->name);
		value = param->conv_fn(param->thres_high, EConvType_Scale_to_Raw) - param->conv_fn(param->thres_low, EConvType_Scale_to_Raw);
		sprintf(write_value, "%d", (unsigned int) (value + 0.5));
		if (line_to_file(filename, write_value) != RET_SUCCESS) {
			perror("Error writing Hysteresis value");
		}
	}

	if (param->id != EAlarmTemp_TH) {
		// Enable the alarm threshold low
		sprintf(filename, "%s/events/%s_falling_en", gDevicePath, param->name);
		if (line_to_file(filename, "1") != RET_SUCCESS) {
			perror("Error Enable low threshold");
		}
	}

	// Enable the alarm threshold high
	sprintf(filename, "%s/events/%s_rising_en", gDevicePath, param->name);
	if (line_to_file(filename, "1") != RET_SUCCESS) {
		perror("Error Enabling high threshold");
	}

	return RET_SUCCESS;
}


static int get_iio_node(const char * deviceName)
{
	struct dirent **namelist;
	char file[MAX_PATH_SIZE];
	char name[MAX_NAME_SIZE];
	int i,n;
	int flag = 0;

	n = scandir(SYS_PATH_IIO, &namelist, 0, alphasort);
	if (n < 0)
		return RET_ERR_DEV_NOT_FOUND;

	for (i=0; i < n; i++)
	{
		sprintf(file, "%s/%s/name", SYS_PATH_IIO, namelist[i]->d_name);
		if ((line_from_file(file,name) == 0) && (strcmp(name,deviceName) ==  0))
		{
			flag =1;
			strcpy(gNodeName, namelist[i]->d_name);
			sprintf(gDevicePath, "%s/%s", SYS_PATH_IIO, gNodeName);
			break;
		}
	}

	if(flag == 0) return RET_ERR_DEV_NOT_FOUND;

	return RET_SUCCESS;
}


static void do_event_action(enum iio_event_type ev_type, enum iio_chan_type ch_type, enum iio_event_direction ev_dir, int channel_number)
{

	enum XADC_Alarm alarm_id = EAlarmMAX;
	bool isActive = false;
	bool isEventValid = false;

	switch(ev_type)
	{
	case IIO_EV_TYPE_THRESH:
		isActive = true;
		isEventValid = true;
		break;
	case IIO_EV_TYPE_THRESH_NOT_ACTIVE:
		isActive = false;
		isEventValid = true;
		break;
	default:
		//not a valid event to handle do nothing.
		perror("Unknown event Type !!\n");
		isEventValid = false;
		return;
	}

	// sanity check for the direction:
	switch(ev_dir)
	{
	case IIO_EV_DIR_EITHER:
	case IIO_EV_DIR_RISING:
	case IIO_EV_DIR_FALLING:
		// do Nothing; as this is just sanity check.
		break;
	default:
		//not a valid event to handle do nothing.
		perror("Unknown event direction !!\n");
		isEventValid = false;
		return;
	}

	switch(ch_type)
	{
	case IIO_VOLTAGE:
		switch(channel_number)
		{
		case 0:
			alarm_id = EAlarmVccInt_TH;
			break;
		case 4:
			alarm_id = EAlarmVccAux_TH;
			break;
		case 2:
			alarm_id = EAlarmVccBRam_TH;
			break;
		case 8:
		default:
			printf("Voltage Threshold for this channel (%d) not supported!\n",channel_number);
			isEventValid = false;
			return;
		}
		break;
	case IIO_TEMP:
		if(channel_number == 0)
		{
			alarm_id = EAlarmTemp_TH;
		}
		else
		{
			printf("Temperature Threshold for this channel (%d) not supported!\n",channel_number);
			isEventValid = false;
			return;
		}
		break;
	default:
		//not a valid event to handle do nothing.
		perror("Unknown event!\n");
		isEventValid = false;
		return;
	}

	assert(isEventValid); //just sanity check... will never reach here for invalid event
	assert(alarm_id >= 0 && alarm_id < EAlarmMAX);

	pthread_mutex_lock(gXadcAlarm[alarm_id].lock);

	gXadcAlarm[alarm_id].isActive = isActive;

	//todo: Important... currently the function being called is in mutex lock.
	//      it shouldn't fail.
	//todo: callback can be called out side mutex.

	if (gXadcAlarm[alarm_id].callback.func)
	{
		gXadcAlarm[alarm_id].callback.func(gXadcAlarm[alarm_id].callback.arg);
	}

	pthread_mutex_unlock(gXadcAlarm[alarm_id].lock);
}

static void * thread_event_handler(void* args)
{
	int fd, event_fd;
	char device_file[MAX_PATH_SIZE];
	struct iio_event_data event;
	enum iio_event_type ev_type;
	enum iio_chan_type ch_type;
	enum iio_event_direction ev_dir;
	int channel_number;

	int ret = 0;

	if (sprintf(device_file, "/dev/%s", gNodeName) < 0)
	{
		perror("Error:Out of Memory");
		ret = 1;
		goto ERR_OUT;
	}

	if ((fd = open(device_file,O_RDONLY)) == -1)
	{
		perror("Failed to open device file");
		ret = 2;
		goto ERR_OUT;
	}

	//get the event_fd

	if (ioctl(fd, IIO_GET_EVENT_FD_IOCTL, &event_fd) == -1)
	{
		perror("IOCTL failed on device file");
		ret = 3;
		goto ERR_OUT;
	}

	close (fd);

	if(event_fd == -1)
	{
		perror("Couldn't obtain event file descriptor\n");
		ret = 3;
		goto ERR_OUT;
	}

	while (gExitNow == false)
	{
		ret=read(event_fd, &event, sizeof(event));
		if(ret == -1)
		{
			if(errno == EAGAIN)
			{
				printf("Nothing available\n");
				continue;
			}
			else
			{
				perror("Failed to read event from device");
				ret = -errno;
				break;
			}
		}
		// Parsing event
		ev_type = IIO_EVENT_CODE_EXTRACT_TYPE(event.id);
		ch_type = IIO_EVENT_CODE_EXTRACT_CHAN_TYPE(event.id);
		ev_dir = IIO_EVENT_CODE_EXTRACT_DIR(event.id);
		channel_number = IIO_EVENT_CODE_EXTRACT_CHAN(event.id);

		do_event_action(ev_type,ch_type,ev_dir,channel_number);
	}

	close (event_fd);

	return NULL;

ERR_OUT:
	exit(ret);
}

static void deinit_mutexes(int max_index)	// currently only for alarm; todo: modify it to handle for the parameters lock as well.
{
	if(max_index > 0)
	{
		while (max_index--)
		{
			assert(gXadcAlarm[max_index].lock != NULL);
			pthread_mutex_destroy(gXadcAlarm[max_index].lock);
			free(gXadcAlarm[max_index].lock);
			gXadcAlarm[max_index].lock = NULL;
		}
	}
	else
	{
		assert(0);
	}
}

static int init_mutexes(void)	// currently only for alarm modify it to handle for the parameters lock as well.
{
	int i;
	int ret = 0;
	for(i=0; i< EAlarmMAX; i++)
	{
		assert(gXadcAlarm[i].lock == NULL);
		gXadcAlarm[i].lock = malloc(sizeof(pthread_mutex_t));
		if (pthread_mutex_init(gXadcAlarm[i].lock, NULL) != 0)
		{
			perror("mutex init failed\n");
			ret = -1;
			/*
			 * unroll the mutext initializaton
			 */
			free(gXadcAlarm[i].lock);
			deinit_mutexes(i);
			break;
		}
	}
	return ret;
}

int xadc_core_init(enum XADC_Init_Type init_type)
{
	int ret = 0;
	gExitNow = false;	//just reassuring

	assert(gInit_state == EXADC_NOT_INITIALIZED);	// Make sure it is only called once.
	assert(init_type >= 0 && init_type < EXADC_NOT_INITIALIZED);

	if (get_iio_node(DEVICE_NAME) != RET_SUCCESS)
	{
		perror(DEVICE_NAME " Device Not Found");
		ret = -1;
		goto EXIT;
	}

	if(init_type == EXADC_INIT_FULL)
	{
		if(init_mutexes() != 0)
		{
			ret = -1;
			goto EXIT;
		}

		if (pthread_create(&gEvent_handler, NULL, thread_event_handler, NULL) != 0)
		{
			perror("Couldn't spwan the event handler thread\n");
			ret = -1;
			deinit_mutexes(EAlarmMAX);
			goto EXIT;
		}
	}

	gInit_state = init_type;

EXIT:
	return ret;
}


int xadc_core_deinit(void)
{
	assert(gInit_state != EXADC_NOT_INITIALIZED);

	gExitNow = true;
	if (gInit_state == EXADC_INIT_FULL)
	{
		pthread_join(gEvent_handler, NULL);
		deinit_mutexes(EAlarmMAX);
	}

	gInit_state = EXADC_NOT_INITIALIZED;
	return 0;
}

void xadc_update_stat(void)		// update the cache
{
	int i = 0;

	assert(gInit_state != EXADC_NOT_INITIALIZED);

	for(i = 0; i < EParamMax; i++)
	{
		if(read_xadc_param(gXadcData+i) != RET_SUCCESS)
		{
			perror("Error Updating the statistic \n");
		}
	}
}

float xadc_touch(enum XADC_Param parameter)
{
	assert(gInit_state != EXADC_NOT_INITIALIZED);

	assert((parameter >= 0) && (parameter < EParamMax));
	if(read_xadc_param(gXadcData + parameter) != RET_SUCCESS)
	{
		perror("Error Updating the statistic \n");
		return 0;
	}
	return gXadcData[parameter].value;
}

float xadc_get_value(enum XADC_Param parameter)	// read the parameter from the cache.
{
	assert(gInit_state != EXADC_NOT_INITIALIZED);
	assert((parameter >= 0) && (parameter < EParamMax));
	return gXadcData[parameter].value;
}

int xadc_set_threshold(enum XADC_Alarm alarm,float threshold_low, float threshold_high, struct Xadc_callback *cb_ptr)
{
	int ret =0;
	assert(gInit_state == EXADC_INIT_FULL);
	assert((alarm >= 0) && (alarm < EAlarmMAX));

	pthread_mutex_lock(gXadcAlarm[alarm].lock);
	gXadcAlarm[alarm].thres_low =threshold_low;
	gXadcAlarm[alarm].thres_high =threshold_high;

	gXadcAlarm[alarm].callback.func = (cb_ptr)? cb_ptr->func : NULL;
	gXadcAlarm[alarm].callback.arg = (cb_ptr)? cb_ptr->arg : NULL;

	write_xadc_threshold(gXadcAlarm + alarm);
	pthread_mutex_unlock(gXadcAlarm[alarm].lock);
	return ret;
}

bool xadc_get_alarm_status(enum XADC_Alarm alarm)
{
	assert(gInit_state == EXADC_INIT_FULL);
	assert((alarm >= 0) && (alarm < EAlarmMAX));
	return gXadcAlarm[alarm].isActive;
}

//utility functions
float conv_voltage(float input, enum EConvType conv_direction)
{
	float result=0;

	switch(conv_direction)
	{
	case EConvType_Raw_to_Scale:
		result = ((input * 3.0 * mV_mul)/multiplier);
		break;
	case EConvType_Scale_to_Raw:
		result = (input/(3.0 * mV_mul))*multiplier;
		break;
	default:
		printf("Convertion type incorrect... Doing no conversion\n");
		//  intentional no break;
	case EConvType_None:
			result = input;
			break;
	}

	return result;
}

float conv_voltage_ext_ch(float input, enum EConvType conv_direction)
{
	float result=0;

	switch(conv_direction)
	{
	case EConvType_Raw_to_Scale:
		result = ((input * mV_mul)/multiplier);
		break;
	case EConvType_Scale_to_Raw:
		result = (input/mV_mul)*multiplier;
		break;
	default:
		printf("Convertion type incorrect... Doing no conversion\n");
		//  intentional no break;
	case EConvType_None:
			result = input;
			break;
	}

	return result;
}

float conv_temperature(float input, enum EConvType conv_direction)
{
	float result=0;

	switch(conv_direction)
	{
	case EConvType_Raw_to_Scale:
		result = ((input * 503.975)/multiplier) - 273.15;
		break;
	case EConvType_Scale_to_Raw:
		result = (input + 273.15)*multiplier/503.975;
		break;
	default:
		printf("Conversion type incorrect... Doing no conversion\n");
		//  intentional no break;
	case EConvType_None:
			result = input;
			break;
	}

	return result;
}

static int line_from_file(char* filename, char* linebuf)
{
    char* s;
    int i;
    FILE* fp = fopen(filename, "r");
    if (!fp) return RET_CANNOT_OPEN_FILE;
    s = fgets(linebuf, MAX_VALUE_SIZE, fp);
    fclose(fp);
    if (!s) return RET_FILE_READ_FAILED;

    for (i=0; (*s)&&(i<MAX_VALUE_SIZE); i++) {
        if (*s == '\n') *s = 0;
        s++;
    }
    return RET_SUCCESS;
}

static int line_to_file(char* filename, char* linebuf)
{
    int fd;
    fd = open(filename, O_WRONLY);
    if (fd < 0) return RET_CANNOT_OPEN_FILE;
    write(fd, linebuf, strlen(linebuf));
    close(fd);
    return RET_SUCCESS;
}

