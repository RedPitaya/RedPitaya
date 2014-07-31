/**
 * $Id: lcr2.2.c 1246  $
 *
 * @brief Red Pitaya lcr algorythm 
 *
 * @Author cimem 
 *
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

int acquire_data(float t_params[],
                float **s , 
                uint32_t size);

int LCR_data_analasys(float **s ,
                        uint32_t size,
                        uint32_t DC_bias,
                        uint32_t Rs,
                        float complex *Z,
                        double w_out,
                        int f);
/** Print usage information */
void usage() {

    const char *format =
        "%s version %s-%s\n"
        "\n"
        "Usage: %s   [channel] "
                    "[amplitude] "
                    "Rref[real] "
                    "Rref[imag] "
                    "[DC_bias] "
                    "[averaging] "
                    "[frequency] "
                    "[sweep function] "
                    "[steps] [<end frequency> or <wait>] "
                    "[graph octave] \n"
        "\n"
        "\tchannel              Channel to generate signal on [1, 2].\n"
        "\tamplitude            Peak-to-peak signal amplitude in Vpp [0.0 - %1.1f].\n"
        "\tReference element    real value.\n"
        "\tReference element    imaginary value.\n"
        "\tDC_bias              for electrolit capacitors default = 0.\n"
        "\taveraging            number of averaging the measurements [1 - 10].\n"
        "\tfrequency            Signal frequency in Hz [%2.1f - %2.1e].\n"
        "\tcalibration          1 - calibrate, 0 - skip calibration.\n"
        "\tsweep function       1 - frequency sweep, 0 - measurement sweep.\n"
        "\tsteps                steps made between frequency limits.\n"
        "\tend frequency        end frequency in Hz [%2.1f - %2.1e] (if frequency sweep selected).\n"
        "\twait                 wait for user on each measurement step (if measurement sweep selected).\n"
        "\tgraph octave                 octave users car draw graphs with savind the output data.\n"

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

float trapz(float *arrayptr, float dT, int size1) {
  float result = 0;
  int i;
  //printf("size = %d\n", size);
  for (i =0; i < size1 - 1 ; i++) {
    result += (dT / (float)2) * (fabsf( arrayptr[i] + arrayptr[ i+1 ] ));
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
    float result = 0;
    int i;

    for(i = 0; i < length; i++) {
        result = result + arrayptr[i][column];
        //printf("suma_result = %f\n", result); //troubleshooting purposes
    }
    result = result / length;
    //printf("final_result = %f\n", result); //troubleshooting
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

    uint32_t Rs = 996;//TODO User defines this reference resirtor (check the circuit, there are 2 elements one is reference other element is measured)
    //uint32_t Rs = strtod(argv[3], NULL);
    if ( (Rs < 0.0) || (Rs > 50000) ) {
        fprintf(stderr, "Invalid reference element value: %s\n", argv[3]);
        usage();
        return -1;
    }

    uint32_t Rs_imag = 0;//TODO User defines this reference resirtor (check the circuit, there are 2 elements one is reference other element is measured)
    //uint32_t Rs_imag = strtod(argv[4], NULL);
    if ( (Rs_imag < -50000.0) || (Rs_imag > 50000) ) {
        fprintf(stderr, "Invalid reference element value:  %s\n", argv[4]);
        usage();
        return -1;
    }

    float complex Z_load_ref = (float)Rs + Rs_imag*I;

    uint32_t DC_bias = 0;
    //uint32_t DC_bias = strtod(argv[5], NULL);
    if ( (DC_bias < -2.0) || (DC_bias > 2.0) ) {
        fprintf(stderr, "Invalid DC bias:  %s\n", argv[5]);
        usage();
        return -1;
    }

    /* Number of measurments made and are later averaged */
    uint32_t averaging_num = 3; // Sequence takes more time and the result are more stable results (not more accurate)
    //uint32_t averaging_num = strtod(argv[6], NULL);
    if ( (averaging_num < 1) || (averaging_num > 10) ) {
        fprintf(stderr, "Invalid averaging_num:  %s\n", argv[6]);
        usage();
        return -1;
    }
    double start_frequency = 1000;
    //double start_frequency = strtod(argv[7], NULL);
    if ( (start_frequency < 1) || (start_frequency > 6.2e+07) ) {
        fprintf(stderr, "Invalid start frequency:  %s\n", argv[7]);
        usage();
        return -1;
    }

    /* if one wants to skip calibration the parameter can be set to 0 */
    int one_calibration = 1;
    //int one_calibration = strtod(argv[8], NULL);
    if ( (one_calibration < 0) || (one_calibration > 1) ) {
        fprintf(stderr, "Invalid one calibration parameter: %s\n", argv[8]);
        usage();
        return -1;
    }

    int sweep_function = 1; //[1] frequency sweep, [0] measurement sweep
    //int sweep_function = strtod(argv[9], NULL);
    if ( (sweep_function < 0) || (sweep_function > 1) ) {
        fprintf(stderr, "Invalid sweep function:  %s\n", argv[9]);
        usage();
        return -1;
    }

    double frequency_steps_number =  18;
    //double frequency_steps_number = strtod(argv[10], NULL);
    if ( (frequency_steps_number < 1) || (frequency_steps_number > 300) ) {
        fprintf(stderr, "Invalid umber of frequency steps:  %s\n", argv[10]);
        usage();
        return -1;
    }

    double end_frequency = 6.2e+07; //max = 6.2e+07
    //double end_frequency = strtod(argv[11], NULL);
    if ( (end_frequency < 1) || (end_frequency > 6.2e+07) ) {
        fprintf(stderr, "Invalid end frequency:  %s\n", argv[11]);
        usage();
        return -1;
    }
    
    int measurement_sweep_user_defined = 8;
    //nt measurement_sweep_user_defined = strtod(argv[10], NULL);
    if ( (measurement_sweep_user_defined < 1) || (measurement_sweep_user_defined > 300) ) {
        fprintf(stderr, "Invalid umber measurement steps steps:  %s\n", argv[10]);
        usage();
        return -1;
    }


    int wait_on_user = 0; //the program will wait for user to correctly connect the leads before next step
    //int wait_on_user = strtod(argv[11], NULL);
    if ( (wait_on_user < 0) || (wait_on_user > 1) ) {
        fprintf(stderr, "Invalid decidion: user wait %s\n", argv[11]);
        usage();
        return -1;
    }

    int draw = 1; //octave output for drawing the graphs of final impedance
    //int draw = strtod(argv[12], NULL);
    if ( (draw < 0) || (draw > 1) ) {
        fprintf(stderr, "Invalid decidion: draw %s\n", argv[12]);
        usage();
        return -1;
    }

    double endfreq = 0; // endfreq not needed in lcr algorythm
    double frequency; // parameter used in one of the foor loops
    int dimension_step = 0; // saving data on the right place in allocated memory
    double measurement_sweep;

    /* setting parameters depending on sweep function parameter [1] frequency sweep, [0] measurement sweep */
    if (sweep_function == 0) {
        end_frequency = start_frequency + 1;
        frequency_steps_number = 0.001;
    }
    else if (sweep_function == 1) {
        measurement_sweep_user_defined = 1; // just in case
        wait_on_user = 0;
    }

    /* end frequency must always be greather than start frequency */
    if ( end_frequency < start_frequency ) {
        fprintf(stderr, "End frequency has to be larger than the start frequency! \n");
        usage();
        return -1;
    }

    /* Frequency step used in frequency sweep */
    double frequency_step = (end_frequency - start_frequency ) / frequency_steps_number;

    /* Check frequency limits */
    if ( (start_frequency < c_min_frequency) || (start_frequency > c_max_frequency ) ) {
        fprintf(stderr, "Invalid frequency: %s\n", argv[7]);
        usage();
        return -1;
    }
    if ( (end_frequency < c_min_frequency) || (end_frequency > c_max_frequency ) ) {
        fprintf(stderr, "Invalid end frequency\n");
        usage();
        return -1;
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
            fprintf(stderr, "Invalid signal type\n");
            usage();
            return -1;
        }
    }

    uint32_t min_periodes = 2; // max 20
    double w_out; //angular velocity used in the algorythm
    int f = 0; // used in for lop, seting the decimation
    /* initializing the variables and pointers for acquire functionality */
    uint32_t size; // nmber of samples varies with number of periodes
    float **s = create_2D_table_size(SIGNALS_NUM, SIGNAL_LENGTH); // raw data saved to this location
    int i, i1; // iterators in for loops
    int equal = 0; //parameter initialized for generator functionality
    int shaping = 0; //parameter initialized for generator functionality
    float complex *Z = (float complex *)malloc( (averaging_num + 1) * sizeof(float complex)); // LCR_data_analasys() returnes data saved to this location
    /* calibrtion results short circuited */
    float **Calib_data_short_for_avreaging = create_2D_table_size((averaging_num + 1), 2); //appendin 2 data values
    float **Calib_data_short  = create_2D_table_size(measurement_sweep_user_defined, 2); //appendin 2 data values
    /* calibrtion results open circuited */
    float **Calib_data_open_for_avreaging = create_2D_table_size((averaging_num + 1), 2); //appendin 2 data values
    float **Calib_data_open = create_2D_table_size(measurement_sweep_user_defined, 2); //appendin 2 data values
    /* calibration load results */
    float **Calib_data_load_for_avreaging = create_2D_table_size((averaging_num + 1), 2); //appendin 2 data values
    float **Calib_data_load = create_2D_table_size(measurement_sweep_user_defined, 2); //appendin 2 data values
    /* measure results */
    float **Calib_data_measure_for_avreaging = create_2D_table_size((averaging_num + 1), 2 ); //appendin 2 data values
    float **Calib_data_measure = create_2D_table_size(measurement_sweep_user_defined, 4); //appendin 2 data values

    /* results are saved in allocated memory which size depends on sweep function (measurement sweep or frequency sweep) */
    int end_results_dimension = 0;
    if (sweep_function == 0 ) { // mesurement sweep defines size of allocated memory
        end_results_dimension = measurement_sweep_user_defined;
        printf("memory_dimension_defined = %d\n", end_results_dimension);
    }
    else if (sweep_function == 1) { // frequency sweep defines size of allocated memory
        end_results_dimension = frequency_steps_number+1;
    }

    /* multidimentional memmory allocation for storing final results */
    float complex *Z_short = (float complex *)malloc( end_results_dimension * sizeof(float complex));
    float complex *Z_open = (float complex *)malloc( end_results_dimension * sizeof(float complex));
    float complex *Z_load = (float complex *)malloc( end_results_dimension * sizeof(float complex));
    float complex *Z_measure = (float complex *)malloc( end_results_dimension * sizeof(float complex));
    float **data_output = create_2D_table_size(end_results_dimension +1, 2); //[0]-frequencies, [1]-real part of imedance, [2]- imaginary part of impendance,
    
    /* Initialization of Oscilloscope application */
    if(rp_app_init() < 0) {
        fprintf(stderr, "rp_app_init() failed!\n");
    return -1;
    }
    
    /* user is inquired to correcty set the connections */
    /**
    if (inquire_user_wait() < 0) {
        printf("error user inquiry at inquire_user_wait\n");
    } 
    */
    if (draw == 1 ) {
        printf("close all;\n");//octave sintax
        printf("clear all;\n");//octave syntax
    }

    int h = 0;// [h=0] - calibration open connections, [h=1] - calibration short circuited, [h=2] calibration load, [h=3] actual measurment
    for (h = 0; h <= 3 ; h++) {
        //printf("step_h(%d) = %d\n",(h+1),h );
        for ( frequency = start_frequency; frequency <= end_frequency; frequency = frequency + frequency_step) {
            printf("frequency_now = %f;\n",frequency );
            w_out = frequency * 2 * M_PI; //omega 
        
            /* Signal generator */
            awg_param_t params;
            /* Prepare data buffer (calculate from input arguments) */
            synthesize_signal(ampl, frequency, type, endfreq, data, &params);
            /* Write the data to the FPGA and set FPGA AWG state machine */
            write_data_fpga(ch, data, &params);

            /* measurement sweep is disabled if frequency sweep is set */
            /* measurement sweep set to 1 when calibration is executing. when measuring user defines  number of measurements */
            if (sweep_function == 0 ) { // sweep_function == 0 (mesurement sweep)
                if (h == 0 || h == 1|| h == 2) {
                    measurement_sweep = 1;
                }
                else {
                    measurement_sweep = measurement_sweep_user_defined;
                }
            }
            else if (sweep_function == 1) { // sweep_function == 1 (frequency sweep)
                measurement_sweep = 1;
            }

            for (i = 0; i < measurement_sweep; i++ ) {  // For measurment sweep is 1. calibration

                for ( i1 = 0; i1 < averaging_num; i1++ ) {

                    /* selection number for decimation */
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

                    /* setting decimtion */
                    if (f != DEC_MAX) {
                        t_params[TIME_RANGE_PARAM] = f;
                    } else {
                        fprintf(stderr, "Invalid decimation DEC\n");
                        usage();
                        return -1;
                    }
                    
                    /* calculating number of samples */
                    size = round( ( min_periodes * 125e6 ) / ( frequency * g_dec[f] ) );
                    //printf("size(%d) = %d;\n" ,(i1+1),size);

                    /* Filter parameters for signal Acqusition */
                    t_params[EQUAL_FILT_PARAM] = equal;
                    t_params[SHAPE_FILT_PARAM] = shaping;

                    /* Setting of parameters in Oscilloscope main module for signal Acqusition */
                    if(rp_set_params((float *)&t_params, PARAMS_NUM) < 0) {
                        fprintf(stderr, "rp_set_params() failed!\n");
                        return -1;
                    }

                    /* Data acqusition - saved to s */
                    if (acquire_data( t_params, s, size) < 0) {
                        printf("error acquiring data @ acquire_data\n");
                        return -1;
                    }
                    
                    /* data manipulation - returnes Z (complex impedance) */
                    if( LCR_data_analasys( s, size, DC_bias, Rs, Z, w_out, f) < 0) {
                        printf("error data analysis LCR_data_analasys\n");
                    }

                    /* Saving data */
                    switch (h) {
                    case 0:
                        Calib_data_short_for_avreaging[i1][1] = creal(*Z);
                        Calib_data_short_for_avreaging[i1][2] = cimag(*Z);
                        //printf("Calib_data_short_avreage_Z_real(%d) = %f\n",(i1+1), Calib_data_short_avreage[i1][2]);
                        //printf("Calib_data_short_avreage_Z_imag(%d) = %f\n",(i1+1), Calib_data_short_avreage[i1][3]);
                        break;
                    case 1:
                        Calib_data_open_for_avreaging[i1][1] = creal(*Z);
                        Calib_data_open_for_avreaging[i1][2] = cimag(*Z);
                        //printf("Calib_data_short_avreage_Z_real(%d) = %f\n",(i1+1), Calib_data_short_avreage[i1][2]);
                        //printf("Calib_data_short_avreage_Z_imag(%d) = %f\n",(i1+1), Calib_data_short_avreage[i1][3]);
                        break;
                    case 2:
                        Calib_data_load_for_avreaging[i1][1] = creal(*Z);
                        Calib_data_load_for_avreaging[i1][2] = cimag(*Z);
                        //printf("Calib_data_short_avreage_Z_real(%d) = %f\n",(i1+1), Calib_data_short_avreage[i1][2]);
                        //printf("Calib_data_short_avreage_Z_imag(%d) = %f\n",(i1+1), Calib_data_short_avreage[i1][3]);
                        break;
                    case 3:
                        Calib_data_measure_for_avreaging[i1][1] = creal(*Z);
                        Calib_data_measure_for_avreaging[i1][2] = cimag(*Z);
                        //printf("Calib_data_short_avreage_Z_real(%d) = %f\n",(i1+1), Calib_data_short_avreage[i1][2]);
                        //printf("Calib_data_short_avreage_Z_imag(%d) = %f\n",(i1+1), Calib_data_short_avreage[i1][3]);
                        break;
                    default:
                        printf("error no function set for h = %d, when saving data\n", h);
                    }

                } // end of for ( i1 = 0; i1 < averaging_num; i1++ ) {
                /* Saving mean value*/
                switch (h) {
                case 0:

                    Calib_data_short[i][1] = mean_array_column(Calib_data_short_for_avreaging, averaging_num, 1); // mean value of real impedance
                    Calib_data_short[i][2] = mean_array_column(Calib_data_short_for_avreaging, averaging_num, 2); // mean value of imaginary impedance
                    printf("avr_real_short_Z(%d) = %f;\n",(i+1), Calib_data_short[i][1]); 
                    printf("avr_imag_short_Z(%d) = %f;\n",(i+1), Calib_data_short[i][2]);
                    break;
                case 1:
                    Calib_data_open[i][1] = mean_array_column(Calib_data_open_for_avreaging, averaging_num, 1); // mean value of real impedance
                    Calib_data_open[i][2] = mean_array_column(Calib_data_open_for_avreaging, averaging_num, 2); // mean value of imaginary impedance
                    printf("avr_real_open_Z(%d) = %f;\n",(i+1), Calib_data_open[i][1]); 
                    printf("avr_real_open_Z(%d) = %f;\n",(i+1), Calib_data_open[i][2]);
                    break;
                case 2:
                    Calib_data_load[i][1] = mean_array_column(Calib_data_load_for_avreaging, averaging_num, 1); // mean value of real impedance
                    Calib_data_load[i][2] = mean_array_column(Calib_data_load_for_avreaging, averaging_num, 2); // mean value of imaginary impedance
                    printf("avr_real_load_Z(%d) = %f;\n",(i+1), Calib_data_load[i][1]); 
                    printf("avr_imag_load_Z(%d) = %f;\n",(i+1), Calib_data_load[i][2]);
                    break;
                case 3:
                    Calib_data_measure[i][0] = i;
                    Calib_data_measure[i][1] = frequency;
                    Calib_data_measure[i][2] = mean_array_column(Calib_data_measure_for_avreaging, averaging_num, 1); // mean value of real impedance
                    Calib_data_measure[i][3] = mean_array_column(Calib_data_measure_for_avreaging, averaging_num, 2); // mean value of imaginary impedance
                    printf("avr_real_measure_Z(%d) = %f;\n",(i+1), Calib_data_measure[i][2]); 
                    printf("avr_imag_measure_Z(%d) = %f;\n",(dimension_step+1), Calib_data_measure[i][3]);
                    break;
                default:
                    printf("error no function set for h = %d, when averaging data\n", h);
                }

                /* Saving data depending on sweep functionality */
                if (sweep_function == 0 ) { //sweep_function == 0 (mesurement sweep), sweep_function == 1 (frequency sweep)
                    dimension_step = i;
                    //printf("on_the_place_msweep = %d\n",dimension_step);
                }
                else if(sweep_function == 1) { //sweep_function == 0 (mesurement sweep), sweep_function == 0 (frequency sweep)
                   dimension_step = ((int)(frequency / frequency_step));
                   //printf("on_the_place_fr = %d\n",dimension_step);
                }
                
                /* Saving data for output */
                printf("frequency(%d) = %f;\n",(dimension_step+1),frequency );
                Z_short[dimension_step] = Calib_data_short[0][1] + Calib_data_short[0][2] *I; //measurement sweep i je vedno 1 tudi pri fr sweep je i vedno 1
                //printf("Z_short(%d) = %f + %f *I\n",(dimension_step+1),creal(Z_short[dimension_step]), cimag(Z_short[dimension_step]));

                Z_open[dimension_step] = Calib_data_open[0][1] + Calib_data_open[0][2] *I;
                //printf("Z_open(%d) = %f + %f *I\n",(dimension_step+1),creal(Z_open[dimension_step]), cimag(Z_open[dimension_step]));

                Z_load[dimension_step] = Calib_data_load[0][1] + Calib_data_load[0][2] *I;
                //printf("Z_load(%d) = %f + %f *I\n",(dimension_step+1),creal(Z_load[dimension_step]), cimag(Z_load[dimension_step]));

                Z_measure[dimension_step] = Calib_data_measure[i][2] + Calib_data_measure[i][3] *I;
                if (draw == 1 ) { 
                    printf("Z_measure(%d) = %f + %f *I\n",(dimension_step+1),creal(Z_measure[dimension_step]), cimag(Z_measure[dimension_step]));
                }
                data_output[0][dimension_step] = Calib_data_measure[i][1]; // [0] frequency row
                //printf("frequency_row(%d) = %f\n",(dimension_step+1), data_output[0][dimension_step]);

                

            } // for (i = 0; i < measurement_sweep; i++ ) {  // For measurment sweep is 1. calibration
        
        
        } // end off for( frequency = start_frequency ; frequency < end_frequency ; frequency += frequency_step) {
    }// end of (h = 0; h =< 4 ; h++) { function step
    /* user is inquired to correcty set the connections */
    /**
    if (inquire_user_wait() < 0) {
        printf("error user inquiry at inquire_user_wait\n");
    } 
    */

        
    /* Data ploting *//*
        printf("figure\n");

        printf("subplot(2,1,1)\n");
        printf("hold on\n");
        printf("plot(X_trapz1(1,:),Y_trapz1(1,:),'r');\n");
        printf("title ('rezultati trapezoidne metode x_trapz 1 in y_trapz1');\n");
        printf("subplot(2,1,2)\n");
        printf("plot(X_trapz2(1,:),Y_trapz2(1,:),'r');\n");
        printf("title ('rezultati trapezoidne metode x_trapz2 in y_trapz2');\n");


        printf("figure\n");

        printf("subplot(2,1,1)\n");
        printf("hold on\n");
        printf("plot(U_load_ref(1,1,:),'r');\n");
        printf("plot(U_load_ref(1,2,:),'g');\n");
        printf("hold off\n");
        printf("title ('U_load_ref 1 - voltage multiplied by reference signal');\n");
        printf("ylabel ('vrednost vzorcev U_load_ref(1,1,:)');\n" );
        printf("xlabel ('samples');\n" );

        printf("subplot(2,1,2)\n");
        printf("hold on\n");
        printf("plot(I_load_ref(1,1,:),'r');\n");
        printf("plot(I_load_ref(1,2,:),'g');\n");
        printf("hold off\n");
        printf("title ('I_load_ref  - voltage multiplied by reference signal');\n");
        printf("ylabel ('vrednost vzorcev I_load_ref');\n" );
        printf("xlabel ('samples');\n" );


        printf("figure\n");

        printf("subplot(4,1,1);\n");
        printf("plot(U_acq(1,2,:),'r');\n");
        printf("title ('Raw Napetost iz zajetih vzorcev na bremenu 1.1  U_acq = s')\n");
        printf("ylabel ('U_acq_1/ V');\n" );
        printf("xlabel ('vzorci');\n" );

        printf("subplot(4,1,2);\n");
        printf("plot(U_acq(1,3,:),'g');\n");
        printf("title ('Raw Napetost iz zajetih vzorcev na bremenu 1.2  U_acq = s');\n");
        printf("ylabel ('U_acq_2/ V');\n" );
        printf("xlabel ('vzorci');\n" );

        printf("subplot(4,1,3);\n");
        printf("plot(U_load(1,:), 'b')\n");
        printf("title ('U_load_ - on the load');\n");
        printf("ylabel ('U_load / V');\n" );
        printf("xlabel ('vzorci');\n" );


        printf("subplot(4,1,4);\n");
        printf("plot(I_load(1,:), 'c')\n");;
        printf("ylabel ('I_load / A');\n" );
        printf("xlabel ('vzorci');\n" );
        printf("title ('I_load_ - on the load');\n");*/

    /* colerating data calibration data and measureents depending on the calibration measurements, 
    if calibration was not made, only measured values are saved */
    if (one_calibration == 1) {
        printf("calibration_was_made_and_data_is_colerated_depending_on_calibration_measurements = 1\n");
        for (i = 0; i < measurement_sweep_user_defined; i++ ) {
            data_output[1][i] = creal( ( ( ( Z_short[i] - Z_measure[i]) * (Z_load[i] - Z_open[i]) ) / ( (Z_measure[i] - Z_open[i]) * (Z_short[i] - Z_load[i]) ) ) * Z_load_ref );
            data_output[2][i] = cimag( ( ( ( Z_short[i] - Z_measure[i]) * (Z_load[i] - Z_open[i]) ) / ( (Z_measure[i] - Z_open[i]) * (Z_short[i] - Z_load[i]) ) ) * Z_load_ref );
            if (draw == 1 ) {
                printf("Z_output(%d) = %f + %f*I\n",i+1, data_output[1][i], data_output[2][i] );
            }
        }
    }
    else {
        for (i = 0; i < measurement_sweep_user_defined; i++ ) {
            data_output[1][i] = creal(Z_measure[i]);
            data_output[2][i] = cimag(Z_measure[i]);
            if (draw == 1 ) {
                printf("Z_output(%d) = %f + %f*I\n",i+1, data_output[1][i], data_output[2][i] );
            }
        }
    }

    if (draw == 1 ) {
        printf("figure\n");

        printf("subplot(2,1,1);\n");
        if (sweep_function == 1 )  {
            printf("plot(frequency,real(Z_output),'r');\n");
            printf("xlabel ('frequency');\n" );
        }
        else {
            printf("plot(real(Z_output),'r');\n");
            printf("xlabel ('samples');\n" );
        }
        
        printf("title ('Impedance on the load with calibration corelation')\n");
        printf("ylabel ('Z_output- real');\n" );
        

        printf("subplot(2,1,2);\n");
        if (sweep_function == 1 )  {
            printf("plot(frequency,real(Z_output),'b');\n");
            printf("xlabel ('frequency');\n" );
        }
        else {
            printf("plot(real(Z_output),'b');\n");
            printf("xlabel ('samples');\n" );
        }
        printf("title ('Impedance on the load with calibration corelation')\n");
        printf("ylabel ('Z_output- imaginary');\n" );


        // drawing raw measured data
        printf("figure\n");

        printf("subplot(2,1,1);\n");
        if (sweep_function == 1 )  {
            printf("plot(frequency,real(Z_measure),'r');\n");
            printf("xlabel ('frequency');\n" );
        }
        else {
            printf("plot(real(Z_measure),'r');\n");
            printf("xlabel ('samples');\n" );
        }
        
        printf("title ('Impedance without calibration corelation')\n");
        printf("ylabel ('Z_measure- real');\n" );
        

        printf("subplot(2,1,2);\n");
        if (sweep_function == 1 )  {
            printf("plot(frequency,imag(Z_measure),'r');\n");
            printf("xlabel ('frequency');\n" );
        }
        else {
            printf("plot(imag(Z_measure),'r');\n");
            printf("xlabel ('samples');\n" );
        }
        printf("title ('Impedance without calibration corelation')\n");
        printf("ylabel ('Z_measure- imaginary');\n" );
        
    }
   


    printf("end_yay_no_errors = 1\n");
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

/**
 * acquire data to memory (s)
 *
 * @param ch    Channel number [0, 1].
 * @param data  AWG data to write to FPGA.
 * @param awg   AWG paramters to write to FPGA.
 */
int acquire_data(float t_params[],
                float **s , 
                uint32_t size) {
    int retries = 150000;
    int j, sig_num, sig_len;
    int ret_val;

    usleep(41754); // generator needs some time to start generating 31754
    while(retries >= 0) {
        if((ret_val = rp_get_signals(&s, &sig_num, &sig_len)) >= 0) {
            /* Signals acquired in s[][]:
             * s[0][i] - TODO
             * s[1][i] - Channel ADC1 raw signal
             * s[2][i] - Channel ADC2 raw signal
             */
    
            for(j = 0; j < MIN(size, sig_len); j++) {
                //printf("s(%d,%d,:) = [%7d, %7d];\n",(averagingi1 +1),(j +1) , (int)s[1][j], (int)s[2][j]);
            }
            break;
        }

        if(retries-- == 0) {
            fprintf(stderr, "Signal scquisition was not triggered!\n");
            break;
        }
        usleep(1000);
    }
    return 1;
}

int LCR_data_analasys(float **s ,
                        uint32_t size,
                        uint32_t DC_bias,
                        uint32_t Rs,
                        float complex *Z,
                        double w_out,
                        int f) {
    int i2, i3;
    float **U_acq = create_2D_table_size(SIGNALS_NUM, SIGNAL_LENGTH);
    /* Used for storing the Voltage and current on the load */
    float *U_load = create_table_size( SIGNAL_LENGTH );
    float *I_load = create_table_size( SIGNAL_LENGTH );
    /* Signals multiplied by the reference signal (sin) */
    float **U_load_ref = create_2D_table_size(SIGNALS_NUM, size); //U_load_ref[1][i] voltage signal 1, U_load_ref[2][i] - voltage signal 2
    float **I_load_ref = create_2D_table_size(SIGNALS_NUM, size); //I_load_ref[1][i] current signal 1, I_load_ref[2][i] - current signal 2
    /* Signals return by trapezoidal method in complex */
    float *X_trapz_U = create_table_size( 1 );
    float *X_trapz_I = create_table_size( 1 );
    float *Y_trapz_U = create_table_size( 1 );
    float *Y_trapz_I = create_table_size( 1 );
    /* Voltage, current and their phases calculated */
    float U_load_amp;
    float Phase_U_load_amp;
    float I_load_amp;
    float Phase_I_load_amp;
    float Z_phase_deg_imag;  // may cuse errors because not complex
    float T; // Sampling time in seconds
    float *t = create_table_size(16384);

    T = ( g_dec[f] / 125e6 );
    //printf("T = %f;\n",T );

    for(i2 = 0; i2 < (size - 1); i2++) {
        t[i2] = i2;
    }


    /* Transform signals from  AD - 14 bit to voltage [ ( s / 2^14 ) * 2 ] */
    for (i2 = 0; i2 < SIGNALS_NUM; i2++) { // only the 1 and 2 are used for i2
        for(i3 = 0; i3 < size; i3 ++ ) { 
            U_acq[i2][i3] = ( s[i2][i3] * (float)( 2 - DC_bias ) ) / (float)16384; //division comes after multiplication, this way no accuracy is lost
            //printf("U_acq(%d,%d,%d) = %f;\n",(i1+1), (i2+1), (i3+1), U_acq[i2][i3] );
        }
    }

    /* Voltage and current on the load can be calculated from gathered data */
    for (i2 = 0; i2 < size; i2++) { 
        U_load[i2] = U_acq[2][i2] - U_acq[1][i2]; // potencial difference gives the voltage
        I_load[i2] = U_acq[2][i2] / (float)Rs; // Curent trough the load is the same as trough thr Rs. ohm's law is used to calculate the current
        //printf("U_load(%d,%d) = %f;\n",(1),(i2+1), U_load[i2]);
        //printf("I_load(%d,%d) = %f;\n",(1),(i2+1), I_load[i2] );
    }

    /* Finding max values, used for ploting */
    /* comented because not used
    float U_load_max = max_array( U_load , SIGNAL_LENGTH );
    float I_load_max = max_array( I_load , SIGNAL_LENGTH );
    printf("U_load_max = %f \n", U_load_max);
    printf("I_load_max = %f\n", I_load_max);
    */
    /* Acquired signals must be multiplied by the reference signals, used for lock in metod */
    for( i2 = 0; i2 < size; i2++) {
        U_load_ref[1][i2] = U_load[i2] * sin( t[i2] * T * w_out );
        U_load_ref[2][i2] = U_load[i2] * cos( t[i2] * T * w_out );
        I_load_ref[1][i2] = I_load[i2] * sin( t[i2] * T * w_out );
        I_load_ref[2][i2] = I_load[i2] * cos( t[i2] * T * w_out );
        /*
        printf("U_load_ref(%d,1,%d) = %f;\n",(i1+1),(i2+1),U_load_ref[1][i2]);
        printf("U_load_ref(%d,2,%d) = %f;\n",(i1+1),(i2+1),U_load_ref[2][i2]);
        printf("I_load_ref(%d,1,%d) = %f;\n",(i1+1),(i2+1),I_load_ref[1][i2]);
        printf("I_load_ref(%d,2,%d) = %f;\n",(i1+1),(i2+1),I_load_ref[2][i2]);
        */
    }

    /* Trapezoidal method for calculating the approximation of an integral */
    X_trapz_U[1] = trapz( U_load_ref[ 1 ], (float)T, size );
    X_trapz_I[2] = trapz( I_load_ref[ 1 ], (float)T, size );
    Y_trapz_U[1] = trapz( U_load_ref[ 2 ], (float)T, size );
    Y_trapz_I[2] = trapz( I_load_ref[ 2 ], (float)T, size );
    /*
    printf("X_trapz1(%d) = %f;\n",(i1+1),X_trapz_U[1] );
    printf("X_trapz2(%d) = %f;\n",(i1+1),X_trapz_I[2] );
    printf("Y_trapz1(%d) = %f;\n",(i1+1),Y_trapz_U[1] );
    printf("Y_trapz2(%d) = %f;\n",(i1+1),Y_trapz_I[2] );
    */
    /* Calculating voltage amplitude and phase */
    U_load_amp = (float)2 * ( sqrtf( pow( X_trapz_U[1] , (float)2 ) + pow( Y_trapz_U[1] , (float)2 )));
    Phase_U_load_amp = atan2f( Y_trapz_U[1], X_trapz_U[1] );
    /*
    printf("U_load_amp(%d) = %f;\n",(i1+1),U_load_amp );
    printf("Phase_U_load_amp(%d) = %f;\n",(i1+1),Phase_U_load_amp );
    */
    /* Calculating current amplitude and phase */
    I_load_amp =(float)2 * (sqrtf( pow( X_trapz_I[2] , (float)2 ) + pow( Y_trapz_I[2] , (float)2 )));
    Phase_I_load_amp = atan2f( Y_trapz_I[2], X_trapz_I[2] );
    
    printf("I_load_amp(%d) = %f;\n",(1),U_load_amp );
    printf("Phase_I_load_amp(%d) = %f;\n",(1),Phase_U_load_amp );
    
    /* Asigning impedance  values (complex value) */
    *Z = (U_load_amp / I_load_amp) + ( Phase_U_load_amp - Phase_I_load_amp ) * I;
    //printf("Z = %.2f + %.2fI;\n", creal(*Z), cimag(*Z));
    
    Z_phase_deg_imag = cimag(*Z) * (180 / M_PI);
    if ( Z_phase_deg_imag <= -180 ) {
        Z_phase_deg_imag += 360;
    }
    else if (Z_phase_deg_imag <= 180) {
        Z_phase_deg_imag -= 360;
    }
    //printf("Z_phase_deg_imag = %f;\n", Z_phase_deg_imag);
    return 1;
}

int inquire_user_wait() {
    while(1) {
        int calibration_continue;
        printf("Please connect the wires correctly. Continue? [1 = yes|0 = skip ] :");
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
    return 1;
}