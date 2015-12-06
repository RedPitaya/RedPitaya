

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "redpitaya/rp.h"

int main(int argc, char **argv){

        /* Print error, if rp_Init() function failed */
        if(rp_Init() != RP_OK){
                fprintf(stderr, "Rp api init failed!\n");
        }

        
        int selected_frequency=10000.0;
        int min_periodes = 8;
        int signal_decimation;
        float wait;

        if      (selected_frequency == 100000.0)            {  wait = 200;    }     
        else if (selected_frequency == 10000.0)             {  wait = 2000;   }    
        else if (selected_frequency == 1000.0)              {  wait = 20000;  }   
        else if (selected_frequency == 100.0)               {  wait = 200000; }

        /*Generate excitation signal*/
        rp_GenReset();
        rp_GenFreq(RP_CH_2, selected_frequency);
        rp_GenAmp(RP_CH_2, 0.25);
        rp_GenOffset(RP_CH_2, 0.25);
        rp_GenWaveform(RP_CH_2, RP_WAVEFORM_SINE);
        rp_GenOutEnable(RP_CH_2);

        
        rp_AcqReset();

        if      (selected_frequency == 100000.0)       {     rp_AcqSetDecimation(RP_DEC_1);     signal_decimation = 1;    }
        else if (selected_frequency == 10000.0)        {     rp_AcqSetDecimation(RP_DEC_8);     signal_decimation = 8;    }
        else if (selected_frequency == 1000.0)         {     rp_AcqSetDecimation(RP_DEC_64);    signal_decimation = 64;   }
        else if (selected_frequency == 100.0)          {     rp_AcqSetDecimation(RP_DEC_1024);  signal_decimation = 1024; }

        uint32_t buff_size = ((min_periodes*125e6)/(selected_frequency*signal_decimation));
        float *buff_in1 = (float *)malloc(buff_size * sizeof(float));
        float *buff_in2 = (float *)malloc(buff_size * sizeof(float));
        
        rp_AcqSetTriggerLevel(0.5);  
        rp_AcqSetTriggerDelay(8192);
        rp_AcqStart();        
        

        rp_AcqSetTriggerSrc(RP_TRIG_SRC_CHA_PE);
        rp_acq_trig_state_t state = RP_TRIG_STATE_TRIGGERED;

        while(1){
                rp_AcqGetTriggerState(&state);
                if(state == RP_TRIG_STATE_TRIGGERED){
                break;
                }
        }
                       
        usleep(wait); 
        rp_AcqGetOldestDataV(RP_CH_1, &buff_size, buff_in1);
        rp_AcqGetOldestDataV(RP_CH_2, &buff_size, buff_in2);
        
        int i;
        for(i = 0; i < buff_size; i++){
               printf("%f\n", buff_in1[i]);
               printf("%f\n", buff_in2[i]);
                       }
        /* Releasing resources */
        free(buff_in1);
        free(buff_in2);
        rp_Release();
        return 0;
}

        
