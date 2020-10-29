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
#include <iostream>

#include "main_osc.h"
#include "fpga_osc.h"
#include "redpitaya/version.h"


#ifdef Z20_250_12
#include "acquire_250.h"
#include "rp-i2c-max7311.h"
#include "rp-i2c-mcp47x6-c.h"
#include "rp-gpio-power.h"
#include "rp-spi.h"
#endif

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
#define DEC_MAX 5


/** Decimation translation table */
static uint32_t g_dec[DEC_MAX] = { 1,  2,  4,  8,  16 };



int get_trigger(int *value, int *edge, const char *str)
{
    if  (strncmp(str, "1P", 2) == 0) {
        *value = 0;
        *edge  = 0;
        return 0;
    }
    if  (strncmp(str, "1N", 1) == 0)  {
        *value = 0;
        *edge  = 1;
        return 0;
    }

    if  (strncmp(str, "2P", 2) == 0) {
        *value = 1;
        *edge  = 0;
        return 0;
    }
    if  (strncmp(str, "2N", 1) == 0)  {
        *value = 1;
        *edge  = 1;
        return 0;
    }
#ifdef Z20_250_12
     if (strncmp(str, "EP", 2) == 0) {
        *value = 2;
        *edge  = 0;
        return 0;
    }
    if  (strncmp(str, "EN", 1) == 0)  {
        *value = 2;
        *edge  = 1;
        return 0;
    }
#endif
    fprintf(stderr, "Unknown trigger value: %s\n", str);
    return -1;
}

int get_trigger_level(float *value, const char *str)
{
    if (sscanf (str,"%f",value) != 1){
        fprintf(stderr, "Unknown trigger value: %s\n", str);
        return -1;
    }
    return 0;
}
#ifndef Z20_250_12
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
#endif

/** Print usage information */
void usage() {

#ifdef Z20_250_12
    const char *format =
            "\n"
            "Usage: %s [OPTION]... SIZE <DEC>\n"
            "\n"
            "  --equalization  -e    Use equalization filter in FPGA (default: disabled).\n"
            "  --shaping       -s    Use shaping filter in FPGA (default: disabled).\n"
            "  --atten1=a      -1 a  Use Channel 1 attenuator setting a [1, 20] (default: 1).\n"
            "  --atten2=a      -2 a  Use Channel 2 attenuator setting a [1, 20] (default: 1).\n"
            "  --dc=c          -d c  Enable DC mode. Setting c use for channels [1, 2, B(Both channels)].\n"
            "                        By default, AC mode is turned on.\n"
            "  --tr_ch=c       -t c  Enable trigger by channel. Setting c use for channels [1P, 1N, 2P, 2N, EP (external channel), EN (external channel)].\n"
            "                        P - positive edge, N -negative edge. By default trigger no set\n"
            "  --tr_level=c    -l c  Set trigger level (default: 0).\n"
            "  --version       -v    Print version info.\n"
            "  --help          -h    Print this message.\n"
            "  --hex           -x    Print value in hex.\n"
            "  --volt          -o    Print value in volt.\n"
            "  --no_reg        -r    Disable load registers config for DAC and ADC.\n"
            "  --calib         -c    Disable calibration parameters\n"
            "    SIZE                Number of samples to acquire [0 - %u].\n"
            "    DEC                 Decimation [%u,%u,%u,%u,%u,...] (default: 1). Valid values are from 1 to 65536\n"
            "\n";

    fprintf( stderr, format, g_argv0, SIGNAL_LENGTH,
             g_dec[0],
             g_dec[1],
             g_dec[2],
             g_dec[3],
             g_dec[4]);
#else
    const char *format =
            "\n"
            "Usage: %s [OPTION]... SIZE <DEC>\n"
            "\n"
            "  --equalization  -e    Use equalization filter in FPGA (default: disabled).\n"
            "  --shaping       -s    Use shaping filter in FPGA (default: disabled).\n"
            "  --gain1=g       -1 g  Use Channel 1 gain setting g [lv, hv] (default: lv).\n"
            "  --gain2=g       -2 g  Use Channel 2 gain setting g [lv, hv] (default: lv).\n"
            "  --tr_ch=c       -t c  Enable trigger by channel. Setting c use for channels [1P, 1N, 2P, 2N].\n"
            "                        P - positive edge, N -negative edge. By default trigger no set\n"
            "  --tr_level=c    -l c  Set trigger level (default: 0).\n"
            "  --version       -v    Print version info.\n"
            "  --help          -h    Print this message.\n"
            "  --hex           -x    Print value in hex.\n"
            "  --volt          -o    Print value in volt.\n"
            "  --calib         -c    Disable calibration parameters\n"
            "    SIZE                Number of samples to acquire [0 - %u].\n"
            "    DEC                 Decimation [%u,%u,%u,%u,%u,...] (default: 1). Valid values are from 1 to 65536\n"
            "\n";

    fprintf( stderr, format, g_argv0, SIGNAL_LENGTH,
             g_dec[0],
             g_dec[1],
             g_dec[2],
             g_dec[3],
             g_dec[4]);
#endif
}



/** Acquire utility main */
int main(int argc, char *argv[])
{
    g_argv0 = argv[0];
    int equal = 0;
    int shaping = 0;
    int hex_mode = 0;
    int use_calib = 1;
    bool cnt_to_vol = false;
    float level_trigger = 0;
    if ( argc < MINARGS ) {
        usage();
        exit ( EXIT_FAILURE );
    }

#ifdef Z20_250_12
    int disabled_load_config = 0;

    /* Command line options */
    static struct option long_options[] = {
            /* These options set a flag. */
            {"equalization", no_argument,       0, 'e'},
            {"shaping",      no_argument,       0, 's'},
            {"atten1",       required_argument, 0, '1'},
            {"atten2",       required_argument, 0, '2'},
            {"dc",           required_argument, 0, 'd'},
            {"tr_ch",        required_argument, 0, 't'},
            {"tr_level",     required_argument, 0, 'l'},
            {"version",      no_argument,       0, 'v'},
            {"help",         no_argument,       0, 'h'},
            {"hex",          no_argument,       0, 'x'},
            {"volt",         no_argument,       0, 'o'},
            {"no_reg",       no_argument,       0, 'r'},
            {"calib",        no_argument,       0, 'c'},
            {0, 0, 0, 0}
    };
#else
    static struct option long_options[] = {
            /* These options set a flag. */
            {"equalization", no_argument,       0, 'e'},
            {"shaping",      no_argument,       0, 's'},
            {"gain1",        required_argument, 0, '1'},
            {"gain2",        required_argument, 0, '2'},
            {"tr_ch",        required_argument, 0, 't'},
            {"tr_level",     required_argument, 0, 'l'},
            {"version",      no_argument,       0, 'v'},
            {"help",         no_argument,       0, 'h'},
            {"hex",          no_argument,       0, 'x'},
            {"volt",         no_argument,       0, 'o'},
            {"calib",        no_argument,       0, 'c'},
            {0, 0, 0, 0}
    };
#endif

#ifdef Z20_250_12
    int atten1 = RP_ATTENUATOR_1_1;
    int atten2 = RP_ATTENUATOR_1_1;
    int dc_mode1 = RP_AC_MODE;
    int dc_mode2 = RP_AC_MODE;
    //rp_max7311::rp_initController();
    rp_max7311::rp_setAttenuator(RP_MAX7311_IN1,RP_ATTENUATOR_1_1);
    rp_max7311::rp_setAttenuator(RP_MAX7311_IN2,RP_ATTENUATOR_1_1);
    rp_max7311::rp_setAC_DC(RP_MAX7311_IN1,RP_AC_MODE);
    rp_max7311::rp_setAC_DC(RP_MAX7311_IN2,RP_AC_MODE);
    usleep(1000);
    const char *optstring = "esx1:2:d:vht:l:orc";
#else
    const char *optstring = "esx1:2:vht:l:oc";
#endif
    
    /* getopt_long stores the option index here. */
    int option_index = 0;

    int ch = -1;
    while ( (ch = getopt_long( argc, argv, optstring, long_options, &option_index )) != -1 ) {
        switch ( ch ) {

        case 'e':
            equal = 1;
            break;

        case 'c':
            use_calib = 0;
            break;

        case 's':
            shaping = 1;
            break;

        case 'x':
            hex_mode = 1;
            break;

        case 'o':
            cnt_to_vol = 1;
            break;
#ifdef Z20_250_12        
        case 'r':
            disabled_load_config = 1;
            break;
#endif
        /* Trigger */
        case 't':
        {
            int trigger = 1;
            int edge = -1;
            if (get_trigger(&trigger, &edge, optarg) != 0) {
                usage();
                return -1;
            }
            t_params[TRIG_MODE_PARAM] = 1;
            t_params[TRIG_SRC_PARAM] = trigger;
            t_params[TRIG_EDGE_PARAM] = edge;
        }
        break;

         /* Trigger level */
        case 'l':
        {
            
            if (get_trigger_level(&level_trigger, optarg) != 0) {
                usage();
                return -1;
            }
            t_params[TRIG_LEVEL_PARAM] = level_trigger;
        }
        break;

        /* Attenuator Channel 1 */
        case '1':
        {
#ifdef Z20_250_12
            int attenuator;
            if (get_attenuator(&attenuator, optarg) != 0) {
                usage();
                return -1;
            }
            if (attenuator == 1) {
                rp_max7311::rp_setAttenuator(RP_MAX7311_IN1,RP_ATTENUATOR_1_1);
                atten1 = RP_ATTENUATOR_1_1;
            }
            if (attenuator == 20) {
                rp_max7311::rp_setAttenuator(RP_MAX7311_IN1,RP_ATTENUATOR_1_20);
                atten1 = RP_ATTENUATOR_1_20;
            }
#else
            int gain1;
            if (get_gain(&gain1, optarg) != 0) {
                usage();
                return -1;
            }
            t_params[GAIN1_PARAM] = gain1;
#endif
        }
        break;

        /* Attenuator Channel 2 */
        case '2':
        {
#ifdef Z20_250_12
            int attenuator;
            if (get_attenuator(&attenuator, optarg) != 0) {
                usage();
                return -1;
            }
            if (attenuator == 1) {
                rp_max7311::rp_setAttenuator(RP_MAX7311_IN2,RP_ATTENUATOR_1_1);
                atten2 = RP_ATTENUATOR_1_1;
            }
            if (attenuator == 20) {
                rp_max7311::rp_setAttenuator(RP_MAX7311_IN2,RP_ATTENUATOR_1_20);
                atten2 = RP_ATTENUATOR_1_20;
            }
#else
            int gain2;
            if (get_gain(&gain2, optarg) != 0) {
                usage();
                return -1;
            }
            t_params[GAIN2_PARAM] = gain2;
#endif
        }
        break;

#ifdef Z20_250_12
        /* DC mode */
        case 'd':
        {
            int dc_mode;
            if (get_dc_mode(&dc_mode, optarg) != 0) {
                usage();
                return -1;
            }
            if (dc_mode == 1 || dc_mode == 3) {
                rp_max7311::rp_setAC_DC(RP_MAX7311_IN1,RP_DC_MODE);
                dc_mode1 = RP_DC_MODE;
            }
            if (dc_mode == 2 || dc_mode == 3) {
                rp_max7311::rp_setAC_DC(RP_MAX7311_IN2,RP_DC_MODE);
                dc_mode2 = RP_DC_MODE;
            }
        }
        break;
#endif

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

#ifdef Z20_250_12
    if (!disabled_load_config) {
        rp_spi_fpga::rp_spi_load_via_fpga("/opt/redpitaya/lib/configs/AD9613BCPZ-250.xml");
    }
#endif



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
        //uint32_t idx;

        // for (idx = 0; idx < DEC_MAX; idx++) {
        //     if (dec == g_dec[idx]) {
        //         break;
        //     }
        // }
        if (dec >= 1 && dec <= 65536) {
            t_params[TIME_RANGE_PARAM] = dec;
        } else {
            fprintf(stderr, "Invalid decimation DEC: %s\n", argv[optind]);
            usage();
            return -1;
        }
    }

    /* Filter parameters */
    t_params[EQUAL_FILT_PARAM] = equal;
    t_params[SHAPE_FILT_PARAM] = shaping;
#ifdef Z20_250_12
    if (t_params[TRIG_SRC_PARAM] == 2){
        rp_setExtTriggerLevel(level_trigger);
    }
#endif
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
        //std::cout << "STEP 1\n";
        while(retries >= 0) {
        //    std::cout << "STEP 2\n";
            if((ret_val = rp_get_signals(&s, &sig_num, &sig_len)) >= 0) {
        //        std::cout << "STEP 3\n";
                /* Signals acquired in s[][]:
                 * s[0][i] - TODO
                 * s[1][i] - Channel ADC1 raw signal
                 * s[2][i] - Channel ADC2 raw signal
                 */
                const char *format_str = (hex_mode == 0) ? "%7d %7d\n" : "0x%08X 0x%08X\n";

                for(i = 0; i < MIN((int)size, sig_len); i++) {
                    int d1 = (int)s[1][i];
                    int d2 = (int)s[2][i];
                    float gain1 = 1;
                    float gain2 = 1;
                    if (use_calib){
#ifdef Z20_250_12
                        d1 = osc_calibrate_value(d1,0,atten1,dc_mode1);
                        d2 = osc_calibrate_value(d2,1,atten2,dc_mode2);
                        if (atten1 == RP_ATTENUATOR_1_20) gain1 = 20;
                        if (atten2 == RP_ATTENUATOR_1_20) gain2 = 20;
#else
                        d1 = osc_calibrate_value(d1,0,t_params[GAIN1_PARAM]);
                        d2 = osc_calibrate_value(d2,1,t_params[GAIN2_PARAM]);
                        if (t_params[GAIN1_PARAM] == 1) gain1 = 20;
                        if (t_params[GAIN2_PARAM] == 1) gain2 = 20;
#endif
                    }
                    if (cnt_to_vol){
                        printf("%f\t%f\n", osc_fpga_cnv_cnt_to_v2(d1) * gain1,  osc_fpga_cnv_cnt_to_v2(d2) *gain2);
                    }
                    else{
                        printf(format_str, d1, d2);
                    }
                }
                break;
            }
        //    std::cout << "STEP 4\n";
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
