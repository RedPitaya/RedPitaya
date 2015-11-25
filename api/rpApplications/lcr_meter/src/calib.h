/**
* $Id: $
*
* @brief Red Pitaya calibration file
*
* @Author Luka Golinar <luka.golinar@gmail.com>
*
* (c) Red Pitaya  http://www.redpitaya.com
*
* This part of code is written in C programming language.
* Please visit http://en.wikipedia.org/wiki/C_(programming_language)
* for more details on the language used herein.
*/

#ifndef __CALIB_H
#define __CALIB_H

#include <stdlib.h>

#include "redpitaya/rp.h"
#include "lcr_meter.h"

#define APP_PATH	"/opt/redpitaya/www/apps/lcr_meter"

int store_calib(const calib_t CALIB_MODE,
				  const float *amplitude_z);

#endif //__CALIB_H
