/**
 * $Id: generate.c 1246 2014-06-02 09:07am pdorazio $
 *
 * @brief Red Pitaya simple signal/function generator with pre-defined
 *        signal types.
 *
 * @Author Ales Bardorfer <ales.bardorfer@redpitaya.com>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "fpga_awg.h"
#include "version.h"

#include <unistd.h>
#include <getopt.h>
#include <sys/param.h>
#include "main_osc.h"
#include "fpga_osc.h"

#include <complex.h>    /* Standart Library of Complex Numbers */
#define M_PI 3.14159265358979323846
/**
 * GENERAL DESCRIPTION:
 *
 * The code below performs a function of a signal generator, which produces
 * a a signal of user-selectable pred-defined Signal shape
 * [Sine, Square, Triangle], Amplitude and Frequency on a selected Channel:
 *
 *
 *                   /-----\
 *   Signal shape -->|     | -->[data]--+-->[FPGA buf 1]--><DAC 1>
 *   Amplitude ----->| AWG |            |
 *   Frequency ----->|     |             -->[FPGA buf 2]--><DAC 2>
 *                   \-----/            ^
 *                                      |
 *   Channel ---------------------------+ 
 *
 *
 * This is achieved by first parsing the four parameters defining the 
 * signal properties from the command line, followed by synthesizing the 
 * signal in data[] buffer @ 125 MHz sample rate within the
 * generate_signal() function, depending on the Signal shape, Amplitude
 * and Frequency parameters. The data[] buffer is then transferred
 * to the specific FPGA buffer, defined by the Channel parameter -
 * within the write_signal_fpga() function.
 * The FPGA logic repeatably sends the data from both FPGA buffers to the
 * corresponding DACs @ 125 MHz, which in turn produces the synthesized
 * signal on Red Pitaya SMA output connectors labeled DAC1 & DAC2.
 *
 * Then it acquires up to 16k samples on both Red Pitaya input
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
/*Setting measuring parameters (LCR Pitaya DT)*/
    
double Rs = 8200; // Set value of shunt resistor 
double DC_Bias = 0; // Set value od DC volatge on outputs 
uint32_t averaging_num = 5; // Number of measurments for averaging
uint32_t min_periodes =15; // max 20
/* frequency sweep */
uint32_t frequency_step = 100;
double one_calibration;

/** Maximal signal frequency [Hz] */
const double c_max_frequency = 62.5e6;

/** Minimal signal frequency [Hz] */
const double c_min_frequency = 0;

/** Maximal signal amplitude [Vpp] */
const double c_max_amplitude = 2.0;

/** AWG buffer length [samples]*/
#define n (16*1024)

/** AWG data buffer */
int32_t data[n];

/** Program name */
const char *g_argv0 = NULL;

/** Signal types */
typedef enum {
    eSignalSine,         ///< Sinusoidal waveform.
    eSignalSquare,       ///< Square waveform.
    eSignalTriangle,     ///< Triangular waveform.
    eSignalSweep         ///< Sinusoidal frequency sweep.
} signal_e;

/** AWG FPGA parameters */
typedef struct {
    int32_t  offsgain;   ///< AWG offset & gain.
    uint32_t wrap;       ///< AWG buffer wrap value.
    uint32_t step;       ///< AWG step interval.
} awg_param_t;

/* Forward declarations */
void synthesize_signal(double ampl, double freq, signal_e type, double endfreq,
                       int32_t *data,
                       awg_param_t *params);
void write_data_fpga(uint32_t ch,
                     const int32_t *data,
                     const awg_param_t *awg);
/** Oscilloscope module parameters as defined in main module
 * @see rp_main_params
 */
float t_params[PARAMS_NUM] = { 0, 1e6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

/** Max decimation index */
#define DEC_MAX 6

/** Decimation translation table [COMENTED BECAUSE NOTU USED AT GENERATOR] */ 
static int g_dec[DEC_MAX] = { 1,  8,  64,  1024,  8192,  65536 };

/*the most used variable in for loops*/
int i, i2;

/** Print usage information */
void usage() {

    const char *format =
        "\n"
        "Usage: %s  frequency amplitude samples <DEC> <parameters> <sig. type> <end frequency>\n"
        "\n"
        "\tfrequency          Signal (start) frequency in Hz [%2.1f - %2.1e].\n"
        "\tamplitude          Peak-to-peak signal amplitude in Vpp [0.0 - %1.1f] only Output 1 will be set recomended = 1.0V\n"
        "\tsamples            Number of samples to acquire [0 - %u ].\n"
        "\tDEC                Decimation [%u,%u,%u,%u,%u,%u] (default: 1).\n"
        //"\tchannel          Channel to generate signal on [1, 2].\n"
        //"\ttype               Signal type [sine, sqr, tri, sweep].\n"
        "\tend frequency      Sweep-to frequency in Hz [%2.1f - %2.1e](set this value to start freq. for measurement_sweep)\n"
        "\tMeasurement sweep  number of mesurements (averaged resoults) [max 10]\n"
        "\tCalibration        set to 1 to initiate calibration. default 0\n"
        "\n";

    fprintf( stderr, format, g_argv0, c_min_frequency, c_max_frequency, c_max_amplitude, SIGNAL_LENGTH,g_dec[0],g_dec[1],g_dec[2],g_dec[3],g_dec[4],g_dec[5],c_min_frequency, c_max_frequency);
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

float *create_table() {
    float *new_table = (float *)malloc( 22 * sizeof(float));
    return new_table;
}

float *create_table_size(int num_of_el) {
    float *new_table = (float *)malloc( num_of_el * sizeof(float));
    return new_table;
}

float **create_2D_table_size(int num_of_rows, int num_of_cols) {
    float **new_table = (float **)malloc( num_of_rows * sizeof(float*));
        for(i = 0; i < num_of_rows; i++) {
            new_table[i] = create_table_size(num_of_cols);
        }
    return new_table;
}

float max_array(float *arrayptr, int numofelements) {
  int i = 0;
  float max = -100000;

  for(i; i < numofelements; i++)
  {
    if(max < arrayptr[i])
    {
      max = arrayptr[i];
    }
  }
  return max;
}

float *trapz(float *arrayptr, float *dT, int size) {
  float *result = (float *)malloc(sizeof(float));
  int i;
  //printf("size = %d\n", size);
  for (i =0; i < size ; i++) {
    result[0] += fabsf((dT[ i+1 ] - dT[ i ]) * ( arrayptr[i] - arrayptr[ i+1 ] )/(float)2);
  }
    return result;
}

/** lcr main */
int main(int argc, char *argv[])
{
    g_argv0 = argv[0];
    int equal = 0;
    int shaping = 0;    

    if ( argc < 3 ) {

        usage();
        return -1;
    }

    /* Signal frequency argument parsing */
    double start_frequency = strtod(argv[1], NULL);
    /* Check frequency limits */
    if ( (start_frequency < c_min_frequency) || (start_frequency > c_max_frequency ) ) {
        fprintf(stderr, "Invalid start frequency: %s\n", argv[1]);
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

    /* Acqusition size */
    uint32_t size = atoi(argv[3]);
    if (size > 16384) {
            fprintf(stderr, "Invalid SIZE: %s\n", argv[3]);
            usage();
            exit( EXIT_FAILURE );
        }

    /*Decimation*/
    uint32_t idx = 1;
    uint32_t dec = atoi(argv[4]);
    for (idx = 0; idx < DEC_MAX; idx++) {
        if (dec == g_dec[idx]) {
            break;
        }
    }
    if (idx != DEC_MAX) {
        t_params[TIME_RANGE_PARAM] = idx;
    } 
    else {
        fprintf(stderr, "Invalid decimation DEC: %s\n", argv[4]);
        usage();
        return -1;
    }
    
    /* Signal type argument parsing */
    /* LCR meter only uses sine signal for outputs for now; TODO enable other signals */
    signal_e type = eSignalSine;
    /*
    if ( strcmp(argv[4], "sine") == 0) {
            type = eSignalSine;
        } else if ( strcmp(argv[5], "sqr") == 0) {
            type = eSignalSquare;
        } else if ( strcmp(argv[5], "tri") == 0) {
            type = eSignalTriangle;
        } else if ( strcmp(argv[5], "sweep") == 0) {
            type = eSignalSweep;   
        } else {
            fprintf(stderr, "Invalid signal type: %s\n", argv[5]);
            usage();
            return -1;
        }
    */

    /* End frequency */
    double end_frequency = 0;
    end_frequency = strtod(argv[5], NULL);
    if (end_frequency > c_max_frequency) {
        end_frequency = c_max_frequency;
        printf("end frequency set too high. now set to max value (%2.1e)\n",c_max_frequency);
    }

    /* Measurement sweep */
    double measurement_sweep = 0;
    measurement_sweep = strtod(argv[6], NULL);
    if (measurement_sweep > 10) {
        measurement_sweep = 10;
        printf("measurement sweep set too high [MAX = 10], changed to max");
    }

    int calibration = 0;
    measurement_sweep = strtod(argv[7], NULL);
    if(calibration ==1){
        printf("calibration initiated\n");
    }

    /* endfreq set to 0 because sweep is done in anothef foor loop */
    double endfreq = 0;

    /* only chanel 1 is used TODO let user decide which chanel will be set for output */
    uint32_t ch = 1;

    /* if user sets the measuring_sweep and end frequency than end frequency will prevail and program will sweep in frequency domain */
    if (end_frequency > start_frequency) {                        
       measurement_sweep = 1;                    
    }

    /*
    * Calibration sequence
    */

    /* the program waits for the user to make a short connection */
    char calibration_continue;
    while(1) {
      printf("Short connection calibration. continue? [y|n] :");
      if (scanf( "%c", &calibration_continue) > 0) 
      {
        if(calibration_continue=='y') 
          {break;}
        else if (calibration_continue=='Y') 
          {break;}
        else if (calibration_continue == 'n') 
          {return 0;}
        else if (calibration_continue == 'N') 
          {return 0;}
        else 
          {return -1;}
      }
      else {
        printf("error when readnig from stdinput (scanf)\n");
        return -1;
      }
    }
    

    /* Memory and pointer initialization for short-circuit calibration results */
    float *Measurement_calibration_short = (float *)malloc(sizeof(float)); /// Warning not yet set!

    /* Memory initialization */
    // derivative tie for trapezoidal function
    float *dT = create_table();
    //time vector
    float *t  = create_table();
    // Acquired data is stored in s array
    float **s = create_2D_table_size(SIGNALS_NUM, SIGNAL_LENGTH);
    // acquired data is converted to volrage and stored in U_acq
    float **U_acq = create_2D_table_size(SIGNALS_NUM, SIGNAL_LENGTH);
    // number of acquired signals
    int sig_num, sig_len;
    // return value from acquire
    int ret_val;
    // number of acquire retries for acquiring the data
    int retries = 150000;
    // acquired signal size
    int signal_size;
    // Used for storing the Voltage and current on the load
    float *U_load = create_table_size( SIGNAL_LENGTH );
    float *I_load = create_table_size( SIGNAL_LENGTH );
    //Signals multiplied by the reference signal (sin)
    float *U_load_ref = create_2D_table_size(SIGNALS_NUM, SIGNAL_LENGTH); //U_load_ref[1][i] voltage signal 1, U_load_ref[2][i] - voltage signal 2
    float *I_load_ref = create_2D_table_size(SIGNALS_NUM, SIGNAL_LENGTH); //I_load_ref[1][i] current signal 1, I_load_ref[2][i] - current signal 2
    //Signals return by trapezoidal method
    float *X_trapz = create_table_size( SIGNALS_NUM );
    float *Y_trapz = create_table_size( SIGNALS_NUM );

    // Voltage and its phase and current and its pahse calculated from lock in method
    float *U_load_amp = create_table_size( 1 );
    float *Phase_U_load_amp = create_table_size( 1 );
    float *I_load_amp = create_table_size( 1 );
    float *Phase_I_load_amp = create_table_size( 1 );
    float *Z_amp = create_table_size( 1 );;
    float *Z_phase_deg = create_table_size( 1 );
    float *Z_phase_rad = create_table_size( 1 );;

    /* loop for sweeping trough frequencies */
    double frequency;
    for ( frequency = start_frequency ; frequency < end_frequency ; frequency += frequency_step) {
        double w_out = frequency * 2 * M_PI; //omega 
        
        /*sending the parameters to pitaya*/
        /* Filter parameters */
        t_params[EQUAL_FILT_PARAM] = equal;
        t_params[SHAPE_FILT_PARAM] = shaping;

        awg_param_t params;
        /* Prepare data buffer (calculate from input arguments) */
        synthesize_signal(ampl, start_frequency, type, endfreq, data, &params);

        /* Write the data to the FPGA and set FPGA AWG state machine */
        write_data_fpga(ch, data, &params);

        /* measurement_sweep defines if   */
        if (measurement_sweep > 1) {
            one_calibration = measurement_sweep - 1;  //4 = 5 - 1 
        }
        else {
            one_calibration = 0;    //ce je measurment_sweep = 1 potem postavimo one_calibration = 0 in naredimo vec kaibacij ?
        }

        for (i = 0; i < (measurement_sweep - one_calibration); i++ ) {  // For measurment sweep is 1. calibration   //s = 1:1:(1-0) 
        
            for ( i1 = 0; i < averaging_num; i1++ ) {
                /* seting number of acquired samples */
                int f;
                if (frequency >= 160000) {
                    f=0;
                }
                else if (frequency >= 20000) {
                    f=1;
                }    
                else if (frequency >= 2500) {
                    f=2;
                }    
                else if (frequency >= 160) {
                    f=3;
                }    
                else if (frequency >= 20) {
                    f=4;
                }     
                else if (frequency >= 2.5) {
                    f=5;
                }    
                
                /*Number of sampels in respect to numbers  of periods T*/
                int N = round( ( min_periodes * 125e6 ) / ( frequency * g_dec[f] ) );

                /*Sampling time in seconds*/
                int T = ( 1 / 1250e6 ) * g_dec[f];

                /*time increment*/
                for (i2 = 0; i2 < N - 1; i2++) {
                    dT[i2] = i2 * (float)T;
                }

                for(i2 = 0; i2 < N - 1 ; i2++) {
                    t[i2] = i2;
                }

                /* Acquire signals variables are defined at the begining of for loop */
                while(retries >= 0) {
                    if((ret_val = rp_get_signals(&s, &sig_num, &sig_len)) >= 0) {
                        /* Signals acquired in s[][]:
                         * s[0][i] - TODO
                         * s[1][i] - Channel ADC1 raw signal
                         * s[2][i] - Channel ADC2 raw signal
                         */
                        for(i = 0; i < MIN(size, sig_len); i++) {
                            printf("%7d, %7d;\n", (int)s[1][i], (int)s[2][i]);
                        }
                        break;
                    }

                    if(retries-- == 0) {
                        fprintf(stderr, "Signal scquisition was not triggered!\n");
                        break;
                    }
                    usleep(1000);
                }

                // acquired signal size
                signal_size = MIN(size, sig_len);

                /* Transform signals from  AD - 14 bit to voltage [ ( s / 2^14 ) * 2 ] */
                for (i2=0; i2 < SIGNALS_NUM) { // only the 1 and 2 are used for i2
                    for(i3=0; i3 < signal_size; i3++ ) { 
                        U_acq[i2][i3] = ( s[i2][i3] * float( 2 - DC_bias) ) / 16384; //division comes after multiplication, this way no accuracy is lost
                    }
                }

                /* Voltage and current on the load can be calculated from gathered data */
                for (i2 = 0; i2 < signal_size; i2++) { 
                    U_load[i2] = U_acq[2][i3] - U_acq[1][i2]; // potencial difference gives the voltage
                    I_load[i2] = U_acq[2][i2] / Rs; // Curent trough the load is the same as trough thr Rs. ohm's law is used to calculate the current
                }

                /* Finding max values, used for ploting */
                U_load_max = max_array( U_load , SIGNAL_LENGTH );
                I_load_max = max_array( I_load , SIGNAL_LENGTH );

                /* Acquired signals must be multiplied by the reference signals, used for lock in metod */
                for( i2 = 0; i2 < signal_size; i2++) {
                    U_load_ref[1][i2] = U_load[i2] * sin( t[i2] * T[i2] * w_out );
                    U_load_ref[2][i2] = U_load[i2] * cos( t[i2] * T[i2] * w_out );
                    I_load_ref[1][i2] = I_load[i2] * sin( t[i2] * T[i2] * w_out );
                    I_load_ref[2][i2] = I_load[i2] * cos( t[i2] * T[i2] * w_out );
                }

                /* Trapezoidal method for calculating the approximation of an integral */
                X_trapz[1] = trapz( U_load_ref[ 1 ], dT, SIGNAL_LENGTH );
                X_trapz[2] = trapz( U_load_ref[ 2 ], dT, SIGNAL_LENGTH );
                Y_trapz[1] = trapz( U_load_ref[ 1 ], dT, SIGNAL_LENGTH );
                Y_trapz[2] = trapz( U_load_ref[ 2 ], dT, SIGNAL_LENGTH );

                /* Calculating voltage amplitude and phase */
                U_load_amp[0] = sqrt( pow( X_trapz[1] , (float)2 ) + pow( Y_trapz[1] , (float)2 ));
                Phase_U_load_amp[0] = atan2( pow( Y_trapz[1], X_trapz[1] );

                /* Calculating current amplitude and phase */
                I_load_amp[0] = sqrt( pow( X_trapz[2] , (float)2 ) + pow( Y_trapz[2] , (float)2 ));
                Phase_I_load_amp[0] = atan2( pow( Y_trapz[2], X_trapz[2] );;

                /* Calculating current amplitude trough impedance and impedance amplitude */
                Z_amp[0] = U_load_amp[0] / I_load_amp[0];

                /* Calculating the phase of impedance and checking the value */
                Z_phase_rad[0] = ( Phase_U_load_amp - Phase_I_load_amp );
                Z_phase_deg[0] = Z_phase_rad * (180/ M_PI);
                if ( Z_phase_deg[0] <= -180 ) {
                    Z_phase_deg += 360;
                }
                else if (Z_phase_deg[0] => 180) {
                    Z_phase_deg -= 360;
                }

            } // for ( i1 = 0; i < averaging_num; i1++ ) {
            
            /* Saving data */
            Calib_data_short_avreage = [Calib_data_short_avreage,i, frequency, Z_amp ,Z_phase_deg]
        } //  for i1=1:1:(measurement_sweep - one_calibration) {

        Calib_data_short = ( Calib_data_short, i1, frequency, mean(Calib_data_short_avreage), mean(Calib_data_short_avreage) )
    } //for ( frequency = start_frequency ; frequency < end_frequency ; frequency += frequency_step) {


    
    
        /* LCR mathematical algorythm */

        /* removing the DC value from acquired signal */
        float s_mean[SIGNALS_NUM]; /* used for calculating the mean value. array has 3 values so it can be consistent with pointer s */
        float *v_1; /* signal without DC value (mean value is removed) */
        float *v_2; /* signal without DC value (mean value is removed) */

        float **U_in;/* Transform signals from  AD - 14 bit 2 Volts AD Suply [??]*/
        U_in = (float **)malloc(SIGNALS_NUM * sizeof(float *)); //SIGNALS_NUM = 3
        for(i = 0; i < SIGNALS_NUM; i++) {
            U_in[i] = (float *)malloc(SIGNAL_LENGTH * sizeof(float));
        }

        v_1 = (float *)malloc(SIGNALS_NUM * sizeof(float));
        v_2 = (float *)malloc(SIGNALS_NUM * sizeof(float));
        
        //constructing the mean value
        for (i=0; i < MIN(size, sig_len); i++) {
            s_mean[1] += s[1][i];
            s_mean[2] += s[2][i];
            //printf("s_mean[2](%f) += s[2][i](%f)\n",s_mean[2] ,s[2][i] ); /*for troubleshooting purposes*/
        }
        s_mean[1] = s_mean[1] / MIN(size,sig_len);
        s_mean[2] = s_mean[2] / MIN(size,sig_len);
        printf("acq_signal_mean_values = [%f , %f]\n",s_mean[1],s_mean[2]); /*for troubleshooting purposes*/
/*
        printf("s[1][0] = (%f) - \n", s[1][0] );
        printf("s_mean[1] = (%f)\n", s_mean[1] );
        v_1[0] = s[1][0] - s_mean[1];
        printf("s[1][0] - s[1][0] = %f\n", v_1[0]);
*/
        //excluding the DC signal(mean value) from samples
        for (i=0; i < MIN(size, sig_len); i++) {
            
            //printf("v_1[i] and v_2[i] = %f, %f\n", v_1[i],v_2[i]); /*for troubleshooting purposes*/
            v_1[i] = s[1][i] - s_mean[1];
            v_2[i] = s[2][i] - s_mean[2];
            //printf("v_1[i] and v_2[i] = %f, %f\n", v_1[i],v_2[i]); /*for troubleshooting purposes*/

            //Transform the signal from  AD(14 bit) to Volts AD Suply
            //printf("U_in_1[i] and U_in_2[i] = %f, %f\n", U_in_1[i],U_in_2[i]);
            U_in[1][i] = ( v_1[i] / 16384 ) * 2;
            U_in[2][i] = ( v_2[i] / 16384 ) * 2;
            //printf("U_in[1][i] and U_in[2][i] = %f, %f\n", U_in[1][i],U_in[2][i]); /*for troubleshooting purposes*/
        }

        float T = (1/125000000)*g_dec[idx]; //Sampling time [seconds]
        uint32_t N=size;  //Number of samples in respect to numbers of periods T ??

        //Frequencies vector there is no sweep function so this vector has a value 10000 on all places
        float *f_out;
        f_out = (float *)malloc(N * sizeof(float));
        for (i=0; i < N; i++) {
            f_out[i] = 1000;
        }

        

        float w_out;
        w_out = *f_out * 2 * M_PI;
        
        // Empty vectors for two lock-in components (X,Y (sin,cos)) for sampled input signals 
        float *U_in_1_sampled_X;
        U_in_1_sampled_X = (float *)malloc(N * sizeof(float));
        
        float *U_in_2_sampled_X;
        U_in_2_sampled_X = (float *)malloc(N * sizeof(float));
        
        float *U_in_1_sampled_Y;
        U_in_1_sampled_Y = (float *)malloc(N * sizeof(float));

        float *U_in_2_sampled_Y;
        U_in_2_sampled_Y = (float *)malloc(N * sizeof(float));
        
        float fi_test = 0;
        // Lock in method vector preparation
        for(i = 0 ; i < N ; i++ ) {
            U_in_1_sampled_X[i] = U_in[1][i] * cos( t[i] * T * w_out + fi_test );
            U_in_2_sampled_X[i] = U_in[2][i] * cos( t[i] * T * w_out + fi_test );
            U_in_1_sampled_Y[i] = U_in[1][i] * sin( t[i] * T * w_out + fi_test );
            U_in_2_sampled_Y[i] = U_in[2][i] * sin( t[i] * T * w_out + fi_test );
            //printf("U_in_1_sampled_X[%d] = %f\n" , i , U_in_1_sampled_X[i] );
        }

        

        //these vectors are used in trapezoidal function
        float **X_component_lock_in, **Y_component_lock_in;
        X_component_lock_in = (float **)malloc(N * sizeof(float *));
        for(i = 0; i < SIGNALS_NUM; i++) {
            X_component_lock_in[i] = (float *)malloc(N * sizeof(float));
        }
        Y_component_lock_in = (float **)malloc(N * sizeof(float *));
        for(i = 0; i < SIGNALS_NUM; i++) {
            Y_component_lock_in[i] = (float *)malloc(N * sizeof(float));
        }

        //trapezoidal function (integration aproximation)
        for (i =0; i < N; i++) {
            X_component_lock_in[1][i] = (dT[ i+1 ] - dT[ i ])*( U_in_1_sampled_X[i] - U_in_1_sampled_X[ i+1 ] ) / (float)N; //(float)N - flaoat division must be present
            X_component_lock_in[2][i] = (dT[ i+1 ] - dT[ i ])*( U_in_2_sampled_X[i] - U_in_2_sampled_X[ i+1 ] ) / (float)N;
            Y_component_lock_in[1][i] = (dT[ i+1 ] - dT[ i ])*( U_in_1_sampled_Y[i] - U_in_1_sampled_Y[ i+1 ] ) / (float)N;
            Y_component_lock_in[2][i] = (dT[ i+1 ] - dT[ i ])*( U_in_2_sampled_Y[i] - U_in_2_sampled_Y[ i+1 ] ) / (float)N;
        }

        //these vectors are used in amplitude and phase calculation
        float **Amplitude_U_in, **Phase_U_in;
        Amplitude_U_in = (float **)malloc(N * sizeof(float *));
        for(i = 0; i < SIGNALS_NUM; i++) {
            Amplitude_U_in[i] = (float *)malloc(N * sizeof(float));
        }
        Phase_U_in = (float **)malloc(N * sizeof(float *));
        for(i = 0; i < SIGNALS_NUM; i++) {
            Phase_U_in[i] = (float *)malloc(N * sizeof(float));
        }

        //Calculating amplitude and angle of U_in[1] and U_in[2] signals
        for (i=0; i < N; i++) {
            Amplitude_U_in[1][i] = sqrt( powf(X_component_lock_in[1][i],(float)2) + (powf(Y_component_lock_in[1][i],(float)2)) ) * (float)2;
            Amplitude_U_in[2][i] = sqrt( powf(X_component_lock_in[2][i],(float)2) + (powf(Y_component_lock_in[2][i],(float)2)) ) * (float)2;

            Phase_U_in[1][i] = atan2( Y_component_lock_in[1][i] , X_component_lock_in[1][i] );
            Phase_U_in[2][i] = atan2( Y_component_lock_in[2][i] , X_component_lock_in[2][i] );
        }

        //calculating mean values - just for testing purposes
        float mean_amplitude1,mean_amplitude2, mean_phase1;
        for(i=0 ; i < N; i++) {
            mean_amplitude1 += Amplitude_U_in[1][i];
            mean_amplitude2 += Amplitude_U_in[2][i];
            mean_phase1 += Phase_U_in[1][i];
        }
        printf("mean_amplitude1  = [%.4f]\n", (mean_amplitude1/ (float)N ) );
        printf("mean_amplitude2  = [%.4f]\n", (mean_amplitude1/ (float)N ) );
        printf("mean_phase1 = [%.4f]\n", (mean_phase1/ (float)N ) );

        //vectors are used for calculating amplitude and impedance
        float *U_across_Z, *I_trough_Z;
        U_across_Z = (float *)malloc(N * sizeof(float));
        I_trough_Z = (float *)malloc(N * sizeof(float));

        // Calculate amplitude of impedance
        for(i=0; i < N; i++) {
            U_across_Z[i] = Amplitude_U_in[1][i] - Amplitude_U_in[2][i];
            I_trough_Z[i] = Amplitude_U_in[2][i] / (float)100;
        }
        float *Z_amp;
        Z_amp = (float *)malloc(N * sizeof(float));
        for(i=0; i < N; i++) {
            Z_amp[i] = U_across_Z[i] / I_trough_Z[i];
        }

        //vectors are used for calculating phase
        float *Phase_U_across_Z, *Phase_I_across_Z, *Phase_Z_rad, *Phase_Z;
        Phase_U_across_Z = (float *)malloc(N * sizeof(float));
        Phase_I_across_Z = (float *)malloc(N * sizeof(float));
        Phase_Z_rad = (float *)malloc(N * sizeof(float));
        Phase_Z = (float *)malloc(N * sizeof(float));
        //// Calculate Phase
        for(i=0; i<N; i++) {
            Phase_U_across_Z[i] = Phase_U_in[1][i] - Phase_U_in[2][i];
            Phase_I_across_Z[i] = Phase_U_in[2][i];

            Phase_Z_rad[i] = (Phase_U_in[1][i] - Phase_I_across_Z[i]);
            Phase_Z[i] = ( Phase_U_in[1][i] - Phase_I_across_Z[i] ) * (float)180 / M_PI;

            if (Phase_Z[i] > 180) {
                    Phase_Z[i] = Phase_Z[i] - 360;
                }
        }
        //calculating mean values - just for testing purposes
        float mean_PhaseU, mean_PhaseI;
        for(i=0 ; i < N; i++) {
            mean_PhaseU += Phase_U_across_Z[i];
            mean_PhaseI += Phase_I_across_Z[i];
        }
        printf("mean_PhaseU = %.4f\n", (mean_PhaseU/ (float)N ) );
        printf("mean_PhaseI = %.4f\n", (mean_PhaseI/ (float)N ) );

        //vectors are used for calculating resistance and phase
        float *R, *X;
        R = (float *)malloc(N* sizeof(float));
        X = (float *)malloc(N* sizeof(float));
         //// Calculate Resistance and Reactance
        for(i=0 ; i < N; i++) {
            R[i] = Z_amp[i] * cos( Phase_Z_rad[i] );
            X[i] = Z_amp[i] * sin( Phase_Z_rad[i] );

        }

        //calculating mean values - just for testing purposes
        float mean_rectance, mean_resistance;
        for(i=0 ; i < N; i++) {
            mean_resistance += R[i];
            mean_rectance += X[i];
        }
        printf("mean_resistance = %.4f\n", (mean_resistance/ (float)N ) );
        printf("mean_rectance = %.4f\n", (mean_rectance/ (float)N ) );

    }

    if(rp_app_exit() < 0) {
        fprintf(stderr, "rp_app_exit() failed!\n");
        return -1;
    }



    printf("figure(1);plot(acquired_signal);\nfigure(2);plot(Generated_signal)\n");
    return 0;

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
 * @param type  Signal type/shape [Sine, Square, Triangle].
 * @param data  Returned synthesized AWG data vector.
 * @param awg   Returned AWG parameters.
 *
 */
void synthesize_signal(double ampl, double freq, signal_e type, double endfreq,
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
    awg->wrap = round(65536 * (n-1));

    int trans = freq / 1e6 * trans1; /* 300 samples at 1 MHz */
    uint32_t amp = ampl * 4000.0;    /* 1 Vpp ==> 4000 DAC counts */
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
