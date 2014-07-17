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
 */

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

/** Oscilloscope module parameters as defined in main module
 * @see rp_main_params
 */
float t_params[PARAMS_NUM] = { 0, 1e6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

/** Max decimation index */
#define DEC_MAX 6

/** Decimation translation table */
static int g_dec[DEC_MAX] = { 1,  8,  64,  1024,  8192,  65536 };

/* Forward declarations */
void synthesize_signal(double ampl, double freq, signal_e type, double endfreq,
                       int32_t *data,
                       awg_param_t *params);
void write_data_fpga(uint32_t ch,
                     const int32_t *data,
                     const awg_param_t *awg);

/** Print usage information */
void usage() {

    const char *format =
        "%s version %s-%s\n"
        "\n"
        "Usage: %s   channel amplitude frequency <type> <end frequency>\n"
        "\n"
        "\tchannel     Channel to generate signal on [1, 2].\n"
        "\tamplitude   Peak-to-peak signal amplitude in Vpp [0.0 - %1.1f].\n"
        "\tfrequency   Signal frequency in Hz [%2.1f - %2.1e].\n"
        "\ttype        Signal type [sine, sqr, tri, sweep].\n"
        "\tend frequency   Sweep-to frequency in Hz [%2.1f - %2.1e].\n"
        "\n";

    fprintf( stderr, format, g_argv0, VERSION_STR, REVISION_STR,
             g_argv0, c_max_amplitude, c_min_frequency, c_max_frequency);
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
    int i;
        for(i = 0; i < num_of_rows; i++) {
            new_table[i] = create_table_size(num_of_cols);
        }
    return new_table;
}

float max_array(float *arrayptr, int numofelements) {
  int i = 0;
  float max = -100000;//seting the minimum value possible

  for(i = 0; i < numofelements; i++)
  {
    if(max < arrayptr[i])
    {
      max = arrayptr[i];
    }
  }
  return max;
}

float trapz(float *arrayptr, float *dT, int size) {
  float result;
  int i;
  //printf("size = %d\n", size);
  for (i =0; i < size ; i++) {
    result += fabsf((dT[ i+1 ] - dT[ i ]) * ( arrayptr[i] - arrayptr[ i+1 ] )/(float)2);
  }
    return result;
}

float mean_array(float *arrayptr, int numofelements) {
  int i = 1;
  float mean = 0;

  for(i = 0; i < numofelements; i++)
  {
    mean += arrayptr[i];
  }

  mean = mean / numofelements;
  return mean;
}

float mean_array_column(float **arrayptr, int length, int column) {
    float result;
    int i;
    for(i=0; i < length; i++) {
        result += arrayptr[i][column];
        printf("result = %f\n", result);
    }
    //printf("return = %f\n",(result / length) );
    result = result / length;
    return result;
}

/** Signal generator main */
int main(int argc, char *argv[])
{
    /* argument check */
    g_argv0 = argv[0];    
    /* all arguments are hardcoded in program and do not need to be acquired from the user
    if ( argc < 4 ) {

        usage();
        return -1;
    }
    */
    /* Channel argument parsing */
    //uint32_t ch = atoi(argv[1]) - 1; /* Zero based internally */
    uint32_t ch = 0;
    if (ch > 1) {
        fprintf(stderr, "Invalid channel: %s\n", argv[1]);
        usage();
        return -1;
    }

    /* Signal amplitude argument parsing */
    //double ampl = strtod(argv[2], NULL);
    double ampl = 2;
    if ( (ampl < 0.0) || (ampl > c_max_amplitude) ) {
        fprintf(stderr, "Invalid amplitude: %s\n", argv[2]);
        usage();
        return -1;
    }

    /* Signal frequency argument parsing */
    //double freq = strtod(argv[3], NULL);
    /* Frequencies set for later use in the program. Start, end and step frequencies are defined by the user */
    double start_frequency = 10000;
    double frequency_step =  10000;
    double end_frequency =   30000;
    double endfreq = 0;/* endfreq is used in prebuild sweep program and is not needed in lcr meter because sweep is defined in the for loop */
    double frequency; //frequency in a for loop

    /* Check frequency limits */
    if ( (start_frequency < c_min_frequency) || (start_frequency > c_max_frequency ) ) {
        fprintf(stderr, "Invalid frequency: %s\n", argv[3]);
        usage();
        return -1;
    }

    /* Argument set user. If there is no frequency sweep, measurement sweep is used  */
    double measurement_sweep = 2;

    /* if one wants to skip calibration the parameter can be set to 0 */
    int one_calibration;

    /* Number of measurments made and are later averaged */
    uint32_t averaging_num = 4; // Sequence takes more time and the result are more stable results (not more accurate)

    if (argc > 5) {
        endfreq = strtod(argv[5], NULL);
    }

    /* Signal type argument parsing */
    signal_e type = eSignalSine;
    if (argc > 4) {
        if ( strcmp(argv[4], "sine") == 0) {
            type = eSignalSine;
        } else if ( strcmp(argv[4], "sqr") == 0) {
            type = eSignalSquare;
        } else if ( strcmp(argv[4], "tri") == 0) {
            type = eSignalTriangle;
        } else if ( strcmp(argv[4], "sweep") == 0) {
            type = eSignalSweep;   
        } else {
            fprintf(stderr, "Invalid signal type: %s\n", argv[4]);
            usage();
            return -1;
        }
    }

    /* initializing the variables and pointers for acquire functionality */
    uint32_t size = 16384;
    float **s = create_2D_table_size(SIGNALS_NUM, SIGNAL_LENGTH);
    //float **d = create_2D_table_size(SIGNALS_NUM, SIGNAL_LENGTH);
    int sig_num, sig_len;
    int j, i, i1, i2, i3;
    int ret_val;
    int retries = 150000;
    int equal = 0;
    int shaping = 0;
    
    /* Parameters used by the algorythm */
    uint32_t DC_bias = 0;
    int f = 0; // used in for lop for seting the decimation

    uint32_t Rs = 1000;//TODO User defines this reference resirtor (check the circuit, there are 2 elements one is reference other element is measured)

    uint32_t min_periodes =15; // max 20

    /* Memory allocations for data generated and used in for loops below */
    int N; //Number of samples in respect to numbers of periods T
    float T; // Sampling time in seconds
    float *dT = create_table_size(16384); // time increment
    float *t = create_table_size(16384);
    /* Acquired signal is concverted to voltage and stored in U_acq */
    float **U_acq = create_2D_table_size(SIGNALS_NUM, SIGNAL_LENGTH);
    /* Used for storing the Voltage and current on the load */
    float *U_load = create_table_size( SIGNAL_LENGTH );
    float *I_load = create_table_size( SIGNAL_LENGTH );
    /* Signals multiplied by the reference signal (sin) */
    float **U_load_ref = create_2D_table_size(SIGNALS_NUM, SIGNAL_LENGTH); //U_load_ref[1][i] voltage signal 1, U_load_ref[2][i] - voltage signal 2
    float **I_load_ref = create_2D_table_size(SIGNALS_NUM, SIGNAL_LENGTH); //I_load_ref[1][i] current signal 1, I_load_ref[2][i] - current signal 2
    /* Signals return by trapezoidal method in complex */
    float *X_trapz = create_table_size( SIGNALS_NUM );
    float *Y_trapz = create_table_size( SIGNALS_NUM );
    /* Voltage, current and their phases calculated */
    float U_load_amp;
    float Phase_U_load_amp;
    float I_load_amp;
    float Phase_I_load_amp;
    float complex Z;
    float Z_phase_deg_imag;  // may cuse errors because not complex
    /* calibrtion results short circuited */
    float **Calib_data_short_avreage = create_2D_table_size((averaging_num + 1), averaging_num ); //appendin 4 data values
    float **Calib_data_short  = create_2D_table_size(averaging_num, 4); //appendin 4 data values
    /* calibrtion results short circuited */
    float **Calib_data_open_avreage = create_2D_table_size((averaging_num + 1), averaging_num ); //appendin 4 data values
    float **Calib_data_open = create_2D_table_size(averaging_num, 4); //appendin 4 data values
    /* calibrtion results open circuited */
    //float **Calib_data_open_avreage = create_2D_table_size((averaging_num + 1), averaging_num ); //appendin 4 data values
    //float **Calib_data_open  = create_2D_table_size(averaging_num, 4); //appendin 4 data values
    double w_out;
    /* Filter parameters for signal Acquire */
    t_params[EQUAL_FILT_PARAM] = equal;
    t_params[SHAPE_FILT_PARAM] = shaping;
    /* Initialization of Oscilloscope application for signal Acquire */
    if(rp_app_init() < 0) {
        fprintf(stderr, "rp_app_init() failed!\n");
        return -1;
    }

     /* Setting of parameters in Oscilloscope main module for signal Acquire */
    if(rp_set_params((float *)&t_params, PARAMS_NUM) < 0) {
        fprintf(stderr, "rp_set_params() failed!\n");
        return -1;
    }

    printf("initiate calibration sequence? [1|0] :");
    if (fscanf(stdin, "%d", &one_calibration) < 0) {
        printf("problem with fscaning calibration confirmation\n");
        return -1;
    }
    if (one_calibration == 1) {
        printf("initiating calibration sequence\n");

        /* Calibration sequence */

        /* program waits for the user to make a short connection */
        int calibration_continue;
        while(1) {
          printf("Open connection calibration. please connect the wires correctly and press 1 and return. Continue? [1 = yes|0 = exit lcr ] :");
          if (fscanf(stdin, "%d", &calibration_continue) > 0) 
          {
            if(calibration_continue == 1)  {
                calibration_continue = 0;
                break;
            }
            else if(calibration_continue == 0)  {
                return -1;
            }
            else {} //ask again
          }
          else {
            printf("error when readnig from stdinput (scanf)\n");
            return -1;
          }
        }


        /* LCR algorythm for calibration in open cicuited mode */
        for ( frequency = start_frequency; frequency < end_frequency; frequency += frequency_step) {

            w_out = frequency * 2 * M_PI; //omega 

                /* Signal generator sequence */
                awg_param_t params;
                /* Prepare data buffer (calculate from input arguments) */

                synthesize_signal(ampl, frequency, type, endfreq, data, &params);

                /* Write the data to the FPGA and set FPGA AWG state machine */
                write_data_fpga(ch, data, &params);

            /* measurement_sweep defines if */
            if (measurement_sweep > 1) {
                one_calibration = measurement_sweep - 1;  //4 = 5 - 1 
            }
            else {
                one_calibration = 0;    //ce je measurment_sweep = 1 potem postavimo one_calibration = 0 in naredimo vec kaibacij ?
            }

            for (i = 0; i < (measurement_sweep - one_calibration); i++ ) {  // For measurment sweep is 1. calibration   //s = 1:1:(1-0) 

                for ( i1 = 0; i1 < averaging_num; i1++ ) {
                    /* seting number of acquired samples */
                    
                    if (frequency >= 160000) {
                        f=0;
                        printf("f = 0\n");
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
                    /* Signal acquire */
                    while(retries >= 0) {  
                        if((ret_val = rp_get_signals(&s, &sig_num, &sig_len)) >= 0) {
                            /* Signals acquired in s[][]:
                             * s[0][i] - TODO
                             * s[1][i] - Channel ADC1 raw signal
                             * s[2][i] - Channel ADC2 raw signal
                            */
                            for(j = 0; j < MIN(size, sig_len); j++) {
                               //printf("%7d %7d\n", (int)s[1][i], (int)s[2][i]);
                            }
                            break;
                        }
                        if(retries-- == 0) {
                            fprintf(stderr, "Signal scquisition was not triggered!\n");
                            break;
                        }
                        usleep(1000);
                    }

                    /* Transform signals from  AD - 14 bit to voltage [ ( s / 2^14 ) * 2 ] */
                    for (i2 = 0; i2 < SIGNALS_NUM; i2++) { // only the 1 and 2 are used for i2
                        for(i3=0; i3 < size; i3++ ) { 
                            U_acq[i2][i3] = ( s[i2][i3] * (float)( 2 - DC_bias ) ) / 16384; //division comes after multiplication, this way no accuracy is lost
                        }
                    }

                    /* Voltage and current on the load can be calculated from gathered data */
                    for (i2 = 0; i2 < size; i2++) { 
                        U_load[i2] = U_acq[2][i3] - U_acq[1][i2]; // potencial difference gives the voltage
                        I_load[i2] = U_acq[2][i2] / Rs; // Curent trough the load is the same as trough thr Rs. ohm's law is used to calculate the current
                    }

                    /* Finding max values, used for ploting */
                    /* COMENTED BECAUSE NOT USED
                    U_load_max = max_array( U_load , SIGNAL_LENGTH );
                    I_load_max = max_array( I_load , SIGNAL_LENGTH );
                    */

                    //setting decimtion
                    t_params[TIME_RANGE_PARAM] = f;
                        
                    N = round( ( min_periodes * 125e6 ) / ( frequency * g_dec[f] ) );

                    T = ( g_dec[f] / 125e6 );

                    for (i2 = 0; i2 < (N - 1); i2++) {
                        dT[i2] = i2 * (float)T;
                    }

                    for(i2 = 0; i2 < (N - 1); i2++) {
                        t[i2] = i2;
                    }

                    /* Acquired signals must be multiplied by the reference signals, used for lock in metod */
                    for( i2 = 0; i2 < size; i2++) {
                        U_load_ref[1][i2] = U_load[i2] * sin( t[i2] * T * w_out );
                        U_load_ref[2][i2] = U_load[i2] * cos( t[i2] * T * w_out );
                        I_load_ref[1][i2] = I_load[i2] * sin( t[i2] * T * w_out );
                        I_load_ref[2][i2] = I_load[i2] * cos( t[i2] * T * w_out );
                    }

                    /* Trapezoidal method for calculating the approximation of an integral */
                    X_trapz[1] = trapz( U_load_ref[ 1 ], dT, SIGNAL_LENGTH );
                    X_trapz[2] = trapz( U_load_ref[ 2 ], dT, SIGNAL_LENGTH );
                    Y_trapz[1] = trapz( U_load_ref[ 1 ], dT, SIGNAL_LENGTH );
                    Y_trapz[2] = trapz( U_load_ref[ 2 ], dT, SIGNAL_LENGTH );


                    /* Calculating voltage amplitude and phase */
                    U_load_amp = sqrtf( pow( X_trapz[1] , (float)2 ) + pow( Y_trapz[1] , (float)2 ));
                    Phase_U_load_amp = atan2f( Y_trapz[1], X_trapz[1] );

                    /* Calculating current amplitude and phase */
                    I_load_amp = sqrtf( pow( X_trapz[2] , (float)2 ) + pow( Y_trapz[2] , (float)2 ));
                    Phase_I_load_amp = atan2f( Y_trapz[2], X_trapz[2] );

                    /* Asigning impedance  values (complex value) */
                    Z = (U_load_amp / I_load_amp) + ( Phase_U_load_amp - Phase_I_load_amp ) * I;
                    //printf("Z = %.2f %+.2fi\n", creal(Z), cimag(Z));
                    
                    Z_phase_deg_imag = cimag(Z) * (180 / M_PI);
                    if ( Z_phase_deg_imag <= -180 ) {
                        Z_phase_deg_imag += 360;
                    }
                    else if (Z_phase_deg_imag <= 180) {
                        Z_phase_deg_imag -= 360;
                    }


                } // for ( i1 = 0; i < averaging_num; i1++ ) {
                /* Saving data */
                Calib_data_short_avreage[i1][0] = i1;
                Calib_data_short_avreage[i1][1] = frequency;
                Calib_data_short_avreage[i1][2] = creal(Z);
                Calib_data_short_avreage[i1][3] = cimag(Z);
                //printf("real Z[%d][2] = %f\n",i1, Calib_data_short_avreage[i1][2]);
                //printf("imag Z[%d][2] = %f\n",i1, Calib_data_short_avreage[i1][3]);
                //printf("Imag Z[%d][3] = %f\n",i1, Calib_data_short_avreage[i1][3]);
            } // for (i = 0; i < (measurement_sweep - one_calibration); i++ ) { 
            Calib_data_short[i][0] = i;
            Calib_data_short[i][1] = frequency;
            Calib_data_short[i][2] = mean_array_column(Calib_data_short_avreage, averaging_num, 2); // mean value of real impedance
            Calib_data_short[i][3] = mean_array_column(Calib_data_short_avreage, averaging_num, 3); // mean value of imaginary impedance
            //printf("mean_array_column(%f, %d, 2) = %f \n",Calib_data_short_avreage[i1][2] ,averaging_num, (mean_array_column(Calib_data_short_avreage, averaging_num, 2)));
            printf("avr real closed Z[%d][2] = %f\n",i, Calib_data_short[i][2]); 
            printf("avr imag closed Z[%d][3] = %f\n",i, Calib_data_short[i][3]);       
        } //for ( frequency = start_frequency ; frequency < end_frequency ; frequency += frequency_step) {


        /* User prompt short connection calibration */
        while(1) {
          printf("Open connection calibration. please connect the wires correctly and press 1 and return. Continue? [1 = yes|0 = exit lcr ] :");
          if (fscanf(stdin, "%d", &calibration_continue) > 0) 
          {
            if(calibration_continue == 1)  {
                calibration_continue = 0;
                break;
            }
            else if(calibration_continue == 0)  {
                return -1;
            }
            else {} //ask again
          }
          else {
            printf("error when readnig from stdinput (scanf)\n");
            return -1;
          }
        }




        /* LCR algorythm for calibration in open cicuited mode */
        for ( frequency = start_frequency; frequency < end_frequency; frequency += frequency_step) {

            w_out = frequency * 2 * M_PI; //omega 

                /* Signal generator sequence */
                awg_param_t params;
                /* Prepare data buffer (calculate from input arguments) */

                synthesize_signal(ampl, frequency, type, endfreq, data, &params);

                /* Write the data to the FPGA and set FPGA AWG state machine */
                write_data_fpga(ch, data, &params);

            /* measurement_sweep defines if */
            if (measurement_sweep > 1) {
                one_calibration = measurement_sweep - 1;  //4 = 5 - 1 
            }
            else {
                one_calibration = 0;    //ce je measurment_sweep = 1 potem postavimo one_calibration = 0 in naredimo vec kaibacij ?
            }

            for (i = 0; i < (measurement_sweep - one_calibration); i++ ) {  // For measurment sweep is 1. calibration   //s = 1:1:(1-0) 

                for ( i1 = 0; i1 < averaging_num; i1++ ) {
                    /* seting number of acquired samples */
                    
                    if (frequency >= 160000) {
                        f=0;
                        printf("f = 0\n");
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
                    /* Signal acquire */
                    while(retries >= 0) {  
                        if((ret_val = rp_get_signals(&s, &sig_num, &sig_len)) >= 0) {
                            /* Signals acquired in s[][]:
                             * s[0][i] - TODO
                             * s[1][i] - Channel ADC1 raw signal
                             * s[2][i] - Channel ADC2 raw signal
                            */
                            for(j = 0; j < MIN(size, sig_len); j++) {
                               //printf("%7d %7d\n", (int)s[1][i], (int)s[2][i]);
                            }
                            break;
                        }
                        if(retries-- == 0) {
                            fprintf(stderr, "Signal scquisition was not triggered!\n");
                            break;
                        }
                        usleep(1000);
                    }

                    /* Transform signals from  AD - 14 bit to voltage [ ( s / 2^14 ) * 2 ] */
                    for (i2 = 0; i2 < SIGNALS_NUM; i2++) { // only the 1 and 2 are used for i2
                        for(i3=0; i3 < size; i3++ ) { 
                            U_acq[i2][i3] = ( s[i2][i3] * (float)( 2 - DC_bias ) ) / 16384; //division comes after multiplication, this way no accuracy is lost
                        }
                    }

                    /* Voltage and current on the load can be calculated from gathered data */
                    for (i2 = 0; i2 < size; i2++) { 
                        U_load[i2] = U_acq[2][i3] - U_acq[1][i2]; // potencial difference gives the voltage
                        I_load[i2] = U_acq[2][i2] / Rs; // Curent trough the load is the same as trough thr Rs. ohm's law is used to calculate the current
                    }

                    /* Finding max values, used for ploting */
                    /* COMENTED BECAUSE NOT USED
                    U_load_max = max_array( U_load , SIGNAL_LENGTH );
                    I_load_max = max_array( I_load , SIGNAL_LENGTH );
                    */

                    //setting decimtion
                    t_params[TIME_RANGE_PARAM] = f;
                        
                    N = round( ( min_periodes * 125e6 ) / ( frequency * g_dec[f] ) );

                    T = ( g_dec[f] / 125e6 );

                    for (i2 = 0; i2 < (N - 1); i2++) {
                        dT[i2] = i2 * (float)T;
                    }

                    for(i2 = 0; i2 < (N - 1); i2++) {
                        t[i2] = i2;
                    }

                    /* Acquired signals must be multiplied by the reference signals, used for lock in metod */
                    for( i2 = 0; i2 < size; i2++) {
                        U_load_ref[1][i2] = U_load[i2] * sin( t[i2] * T * w_out );
                        U_load_ref[2][i2] = U_load[i2] * cos( t[i2] * T * w_out );
                        I_load_ref[1][i2] = I_load[i2] * sin( t[i2] * T * w_out );
                        I_load_ref[2][i2] = I_load[i2] * cos( t[i2] * T * w_out );
                    }

                    /* Trapezoidal method for calculating the approximation of an integral */
                    X_trapz[1] = trapz( U_load_ref[ 1 ], dT, SIGNAL_LENGTH );
                    X_trapz[2] = trapz( U_load_ref[ 2 ], dT, SIGNAL_LENGTH );
                    Y_trapz[1] = trapz( U_load_ref[ 1 ], dT, SIGNAL_LENGTH );
                    Y_trapz[2] = trapz( U_load_ref[ 2 ], dT, SIGNAL_LENGTH );


                    /* Calculating voltage amplitude and phase */
                    U_load_amp = sqrtf( pow( X_trapz[1] , (float)2 ) + pow( Y_trapz[1] , (float)2 ));
                    Phase_U_load_amp = atan2f( Y_trapz[1], X_trapz[1] );

                    /* Calculating current amplitude and phase */
                    I_load_amp = sqrtf( pow( X_trapz[2] , (float)2 ) + pow( Y_trapz[2] , (float)2 ));
                    Phase_I_load_amp = atan2f( Y_trapz[2], X_trapz[2] );

                    /* Asigning impedance  values (complex value) */
                    Z = (U_load_amp / I_load_amp) + ( Phase_U_load_amp - Phase_I_load_amp ) * I;
                    //printf("Z = %.2f %+.2fi\n", creal(Z), cimag(Z));
                    
                    Z_phase_deg_imag = cimag(Z) * (180 / M_PI);
                    if ( Z_phase_deg_imag <= -180 ) {
                        Z_phase_deg_imag += 360;
                    }
                    else if (Z_phase_deg_imag <= 180) {
                        Z_phase_deg_imag -= 360;
                    }


                } // for ( i1 = 0; i < averaging_num; i1++ ) {
                /* Saving data */
                Calib_data_open_avreage[i1][0] = i1;
                Calib_data_open_avreage[i1][1] = frequency;
                Calib_data_open_avreage[i1][2] = creal(Z);
                Calib_data_open_avreage[i1][3] = cimag(Z);
                //printf("real Z[%d][2] = %f\n",i1, Calib_data_open_avreage[i1][2]);
                //printf("imag Z[%d][2] = %f\n",i1, Calib_data_open_avreage[i1][3]);
                //printf("Imag Z[%d][3] = %f\n",i1, Calib_data_open_avreage[i1][3]);
            } // for (i = 0; i < (measurement_sweep - one_calibration); i++ ) { 
            Calib_data_open[i][0] = i;
            Calib_data_open[i][1] = frequency;
            Calib_data_open[i][2] = mean_array_column(Calib_data_open_avreage, averaging_num, 2); // mean value of real impedance
            Calib_data_open[i][3] = mean_array_column(Calib_data_open_avreage, averaging_num, 3); // mean value of imaginary impedance
            //printf("mean_array_column(%f, %d, 2) = %f \n",Calib_data_open_avreage[i1][2] ,averaging_num, (mean_array_column(Calib_data_short_avreage, averaging_num, 2)));
            printf("avr real open Z[%d][2] = %f\n",i, Calib_data_open[i][2]); 
            printf("avr imag open Z[%d][3] = %f\n",i, Calib_data_open[i][3]);       
        } //for ( frequency = start_frequency ; frequency < end_frequency ; frequency += frequency_step) {
    } //prompt user for skiping the calibration sequence
    else if ( one_calibration == 0 ) {
        printf("calibration sequence skipped\n");
    }
    else {
        printf("wrong key pressed exiting program...\n");
        return -1;
    }
    
    

    printf("end, yay, no errors!\n");
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
