     
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "fpga_awg.h"
#include <unistd.h>
#include <errno.h>


/** AWG buffer length [samples]*/
#define bufferSize (16*1024)
#define nSamples (16*1024)
/** AWG data buffer */
int32_t data[bufferSize];


/** AWG FPGA parameters */
typedef struct {
    int32_t  offsgain;   ///< AWG offset & gain.
    uint32_t wrap;       ///< AWG buffer wrap value.
    uint32_t step;       ///< AWG step interval.
} awg_param_t;

/* Forward declarations */

void write_data_fpga(uint32_t ch,
                     const int32_t *data,
                     const awg_param_t *awg);

int setSignal(double frq1, double frq2){
 // fprintf(stderr, "Hello setSignal()\n");
  int retval = EXIT_SUCCESS;
// channel a: 
 // double frq=0 ;
  uint32_t ch = 0 ;
  uint32_t i ;
  awg_param_t awg ;
  double ampl = 2.0 ;
  uint32_t amp = ampl * 4000.0;    /* 1 Vpp ==> 4000 DAC counts */
  const int dcoffs = -155;
  awg.offsgain = (dcoffs << 16) + 0x1fff;
  //frq=150e3 ;
  awg.step = round(65536 * frq1/c_awg_smpl_freq * nSamples)-0; 
  awg.wrap = round(65536 * nSamples - 1);
  for(i = 0; i < nSamples; i++) {
    data[i] =  round(amp * cos(2*M_PI*(double)i/(double)nSamples));
    }
  write_data_fpga(ch, data, &awg );
// channel b:
  ch = 1 ;
  ampl = 2.0 ;
  amp = ampl * 4000.0;    /* 1 Vpp ==> 4000 DAC counts */
  awg.offsgain = (dcoffs << 16) + 0x1fff;
 // frq=200e3 ;
  awg.step = round(65536 * frq2/c_awg_smpl_freq * nSamples)-0; 
  awg.wrap = round(65536 * nSamples - 1);
  for(i = 0; i < nSamples; i++) {
    data[i] = round(amp * cos(2*M_PI*(double)i/(double)nSamples));
    }
  fprintf(stderr,"setup waveform data..\n") ;
  write_data_fpga(ch, data, &awg );
//fpga_awg_init();
  return retval ;
  }

void setFrq(double frq1, double frq2){
 fpga_awg_init();
  uint32_t step=round(65536 * frq1/c_awg_smpl_freq * nSamples)-0; 
  g_awg_reg->cha_count_step     = step;
  step=round(65536 * frq2/c_awg_smpl_freq * nSamples)-0; 
  g_awg_reg->chb_count_step     = step;
  fpga_awg_exit();
  }



void write_data_fpga(uint32_t ch, const int32_t *data, const awg_param_t *awg) {
  uint32_t i;
  fpga_awg_init();
  if(ch == 0) {
    /* Channel A */
    g_awg_reg->state_machine_conf = 0x000041;
    g_awg_reg->cha_scale_off      = awg->offsgain;
    g_awg_reg->cha_count_wrap     = awg->wrap;
    g_awg_reg->cha_count_step     = awg->step;
    g_awg_reg->cha_start_off      = 0;
    for(i = 0; i < bufferSize; i++) {
      g_awg_cha_mem[i] = data[i];
      }
    } else {
    /* Channel B */
    g_awg_reg->state_machine_conf = 0x410000;
    g_awg_reg->chb_scale_off      = awg->offsgain;
    g_awg_reg->chb_count_wrap     = awg->wrap;
    g_awg_reg->chb_count_step     = awg->step;
    g_awg_reg->chb_start_off      = 0;
    for(i = 0; i < bufferSize; i++) {
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
