/**
 * $Id: $
 *
 * @brief Red Pitaya Applications library common module interface
 *
 * @Author Luka Golinar, RedPitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
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
    // if (frequency <= 10){
    //     *api_dec = RP_DEC_4096;
    //     *dec_val = 4096;
    //     return;
    // }

    // if (frequency <= 100){
    //     *api_dec = RP_DEC_1024;
    //     *dec_val = 1024;
    //     return;
    // }

    // if (frequency <= 1000){
    //     *api_dec = RP_DEC_128;
    //     *dec_val = 128;
    //     return;
    // }

    // if (frequency <= 10000){
    //     *api_dec = RP_DEC_4;
    //     *dec_val = 4;
    //     return;
    // }

    // *api_dec = RP_DEC_1;
    // *dec_val = 1;

    // if (adc_rate <= 125e6){
    //     switch((int)frequency) {
    //         case 1000000:
    //             *api_dec = RP_DEC_1;
    //             *dec_val = 1;
    //             break;
    //         case 100000:
    //             *api_dec = RP_DEC_1;
    //             *dec_val = 1;
    //             break;
    //         case 10000:
    //             *api_dec = RP_DEC_4;
    //             *dec_val = 4;
    //             break;
    //         case 1000:
    //             *api_dec = RP_DEC_128;
    //             *dec_val = 128;
    //             break;
    //         case 100:
    //             *api_dec = RP_DEC_1024;
    //             *dec_val = 1024;
    //             break;
    //         case 10:
    //             *api_dec = RP_DEC_4096;
    //             *dec_val = 4096;
    //             break;
    //         default:
    //             *api_dec = RP_DEC_1;
    //             *dec_val = 1;
    //             break;
    //     }
    //     return;
    // }

    // if (adc_rate == 250e6){
    //     switch((int)frequency) {
    //         case 1000000:
    //             *api_dec = RP_DEC_1;
    //             *dec_val = 1;
    //             break;
    //         case 100000:
    //             *api_dec = RP_DEC_4;
    //             *dec_val = 4;
    //             break;
    //         case 10000:
    //             *api_dec = RP_DEC_16;
    //             *dec_val = 16;
    //             break;
    //         case 1000:
    //             *api_dec = RP_DEC_128;
    //             *dec_val = 128;
    //             break;
    //         case 100:
    //             *api_dec = RP_DEC_2048;
    //             *dec_val = 2048;
    //             break;
    //         case 10:
    //             *api_dec = RP_DEC_8192;
    //             *dec_val = 8192;
    //             break;
    //         default:
    //             *api_dec = RP_DEC_1;
    //             *dec_val = 1;
    //             break;
    //         }
    //     return;
    // }

    // *api_dec = RP_DEC_1;
    // *dec_val = 1;
    // fprintf(stderr,"[Fatal error:lcr_getDecimationValue] undefined adc rate %d\n",adc_rate);
}
