/* Red Pitaya C API example Acquiring a signal from a buffer  
 * This application acquires a signal on a specific channel */

#include <stdio.h>
#include <stdlib.h>

#include "redpitaya/rp.h"

int main(int argc, char **argv){

        /* Print error, if rp_Init() function failed */
        if(rp_Init() != RP_OK){
                fprintf(stderr, "Rp api init failed!\n");
        }

        /*LOOB BACK FROM OUTPUT 1 - ONLY FOR TESTING*/ 
        rp_GenReset();
        rp_GenFreq(RP_CH_1, 20000.0);
        rp_GenAmp(RP_CH_1, 1.0);
        rp_GenWaveform(RP_CH_1, RP_WAVEFORM_SINE);
        rp_GenOutEnable(RP_CH_1);
        
        int seconds=1;
        uint32_t buff_size = 16384;
        sleep(seconds);
        float *buff = (float *)malloc(buff_size * sizeof(float));


        rp_AcqReset();
        rp_AcqSetDecimation(1);
        rp_AcqSetTriggerLevel(0.1); 
        rp_AcqSetTriggerDelayNs(0);
        rp_AcqStart();
        rp_AcqSetTriggerSrc(RP_TRIG_SRC_CHA_PE);
        rp_acq_trig_state_t state = RP_TRIG_STATE_TRIGGERED;

        while(1){
                rp_AcqGetTriggerState(&state);
                if(state == RP_TRIG_STATE_TRIGGERED){
                sleep(seconds);     
                break;
                }
        }

        rp_AcqGetOldestDataV(RP_CH_1, &buff_size, buff);

        int i;
        for(i = 0; i < buff_size; i++){
                printf("%f\n", buff[i]);
        }
       
	/* Releasing resources */
	free(buff);
	rp_Release();

	return 0;
}
