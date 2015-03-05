/* This is a red pitaya test application for deep averagning */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>

#include "../../api-mockup/rpbase/src/rp.h"

int main(int argc, char **argv){

    uint32_t buff_size = 16 * 1024;
    uint32_t i;
    uint32_t seq_len = 250;
    uint32_t count;
    rp_acq_trig_src_t run;
    int32_t *buffer = (int32_t *)malloc(buff_size * sizeof(int32_t));
    printf("Starting RP API\n");
    /* Print error, if rp_Init() function failed */

    if(rp_Init() != RP_OK){
    	fprintf(stderr, "Rp api init failed!\n");
    }

	/* Generating frequency */
    rp_GenFreq(RP_CH_1, 1000000.0);
 
    /* Generating amplitude */
    rp_GenAmp(RP_CH_1, 1.0);
 
    /* Generating wave form */
    rp_GenWaveform(RP_CH_1, RP_WAVEFORM_SINE);
 
    /* Enable channel */
    rp_GenOutEnable(RP_CH_1);

    printf("Successful start API\n");
    printf("Setting seq len.\n");
    rp_SetDeepDataSeqLen(seq_len);
    printf("Printing seq len.\n");
    rp_SetDeepAvgCount(100);
    printf("Successfuly set count.\n");
    rp_SetDeepAvgShift(0);
    printf("Successfuly set shift.\n");
    rp_SetDeepAvgDebTim(0);
    printf("Set shift\n");
    rp_AcqSetTriggerSrc(RP_TRIG_SRC_CHA_PE);
    rp_AcqSetTriggerLevel(0);
    rp_AcqSetTriggerHyst(0.1);
    printf("Set trigger\n");
    rp_DeepAvgStart();
	
    printf("Started\n");
    printf("Do while\n");

    do{
        rp_AcqSetTriggerSrc(RP_TRIG_SRC_NOW);
        rp_GetDeepAvgRunState(&run);
        printf("Count: %d  || RunState: %d\n",rp_GetDeepAvgCount(&count), run);
    }while(run);
	
    printf("Did while.\n");
    rp_GetDeepAvgCount(&count);
    printf("Count: %d\n", count);
    printf("Sequence length: %d\n", seq_len);

    printf("Getting data.\n");
    rp_GetDeepAvgRawData(RP_CH_1, &buff_size, buffer);
    printf("Successfuly set data.\n");

    for(i = 0; i< seq_len; i++){
    	printf("Data %d: %d", i,buffer[i]);
    }
    printf("Done.\n");

}
