#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"


/** Maximal signal frequency [Hz] */
const double c_max_frequency = SAMPLE_RATE/2.0;

/** Minimal signal frequency [Hz] */
const double c_min_frequency = 0;

/** Maximal signal amplitude [Vpp] */
const double c_max_amplitude = 2.0;

/** Program name */
const char *g_argv0 = NULL;



#ifndef Z20_250_12

void usage() {

    const char *format =
        "%s version %s-%s\n"
        "\n"
        "Usage: %s   channel amplitude frequency <type> <end frequency> <calib>\n"
        "\n"
        "\tchannel         Channel to generate signal on [1, 2].\n"
        "\tamplitude       Peak-to-peak signal amplitude in Vpp [0.0 - %1.1f].\n"
        "\tfrequency       Signal frequency in Hz [%2.2f - %2.1e].\n"
        "\ttype            Signal type [sine, sqr, tri, sweep].\n"
        "\tend frequency   Sweep-to frequency in Hz [%2.2f - %2.1e].\n"
        "\tcalib           Disable calibration [-c]. By default calibration enabled.\n"
        "\n";

    fprintf( stderr, format, g_argv0, VERSION_STR, REVISION_STR,
             g_argv0, c_max_amplitude, c_min_frequency, c_max_frequency);
}

#else

void usage() {

    const char *format =
        "%s version %s-%s\n"
        "\n"
        "Usage: %s   channel amplitude frequency <gain> <type> <end frequency> <calib>\n"
        "\n"
        "\tchannel         Channel to generate signal on [1, 2].\n"
        "\tamplitude       Peak-to-peak signal amplitude in Vpp [0.0 - %1.1f].\n"
        "\tfrequency       Signal frequency in Hz [%2.2f - %2.1e].\n"
        "\tgain            Gain output value [x1, x5] (default value x1).\n"
        "\ttype            Signal type [sine, sqr, tri, sweep].\n"
        "\tend frequency   Sweep-to frequency in Hz [%2.2f - %2.1e].\n"
        "\tcalib           Disable calibration [-c]. By default calibration enabled.\n"
        "\n";

    fprintf( stderr, format, g_argv0, VERSION_STR, REVISION_STR,
             g_argv0, c_max_amplitude, c_min_frequency, c_max_frequency,c_min_frequency, c_max_frequency);
}

#endif


/** Signal generator main */
int main(int argc, char *argv[])
{
    g_argv0 = argv[0];    
    config_t config; 

    if ( argc < 4 ) {

        usage();
        return -1;
    }

    /* Channel argument parsing */
    config.ch = atoi(argv[1]) - 1; /* Zero based internally */
    if (config.ch > 1) {
        fprintf(stderr, "Invalid channel: %s\n", argv[1]);
        usage();
        return -1;
    }


    /* Signal amplitude argument parsing */
    config.amp = strtod(argv[2], NULL);
    if ( (config.amp < 0.0) || (config.amp > c_max_amplitude) ) {
        fprintf(stderr, "Invalid amplitude: %s\n", argv[2]);
        usage();
        return -1;
    }

    /* Signal frequency argument parsing */
    config.freq = strtod(argv[3], NULL);
    config.end_freq = 0;
    config.calib = true;

    if (argc > (5 + PARAMETER_CORRECT)) {
        config.end_freq = strtod(argv[5 + PARAMETER_CORRECT], NULL);
    }

    if (argc > (6 + PARAMETER_CORRECT)) {
        if ( strcmp(argv[6 + PARAMETER_CORRECT], "-c") == 0) {
           config.calib = false;   
        } else {
            fprintf(stderr, "Invalid parameter: %s\n", argv[4 + PARAMETER_CORRECT]);
            usage();
            return -1;
        }
    }

    /* Signal type argument parsing */
    config.type = eSignalSine;
    if (argc > (4 + PARAMETER_CORRECT)) {
        if ( strcmp(argv[4 + PARAMETER_CORRECT], "sine") == 0) {
            config.type = eSignalSine;
        } else if ( strcmp(argv[4 + PARAMETER_CORRECT], "sqr") == 0) {
            config.type = eSignalSquare;
        } else if ( strcmp(argv[4 + PARAMETER_CORRECT], "tri") == 0) {
            config.type = eSignalTriangle;
        } else if ( strcmp(argv[4 + PARAMETER_CORRECT], "sweep") == 0) {
            config.type = eSignalSweep;   
        } else {
            fprintf(stderr, "Invalid signal type: %s\n", argv[4 + PARAMETER_CORRECT]);
            usage();
            return -1;
        }
    }

#ifdef Z20_250_12
    /* Gain argument parsing */
    config.gain = v_2;
    if (argc > 5) {
        if ( strcmp(argv[4], "x1") == 0) {
            config.gain = v_2;
        } else if ( strcmp(argv[4], "x5") == 0) {
            config.gain = v_10;
        } else {
            fprintf(stderr, "Invalid gain type: %s\n", argv[4]);
            usage();
            return -1;
        }
    }
#endif

    /* Check frequency limits */
    if ( (config.freq < c_min_frequency) || (config.freq > c_max_frequency ) ) {
        fprintf(stderr, "Invalid frequency: %s\n", argv[3]);
        usage();
        return -1;
    }

    return gen(config);
}