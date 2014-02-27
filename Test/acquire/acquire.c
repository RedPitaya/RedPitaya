/**
 * $Id: acquire.c 1246 2014-02-22 19:05:19Z ales.bardorfer $
 *
 * @brief Red Pitaya simple signal acquisition utility.
 *
 * @Author Ales Bardorfer <ales.bardorfer@redpitaya.com>
 *         Jure Menart <juremenart@gmail.com>
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
#include <sys/param.h>

#include "main_osc.h"
#include "fpga_osc.h"
#include "version.h"

/**
 * GENERAL DESCRIPTION:
 *
 * The code below acquires up to 16k samples on both Red Pitaya input
 * channels labeled ADC1 & ADC2.
 * 
 * It utilizes the routines of the Oscilloscope module for:
 *   - Triggered ADC signal acqusition to the FPGA buffers.
 *   - Parameter defined averaging & decimation.
 *   - Data transfer to SW buffers.
 *
 * Although the Oscilloscope routines export many functionalities, this 
 * simple signal acquisition utility only exploits a few of them:
 *   - Synchronization between triggering & data readout.
 *   - Only AUTO triggering is used by default.
 *   - Only decimation is parsed to t_params[8].
 *
 * Please feel free to exploit any other scope functionalities via 
 * parameters defined in t_params.
 *
 */

/** Program name */
const char *g_argv0 = NULL;

/** Oscilloclope module parameters as defined in main module
 * @see rp_main_params
 */
float t_params[PARAMS_NUM] = { 0, 1e6, 0, 0, 0, 0, 0, 0, 0, 0 };

/** Max decimation index */
#define DEC_MAX 6

/** Decimation translation table */
static int g_dec[DEC_MAX] = { 1,  8,  64,  1024,  8192,  65536 };


/** Print usage information */
void usage() {

    const char *format =
        "%s version %s-%s\n"
        "\n"
        "Usage: %s  size <dec>\n"
        "\n"
        "\tsize     Number of samples to acquire [0 - %u].\n"
        "\tdec      Decimation [%u,%u,%u,%u,%u,%u] (default=1).\n"
        "\n";

    fprintf( stderr, format, g_argv0, VERSION_STR, REVISION_STR,
             g_argv0, SIGNAL_LENGTH,
             g_dec[0],
             g_dec[1],
             g_dec[2],
             g_dec[3],
             g_dec[4],
             g_dec[5]);
}

/** Acquire utility main */
int main(int argc, char *argv[])
{
    g_argv0 = argv[0];

    if ( argc < 2 ) {

        usage();
        return -1;
    }

    /* Acquisition size */
    uint32_t size = atoi(argv[1]);
    if (size > SIGNAL_LENGTH) {
        fprintf(stderr, "Invalid size: %s\n", argv[1]);
        usage();
        return -1;
    }

    /* Optional acquisition decimation */
    if (argc > 2 ) {
        uint32_t dec = atoi(argv[2]);
        uint32_t idx;

        for (idx=0; idx< DEC_MAX; idx++) {
            if (dec == g_dec[idx]) {
                break;
            }
        }

        if (idx != DEC_MAX) {
            t_params[8] = idx;
        } else {
            fprintf(stderr, "Invalid decimation: %s\n", argv[2]);
            usage();
            return -1;
        }
    }

    /* Initialization of Oscilloscope application */
    if(rp_app_init() < 0) {
        fprintf(stderr, "rp_app_init() failed!\n");
        return -1;
    }

    /* Setting of parameters in Oscilloscope main module */
    if(rp_set_params((float *)&t_params, PARAMS_NUM) < 0) {
        fprintf(stderr, "rp_set_params() failed!\n");
        return -1;
    }

    {
        float **s;
        int sig_num, sig_len;
        int i;
        int ret_val;

        int retries = 150000;
	
	
	

        s = (float **)malloc(SIGNALS_NUM * sizeof(float *));
        for(i = 0; i < SIGNALS_NUM; i++) {
            s[i] = (float *)malloc(SIGNAL_LENGTH * sizeof(float));
        }

        while(retries >= 0) {
            if((ret_val = rp_get_signals(&s, &sig_num, &sig_len)) >= 0) {
                /* Signals acquired in s[][]:
                 * s[0][i] - TODO
                 * s[1][i] - Channel ADC1 raw signal
                 * s[2][i] - Channel ADC2 raw signal
                 */
		
		
		
		
		
		
                for(i = 0; i < MIN(size, sig_len); i++) {
                    printf("%7d %7d\n", (int)s[1][i], (int)s[2][i]);
                }
                break;
            }

            if(retries-- == 0) {
                fprintf(stderr, "Signal scquisition was not triggered!\n");
                break;
            }
            usleep(1000);
        }
    }

    if(rp_app_exit() < 0) {
        fprintf(stderr, "rp_app_exit() failed!\n");
        return -1;
    }

    return 0;
}
