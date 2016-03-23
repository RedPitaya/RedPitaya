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
typedef enum {
    eSignalSine,         // Sinusoidal waveform
    eSignalSquare,       // Square waveform
    eSignalTriangle,     // Triangular waveform
    eSignalSweep,        // Sinusoidal frequency sweep
	eSignalConst         // Constant signal
} signal_e;

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
// static int g_dec[DEC_MAX] = { 1,  8,  64,  1024,  8192,  65536 };

/** Forward declarations */
void synthesize_signal(uint32_t size, double freq, awg_param_t *params);
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
    if (argc<8) {
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
    /// Phase
	double phase = strtod(argv[4], NULL);
    if ( (phase < 0) ) {
        fprintf(stderr, "Invalid phase!\n\n");
        usage();
        return -1;
		
    }/// Periodes
	double periodes = strtod(argv[8], NULL);
    if ( (periodes < 0) ) {
        fprintf(stderr, "Invalid periodes!\n\n");
        usage();
        return -1;
    }
	
		
	/** Parameters initialization and calculation */
  //  double    w_out = frequency * 2 * M_PI; // angular velocity
   // uint32_t  min_periodes = 1; // max 20
    uint32_t  size; // number of samples varies with number of periodes
  //  signal_e type = eSignalSine;
     uint32_t  idx = 3; //setting the decimation index (ind=3 => dec=1024)
    int       equal = 0; // parameter initialized for generator functionality
    int       shaping = 0; // parameter initialized for generator functionality
    //int       mode = 1; // parameter initialized for generator functionality
	double    freq_act = 0;
	//double ampl = 4000;	 //ADC count 4000 = 1Vpp output
	awg_param_t params;
	
	 /** Memory allocation */
    float **s = create_2D_table_size(SIGNALS_NUM, SIGNAL_LENGTH); // raw data saved to this location
	// uint32_t *r  = create_table_size(SIGNAL_LENGTH); //refference signal
	int32_t data[n];
	/* Initialization of Oscilloscope application */
    if(rp_app_init() < 0) {
        fprintf(stderr, "rp_app_init() failed!\n");
        return -1;
    }
	
	
	while(1) {
    /// Showtime.
			/* Calculate/recalculate refference signal if frequency has changed*/
			if (freq_act != frequency){
				freq_act=frequency;
				//size=round(1953125e5/freq_act);  //calculating number of samples 
				size = 10;
				
				printf("%7d\n", (int)size);
				for (int i=0; i<size; i++){
					
					//	r[i]=cos(i * 
				}
			}

                     

            /* Filter parameters for signal Acqusition */
            t_params[EQUAL_FILT_PARAM] = equal;
            t_params[SHAPE_FILT_PARAM] = shaping;
            t_params[TIME_RANGE_PARAM] = idx;
			
            //t_params[GEN_SIG_MOD_CH1] = mode;

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
    /* Prepare data buffer (calculate from input arguments) */
    
    synthesize_signal(size, freq_act, &params);
	for(int i = 0; i < n; i++) {
           data[i]=0;
		   }			
	for(int i = 0; i < size; i++) {
           data[i]=round(s[1][i]);
            // data[i] = round(ampl * cos(2*M_PI*(double)i/(double)n));
			/* Truncate to max value if needed */
			if (data[i] > 8191) {
			data[i] = 8191;
    }
       
    }
	//fprintf(stderr, "dziala2\n");
	
   	/// Write the data to the FPGA and set FPGA AWG state machine
    write_data_fpga(ch_out, data, &params);	
   }
}


/**
 * Synthesize a desired signal.
 *
 * Generates/synthesized  a signal, based on three pre-defined signal
 * types/shapes, signal amplitude & frequency. The data[] vector of 
 * samples at 125 MHz is generated to be re-played by the FPGA AWG module.
 *
 * @param ampl  Signal amplitude [Vpp].
 * @param freq  Signal frequency [Hz].
 * @param awg   Returned AWG parameters.
 *
 */
void synthesize_signal(uint32_t  size, double freq, awg_param_t *awg) {

    /* Various locally used constants - HW specific parameters */
    const int dcoffs = -155;

    /* This is where frequency is used... */
    awg->offsgain = (dcoffs << 16) + 0x1fff;
    awg->step = round(65536 * freq/c_awg_smpl_freq * n);
    awg->wrap = round(65536 * n - 1);
	printf("%7d", (int)awg->step);
	printf("%7d\n", (int)awg->wrap);
}


/**
 * Write synthesized data[] to FPGA buffer.
 *
 * @param ch    Channel number [0, 1].
 * @param data  AWG data to write to FPGA.
 * @param awg   AWG paramters to write to FPGA.
 */
void write_data_fpga(uint32_t ch,
                     const int32_t *data,
                     const awg_param_t *awg) {

    uint32_t i;
	
	

    fpga_awg_init();

    if(ch == 0) {
        /* Channel A */
        g_awg_reg->state_machine_conf = 0x000061;
        g_awg_reg->cha_scale_off      = awg->offsgain;
        g_awg_reg->cha_count_wrap     = awg->wrap;
        g_awg_reg->cha_count_step     = awg->step;
        g_awg_reg->cha_start_off      = 0;

        for(i = 0; i < n; i++) {
            g_awg_cha_mem[i] = data[i];
        }
    } else {
        /* Channel B */
        g_awg_reg->state_machine_conf = 0x410000;
        g_awg_reg->chb_scale_off      = awg->offsgain;
        g_awg_reg->chb_count_wrap     = awg->wrap;
        g_awg_reg->chb_count_step     = awg->step;
        g_awg_reg->chb_start_off      = 0;

        for(i = 0; i < n; i++) {
            g_awg_chb_mem[i] = data[i];
        }
    }

    /* Enable both channels */
    /* TODO: Should this only happen for the specified channel?
     *       Otherwise, the not-to-be-affected channel is restarted as well
     *       causing unwanted disturbances on that channel.
     */
    g_awg_reg->state_machine_conf = 0x110011;

    fpga_awg_exit();
		
}

/**
 * Acquire data from FPGA to memory (s).
 *
 * @param **s   Points to a memory where data is saved.
 * @param size  Size of data.
 */
int acquire_data(float **s ,uint32_t size) {

    int retries = 500000;
    int j, sig_num, sig_len;
    int ret_val;
    while(retries >= 0) {
        if((ret_val = rp_get_signals(&s, &sig_num, &sig_len)) >= 0) {
            /* Signals acquired in s[][]:
             * s[0][i] - TODO
             * s[1][i] - Channel ADC1 raw signal
             * s[2][i] - Channel ADC2 raw signal
             */
            for(j = 0; j < MIN(size, sig_len); j++) {
                printf("%7d, %7d\n",(int)s[1][j], (int)s[2][j]);
            }
            break;
        }
        if(retries-- == 0) {
            fprintf(stderr, "Signal acquisition was not triggered!\n");
            break;
        }
    }
    return 1;
}