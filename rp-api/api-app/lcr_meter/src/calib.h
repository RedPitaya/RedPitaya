/**
* $Id: $
*
* @brief Red Pitaya calibration file
*
* @Author Luka Golinar <luka.golinar@gmail.com>
*
* (c) Red Pitaya  http://www.redpitaya.com
*/

#ifndef __CALIB_H
#define __CALIB_H

#include <stdlib.h>

#include "rp.h"
#include "lcr_meter.h"
#include "utils.h"
#include "lcrApp.h"

int store_calib(const calib_t CALIB_MODE,
				float _Complex *amplitude_z);

#endif //__CALIB_H
