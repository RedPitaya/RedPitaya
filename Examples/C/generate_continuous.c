/* Red Pitaya C API example Generating continuous signal
* This application generates a specific signal */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "rp.h"

int main(int argc, char **argv, int count, int freq){

    /* Print error, if rp_Init() function failed */
    if(rp_Init() != RP_OK){
        fprintf(stderr, "Rp api init failed!\n");
    }
    int count=0;
    int freq= 1000;
    while (count<= 1000)
    {       
    
       /* Generating frequency */
       rp_GenFreq(RP_CH_1, freq);

       /* Generating amplitude */
       rp_GenAmp(RP_CH_1, 1.0);

      /* Generating wave form */
      rp_GenWaveform(RP_CH_1, RP_WAVEFORM_SINE);

      /* Enable channel */
      rp_GenOutEnable(RP_CH_1);

       /* Releasing resources */
      rp_Release();
       count++,
       freq=freq + 50; 
       delay(0.001)
    return 0;
}
