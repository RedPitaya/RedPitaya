/**
 * $Id: $
 *
 * @brief Red Pitaya Applications library common module interface
 *
 * @Author Luka Golinar, RedPitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <string.h>
#include <unistd.h>
#include <vector>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syslog.h>
#include <math.h>

#include "utils.h"

void lcr_getDecimationValue(float frequency,
                            int *dec_val,
                            uint32_t adc_rate){

    auto decimation = adc_rate / (frequency * 2048);

     if (decimation < 16){
        if (decimation >= 8)
            decimation = 8;
        else
            if (decimation >= 4)
                decimation = 4;
            else
                if (decimation >= 2)
                    decimation = 2;
                else
                    decimation = 1;
    }
    if (decimation > 65536){
        decimation = 65536;
    }

    *dec_val = decimation;
}
