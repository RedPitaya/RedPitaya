/**
 *
 * @brief Red Pitaya simple signal acquisition utility.
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <sys/param.h>
#include <iostream>

#include "rp.h"
#include "version.h"
#include "options.h"

#ifdef Z20_250_12
#include "acquire_250.h"
#include "rp-i2c-max7311.h"
#include "rp-i2c-mcp47x6-c.h"
#include "rp-gpio-power.h"
#include "rp-spi.h"
#endif

/** Program name */
const char *g_argv0 = NULL;


/** Acquire utility main */
int main(int argc, char *argv[])
{
    g_argv0 = argv[0];
    auto option = parse(argc,argv);

    if (option.error){
        usage(g_argv0);
        return -1;
    }

#ifdef Z20_250_12
    if (!option.disableReset) {
        rp_spi_fpga::rp_spi_load_via_fpga("/opt/redpitaya/lib/configs/AD9613BCPZ-250.xml");
    }
#endif






   

//     /* Filter parameters */
//     t_params[EQUAL_FILT_PARAM] = equal;
//     t_params[SHAPE_FILT_PARAM] = shaping;
// #ifdef Z20_250_12
//     if (t_params[TRIG_SRC_PARAM] == 2){
//         rp_setExtTriggerLevel(level_trigger);
//     }
// #endif
//     /* Initialization of Oscilloscope application */
//     if(rp_app_init() < 0) {
//         fprintf(stderr, "rp_app_init() failed!\n");
//         return -1;
//     }

//     /* Setting of parameters in Oscilloscope main module */
//     if(rp_set_params((float *)&t_params, PARAMS_NUM) < 0) {
//         fprintf(stderr, "rp_set_params() failed!\n");
//         return -1;
//     }

//     {
//         float **s;
//         int sig_num, sig_len;
//         int i;
//         int ret_val;

//         int retries = 150000;

//         s = (float **)malloc(SIGNALS_NUM * sizeof(float *));
//         for(i = 0; i < SIGNALS_NUM; i++) {
//             s[i] = (float *)malloc(SIGNAL_LENGTH * sizeof(float));
//         }
//         while(retries >= 0) {
//             if((ret_val = rp_get_signals(&s, &sig_num, &sig_len)) >= 0) {
//                 /* Signals acquired in s[][]:
//                  * s[0][i] - TODO
//                  * s[1][i] - Channel ADC1 raw signal
//                  * s[2][i] - Channel ADC2 raw signal
//                  */
//                 const char *format_str = (hex_mode == 0) ? "%7d %7d\n" : "0x%08X 0x%08X\n";

//                 for(i = 0; i < MIN((int)size, sig_len); i++) {
//                     int d1 = (int)s[1][i];
//                     int d2 = (int)s[2][i];
//                     float gain1 = 1;
//                     float gain2 = 1;
//                     if (use_calib){
// #ifdef Z20_250_12
//                         d1 = osc_calibrate_value(d1,0,atten1,dc_mode1);
//                         d2 = osc_calibrate_value(d2,1,atten2,dc_mode2);
//                         if (atten1 == RP_ATTENUATOR_1_20) gain1 = 20;
//                         if (atten2 == RP_ATTENUATOR_1_20) gain2 = 20;
// #else
//                         d1 = osc_calibrate_value(d1,0,t_params[GAIN1_PARAM]);
//                         d2 = osc_calibrate_value(d2,1,t_params[GAIN2_PARAM]);
//                         if (t_params[GAIN1_PARAM] == 1) gain1 = 20;
//                         if (t_params[GAIN2_PARAM] == 1) gain2 = 20;
// #endif
//                     }
//                     if (cnt_to_vol){
//                         printf("%f\t%f\n", osc_fpga_cnv_cnt_to_v2(d1) * gain1,  osc_fpga_cnv_cnt_to_v2(d2) *gain2);
//                     }
//                     else{
//                         printf(format_str, d1, d2);
//                     }
//                 }
//                 break;
//             }
//             if(retries-- == 0) {
//                 fprintf(stderr, "Signal scquisition was not triggered!\n");
//                 break;
//             }
//             usleep(1000);
//         }
//     }

//     if(rp_app_exit() < 0) {
//         fprintf(stderr, "rp_app_exit() failed!\n");
//         return -1;
//     }
    return 0;
}
