/* Red Pitaya C API example Generating signal pulse on an external trigger 
 * This application generates a specific signal */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "rp.h"

int main(int argc, char **argv){

        /* Burst count */


        /* Print error, if rp_Init() function failed */
        if(rp_Init() != RP_OK){
                fprintf(stderr, "Rp api init failed!\n");
        }

        rp_GenReset();

        rp_GenWaveform(RP_CH_1, RP_WAVEFORM_SINE);
        rp_GenFreq(RP_CH_1, 1000);
        rp_GenAmp(RP_CH_1, 1.0);

        rp_GenMode(RP_CH_1, RP_GEN_MODE_BURST);
        rp_GenBurstCount(RP_CH_1, 1);
        rp_GenBurstRepetitions(RP_CH_1, 10000);
        rp_GenBurstPeriod(RP_CH_1, 5000);

        rp_GenOutEnable(RP_CH_1);
        rp_GenTriggerOnly(RP_CH_1);

        rp_Release();
}



