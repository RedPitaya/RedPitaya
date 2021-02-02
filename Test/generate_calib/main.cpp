#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "version.h"
#include "rp.h"

#ifdef Z20_250_12
#include "rp-spi.h"
#include "rp-i2c-max7311.h"
#include "rp-gpio-power.h"
#define SAMPLE_RATE 250e6
#define ARG_CORRECTION 0
#endif


#ifdef Z20
#define SAMPLE_RATE 122.880e6
#define ARG_CORRECTION 1
#endif

#if defined Z10 || defined Z20_125
#define SAMPLE_RATE 125e6
#define ARG_CORRECTION 0
#endif


/** Maximal signal frequency [Hz] */
const double c_max_frequency = SAMPLE_RATE/2.0;

/** Minimal signal frequency [Hz] */
const double c_min_frequency = 0;

/** Maximal signal amplitude [Vpp] */
const double c_max_amplitude = 2.0;

/** Program name */
const char *g_argv0 = NULL;

#ifdef Z20_250_12
/** Print usage information */
void usage() {

    const char *format =
        "%s version %s-%s\n"
        "\n"
        "Usage: %s   channel amplitude frequency <type> <gain> <end frequency>\n"
        "\n"
        "\tchannel     Channel to generate signal on [1, 2].\n"
        "\tamplitude   Peak-to-peak signal amplitude in Vpp [0.0 - %1.1f].\n"
        "\tfrequency   Signal frequency in Hz [%2.1f - %2.3e].\n"
        "\ttype        Signal type [sine, sqr, tri, saw_up, saw_down, dc].\n"
        "\tgain        Gain output value [x1, x5] (default value x1).\n"
        "\tend frequency   Sweep-to frequency in Hz [%2.1f - %2.3e].\n"
        "\n";

    fprintf( stderr, format, g_argv0, VERSION_STR, REVISION_STR,
             g_argv0, c_max_amplitude, c_min_frequency, c_max_frequency,c_min_frequency, c_max_frequency);
}
#else
/** Print usage information */
void usage() {

    const char *format =
        "%s version %s-%s\n"
        "\n"
        "Usage: %s   channel amplitude frequency <type> <end frequency>\n"
        "\n"
        "\tchannel     Channel to generate signal on [1, 2].\n"
        "\tamplitude   Peak-to-peak signal amplitude in Vpp [0.0 - %1.1f].\n"
        "\tfrequency   Signal frequency in Hz [%2.1f - %2.3e].\n"
        "\ttype        Signal type [sine, sqr, tri, saw_up, saw_down, dc].\n"
        "\tend frequency   Sweep-to frequency in Hz [%2.1f - %2.3e].\n"
        "\n";

    fprintf( stderr, format, g_argv0, VERSION_STR, REVISION_STR,
             g_argv0, c_max_amplitude, c_min_frequency, c_max_frequency);
} 
#endif

int main(int argc, char **argv){

    g_argv0 = argv[0];    

    if ( argc < 4 ) {

        usage();
        return -1;
    }

 /* Channel argument parsing */
    uint32_t ch = atoi(argv[1]) - 1; /* Zero based internally */
    if (ch > 1) {
        fprintf(stderr, "Invalid channel: %s\n", argv[1]);
        usage();
        return -1;
    }

    /* Signal amplitude argument parsing */
    double ampl = strtod(argv[2], NULL);
    if ( (ampl < 0.0) || (ampl > c_max_amplitude) ) {
        fprintf(stderr, "Invalid amplitude: %s\n", argv[2]);
        usage();
        return -1;
    }

    /* Signal frequency argument parsing */
    double freq = strtod(argv[3], NULL);
    double endfreq;
    endfreq = 0;

    if (argc > (6 - ARG_CORRECTION)) {
        endfreq = strtod(argv[6 - ARG_CORRECTION], NULL);
    }

    /* Signal type argument parsing */
    rp_waveform_t type = RP_WAVEFORM_SINE;
    if (argc > 4) {
        if ( strcmp(argv[4], "sine") == 0) {
            type = RP_WAVEFORM_SINE;
        } else if ( strcmp(argv[4], "sqr") == 0) {
            type = RP_WAVEFORM_SQUARE;
        } else if ( strcmp(argv[4], "tri") == 0) {
            type = RP_WAVEFORM_TRIANGLE;
		} else if ( strcmp(argv[4], "saw_up") == 0) {
            type = RP_WAVEFORM_RAMP_UP;
		} else if ( strcmp(argv[4], "saw_down") == 0) {
            type = RP_WAVEFORM_RAMP_DOWN;
		} else if ( strcmp(argv[4], "dc") == 0) {
            type = RP_WAVEFORM_DC;
        } else {
            fprintf(stderr, "Invalid signal type: %s\n", argv[4]);
            usage();
            return -1;
        }
    }

#ifdef Z20_250_12
	  /* Gain argument parsing */
    rp_gen_gain_t gain = RP_GAIN_1X;
    if (argc > 5) {
        if ( strcmp(argv[5], "x1") == 0) {
            gain = RP_GAIN_1X;
        } else if ( strcmp(argv[5], "x5") == 0) {
            gain = RP_GAIN_5X;
        } else {
            fprintf(stderr, "Invalid gain type: %s\n", argv[4]);
            usage();
            return -1;
        }
    }
#endif

    /* Check frequency limits */
    if ( (freq < c_min_frequency) || (freq > c_max_frequency ) ) {
        fprintf(stderr, "Invalid frequency: %s\n", argv[3]);
        usage();
        return -
		1;
    }

// printf("ch %d\n",ch);
// printf("amp %f\n",ampl);
// printf("freq %f\n",freq);
// printf("type %d\n",(int)type);
// #ifdef Z20_250_12
// printf("gain %d\n",(int)gain);
// #endif
// printf("freq end %f\n",endfreq);


	/* Print error, if rp_Init() function failed */
	if(rp_InitReset(false) != RP_OK){
		fprintf(stderr, "Rp api init failed!\n");
	}
	if (ch == 0){
		rp_GenOutDisable(RP_CH_1);
		rp_GenFreq(RP_CH_1, freq);
		rp_GenWaveform(RP_CH_1, type);
		#ifdef Z20_250_12
			rp_GenSetGainOut(RP_CH_1,gain);
		#endif
		rp_GenAmp(RP_CH_1, ampl);
	 	rp_GenOffset(RP_CH_1, 0);
		rp_GenOutEnable(RP_CH_1);
	}

	if (ch == 1){
		rp_GenOutDisable(RP_CH_2);
		rp_GenFreq(RP_CH_2, freq);
		rp_GenWaveform(RP_CH_2, type);
		#ifdef Z20_250_12
			rp_GenSetGainOut(RP_CH_2,gain);
		#endif
		rp_GenAmp(RP_CH_2, ampl);
	 	rp_GenOffset(RP_CH_2, 0);
		rp_GenOutEnable(RP_CH_2);
	}

	rp_Release();

	return 0;
}
