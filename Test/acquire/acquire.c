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
#include <getopt.h>
#include <string.h>
#include <sys/param.h>

#include "main_osc.h"
#include "fpga_osc.h"
#include "redpitaya/version.h"

/**
 * GENERAL DESCRIPTION:
 *
 * The code below acquires up to 16k samples on both Red Pitaya input
 * channels labeled ADC1 & ADC2.
 * 
 * It utilizes the routines of the Oscilloscope module for:
 *   - Triggered ADC signal Acquisition to the FPGA buffers.
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

/** Minimal number of command line arguments */
#define MINARGS 2

/** Oscilloscope module parameters as defined in main module
 * @see rp_main_params
 */
float t_params[PARAMS_NUM] = { 0, 1e6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

/** Max decimation index */
#define DEC_MAX 6

/** Decimation translation table */
static int g_dec[DEC_MAX] = { 1,  8,  64,  1024,  8192,  65536 };


/** Print usage information */
void usage() {

    const char *format =
            "\n"
            "Usage: %s [OPTION]... SIZE <DEC>\n"
            "\n"
            "  --equalization  -e    Use equalization filter in FPGA (default: disabled).\n"
            "  --shaping       -s    Use shaping filter in FPGA (default: disabled).\n"
            "  --gain1=g       -1 g  Use Channel 1 gain setting g [lv, hv] (default: lv).\n"
            "  --gain2=g       -2 g  Use Channel 2 gain setting g [lv, hv] (default: lv).\n"
            "  --version       -v    Print version info.\n"
            "  --help          -h    Print this message.\n"
            "\n"
            "    SIZE                Number of samples to acquire [0 - %u].\n"
            "    DEC                 Decimation [%u,%u,%u,%u,%u,%u] (default: 1).\n"
            "\n";

    fprintf( stderr, format, g_argv0, SIGNAL_LENGTH,
             g_dec[0],
             g_dec[1],
             g_dec[2],
             g_dec[3],
             g_dec[4],
             g_dec[5]);
}

/** Gain string (lv/hv) to number (0/1) transformation */
int get_gain(int *gain, const char *str)
{
    if ( (strncmp(str, "lv", 2) == 0) || (strncmp(str, "LV", 2) == 0) ) {
        *gain = 0;
        return 0;
    }
    if ( (strncmp(str, "hv", 2) == 0) || (strncmp(str, "HV", 2) == 0) ) {
        *gain = 1;
        return 0;
    }

    fprintf(stderr, "Unknown gain: %s\n", str);
    return -1;
}


/** Acquire utility main */
int main(int argc, char *argv[])
{
    g_argv0 = argv[0];
    int equal = 0;
    int shaping = 0;

    if ( argc < MINARGS ) {
        usage();
        exit ( EXIT_FAILURE );
    }

    /* Command line options */
    static struct option long_options[] = {
            /* These options set a flag. */
            {"equalization", no_argument,       0, 'e'},
            {"shaping",      no_argument,       0, 's'},
            {"gain1",        required_argument, 0, '1'},
            {"gain2",        required_argument, 0, '2'},
            {"version",      no_argument,       0, 'v'},
            {"help",         no_argument,       0, 'h'},
            {0, 0, 0, 0}
    };
    const char *optstring = "es1:2:vh";

    /* getopt_long stores the option index here. */
    int option_index = 0;

    int ch = -1;
    while ( (ch = getopt_long( argc, argv, optstring, long_options, &option_index )) != -1 ) {
        switch ( ch ) {

        case 'e':
            equal = 1;
            break;

        case 's':
            shaping = 1;
            break;

        /* Gain Channel 1 */
        case '1':
        {
            int gain1;
            if (get_gain(&gain1, optarg) != 0) {
                usage();
                return -1;
            }
            t_params[GAIN1_PARAM] = gain1;
        }
        break;

        /* Gain Channel 2 */
        case '2':
        {
            int gain2;
            if (get_gain(&gain2, optarg) != 0) {
                usage();
                return -1;
            }
            t_params[GAIN2_PARAM] = gain2;
        }
        break;

        case 'v':
            fprintf(stdout, "%s version %s-%s\n", g_argv0, VERSION_STR, REVISION_STR);
            exit( EXIT_SUCCESS );
            break;

        case 'h':
            usage();
            exit( EXIT_SUCCESS );
            break;

        default:
            usage();
            exit( EXIT_FAILURE );
        }
    }

    /* Acquisition size */
    uint32_t size = 0;
    if (optind < argc) {
        size = atoi(argv[optind]);
        if (size > SIGNAL_LENGTH) {
            fprintf(stderr, "Invalid SIZE: %s\n", argv[optind]);
            usage();
            exit( EXIT_FAILURE );
        }
    } else {
        fprintf(stderr, "SIZE parameter missing\n");
        usage();
        exit( EXIT_FAILURE );
    }
    optind++;

    /* Optional decimation */
    if (optind < argc) {
        uint32_t dec = atoi(argv[optind]);
        uint32_t idx;

        for (idx = 0; idx < DEC_MAX; idx++) {
            if (dec == g_dec[idx]) {
                break;
            }
        }

        if (idx != DEC_MAX) {
            t_params[TIME_RANGE_PARAM] = idx;
        } else {
            fprintf(stderr, "Invalid decimation DEC: %s\n", argv[optind]);
            usage();
            return -1;
        }
    }

    /* Filter parameters */
    t_params[EQUAL_FILT_PARAM] = equal;
    t_params[SHAPE_FILT_PARAM] = shaping;


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
