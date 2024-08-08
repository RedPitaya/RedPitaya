#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "generate.h"


// /** Maximal signal frequency [Hz] */
uint32_t g_max_frequency = 0;

// /** Minimal signal frequency [Hz] */
uint32_t g_min_frequency = 0;

// /** Maximal signal amplitude [Vpp] */
double g_max_amplitude = 0;

/** Program name */
const char *g_argv0 = NULL;


const char format_125_14[] =
        "%s version %s-%s\n"
        "\n"
        "Usage: %s   channel amplitude frequency <type> <end frequency> <calib> <debug>\n"
        "\n"
        "\tchannel         Channel to generate signal on [1, 2].\n"
        "\tamplitude       Peak-to-peak signal amplitude in Vpp [0.0 - %1.1f].\n"
        "\tfrequency       Signal frequency in Hz [%d - %d].\n"
        "\ttype            Signal type [sine, sqr, tri, sweep, noise, dc%s].\n"
        "\tend frequency   Sweep-to frequency in Hz [%d - %d].\n"
        "\tcalib           Disable calibration [-c]. By default calibration enabled.\n"
        "\tdebug           Debug FPGA registers [-d].\n"
        "\n"
        "Setting the frequency to 0 will disable the generator completely.\n"
        "\n";

const char format_250_12[] =
        "%s version %s-%s\n"
        "\n"
        "Usage: %s   channel amplitude frequency <gain> <type> <end frequency> <calib> <debug>\n"
        "\n"
        "\tchannel         Channel to generate signal on [1, 2].\n"
        "\tamplitude       Peak-to-peak signal amplitude in Vpp [0.0 - %1.1f].\n"
        "\tfrequency       Signal frequency in Hz [%d - %d].\n"
        "\tgain            Gain output value [x1, x5] (default value x1).\n"
        "\ttype            Signal type [sine, sqr, tri, sweep, noise, dc%s].\n"
        "\tend frequency   Sweep-to frequency in Hz [%d - %d].\n"
        "\tcalib           Disable calibration [-c]. By default calibration enabled.\n"
        "\tdebug           Debug FPGA registers [-d].\n"
        "\n"
        "Setting the frequency to 0 will disable the generator completely.\n"
        "\n";


void usage_125_14() {
    auto list = getARBList();
    std::string s = "";
    for(auto &itm : list){
        s += ", ";
        s += itm;
    }

    fprintf( stderr, format_125_14, g_argv0, VERSION_STR, REVISION_STR,
             g_argv0, g_max_amplitude, g_min_frequency, g_max_frequency, s.c_str(), g_min_frequency, g_max_frequency);
}

void usage_250_12() {
    auto list = getARBList();
    std::string s = "";
    for(auto &itm : list){
        s += ", ";
        s += itm;
    }

    fprintf( stderr, format_250_12, g_argv0, VERSION_STR, REVISION_STR,
             g_argv0, g_max_amplitude, g_min_frequency, g_max_frequency, s.c_str(), g_min_frequency, g_max_frequency);
}

void usage(models_t model){
    if (model == RP_125_14)
        usage_125_14();
    if (model == RP_250_12)
        usage_250_12();
}



/** Signal generator main */
int main(int argc, char *argv[])
{
    loadARBList();
    auto channels = getChannels();
    auto model = getModel();
    if (channels == 0){
        fprintf(stderr,"There are no FAST DAC outputs on this board.\n");
        return 0;
    }

    g_argv0 = argv[0];
    config_t config;

    g_max_amplitude = fullScale() * 2;
    g_max_frequency = getMaxSpeed() / 2;

    if ( argc < 4 ) {
        usage(model);
        return -1;
    }

    /* Channel argument parsing */
    config.ch = atoi(argv[1]) - 1; /* Zero based internally */
    if (config.ch > 1) {
        fprintf(stderr, "Invalid channel: %s\n", argv[1]);
        usage(model);
        return -1;
    }

    /* Signal amplitude argument parsing */
    config.amp = strtod(argv[2], NULL);
    if ( (config.amp < 0.0) || (config.amp > g_max_amplitude) ) {
        fprintf(stderr, "Invalid amplitude: %s\n", argv[2]);
        usage(model);
        return -1;
    }

    /* Signal frequency argument parsing */
    config.freq = strtod(argv[3], NULL);
    config.end_freq = 0;
    config.calib = true;
    uint8_t PARAMETER_CORRECT = 0;
    if (model == RP_250_12) PARAMETER_CORRECT = 1;

    if (argc > (5 + PARAMETER_CORRECT)) {
        config.end_freq = strtod(argv[5 + PARAMETER_CORRECT], NULL);
    }

    if (argc > (6 + PARAMETER_CORRECT)) {
        if ( strcmp(argv[6 + PARAMETER_CORRECT], "-c") == 0) {
           config.calib = false;
        } else {
            fprintf(stderr, "Invalid parameter: %s\n", argv[4 + PARAMETER_CORRECT]);
            usage(model);
            return -1;
        }
    }

    for(int i = 1; i < argc; i++){
         if ( strcmp(argv[i], "-d") == 0) {
           config.regDebug = true;
        }
    }

     for(int i = 1; i < argc; i++){
         if ( strcmp(argv[i], "50Ohm") == 0) {
           config.load = RP_GEN_50Ohm;
        }
    }

    /* Signal type argument parsing */
    config.type = RP_WAVEFORM_SINE;
    if (argc > (4 + PARAMETER_CORRECT)) {
        if ( strcmp(argv[4 + PARAMETER_CORRECT], "sine") == 0) {
            config.type = RP_WAVEFORM_SINE;
        } else if ( strcmp(argv[4 + PARAMETER_CORRECT], "sqr") == 0) {
            config.type = RP_WAVEFORM_SQUARE;
        } else if ( strcmp(argv[4 + PARAMETER_CORRECT], "tri") == 0) {
            config.type = RP_WAVEFORM_TRIANGLE;
        } else if ( strcmp(argv[4 + PARAMETER_CORRECT], "dc") == 0) {
            config.type = RP_WAVEFORM_DC;
        } else if ( strcmp(argv[4 + PARAMETER_CORRECT], "noise") == 0) {
            config.type = RP_WAVEFORM_NOISE;
        } else if ( strcmp(argv[4 + PARAMETER_CORRECT], "sweep") == 0) {
            config.type = RP_WAVEFORM_SWEEP;
        } else {
            auto list = getARBList();
            std::string s = "";

            bool find = false;
            for(auto &itm : list){
                if ( strcmp(argv[4 + PARAMETER_CORRECT], itm.c_str()) == 0) {
                    find = true;
                    config.arb = itm;
                }
            }

            if (!find){
                fprintf(stderr, "Invalid signal type: %s\n", argv[4 + PARAMETER_CORRECT]);
                usage(model);
                return -1;
            }
        }
    }

    /* Gain argument parsing */
    config.gain = RP_GAIN_1X;
    if (rp_HPGetIsGainDACx5OrDefault()){
        if (argc > 5) {
            if ( strcmp(argv[4], "x1") == 0) {
                config.gain = RP_GAIN_1X;
            } else if ( strcmp(argv[4], "x5") == 0) {
                config.gain = RP_GAIN_5X;
            } else {
                fprintf(stderr, "Invalid gain type: %s\n", argv[4]);
                usage(model);
                return -1;
            }
        }
    }

    /* Check frequency limits */
    if ( (config.freq < g_min_frequency) || (config.freq > g_max_frequency ) ) {
        fprintf(stderr, "Invalid frequency: %s\n", argv[3]);
        usage(model);
        return -1;
    }

    return gen(config);
}