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

}