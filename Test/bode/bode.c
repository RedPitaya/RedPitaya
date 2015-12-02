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
 * 
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

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

const double c_max_frequency = 62.5e6; // Maximal signal frequency [Hz]
const double c_min_frequency = 0; // Minimal signal frequency [Hz]
const double c_max_amplitude = 1.0; // Maximal signal amplitude [V]

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
int bode_data_analysis(float **s ,
                       uint32_t size,
                       double DC_bias,
                       float *Amplitude,
                       float *Phase,
                       double w_out,
                       int f);
                       
/** Print usage information */
void usage() {
    const char *format =
            "Bode analyzer version %s, compiled at %s\n"
            "\n"
            "Usage:\t%s [channel] "
                       "[amplitude] "
                       "[dc bias] "
                       "[averaging] "
                       "[count/steps] "
                       "[start freq] "
                       "[stop freq] "
                       "[scale type]\n"
            "\n"
            "\tchannel            Channel to generate signal on [1 / 2].\n"
            "\tamplitude          Signal amplitude in V [0 - 1, which means max 2Vpp].\n"
            "\tdc bias            DC bias/offset/component in V [0 - 1].\n"
            "\t                   Max sum of amplitude and DC bias is (0-1]V.\n"
            "\taveraging          Number of samples per one measurement [>1].\n"
            "\tcount/steps        Number of measurements [>2].\n"
            "\tstart freq         Lower frequency limit in Hz [3 - 62.5e6].\n"
            "\tstop freq          Upper frequency limit in Hz [3 - 62.5e6].\n"
            "\tscale type         0 - linear, 1 - logarithmic.\n"
            "\n"
            "Output:\tfrequency [Hz], phase [deg], amplitude [dB]\n";

    fprintf(stderr, format, VERSION_STR, __TIMESTAMP__, g_argv0);
}

/* Gain string (lv/hv) to number (0/1) transformation, currently not needed
int get_gain(int *gain, const char *str) {
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
*/

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

/** Trapezoidal method for integration */
float trapz(float *arrayptr, float T, int size1) {
  float result = 0;
  int i;
  
  for (i =0; i < size1 - 1 ; i++) {
    result += ( arrayptr[i] + arrayptr[ i+1 ]  );
  }
  
  result = (T / (float)2) * result;
  return result;
}

/** Finds a mean value of an array */
float mean_array(float *arrayptr, int numofelements) {
  int i = 1;
  float mean = 0;

  for(i = 0; i < numofelements; i++) {
    mean += arrayptr[i];
  }

  mean = mean / numofelements;
  return mean;
}

/** Finds a mean value of an array by columns, acquiring values from rows */
float mean_array_column(float **arrayptr, int length, int column) {
    float result = 0;
    int i;

    for(i = 0; i < length; i++) {
        result = result + arrayptr[i][column];
    }
    
    result = result / length;
    return result;
}

/** Bode analyzer */
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
    if (argc<9) {
        fprintf(stderr, "Too few arguments!\n\n");
        usage();
        return -1;
    }
    
    /** Argument parsing */
    /// Channel
    unsigned int ch = atoi(argv[1])-1; // Zero-based internally
    if (ch > 1) {
        fprintf(stderr, "Invalid channel value!\n\n");
        usage();
        return -1;
    }
    /// Amplitude
    double ampl = strtod(argv[2], NULL);
    if ( (ampl < 0) || (ampl > c_max_amplitude) ) {
        fprintf(stderr, "Invalid amplitude value!\n\n");
        usage();
        return -1;
    }
    /// DC bias
    double DC_bias = strtod(argv[3], NULL);
    if ( (DC_bias < 0) || (DC_bias > 1) ) {
        fprintf(stderr, "Invalid dc bias value!\n\n");
        usage();
        return -1;
    }
    if ( ampl+DC_bias>1 || ampl+DC_bias<=0 ) {
        fprintf(stderr, "Invalid ampl+dc value!\n\n");
        usage();
        return -1;
    }
    /// Averaging
    unsigned int averaging_num = strtod(argv[4], NULL);
    if ( averaging_num < 1 ) {
        fprintf(stderr, "Invalid averaging value!\n\n");
        usage();
        return -1;
    }
    /// Count/steps
    unsigned int steps = strtod(argv[5], NULL);
    if ( steps < 2 ) {
        fprintf(stderr, "Invalid count/steps value!\n\n");
        usage();
        return -1;
    }
    /// Frequency
    double start_frequency = strtod(argv[6], NULL);
    if ( (start_frequency < c_min_frequency) || (start_frequency > c_max_frequency) ) {
        fprintf(stderr, "Invalid start freq!\n\n");
        usage();
        return -1;
    }
    double end_frequency = strtod(argv[7], NULL);
    if ( (end_frequency < c_min_frequency) || (end_frequency > c_max_frequency) ) {
        fprintf(stderr, "Invalid end freq!\n\n");
        usage();
        return -1;
    }
    if ( end_frequency < start_frequency ) {
        fprintf(stderr, "End frequency has to be greater than the start frequency!\n\n");
        usage();
        return -1;
    }
    /// Scale type (0=lin, 1=log)
    unsigned int scale_type = strtod(argv[8], NULL);
    if ( scale_type > 1 ) {
        fprintf(stderr, "Invalid scale type!\n\n");
        usage();
        return -1;
    }

    /** Parameters initialization and calculation */
    double frequency_step;
    double a,b,c;
    double    endfreq = 0; // endfreq set for generate's sweep
    double    k;
    double    w_out; // angular velocity
    uint32_t  min_periodes = 10; // max 20
    uint32_t  size; // number of samples varies with number of periodes
    signal_e type = eSignalSine;
    int       f = 0; // used in for lop, setting the decimation
    int       i1, fr; // iterators in for loops
    int       equal = 0; // parameter initialized for generator functionality
    int       shaping = 0; // parameter initialized for generator functionality
    int transientEffectFlag = 1;
    int stepsTE = 10; // number of steps for transient effect(TE) elimination
    int TE_step_counter;
    int progress_int;
    char command[70];
    char hex[45];
    // if user sets less than 10 steps than stepsTE is decresed
    // for transient efect to be eliminated only 10 steps of measurements is eliminated
    if (steps < 10){
        stepsTE = steps;
    }
    TE_step_counter = stepsTE ;

    /// If logarythmic scale is selected start and end frequencies are defined to compliment logaritmic output
    if(scale_type) {
        b = log10f( end_frequency );
        a = log10f( start_frequency );
        c = ( b - a ) /( steps - 1);
    } else {
        frequency_step = ( end_frequency - start_frequency ) / ( steps - 1);
    }
    

    /** Memory allocation */
    float **s = create_2D_table_size(SIGNALS_NUM, SIGNAL_LENGTH); // raw data saved to this location
    float *Amplitude                = (float *)malloc( (averaging_num + 1) * sizeof(float));
    float *Amplitude_output         = (float *)malloc( (steps + 1) * sizeof(float));
    float *Phase                    = (float *)malloc( (averaging_num + 1) * sizeof(float));
    float *Phase_output             = (float *)malloc( (steps + 1) * sizeof(float));
    float **data_for_avreaging      = create_2D_table_size((averaging_num + 1), 2 );
    float *measured_data_amplitude  = (float *)malloc((2) * sizeof(float) );
    float *measured_data_phase      = (float *)malloc((2) * sizeof(float) );
    float *frequency                = (float *)malloc((steps + 1) * sizeof(float) );
    
    /* Initialization of Oscilloscope application */
    if(rp_app_init() < 0) {
        fprintf(stderr, "rp_app_init() failed!\n");
        return -1;
    }

    /* We try to open a data file */
    FILE *try_open = fopen("/tmp/bode_data/data_frequency", "w");

    /* If the directory doesn't exist yet, we first have to create it. */
    if(try_open == NULL){

        int b_number;
        char command[30];
        strcpy(command, "mkdir /tmp/bode_data");
        system(command);

        /* We must also create all the files for storing the data */
        for(b_number = 0; b_number < 3; b_number++){

            switch(b_number){
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
            system(command);
        }
        /* Change permissions */
        strcpy(command, "chmod -R 777 /tmp/bode_data");
        system(command);
    }

    /* Opening files */
    FILE *file_frequency = fopen("/tmp/bode_data/data_frequency", "w");
    FILE *file_amplitude = fopen("/tmp/bode_data/data_amplitude", "w");
    FILE *file_phase = fopen("/tmp/bode_data/data_phase", "w");

    /// Showtime.
    for ( fr = 0; fr < steps; fr++ ) {
        
        /* scale type dictates frequency used in for iterations */
        if ( scale_type ) { // log scle
            k = powf( 10, ( c * (float)fr ) + a );
            frequency[ fr ] =  k ;
        }
        else { // lin scale
            frequency[ fr ] = start_frequency + ( frequency_step * fr );
        }

        //this section eliminates transient effect that spoiles the measuremets
        // it outputs frequencies below start frequency and increses it to the strat frequency
        if (transientEffectFlag == 1){
            if (TE_step_counter > 0){
                frequency[fr] = (int)(start_frequency - (start_frequency/2) + ((start_frequency/2)*TE_step_counter/stepsTE) );
                TE_step_counter--;
            }
            if (TE_step_counter == 0){
                fr = 0;
                frequency[fr] = start_frequency;
                transientEffectFlag = 0;
            }
            
        }


        if(transientEffectFlag == 1){
            //printf("tran flag = %d\n", transientEffectFlag);
            progress_int = (int)100*(stepsTE - TE_step_counter)/(steps+stepsTE-1);
        }
        else {
            progress_int = (int)100*(stepsTE + fr)/(steps+stepsTE-1);
        }
        
        if (progress_int <= 100){
            FILE *progress_file = fopen("/tmp/bode_data/progress.txt", "w");
            sprintf(hex, "%x", (int)(255 - (255*progress_int/100)));
            strcpy(command, "/opt/redpitaya/bin/monitor 0x40000030 0x" );
            strcat(command, hex);
            
            system(command);
            //printf("progress = %d\n", progress_int);
            fprintf(progress_file , "%d \n" ,  progress_int );
            fclose(progress_file);
        }





        w_out = frequency[ fr ] * 2 * M_PI; // omega - angular velocity

       /**
        * At first the signal generator generates a signal before the
        * measuring proces begins. First results are inaccurate otherwise.
        */
        awg_param_t params;
        /// Prepare data buffer (calculate from input arguments)
        synthesize_signal(ampl, frequency[fr], type, endfreq, data, &params);
        /// Write the data to the FPGA and set FPGA AWG state machine
        write_data_fpga(ch, data, &params);
        usleep(1000);


        for ( i1 = 0; i1 < averaging_num; i1++ ) {

            /* decimation changes depending on frequency */
            if      (frequency[fr] >= 160000){      f=0;    }
            else if (frequency[fr] >= 20000) {      f=1;    }    
            else if (frequency[fr] >= 2500)  {      f=2;    }    
            else if (frequency[fr] >= 160)   {      f=3;    }    
            else if (frequency[fr] >= 20)    {      f=4;    }     
            else if (frequency[fr] >= 2.5)   {      f=5;    }

            /* setting decimtion */
            if (f != DEC_MAX) {
                t_params[TIME_RANGE_PARAM] = f;
            } else {
                fprintf(stderr, "Invalid decimation DEC\n");
                usage();
                return -1;
            }
            
            /* calculating num of samples */
            size = round( ( min_periodes * 125e6 ) / ( frequency[fr] * g_dec[f] ) );

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

            /* data manipulation - returnes Z (complex impedance) */
            if( bode_data_analysis( s, size, DC_bias, Amplitude, Phase, w_out, f) < 0) {
                printf("error data analysis bode_data_analysis\n");
                return -1;
            }

            /* Saving data */
            data_for_avreaging[ i1 ][ 1 ] = *Amplitude;
            data_for_avreaging[ i1 ][ 2 ] = *Phase;
        } // avearging loop end

        /* Calculating and saving mean values */
        measured_data_amplitude[ 1 ] = mean_array_column( data_for_avreaging, averaging_num, 1 );
        measured_data_phase[ 1 ]     = mean_array_column( data_for_avreaging, averaging_num, 2 );

        if (transientEffectFlag == 0) {
            Amplitude_output[fr] = measured_data_amplitude[ 1 ];
            Phase_output[fr] = measured_data_phase[ 1 ];

            //printf("%.2f    %.5f    %.5f\n", frequency[fr], measured_data_phase[ 1 ], measured_data_amplitude[ 1 ]);

            /* Writing data into files */
            fprintf(file_frequency, "%.5f\n", frequency[fr]);
            fprintf(file_amplitude, "%.5f\n", measured_data_amplitude[1]);
            fprintf(file_phase, "%.5f\n", measured_data_phase[1]);
        }

        

    } // end of frequency sweep loop
   
    /* Closing files */
    fclose(file_frequency);
    fclose(file_phase);
    fclose(file_amplitude);
    
    /* Setting amplitude to 0V - turning off the output. */
    awg_param_t params;
    /* Prepare data buffer (calculate from input arguments) */
    synthesize_signal( 0, 1000, type, endfreq, data, &params );
    /* Write the data to the FPGA and set FPGA AWG state machine */
    write_data_fpga( ch, data, &params );


    for (int po = 0; po < steps; ++po)
    {
        printf("%.2f    %.5f    %.5f\n", frequency[po],Phase_output[po], Amplitude_output[ po ]);
    }
    /** All's well that ends well. */
    return 1;
}

/**
 * Synthesize a desired signal.
 *
 * Generates/synthesized  a signal, based on three pre-defined signal
 * types/shapes, signal amplitude & frequency. The data[] vector of 
 * samples at 125 MHz is generated to be re-played by the FPGA AWG module.
 *
 * @param ampl  Signal amplitude [V].
 * @param freq  Signal Frequency [Hz].
 * @param type  Signal type/shape [Sine, Square, Triangle, Constant].
 * @param data  Returned synthesized AWG data vector.
 * @param awg   Returned AWG parameters.
 *
 */
void synthesize_signal(double ampl, 
                        double freq, 
                        signal_e type, 
                        double endfreq,
                        int32_t *data,
                        awg_param_t *awg) {

    uint32_t i;

    /* Various locally used constants - HW specific parameters */
    const int dcoffs = -155;
    const int trans0 = 30;
    const int trans1 = 300;
    const double tt2 = 0.249;

    /* This is where frequency is used... */
    awg->offsgain = (dcoffs << 16) + 0x1fff;
    awg->step = round(65536 * freq/c_awg_smpl_freq * n);
    awg->wrap = round(65536 * n - 1);

    int trans = freq / 1e6 * trans1; /* 300 samples at 1 MHz */
    uint32_t amp = ampl * 4000.0;    /* 1 V ==> 4000 DAC counts */
    if (amp > 8191) {
        /* Truncate to max value if needed */
        amp = 8191;
    }

    if (trans <= 10) {
        trans = trans0;
    }


    /* Fill data[] with appropriate buffer samples */
    for(i = 0; i < n; i++) {
        
        /* Sine */
        if (type == eSignalSine) {
            data[i] = round(amp * cos(2*M_PI*(double)i/(double)n));
        }
 
        /* Square */
        if (type == eSignalSquare) {
            data[i] = round(amp * cos(2*M_PI*(double)i/(double)n));
            if (data[i] > 0)
                data[i] = amp;
            else 
                data[i] = -amp;

            /* Soft linear transitions */
            double mm, qq, xx, xm;
            double x1, x2, y1, y2;    

            xx = i;       
            xm = n;
            mm = -2.0*(double)amp/(double)trans; 
            qq = (double)amp * (2 + xm/(2.0*(double)trans));
            
            x1 = xm * tt2;
            x2 = xm * tt2 + (double)trans;
            
            if ( (xx > x1) && (xx <= x2) ) {  
                
                y1 = (double)amp;
                y2 = -(double)amp;
                
                mm = (y2 - y1) / (x2 - x1);
                qq = y1 - mm * x1;

                data[i] = round(mm * xx + qq); 
            }
            
            x1 = xm * 0.75;
            x2 = xm * 0.75 + trans;
            
            if ( (xx > x1) && (xx <= x2)) {  
                    
                y1 = -(double)amp;
                y2 = (double)amp;
                
                mm = (y2 - y1) / (x2 - x1);
                qq = y1 - mm * x1;
                
                data[i] = round(mm * xx + qq); 
            }
        }
        
        /* Triangle */
        if (type == eSignalTriangle) {
            data[i] = round(-1.0*(double)amp*(acos(cos(2*M_PI*(double)i/(double)n))/M_PI*2-1));
        }

        /* Sweep */
        /* Loops from i = 0 to n = 16*1024. Generates a sine wave signal that
           changes in frequency as the buffer is filled. */
        double start = 2 * M_PI * freq;
        double end = 2 * M_PI * endfreq;
        if (type == eSignalSweep) {
            double sampFreq = c_awg_smpl_freq; // 125 MHz
            double t = i / sampFreq; // This particular sample
            double T = n / sampFreq; // Wave period = # samples / sample frequency
            /* Actual formula. Frequency changes from start to end. */
            data[i] = round(amp * (sin((start*T)/log(end/start) * ((exp(t*log(end/start)/T)-1)))));
        }
        
        /* Constant */
		if (type == eSignalConst) data[i] = amp;
        
        /* TODO: Remove, not necessary in C/C++. */
        if(data[i] < 0)
            data[i] += (1 << 14);
    }
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
        g_awg_reg->state_machine_conf = 0x000041;
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

    int retries = 150000;
    int j, sig_num, sig_len;
    int ret_val;
    usleep(50000);
    while(retries >= 0) {
        if((ret_val = rp_get_signals(&s, &sig_num, &sig_len)) >= 0) {
            /* Signals acquired in s[][]:
             * s[0][i] - TODO
             * s[1][i] - Channel ADC1 raw signal
             * s[2][i] - Channel ADC2 raw signal
             */
            for(j = 0; j < MIN(size, sig_len); j++) {
                //printf("%7d, %7d\n",(int)s[1][j], (int)s[2][j]);
            }
            break;
        }
        if(retries-- == 0) {
            fprintf(stderr, "Signal scquisition was not triggered!\n");
            break;
        }
        usleep(1000);
    }
    usleep(30000); // delay for pitaya to operate correctly
    return 1;
}

/**
 * Acquired data analysis function for Bode analyzer.
 *
 * @param s          Pointer where data is read from.
 * @param size       Size of data.
 * @param DC_bias    DC component.
 * @param Amplitude  Pointer where to write amplitude data.
 * @param Phase      Pointer where to write phase data.
 * @param w_out      Angular velocity (2*pi*freq).
 * @param f          Decimation selector index.
 */
int bode_data_analysis(float **s ,
                       uint32_t size,
                       double DC_bias,
                       float *Amplitude,
                       float *Phase,
                       double w_out,
                       int f) {
    int i2, i3;
    float **U_acq = create_2D_table_size(SIGNALS_NUM, SIGNAL_LENGTH);
    /* Signals multiplied by the reference signal (sin) */
    float *U1_sampled_X = (float *) malloc( size * sizeof( float ) );
    float *U1_sampled_Y = (float *) malloc( size * sizeof( float ) );
    float *U2_sampled_X = (float *) malloc( size * sizeof( float ) );
    float *U2_sampled_Y = (float *) malloc( size * sizeof( float ) );
    /* Signals return by trapezoidal method in complex */
    float *X_component_lock_in_1 = (float *) malloc( size * sizeof( float ) );
    float *X_component_lock_in_2 = (float *) malloc( size * sizeof( float ) );
    float *Y_component_lock_in_1 = (float *) malloc( size * sizeof( float ) );
    float *Y_component_lock_in_2 = (float *) malloc( size * sizeof( float ) );
    /* Voltage, current and their phases calculated */
    float U1_amp;
    float Phase_U1_amp;
    float U2_amp;
    float Phase_U2_amp;
    float Phase_internal;
    //float Z_phase_deg_imag;  // may cuse errors because not complex
    float T; // Sampling time in seconds
    float *t = create_table_size(16384);

    T = ( g_dec[f] / 125e6 );
    //printf("T = %f;\n",T );

    for(i2 = 0; i2 < (size - 1); i2++) {
        t[i2] = i2;
    }
    //printf("size1 = %d\n", size);

    /* Transform signals from  AD - 14 bit to voltage [ ( s / 2^14 ) * 2 ] */
    for (i2 = 0; i2 < SIGNALS_NUM; i2++) { // only the 1 and 2 are used for i2
        for(i3 = 0; i3 < size; i3 ++ ) { 
            U_acq[i2][i3] = ( ( s[i2][i3] ) * (float)( 2 - DC_bias ) ) / (float)16384 ; //division comes after multiplication, this way no accuracy is lost
            //printf("data(%d,%d) = %f;\n",(i2+1), (i3+1), U_acq[i2][i3] );
        }
    }

    /* Acquired signals must be multiplied by the reference signals, used for lock in metod */
    float ang;
    for( i2 = 0; i2 < size; i2++) {
        ang = (i2 * T * w_out);
        //printf("ang(%d) = %f \n", (i2+1), ang);
        U1_sampled_X[i2] = U_acq[1][i2] * sin( ang );
        U1_sampled_Y[i2] = U_acq[1][i2] * sin( ang+ (M_PI/2) );

        U2_sampled_X[i2] = U_acq[2][i2] * sin( ang );
        U2_sampled_Y[i2] = U_acq[2][i2] * sin( ang +(M_PI/2) );
    }

    /* Trapezoidal method for calculating the approximation of an integral */
    X_component_lock_in_1[1] = trapz( U1_sampled_X, (float)T, size );
    Y_component_lock_in_1[1] = trapz( U1_sampled_Y, (float)T, size );

    X_component_lock_in_2[1] = trapz( U2_sampled_X, (float)T, size );
    Y_component_lock_in_2[1] = trapz( U2_sampled_Y, (float)T, size );
    
    /* Calculating voltage amplitude and phase */
    U1_amp = (float)2 * (sqrtf( powf( X_component_lock_in_1[ 1 ] , (float)2 ) + powf( Y_component_lock_in_1[ 1 ] , (float)2 )));
    Phase_U1_amp = atan2f( Y_component_lock_in_1[ 1 ], X_component_lock_in_1[ 1 ] );

    /* Calculating current amplitude and phase */
    U2_amp = (float)2 * (sqrtf( powf( X_component_lock_in_2[ 1 ], (float)2 ) + powf( Y_component_lock_in_2[ 1 ] , (float)2 ) ) );
    Phase_U2_amp = atan2f( Y_component_lock_in_2[1], X_component_lock_in_2[1] );
    
    Phase_internal = Phase_U2_amp - Phase_U1_amp ;

    if (Phase_internal <=  (-M_PI) )
    {
        Phase_internal = Phase_internal +(2*M_PI);
    }
    else if ( Phase_internal >= M_PI )
    {
        Phase_internal = Phase_internal -(2*M_PI) ;
    }
    else 
    {
        Phase_internal = Phase_internal;
    }
   
    *Amplitude = 10*log( U2_amp / U1_amp );
    *Phase = Phase_internal * ( 180/M_PI );

    return 1;
}
