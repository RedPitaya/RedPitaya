/**
 * $Id: lcr.c 1246  $
 *
 * @brief Red Pitaya lcr algorythm 
 *
 * @Author cimem 
 *
 * VERSION : 2.2
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "fpga.h"
#include "version.h"

#include <unistd.h>
#include <getopt.h>
#include <sys/param.h>
#include "main.h"
#include "fpga_awg.h"
 #include "lcr.h"

  /* Standart Library of Complex Numbers */
#define M_PI 3.14159265358979323846


/** Maximal signal frequency [Hz] */
const double c_max_frequency = 62.5e6;

/** Minimal signal frequency [Hz] */
const double c_min_frequency = 0;

/** Maximal signal amplitude [Vpp] */
const double c_max_amplitude_lcr = 2.0;

/** AWG buffer length [samples]*/
#define n (16*1024)

/** AWG data buffer */
int32_t data[n];

/** Program name */
const char *g_argv0 = NULL;

/** Pointers to the FPGA input signal buffers for Channel A and B */
int                  *rp_fpga_cha_signal, *rp_fpga_chb_signal;

/* Working signal for our LCR method */
float               **rp_lcr_signals;

/* Added things */
//pthread_mutex_t       rp_osc_sig_mutex = PTHREAD_MUTEX_INITIALIZER;


/** Oscilloscope module parameters as defined in main module
 * @see rp_main_params
 */
//float t_params[PARAMS_NUM] = { 0, 1e6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

rp_app_params_t  *t_params = NULL;

/** Max decimation index */
#define DEC_MAX 6

/** Decimation translation table */
static int g_dec[DEC_MAX] = { 1,  8,  64,  1024,  8192,  65536 };

/* Forward declarations */
void synthesize_signal_lcr(double ampl, double freq, signal_e_lcr type, double endfreq,
                       int32_t *data,
                       awg_param_t_lcr *params);
void write_data_fpga_lcr(uint32_t ch,
                     const int32_t *data,
                     const awg_param_t_lcr *awg);

int acquire_data(
                float **s , 
                uint32_t size);

int LCR_data_analasys(float **s ,
                        uint32_t size,
                        uint32_t DC_bias,
                        uint32_t R_shunt,
                        float complex *Z,
                        double w_out,
                        int f);

int lcr(uint32_t ch, double ampl, uint32_t DC_bias, float R_shunt, uint32_t averaging_num, int calib_function,  uint32_t Z_load_ref_real, uint32_t Z_load_ref_imag, 
        double steps, int sweep_function, double start_frequency, double end_frequency,  int scale_type, int wait_on_user, float *frequency_lcr, float *phase_lcr, float *amplitude_lcr);
/** Print usage information */
/*
void rp_lcr_worker_fill_params(rp_app_params_t *params);

void rp_lcr_worker_fill_params(rp_app_params_t *params){
    rp_copy_params(params, (rp_app_params_t **)&t_params);
}
*/
void usage() {

    const char *format =
        "%s version %s-%s\n"
        "\n"
        "Usage: %s   [channel] "
                    "[amplitude] "
                    "[DC_bias] "
                    "[R_shunt] "
                    "[averaging] "
                    "[calib function] "
                    "Z_load_ref[real] "
                    "Z_load_ref[imag] "
                    "[steps] "
                    "[sweep function] "
                    "[start frequnecy] "
                    "[stop frequency]"
                    "[scale type]"
                    "[wait]\n"
        "\n"
        "\tchannel              Channel to generate signal on [1, 2].\n"
        "\tamplitude            Peak-to-peak signal amplitude in Vpp [0.0 - %1.1f].\n"
        "\tDC bias              for electrolit capacitors default = 0.\n"
        "\tshunt resistior      in ohms\n"
        "\taveraging            number of averaging the measurements [1 - 10].\n"
        "\tcalib function       0(no calibration), 1(open and short calib), 2(zloadref calib).\n"
        "\tref impedance real   Z_load_ref real value.\n"
        "\tref impedance imag   Z_load_ref imaginary value.\n"
        "\tsteps                steps made between frequency limits.\n"
        "\tsweep function       1 - frequency sweep, 0 - measurement sweep.\n"
        "\tstart frequency      Signal frequency in Hz [%2.1f - %2.1e].\n"
        "\tstop frequency       Signal frequency in Hz [%2.1f - %2.1e].\n"
        "\tscale type           x scale 0 - linear 1 -log\n"
        "\twait                 wait for user before each measurement step\n"
        "\n";

    fprintf( stderr, format, g_argv0, VERSION_STR, REVISION_STR,
             g_argv0, c_max_amplitude_lcr, c_min_frequency, c_max_frequency);
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

float trapz(float *arrayptr, float T, int size1) {
  float result = 0;
  int i;
  //printf("size = %d\n", size);
  for (i =0; i < size1 - 1 ; i++) {
    result +=  ( arrayptr[i] + arrayptr[ i+1 ]  );
   
  }
    result = (T / (float)2) * result;
    //printf("return = %f\n",result);
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

/** lcr main */
int lcr(uint32_t ch, double ampl, uint32_t DC_bias, float R_shunt, uint32_t averaging_num, int calib_function,  uint32_t Z_load_ref_real, uint32_t Z_load_ref_imag, 
        double steps, int sweep_function, double start_frequency, double end_frequency,  int scale_type, int wait_on_user, float *frequency_lcr, float *phase_lcr, float *amplitude_lcr)
{

    /* argument check 
    g_argv0 = argv[0];    
    
    if ( argc < 15 ) {
        usage();
        return -1;
    }
    */
    
    printf("Starting LCR\n");
    /* Channel argument parsing */
    //uint32_t ch = atoi(argv[1]) - 1; /* Zero based internally */
    ch = 0;
    if (ch > 1) {
        //fprintf(stderr, "Invalid channel: %s\n", argv[1]);
        usage();
        return -1;
    }

    /* Signal amplitude argument parsing */
    //double ampl = strtod(argv[2], NULL);
    ampl = 1.8;
    if ( (ampl < 0.0) || (ampl > c_max_amplitude_lcr) ) {
        //fprintf(stderr, "Invalid amplitude: %s\n", argv[2]);
        usage();
        return -1;
    }

    //uint32_t DC_bias = strtod(argv[3], NULL);
    DC_bias = 0;
    if ( (DC_bias < -2.0) || (DC_bias > 2.0) ) {
        //fprintf(stderr, "Invalid DC bias:  %s\n", argv[5]);
        usage();
        return -1;
    }

    //float R_shunt = strtod(argv[4], NULL);
    R_shunt = 996;
    if ( (R_shunt < 0.0) || (R_shunt > 50000) ) {
        //fprintf(stderr, "Invalid reference element value: %s\n", argv[3]);
        usage();
        return -1;
    }

    /* Number of measurments made and are later averaged */
    //uint32_t averaging_num = strtod(argv[5], NULL);
    averaging_num = 1;
    if ( (averaging_num < 1) || (averaging_num > 10) ) {
        //fprintf(stderr, "Invalid averaging_num:  %s\n", argv[6]);
        usage();
        return -1;
    }

    /* if one wants to skip calibration the parameter can be set to 0 */
    //int calib_function = strtod(argv[6], NULL);
    calib_function = 0;
    if ( (calib_function < 0) || (calib_function > 2) ) {
        //fprintf(stderr, "Invalid one calibration parameter: %s\n", argv[8]);
        usage();
        return -1;
    }

    //uint32_t Z_load_ref_real = strtod(argv[7], NULL);
    Z_load_ref_real = 0;
    if ( (Z_load_ref_real < -50000.0) || (Z_load_ref_real > 50000) ) {
        //fprintf(stderr, "Invalid reference element value:  %s\n", argv[4]);
        usage();
        return -1;
    }

    //uint32_t Z_load_ref_imag = strtod(argv[8], NULL);
    Z_load_ref_imag = 0;
    if ( (Z_load_ref_imag < -50000.0) || (Z_load_ref_imag > 50000) ) {
        //fprintf(stderr, "Invalid reference element value:  %s\n", argv[4]);
        usage();
        return -1;
    }

    // complex number construction
    float complex Z_load_ref = Z_load_ref_real + Z_load_ref_imag*I;

    //double steps = strtod(argv[9], NULL);
    steps =  5;
    if ( (steps < 1) || (steps > 300) ) {
        //fprintf(stderr, "Invalid umber of steps:  %s\n", argv[9]);
        usage();
        return -1;
    }

    /* [1] frequency sweep, [0] measurement sweep */
    //int sweep_function = strtod(argv[10], NULL);
    sweep_function = 0; 
    if ( (sweep_function < 0) || (sweep_function > 1) ) {
        //fprintf(stderr, "Invalid sweep function:  %s\n", argv[10]);
        usage();
        return -1;
    }

    //double start_frequency = strtod(argv[11], NULL);
    start_frequency = 4000;
    if ( (start_frequency < 1) || (start_frequency > 1000000) ) {
        //fprintf(stderr, "Invalid start frequency:  %s\n", argv[7]);
        usage();
        return -1;
    }

    //double end_frequency = strtod(argv[12], NULL);
    end_frequency = 8000; //max = 6.2e+07
    if ( (end_frequency < 1) || (end_frequency > 1000000) ) {
        //fprintf(stderr, "Invalid end frequency: %s\n", argv[12]);
        usage();
        return -1;
    }

    //int scale_type = strtod(argv[13], NULL);
    scale_type = 0; //the program will wait for user to correctly connect the leads before next step
    if ( (scale_type < 0) || (scale_type > 1) ) {
        //fprintf(stderr, "Invalid decidion:scale type: %s\n", argv[13]);
        usage();
        return -1;
    }

    //int wait_on_user = strtod(argv[14], NULL);
    wait_on_user = 0; //the program will wait for user to correctly connect the leads before next step
    if ( (wait_on_user < 0) || (wait_on_user > 1) ) {
        //fprintf(stderr, "Invalid decidion: user wait argument %s\n", argv[14]);
        usage();
        return -1;
    }


    /* depending on sweep funcrion num of steps is given to certan foo loop */
    double  frequency_steps_number;
    int     measurement_sweep_user_defined;
    double  frequency_step;
    double  a,b,c;
    printf("Definition of arguments passed.\n");
    if(scale_type) { //if logaritmic scale required start and end frequency are transformed
        b = log10( end_frequency );
        a = log10( start_frequency );
        c = ( b - a ) /( steps - 1);
    }

    if (sweep_function == 1){ //frequency sweep
        frequency_steps_number = steps;
        frequency_step = (end_frequency - start_frequency ) / ( frequency_steps_number - 1);
        measurement_sweep_user_defined = 1;
    }
    else if (sweep_function == 0){ // measurement sweep
        measurement_sweep_user_defined = steps;
        frequency_step = 0;
        frequency_steps_number = 1;
    }

    /* end frequency must always be greather than start frequency */
    if ( end_frequency < start_frequency ) {
        fprintf(stderr, "End frequency has to be greater than the start frequency! \n");
        usage();
        return -1;
    }

    /* allocated memory which size depends on sweep function (measurement sweep or frequency sweep) */
    int end_results_dimension = 0;
    if (sweep_function == 0 ) { // mesurement sweep defines size of allocated memory
        end_results_dimension = measurement_sweep_user_defined;
    }
    else if (sweep_function == 1) { // frequency sweep defines size of allocated memory
        end_results_dimension = frequency_steps_number;
    }
    printf("Starting signal type.\n");
    /* Signal type argument parsing */
    signal_e_lcr type = eSignalSine_lcr;
    printf("Signal type set succesfuly\n");
    double  endfreq = 0; // endfreq set for inbulild sweep (generate)
    int     dimension_step = 0; // saving data on the right place in allocated memory
    double  measurement_sweep;

    uint32_t  min_periodes = 2; // max 20
    double    w_out; //angular velocity used in the algorythm
    float       f = 0; // used in for lop, seting the decimation
    uint32_t  size; // nmber of samples varies with number of periodes
    float   **s = create_2D_table_size(SIGNALS_NUM, SIGNAL_LENGTH); // raw data saved to this location
    int       i, i1, fr, h; // iterators in for loops
    //int       equal = 0; //parameter initialized for generator functionality
    //int       shaping = 0; //parameter initialized for generator functionality

    /* LCR_data_analasys() saves data to *Z */
    float complex *Z = (float complex *)malloc( (averaging_num + 1) * sizeof(float complex));
    
    /* calibrtion results short circuited */
    float **Calib_data_short_for_avreaging = create_2D_table_size((averaging_num + 1), 2);
    float **Calib_data_short  = create_2D_table_size(measurement_sweep_user_defined, 2);
    
    /* calibrtion results open circuited */
    float **Calib_data_open_for_avreaging = create_2D_table_size((averaging_num + 1), 2);
    float **Calib_data_open = create_2D_table_size(measurement_sweep_user_defined, 2);
    
    /* calibration load results */
    float **Calib_data_load_for_avreaging = create_2D_table_size((averaging_num + 1), 2);
    float **Calib_data_load = create_2D_table_size(measurement_sweep_user_defined, 2);
    
    /* measurement results */
    float **Calib_data_measure_for_avreaging = create_2D_table_size((averaging_num + 1), 2 );
    float **Calib_data_measure = create_2D_table_size(measurement_sweep_user_defined, 4);

    /* multidimentional memmory allocation for storing final results */
    float complex *Z_short = (float complex *)malloc( end_results_dimension * sizeof(float complex));
    float complex *Z_open = (float complex *)malloc( end_results_dimension * sizeof(float complex));
    float complex *Z_load = (float complex *)malloc( end_results_dimension * sizeof(float complex));
    float complex *Z_measure = (float complex *)malloc( end_results_dimension * sizeof(float complex));

    float *calib_data_combine = (float *)malloc( 2 * sizeof(float)); //[0]-frequencies, [1]-real part of imedance, [2]- imaginary part of impendance,
    float *PhaseZ = (float *)malloc((end_results_dimension + 1) * sizeof(float) ); //phase
    float *AmplitudeZ = (float *)malloc((end_results_dimension + 1) * sizeof(float) ); //Amplitude
    float *frequency = (float *)malloc((end_results_dimension + 1) * sizeof(float) ); //frequency
    printf("Init of variables started succesfully.\n");

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

    float k;
    printf("Starting the double for loop.\n");

    // [h=0] - calibration open connections, [h=1] - calibration short circuited, [h=2] calibration load, [h=3] actual measurment
    for (h = 0; h <= 3 ; h++) {
        if (!calib_function) {
            h = 3;
        }

        for ( fr = 0; fr < frequency_steps_number; fr++ ) {

            if ( scale_type ) { //log scle
                k = powf( 10, ( c * (float)fr ) + a );
                frequency[ fr ] =  k ;
            }
            else { // lin scale
                frequency[ fr ] = start_frequency + ( frequency_step * fr );
            }

            w_out = frequency[fr] * 2 * M_PI; // omega - angular velocity

            
            awg_param_t_lcr params;
            printf("Starting fpga write.\n");
            /* Prepare data buffer (calculate from input arguments) */
            synthesize_signal_lcr(ampl, frequency[fr], type, endfreq, data, &params);

            printf("signal generation complete\n");
            /* Write the data to the FPGA and set FPGA AWG state machine */
            write_data_fpga_lcr(ch, data, &params);
            printf("Wrtie to fpga succesful.\n");
            /* if measurement sweep selected, only one calibration measurement is made */
            if (sweep_function == 0 ) { // sweep_function == 0 (mesurement sweep)
                if (h == 0 || h == 1|| h == 2) {
                    measurement_sweep = 1;
                }
                else {// when gathering measurement results sweep is defined by argument for num of steps
                    measurement_sweep = measurement_sweep_user_defined;
                }
            }
            else if (sweep_function == 1) { // sweep_function == 1 (frequency sweep)
                measurement_sweep = 1; //when frequency sweep selected only one measurement for each fr is made
            }

            for (i = 0; i < measurement_sweep; i++ ) {

                for ( i1 = 0; i1 < averaging_num; i1++ ) {

                    /* decimation changes depending on frequency */
                    if (frequency[fr] >= 160000)     {      f=0;    }
                    else if (frequency[fr] >= 20000) {      f=1;    }    
                    else if (frequency[fr] >= 2500)  {      f=2;    }    
                    else if (frequency[fr] >= 160)   {      f=3;    }    
                    else if (frequency[fr] >= 20)    {      f=4;    }     
                    else if (frequency[fr] >= 2.5)   {      f=5;    }

                    //printf(" name_time = %s\n", t_params[TIME_RANGE_PARAM].name);
                    /* setting decimtion */
                    if (f != DEC_MAX) {
                        //Function for setting the time range parameter
                        printf("Statting to write data to rp_main_params\n");
                        rp_set_time_range(f);     
                    } else {
                        fprintf(stderr, "Invalid decimation DEC\n");
                        usage();
                        return -1;
                    }
                    
                    /* calculating num of samples */
                    size = round( ( min_periodes * 125e6 ) / ( frequency[fr] * g_dec[(int)f] ) );

                    /* ADC Data acqusition - saved to s */
                    printf("Starting acqusition data\n");
                    if (acquire_data(s, size) < 0) {
                        printf("error acquiring data @ acquire_data\n");
                        return -1;
                    }
                    printf("Acquisition data succesful\n");
                    printf("Statting LCR analysis function.\n");
                    /* data manipulation - returnes Z (complex impedance) */
                    if( LCR_data_analasys( s, size, DC_bias, R_shunt, Z, w_out, f) < 0) {
                        printf("error data analysis LCR_data_analasys\n");
                    }

                    /* Saving data */
                    switch (h) {
                    case 0:
                        Calib_data_short_for_avreaging[ i1 ][ 1 ] = creal(*Z);
                        Calib_data_short_for_avreaging[ i1 ][ 2 ] = cimag(*Z);
                        break;
                    case 1:
                        Calib_data_open_for_avreaging[ i1 ][ 1 ] = creal(*Z);
                        Calib_data_open_for_avreaging[ i1 ][ 2 ] = cimag(*Z);
                        break;
                    case 2:
                        Calib_data_load_for_avreaging[ i1 ][ 1 ] = creal(*Z);
                        Calib_data_load_for_avreaging[ i1 ][ 2 ] = cimag(*Z);
                        break;
                    case 3:
                        Calib_data_measure_for_avreaging[ i1 ][ 1 ] = creal(*Z);
                        Calib_data_measure_for_avreaging[ i1 ][ 2 ] = cimag(*Z);
                        break;
                    default:
                        printf("error no function set for h = %d, when saving data\n", h);
                    }

                } // avearging loop end

                /* Saving mean values */
                switch (h) {
                case 0:

                    Calib_data_short[ i ][ 1 ] = mean_array_column( Calib_data_short_for_avreaging, averaging_num, 1);
                    Calib_data_short[ i ][ 2 ] = mean_array_column( Calib_data_short_for_avreaging, averaging_num, 2);
                    break;
                case 1:
                    Calib_data_open[ i ][ 1 ] = mean_array_column(Calib_data_open_for_avreaging, averaging_num, 1);
                    Calib_data_open[ i ][ 2 ] = mean_array_column(Calib_data_open_for_avreaging, averaging_num, 2);
                    break;
                case 2:
                    Calib_data_load[ i ][ 1 ] = mean_array_column(Calib_data_load_for_avreaging, averaging_num, 1);
                    Calib_data_load[ i ][ 2 ] = mean_array_column(Calib_data_load_for_avreaging, averaging_num, 2);
                    break;
                case 3:
                    Calib_data_measure[ i ][ 1 ] = mean_array_column( Calib_data_measure_for_avreaging, averaging_num, 1 );
                    Calib_data_measure[ i ][ 2 ] = mean_array_column( Calib_data_measure_for_avreaging, averaging_num, 2 );
                    break;
                default:
                    printf("error no function set for h = %d, when averaging data\n", h);
                }

                /* dimension step defines index for sorting data depending on sweep function */
                if (sweep_function == 0 ) { //sweep_function == 0 (mesurement sweep)
                    dimension_step = i;
                }
                else if(sweep_function == 1) { //sweep_function == 1 (frequency sweep)
                   dimension_step = fr;
                }
                
                /* Saving data for output */
                //printf("frequency(%d) = %f;\n",(dimension_step),frequency[fr]);
                /* vector must be populated with the same values when measuremen sweep selected*/
                Z_short[ dimension_step ] =  Calib_data_short[ 0 ][ 1 ] + Calib_data_short[ 0 ][ 2 ] *I;
                Z_open[ dimension_step ]  =  Calib_data_open[ 0 ][ 1 ] + Calib_data_open[ 0 ][ 2 ] *I;
                Z_load[ dimension_step ]  =  Calib_data_load[ 0 ][ 1 ] + Calib_data_load[ 0 ][ 2 ] *I;

                Z_measure[dimension_step] = Calib_data_measure[i][1] + Calib_data_measure[i][2] *I;
            } // end of measurement sweep loop
        
        
        } // end of frequency sweep loop
    } // end of function step loop
    /* user is inquired to correcty set the connections */
    /**
    if (inquire_user_wait() < 0) {
        printf("error user inquiry at inquire_user_wait\n");
    } 
    */


    /* combining data from calibration measureents, if calibration wasn't made, only measurement data is saved */

    for (i = 0; i < end_results_dimension ; i++ ) {
        if (calib_function == 1) { // calib. was made including Z_load
            calib_data_combine[ 1 ] = creal( ( ( ( Z_short[i] - Z_measure[i]) * (Z_load[i] - Z_open[i]) ) / ( (Z_measure[i] - Z_open[i]) * (Z_short[i] - Z_load[i]) ) ) * Z_load_ref );
            calib_data_combine[ 2 ] = cimag( ( ( ( Z_short[i] - Z_measure[i]) * (Z_load[i] - Z_open[i]) ) / ( (Z_measure[i] - Z_open[i]) * (Z_short[i] - Z_load[i]) ) ) * Z_load_ref );
        }

        else if (calib_function == 0) { // no calib. were made, outputing data from measurements
            calib_data_combine[ 1 ] = creal( Z_measure[ i ]);
            calib_data_combine[ 2 ] = cimag( Z_measure[ i ]);
        }

        else if (calib_function == 2) { // calibration without Z_load
            calib_data_combine[ 1 ] = creal( ( ( ( Z_short[i] - Z_measure[i]) * ( Z_open[i]) ) / ( (Z_measure[i] - Z_open[i]) * (Z_short[i] - Z_load[i]) ) ) );
            calib_data_combine[ 2 ] = cimag( ( ( ( Z_short[i] - Z_measure[i]) * ( Z_open[i]) ) / ( (Z_measure[i] - Z_open[i]) * (Z_short[i] - Z_load[i]) ) ) );
        }
        
        /* Phase and amplitude calculation */
        PhaseZ[i] = ( 180 / M_PI) * (atan2f( calib_data_combine[ 2 ], calib_data_combine[ 1 ] ));
        AmplitudeZ[i] = sqrtf( powf( calib_data_combine[ 1 ], 2 ) + powf(calib_data_combine[ 2 ], 2 ) );
        if (!sweep_function) {
            printf(" %.0f    %.5f    %.5f\n", frequency[0],PhaseZ[ i ],AmplitudeZ[ i ]);
            frequency_lcr[i] = frequency[0];
            amplitude_lcr[i] = AmplitudeZ[i];
            phase_lcr[i] = PhaseZ[i];
        }
        else {
            printf(" %.0f    %.5f    %.5f\n", frequency[i],PhaseZ[ i ],AmplitudeZ[ i ]);
            frequency_lcr[i] = frequency[i];
            amplitude_lcr[i] = AmplitudeZ[i];
            phase_lcr[i] = PhaseZ[i];
        }
    
       
    }
    printf("%f\n", frequency[0]);
    printf("Ending function with no errors what so ever. And also, I'm bataman.\n");

    return 4000;

}

/**
 * Synthesize a desired signal.
 *
 * Generates/synthesized  a signal, based on three pre-defined signal
 * types/shapes, signal amplitude & frequency. The data[] vector of 
 * samples at 125 MHz is generated to be re-played by the FPGA AKot obljubljeno pošiljam dokumentacijo. Če so v njej kakšne napake, ali bi moral kateri del opisati bolj podrobno bom vesel če mi to sporočiš.

Koda za logaritemsko skalo je na koncu dokumenta nepravilna. Popraviti jo bo potrebno v prihodnjih dneh.

Razvoj kode je mogoče spremljati na githubu uporabnika PitayaDT https://github.com/PitayaDT/RedPitaya

    

Lep pozdrav
Martin
WG module.
 *
 * @param ampl  Signal amplitude [Vpp].
 * @param freq  Signal frequency [Hz].
 * @param type  Signal type/shape [Sine, Square, Triangle].
 * @param data  Returned synthesized AWG data vector.
 * @param awg   Returned AWG parameters.
 *
 */
void synthesize_signal_lcr(double ampl, double freq, signal_e_lcr type, double endfreq,
                       int32_t *data,
                       awg_param_t_lcr *awg) {

    uint32_t i;

    /* Various locally used constants - HW specific parameters */
    const int dcoffs = -155;
    const int trans0 = 30;
    const int trans1 = 300;
    //const double tt2 = 0.249;

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
        if (type == eSignalSine_lcr) {
            data[i] = round(amp * cos(2*M_PI*(double)i/(double)n));
        }
 
        /* Square 
        if (type == eSignalSquare) {
            data[i] = round(amp * cos(2*M_PI*(double)i/(double)n));
            if (data[i] > 0)
                data[i] = amp;
            else 
                data[i] = -amp;

            
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
        }*/
        
        /* Triangle 
        if (type == eSignalTriangle) {
            data[i] = round(-1.0*(double)amp*(acos(cos(2*M_PI*(double)i/(double)n))/M_PI*2-1));
        }*/

        /* Sweep */
        /* Loops from i = 0 to n = 16*1024. Generates a sine wave signal that
           changes in frequency as the buffer is filled. 
        double start = 2 * M_PI * freq;
        double end = 2 * M_PI * endfreq;
        if (type == eSignalSweep) {
            double sampFreq = c_awg_smpl_freq; // 125 MHz
            double t = i / sampFreq; // This particular sample
            double T = n / sampFreq; // Wave period = # samples / sample frequency
            // Actual formula. Frequency changes from start to end. 
            data[i] = round(amp * (sin((start*T)/log(end/start) * ((exp(t*log(end/start)/T)-1)))));
        }*/
        
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
void write_data_fpga_lcr(uint32_t ch,
                     const int32_t *data,
                     const awg_param_t_lcr *awg) {

    uint32_t i;
    uint32_t state_machine = g_awg_reg->state_machine_conf;
    fpga_awg_init();

    if(ch == 0) {
        /* Channel A */

        g_awg_reg->state_machine_conf = 0x000041;
        g_awg_reg->state_machine_conf = state_machine | 0xC0;
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
     /* Added */
    //g_awg_reg->state_machine_conf = state_machine | (mode_mask<<16);
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

 /* Write fpga made to work with trigged mode set to single and fixed adress pointers */

 void write_data_fpga_lcr_single(uint32_t ch, const int32_t *data,
                     const awg_param_t_lcr *awg){
    uint32_t i;
    uint32_t state_machine = g_awg_reg->state_machine_conf;

    /* Triger mode always to be single */
    int mode_mask = 0x20;

    if(ch == 0) {
        /* Channel A */
        state_machine &= ~0xff;

        g_awg_reg->state_machine_conf = state_machine | 0xC0;
        g_awg_reg->cha_scale_off      = awg->offsgain;
        g_awg_reg->cha_count_wrap     = awg->wrap;
        g_awg_reg->cha_count_step     = awg->step;
        g_awg_reg->cha_start_off      = 0;

        for(i = 0; i < AWG_SIG_LEN; i++) {
            g_awg_cha_mem[i] = data[i];
        }

        g_awg_reg->state_machine_conf = state_machine | mode_mask;
    } else {
        /* Channel B */
        state_machine &= ~0xff0000;

        g_awg_reg->state_machine_conf = state_machine | 0xC00000;
        g_awg_reg->chb_scale_off      = awg->offsgain;
        g_awg_reg->chb_count_wrap     = awg->wrap;
        g_awg_reg->chb_count_step     = awg->step;
        g_awg_reg->chb_start_off      = 0;

        for(i = 0; i < AWG_SIG_LEN; i++) {
            g_awg_chb_mem[i] = data[i];
        }

        g_awg_reg->state_machine_conf = state_machine | (mode_mask<<16);
    }
 }


int acquire_data(
                float **s , 
                uint32_t size) {
    int retries = 150;
    int j, sig_num, sig_len;
    int ret_val;
    printf("Int acquire function.\n");

    usleep(71754); // generator needs some time to start generating 31754
    printf("Starting while loop.\n");
    while(retries >= 0) {
        if((ret_val = rp_get_signals(&s, &sig_num, &sig_len)) >= 0) {
            printf("Signal acquire succesful.\n");
            /* Signals acquired in s[][]:
             * s[0][i] - TODO
             * s[1][i] - Channel ADC1 raw signal
             * s[2][i] - Channel ADC2 raw signal
             */
    
            for(j = 0; j < MIN(size, sig_len); j++) {
                //printf("[%7d, %7d];\n",(int)s[1][j], (int)s[2][j]);
            }
            break;
        }
        printf("%d\n", ret_val);
        if(retries-- == 0) {
            fprintf(stderr, "Signal scquisition was not triggered!\n");
            break;
        }
        usleep(1000);
    }
    usleep(41754); // generator needs some time to start generating 31754
    return 1;
}

int LCR_data_analasys(float **s ,
                        uint32_t size,
                        uint32_t DC_bias,
                        uint32_t R_shunt,
                        float complex *Z,
                        double w_out,
                        int f) {
    int i2, i3;
    float **U_acq = create_2D_table_size(SIGNALS_NUM, SIGNAL_LENGTH);
    /* Used for storing the Voltage and current on the load */
    float *U_dut = create_table_size( SIGNAL_LENGTH );
    float *I_dut = create_table_size( SIGNAL_LENGTH );
    /* Signals multiplied by the reference signal (sin) */
    float *U_dut_sampled_X = (float *) malloc( size * sizeof( float ) );
    float *U_dut_sampled_Y = (float *) malloc( size * sizeof( float ) );
    float *I_dut_sampled_X = (float *) malloc( size * sizeof( float ) );
    float *I_dut_sampled_Y = (float *) malloc( size * sizeof( float ) );
    /* Signals return by trapezoidal method in complex */
    float *X_component_lock_in_1 = (float *) malloc( size * sizeof( float ) );
    float *X_component_lock_in_2 = (float *) malloc( size * sizeof( float ) );
    float *Y_component_lock_in_1 = (float *) malloc( size * sizeof( float ) );
    float *Y_component_lock_in_2 = (float *) malloc( size * sizeof( float ) );
    /* Voltage, current and their phases calculated */
    float U_dut_amp;
    float Phase_U_dut_amp;
    float I_dut_amp;
    float Phase_I_dut_amp;
    float Phase_Z_rad;
    float Z_amp;
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

    /* Voltage and current on the load can be calculated from gathered data */
    for (i2 = 0; i2 < size; i2++) { 
        U_dut[i2] = (U_acq[1][i2] - U_acq[2][i2]); // potencial difference gives the voltage
        I_dut[i2] = (U_acq[2][i2] / R_shunt); // Curent trough the load is the same as trough thr R_shunt. ohm's law is used to calculate the current
        //printf("U_dut(%d,%d) = %f;\n",(1),(i2+1), U_dut[i2]);
        //printf("I_dut(%d,%d) = %f;\n",(1),(i2+1), I_dut[i2] );
    }

    /* Finding max values, used for ploting */
    /* comented because not used
    float U_load_max = max_array( U_dut , SIGNAL_LENGTH );
    float I_load_max = max_array( I_dut , SIGNAL_LENGTH );
    printf("U_load_max = %f \n", U_load_max);
    printf("I_load_max = %f\n", I_load_max);
    */
    /* Acquired signals must be multiplied by the reference signals, used for lock in metod */
    float ang;
    for( i2 = 0; i2 < size; i2++) {
        ang = (i2 * T * w_out);
        //printf("ang(%d) = %f \n", (i2+1), ang);
        U_dut_sampled_X[i2] = U_dut[i2] * sin( ang );
        U_dut_sampled_Y[i2] = U_dut[i2] * sin( ang+ (M_PI/2) );

        I_dut_sampled_X[i2] = I_dut[i2] * sin( ang );
        I_dut_sampled_Y[i2] = I_dut[i2] * sin( ang +(M_PI/2) );
        //printf("U_dut_sampled_X(%d) = %f;\n",(i2+1),U_dut_sampled_X[i2]);
        //printf("U_dut_sampled_Y(%d) = %f;\n",(i2+1),U_dut_sampled_Y[i2]);
        //printf("I_dut_sampled_X(%d) = %f;\n",(i2+1),I_dut_sampled_X[i2]);
        //printf("I_dut_sampled_Y(%d) = %f;\n",(i2+1),I_dut_sampled_Y[i2]);
    }

    /* Trapezoidal method for calculating the approximation of an integral */
    X_component_lock_in_1[1] = trapz( U_dut_sampled_X, (float)T, size );
    Y_component_lock_in_1[1] = trapz( U_dut_sampled_Y, (float)T, size );

    X_component_lock_in_2[1] = trapz( I_dut_sampled_X, (float)T, size );
    Y_component_lock_in_2[1] = trapz( I_dut_sampled_Y, (float)T, size );
    
    //printf("X_component_lock_in_1(%d) = %f;\n",(1),X_component_lock_in_1[1] );
    //printf("Y_component_lock_in_1(%d) = %f;\n",(1),Y_component_lock_in_1[1] );

    //printf("X_component_lock_in_2(%d) = %f;\n",( 1 ),X_component_lock_in_2[ 2 ] );
    //printf("Y_component_lock_in_1(%d) = %f;\n",( 1 ),Y_component_lock_in_2[ 2 ] );
    
    /* Calculating voltage amplitude and phase */
    U_dut_amp = (float)2 * (sqrtf( powf( X_component_lock_in_1[ 1 ] , (float)2 ) + powf( Y_component_lock_in_1[ 1 ] , (float)2 )));
    Phase_U_dut_amp = atan2f( Y_component_lock_in_1[ 1 ], X_component_lock_in_1[ 1 ] );
    //printf("U_load_amp(%d) = %f;\n",(1),U_load_amp );
    //printf("Phase_U_load_amp(%d) = %f;\n",(1),Phase_U_load_amp );

    /* Calculating current amplitude and phase */
    I_dut_amp = (float)2 * (sqrtf( powf( X_component_lock_in_2[ 1 ], (float)2 ) + powf( Y_component_lock_in_2[ 1 ] , (float)2 ) ) );
    Phase_I_dut_amp = atan2f( Y_component_lock_in_2[1], X_component_lock_in_2[1] );
    //printf("I_load_amp(%d) = %f;\n",(1),I_load_amp );
    //printf("Phase_I_load_amp(%d) = %f;\n",(1),Phase_I_load_amp );
    
    /* Asigning impedance  values (complex value) */

    Phase_Z_rad =  Phase_U_dut_amp - Phase_I_dut_amp;
    Z_amp = U_dut_amp / I_dut_amp; // forming resistance
    
           

    if (Phase_Z_rad <=  (-M_PI) )
    {
        Phase_Z_rad = Phase_Z_rad +(2*M_PI);
    }
    else if ( Phase_Z_rad >= M_PI )
    {
        Phase_Z_rad = Phase_Z_rad -(2*M_PI) ;
    }
    else 
    {
        Phase_Z_rad = Phase_Z_rad;
    } 
 
   
    *Z =  ( ( Z_amp ) * cosf( Phase_Z_rad ) )  +  ( ( Z_amp ) * sinf( Phase_Z_rad ) ) * I; // R + jX

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



/*
void run_lcr(){

    float *frequency_lcr = (float *)malloc(300 * sizeof(float));
    float *amplitude_lcr = (float *)malloc(300 * sizeof(float));
    float *phase_lcr = (float *)malloc(300 * sizeof(float));

    lcr(0, 1, 0, 996, 1, 0, 0, 0, 1, 1, 4000, 8000,  0, 0, frequency_lcr, phase_lcr, amplitude_lcr);
}
*/

/*
int rp_osc_set_signals_lcr(float **source, int index)
{
    //pthread_mutex_lock(&rp_osc_sig_mutex);

    memcpy(&rp_lcr_signals[0][0], &source[0][0], sizeof(float)*SIGNAL_LENGTH);
    memcpy(&rp_lcr_signals[1][0], &source[1][0], sizeof(float)*SIGNAL_LENGTH);
    memcpy(&rp_lcr_signals[2][0], &source[2][0], sizeof(float)*SIGNAL_LENGTH);
    //rp_osc_sig_last_idx = index;

    //rp_osc_signals_dirty = 1;
    //pthread_mutex_unlock(&rp_osc_sig_mutex);

    return 0;
}

void lcr_acquire(float **rp_osc_signals){
     Initializing the pointers for the FPGA input signal buffers
    osc_fpga_get_sig_ptr(&rp_fpga_cha_signal, &rp_fpga_chb_signal);z
    /We inicialize our working signal 
    rp_create_signals(&rp_lcr_signals);

     We fill the rp_lcr_signals with data directly from the FPGA buffer.
    rp_osc_set_signals_lcr(rp_lcr_signals, SIGNAL_LENGTH-1);
}

void simple_data_acquisition(float   **s){

    signal_e_lcr type = eSignalSine_lcr;
    awg_param_t_lcr params;
    synthesize_signal_lcr(2.0, 9000.0, type, 1000000.0, data, &params);

    write_data_fpga_lcr_single(0, data, &params);

    lcr_acquire(s);

    printf("Succesfuly init synthesize_signal_lcr and write_data_fpga_lcr_single!!\n");
    
    
}
*/