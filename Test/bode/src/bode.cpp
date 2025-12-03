/**
 * $Id: bode.c 1246  $
 *
 * @brief Red Pitaya Bode plotter
 *
 * @Author1 Martin Cimerman (main developer,concept program translation)
 * @Author2 Zumret Topcagic (concept code developer)
 * @Author3 Luka Golinar (functioanlity of web interface)
 * @Author4 Peter Miklavcic (manpage and code review)
 * Contact: <cim.martin@gmail.com>, <luka.golinar@gmail.com>
 *
 * GENERAL DESCRIPTION:
 *
 * The code below defines the Bode analyzer on a Red Pitaya.
 * It uses acquire and generate from the Test/ folder.
 * Data analysis returns frequency, phase and amplitude.
 *
 * VERSION: VERSION defined in Makefile
 */

#include <complex.h>
#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <unistd.h>

#include "bodeApp.h"
#include "common/version.h"
#include "rp.h"

const char* g_argv0 = NULL;  // Program name

double c_max_frequency = 0;          // Maximal signal frequency [Hz]
const double c_min_frequency = 0;    // Minimal signal frequency [Hz]
const double c_max_amplitude = 1.0;  // Maximal signal amplitude [V]

/** Print usage information */
void usage() {
    const char* format =
        "Bode analyzer version %s, compiled at %s\n"
        "\n"
        "Usage:\t%s [channel] "
        "[amplitude] "
        "[dc bias] "
        "[averaging] "
        "[count/steps] "
        "[start freq] "
        "[stop freq] "
        "[scale type] "
        "[probe]\n"
        "or\n"
        "\t%s -calib\n"
        "\n"
        "\tchannel            Channel to generate signal on [1 / 2].\n"
        "\tamplitude          Signal amplitude in V [0 - 1, which means max 2Vpp].\n"
        "\tdc bias            DC bias/offset/component in V [0 - 1].\n"
        "\t                   Max sum of amplitude and DC bias is (0-1]V.\n"
        "\taveraging          Number of samples per one measurement [>1].\n"
        "\tcount/steps        Number of measurements [>2].\n"
        "\tstart freq         Lower frequency limit in Hz [3 - 62.5e6].\n"
        "\tstop freq          Upper frequency limit in Hz [3 - 62.5e6].\n"
        "\tscale type         0 - linear, 1 - logarithmic. [0,1]\n"
        "\tprobe              Probe value [1-1000].\n"
        "\t-calib             Starts calibration mode. The calibration values will be saved in:" BA_CALIB_FILENAME
        "\n"
        "Output:\tfrequency [Hz], phase [deg], amplitude [dB]\n";

    fprintf(stderr, format, VERSION_STR, __TIMESTAMP__, g_argv0, g_argv0);
}

/** Bode analyzer */
int main(int argc, char* argv[]) {

    bool calibMode = false;
    unsigned int ch = 0;
    double ampl = 1;
    double DC_bias = 0;
    unsigned int averaging_num = 1;
    unsigned int steps = 500;
    double start_frequency = 100;
    double end_frequency = rpApp_BaGetADCSpeed() / 2.0;
    c_max_frequency = end_frequency;
    unsigned int scale_type = 1;
    int ignored __attribute__((unused));
    int probe = 1;

    /** Set program name */
    g_argv0 = argv[0];

    auto dac_channels = rpApp_BaGetDACChannels();
    if (dac_channels == 0) {
        fprintf(stderr, "There are no FAST DAC outputs on this board.\n");
        return 0;
    }

    auto adc_channels = rpApp_BaGetADCChannels();
    if (adc_channels < 2) {
        fprintf(stderr, "There are no FAST ADC inputs on this board.\n");
        return 0;
    }

    /**
     * Manpage
     *
     * usage() prints its output to stderr, nevertheless main returns
     * zero as calling lcr without any arguments is not an error.
     */
    if (argc == 2) {
        if (strncmp(argv[1], "-calib", 6) == 0) {
            calibMode = true;
        } else {
            usage();
            return 0;
        }
    }

    if (!calibMode) {
        /** Argument check */
        if (argc < 10) {
            fprintf(stderr, "Too few arguments!\n\n");
            usage();
            return -1;
        }

        /** Argument parsing */
        /// Channel
        ch = atoi(argv[1]) - 1;  // Zero-based internally
        if (ch > 1) {
            fprintf(stderr, "Invalid channel value!\n\n");
            usage();
            return -1;
        }
        /// Amplitude
        ampl = strtod(argv[2], NULL);
        if ((ampl < 0) || (ampl > c_max_amplitude)) {
            fprintf(stderr, "Invalid amplitude value!\n\n");
            usage();
            return -1;
        }
        /// DC bias
        DC_bias = strtod(argv[3], NULL);
        if ((DC_bias < 0) || (DC_bias > 1)) {
            fprintf(stderr, "Invalid dc bias value!\n\n");
            usage();
            return -1;
        }
        if (ampl + DC_bias > 1 || ampl + DC_bias <= 0) {
            fprintf(stderr, "Invalid ampl+dc value!\n\n");
            usage();
            return -1;
        }
        /// Averaging
        averaging_num = strtod(argv[4], NULL);
        if (averaging_num < 1) {
            fprintf(stderr, "Invalid averaging value!\n\n");
            usage();
            return -1;
        }
        /// Count/steps
        steps = strtod(argv[5], NULL);
        if (steps < 2) {
            fprintf(stderr, "Invalid count/steps value!\n\n");
            usage();
            return -1;
        }
        /// Frequency
        start_frequency = strtod(argv[6], NULL);
        if ((start_frequency < c_min_frequency) || (start_frequency > c_max_frequency)) {
            fprintf(stderr, "Invalid start freq!\n\n");
            usage();
            return -1;
        }
        end_frequency = strtod(argv[7], NULL);
        if ((end_frequency < c_min_frequency) || (end_frequency > c_max_frequency)) {
            fprintf(stderr, "Invalid end freq!\n\n");
            usage();
            return -1;
        }
        if (end_frequency < start_frequency) {
            fprintf(stderr, "End frequency has to be greater than the start frequency!\n\n");
            usage();
            return -1;
        }
        /// Scale type (0=lin, 1=log)
        scale_type = strtod(argv[8], NULL);
        if (scale_type > 1) {
            fprintf(stderr, "Invalid scale type!\n\n");
            usage();
            return -1;
        }

        probe = strtod(argv[9], NULL);
        if (probe < 1 || probe > 1000) {
            fprintf(stderr, "Invalid probe value!\n\n");
            usage();
            return -1;
        }
    }

    /** Parameters initialization and calculation */
    //double frequency_step;
    double a, b, c;
    uint32_t periods_number = 8;  // max 20

    rp_ba_buffer_t buffer(ADC_BUFFER_SIZE);

    /* We try to open a data file */
    FILE* try_open = fopen("/tmp/bode_data/data_frequency", "w");

    /* If the directory doesn't exist yet, we first have to create it. */
    if (try_open == NULL) {

        int b_number;
        char command[50];
        strcpy(command, "mkdir /tmp/bode_data");
        ignored = system(command);

        /* We must also create all the files for storing the data */
        for (b_number = 0; b_number < 3; b_number++) {

            switch (b_number) {
                case 0:
                    strcpy(command, "touch /tmp/bode_data/data_frequency");
                    break;
                case 1:
                    strcpy(command, "touch /tmp/bode_data/data_amplitude");
                    break;
                case 2:
                    strcpy(command, "touch /tmp/bode_data/data_phase");
                    break;
            }
            /* Execute the command */
            ignored = system(command);
        }
        /* Change permissions */
        strcpy(command, "chmod -R 777 /tmp/bode_data");
        ignored = system(command);
    }

    /* Opening files */
    FILE* file_frequency = fopen("/tmp/bode_data/data_frequency", "w");
    FILE* file_amplitude = fopen("/tmp/bode_data/data_amplitude", "w");
    FILE* file_phase = fopen("/tmp/bode_data/data_phase", "w");

    if (calibMode) {
        rpApp_BaResetCalibration();
    } else {
        rpApp_BaReadCalibration();
    }

    int cur_step = 0;
    float freq_step = 0;

    if (scale_type) {
        b = log10f(end_frequency);
        a = log10f(start_frequency);
        c = (b - a) / (steps - 1);
    } else {
        freq_step = (static_cast<float>(end_frequency) - static_cast<float>(start_frequency)) / (steps - 1);
    }

    // fprintf(stderr, "a %f b %f c %f\n", a, b, c);

    float old_freq = start_frequency;
    //float old_steps = steps;
    rp_Init();
    rpApp_BaInit();
    rpApp_BaSafeThreadAcqPrepare();
    printf("Frequency [Hz]         Amplitude [dB]        Phase [deg]\n");
    while (steps > 0) {

        float amplitude = 0, phase_out = 0;
        float current_freq = 0.;
        //float next_freq = 0.;

        if (scale_type) {
            // Log
            current_freq = pow(10.f, c * cur_step + a);
            //   next_freq = pow(10.f, c * (cur_step + 1) + a);
        } else {
            // Linear
            current_freq = static_cast<float>(old_freq) + freq_step * cur_step;
            //  next_freq = static_cast<float>(old_freq) + freq_step * (cur_step + 1);
        }

        for (unsigned int i = 0; i < averaging_num; ++i) {
            float ampl_out = 0;

            if (rpApp_BaGetAmplPhase(RP_BA_LOGIC_TRAP, ampl, DC_bias, periods_number, buffer, &ampl_out, &phase_out, current_freq, probe, 0) == RP_EOOR)  // isnan && isinf
            {
                --steps;
                continue;
            }
            amplitude += ampl_out;
        }
        amplitude /= averaging_num;

        --steps;
        cur_step++;
        float calib_ampl = rpApp_BaCalibGain(current_freq, amplitude);
        float calib_phase = rpApp_BaCalibPhase(current_freq, phase_out);
        fprintf(file_frequency, "%.2f\n", current_freq);
        fprintf(file_amplitude, "%.10f\n", calib_ampl);
        fprintf(file_phase, "%.10f\n", calib_phase);

        if (calibMode)  // save data in calibration mode
        {
            rpApp_BaWriteCalib(current_freq, amplitude, phase_out);
        }

        printf("%.2f    %.10f    %.10f\n", current_freq, calib_ampl, calib_phase);
    }

    rp_Release();
    rpApp_BaRelease();
    /* Closing files */
    fclose(file_frequency);
    fclose(file_phase);
    fclose(file_amplitude);

    return 1;
}
