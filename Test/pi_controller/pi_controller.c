#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/param.h>

#include "main_osc.h"
#include "fpga_osc.h"
#include "fpga_awg.h"
#include "version.h"

#define M_PI 3.14159265358979323846


const char *g_argv0 = NULL; // Program name

#define n (16*1024) // AWG buffer length [samples]
int32_t data[n]; // AWG data buffer

/** Signal types */
/*
typedef enum {
    eSignalSine,         // Sinusoidal waveform
    eSignalSquare,       // Square waveform
    eSignalTriangle,     // Triangular waveform
    eSignalSweep,        // Sinusoidal frequency sweep
	eSignalConst         // Constant signal
} signal_e;
*/
/** AWG FPGA parameters */
typedef struct {
    int32_t  offsgain;   // AWG offset & gain
    uint32_t wrap;       // AWG buffer wrap value
    uint32_t step;       // AWG step interval
} awg_param_t;

/** Oscilloscope module parameters as defined in main module
 * @see rp_main_params
 */
float t_params[PARAMS_NUM] = { 0, 1e6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

/** Decimation translation table */
#define DEC_MAX 6 // Max decimation index
static int g_dec[DEC_MAX] = { 1,  8,  64,  1024,  8192,  65536 };

/** Forward declarations */
void synthesize_signal(double ampl, double freq, signal_e type, double endfreq,
                       int32_t *data,
                       awg_param_t *params);
void write_data_fpga(uint32_t ch,
                     const int32_t *data,
                     const awg_param_t *awg);
int acquire_data(float **s ,
                 uint32_t size);
				 
				 /** Print usage information */
void usage() {
    const char *format =
            "PI controller\n"
            "\n"
            "Usage:\t%s [channel in] "
                       "[channel out] "
                       "[frequency] "
                       "[phase shift] "
                       "[k_p] "
                       "[k_i] "
                       "[T_int]\n"
            "\n"
            "\tchannel in         Channel to acquire error signal on [1 / 2].\n"
            "\tchannel out        Channel to generate controll signal on [1 / 2].\n"
            "\tfrequency          Refference frequency in Hz [~12000].\n"
            "\tphase shift        Phase shift of ref signal in deg [0 - 360]\n"
            "\tk_p                Proportional coefficient\n"
            "\tk_i                Integrator coefficient\n"
            "\tT_int              Low-pass filter coefficient in s\n"
            "\n";

    fprintf(stderr, format, g_argv0);
}


/** Allocates a memory with size num_of_el, memory has 1 dimension */
float *create_table_size(int num_of_el) {
    float *new_table = (float *)malloc( num_of_el * sizeof(float));
    return new_table;
}

/** Allocates a memory with size num_of_cols*num_of_rows */
float **create_2D_table_size(int num_of_rows, int num_of_cols) {
    float **new_table = (float **)malloc( num_of_rows * sizeof(float*));
    int i;
        for(i = 0; i < num_of_rows; i++) {
            new_table[i] = create_table_size(num_of_cols);
        }
    return new_table;
}

float max_array(float *arrayptr, int numofelements) {
  int i = 0;
  float max = -1e6; // Setting the minimum value possible

  for(i = 0; i < numofelements; i++) {
    if(max < arrayptr[ i ]) max = arrayptr[ i ];
  }

  return max;
}

int main(int argc, char *argv[]) {
	fprintf(stderr, "dziala\n");
	
/** Set program name */
    g_argv0 = argv[0];
    
    /**
     * Manpage
     * 
     * usage() prints its output to stderr, nevertheless main returns
     * zero as calling lcr without any arguments is not an error.
     */
    if (argc==1) {
		usage();
		return 0;
	}
    
    /** Argument check */
    if (argc<7) {
        fprintf(stderr, "Too few arguments!\n\n");
        usage();
        return -1;
    }
    
    /** Argument parsing */
    /// Channel in
    unsigned int ch_in = atoi(argv[1])-1; // Zero-based internally
    if (ch_in > 1) {
        fprintf(stderr, "Invalid channel value!\n\n");
        usage();
        return -1;
    }
    /// Channel out
    unsigned int ch_out = atoi(argv[2])-1; // Zero-based internally
    if (ch_out > 1) {
        fprintf(stderr, "Invalid channel value!\n\n");
        usage();
        return -1;
    }		
    /// Frequency
	double frequency = strtod(argv[3], NULL);
    if ( (frequency < 0) ) {
        fprintf(stderr, "Invalid freq!\n\n");
        usage();
        return -1;
    }	
    /// Phase_shift
	double phase = strtod(argv[4], NULL);
    if ( (phase < 0) ) {
        fprintf(stderr, "Invalid phase!\n\n");
        usage();
        return -1;
    }
	
	/** Parameters initialization and calculation */
  //  double    w_out = frequency * 2 * M_PI; // angular velocity
    uint32_t  min_periodes = 1; // max 20
    uint32_t  size; // number of samples varies with number of periodes
    signal_e type = eSignalSine;
    int       f = 1; // used in for lop, setting the decimation
    int       equal = 0; // parameter initialized for generator functionality
    int       shaping = 0; // parameter initialized for generator functionality
	
	 /** Memory allocation */
    float **s = create_2D_table_size(SIGNALS_NUM, SIGNAL_LENGTH); // raw data saved to this location
	
	/* Initialization of Oscilloscope application */
    if(rp_app_init() < 0) {
        fprintf(stderr, "rp_app_init() failed!\n");
        return -1;
    }
    /// Showtime.

            /* setting decimtion */
            t_params[TIME_RANGE_PARAM] = f;           
            
            /* calculating num of samples */
            size = round( ( min_periodes * 125e6 ) / ( frequency* g_dec[f] ) );

            /* Filter parameters for signal Acqusition */
            t_params[EQUAL_FILT_PARAM] = equal;
            t_params[SHAPE_FILT_PARAM] = shaping;

            /* Setting of parameters in Oscilloscope main module for signal Acqusition */
            if(rp_set_params((float *)&t_params, PARAMS_NUM) < 0) {
                fprintf(stderr, "rp_set_params() failed!\n");
                return -1;
            }

            /* ADC Data acqusition - saved to s */
            if (acquire_data( s, size ) < 0) {
                printf("error acquiring data @ acquire_data\n");
                return -1;
            }     
}