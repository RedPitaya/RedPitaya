/* Red Pitaya C API example Acquiring a signal from a buffer
 * This application acquires a signal on a specific channel */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "rp.h"

#define DATA_SIZE 20
#define OFFSET 10

int main(int argc, char **argv){

        bool failTest = false;
        bool Verbose = false;
        bool PrintBuff = false;
        rp_channel_t ch = RP_CH_1;
        for(int i = 0; i < argc; i++){
                if (strcmp(argv[i],"-v")==0){
                        Verbose = true;
                }
                if (strcmp(argv[i],"-b")==0){
                        PrintBuff = true;
                }
                if (strcmp(argv[i],"-h")==0){
                        printf("For test need pass 500kHz Sine signal on IN1\n");
                        return 0;
                }
        }

        /* Print error, if rp_Init() function failed */
        if(rp_InitReset(false) != RP_OK){
                fprintf(stderr, "Rp api init failed!\n");
        }


        for(int dec = 1; dec < 512; dec *= 2){
                rp_AcqReset();
                rp_AcqSetDecimationFactor(dec);
                rp_AcqSetTriggerLevel(ch, 0.0);
                rp_AcqSetTriggerHyst(0.005);
                rp_AcqSetTriggerDelay(0);

                rp_AcqStart();

                /* After acquisition is started some time delay is needed in order to acquire fresh samples in to buffer*/
                /* Here we have used time delay of one second but you can calculate exact value taking in to account buffer*/
                /*length and smaling rate*/

                usleep(50000);
                rp_AcqSetTriggerSrc(RP_TRIG_SRC_CHA_PE);
                rp_acq_trig_state_t state = RP_TRIG_STATE_TRIGGERED;

                while(1){
                        rp_AcqGetTriggerState(&state);
                        if(state == RP_TRIG_STATE_TRIGGERED){
                        break;
                        }
                }

                bool fillState = false;
                while(!fillState){
                        rp_AcqGetBufferFillState(&fillState);
                }

                int16_t data[DATA_SIZE];
                uint32_t trig_pos;
                uint32_t size = DATA_SIZE;
                rp_AcqGetWritePointerAtTrig(&trig_pos);
                trig_pos = (trig_pos + ADC_BUFFER_SIZE  - OFFSET) % ADC_BUFFER_SIZE;
                rp_AcqGetDataRaw(ch,trig_pos,&size,(int16_t*)&data);
                if (PrintBuff){
                        for(int i = 0; i < DATA_SIZE; i++){
                                printf("%d) %d\n", i-OFFSET, data[i]);
                        }
                }

                if (!(data[9] < data[10] && data[9] < 0 && data[10] > 0)){
                        failTest = true;
                        if (Verbose){
                                printf("Test fail. PE Mode. Decimate: %d; pre trig value: %d; after trig value: %d\n",dec,data[9],data[10]);
                        }
                }else{
                        if (Verbose){
                                printf("Test OK. PE Mode. Decimate: %d; pre trig value: %d; after trig value: %d\n",dec,data[9],data[10]);
                        }
                }
        }


        for(int dec = 1; dec < 512; dec *= 2){
                rp_AcqReset();
                rp_AcqSetDecimationFactor(dec);
                rp_AcqSetTriggerLevel(ch, 0.0);
                rp_AcqSetTriggerHyst(0.005);
                rp_AcqSetTriggerDelay(0);

                rp_AcqStart();

                /* After acquisition is started some time delay is needed in order to acquire fresh samples in to buffer*/
                /* Here we have used time delay of one second but you can calculate exact value taking in to account buffer*/
                /*length and smaling rate*/

                usleep(50000);
                rp_AcqSetTriggerSrc(RP_TRIG_SRC_CHA_NE);
                rp_acq_trig_state_t state = RP_TRIG_STATE_TRIGGERED;

                while(1){
                        rp_AcqGetTriggerState(&state);
                        if(state == RP_TRIG_STATE_TRIGGERED){
                        break;
                        }
                }

                bool fillState = false;
                while(!fillState){
                        rp_AcqGetBufferFillState(&fillState);
                }

                int16_t data[DATA_SIZE];
                uint32_t trig_pos;
                uint32_t size = DATA_SIZE;
                rp_AcqGetWritePointerAtTrig(&trig_pos);
                trig_pos = (trig_pos + ADC_BUFFER_SIZE  - OFFSET) % ADC_BUFFER_SIZE;
                rp_AcqGetDataRaw(ch,trig_pos,&size,(int16_t*)&data);
                if (PrintBuff){
                        for(int i = 0; i < DATA_SIZE; i++){
                                printf("%d) %d\n", i-OFFSET, data[i]);
                        }
                }

                if (!(data[9] > data[10] && data[9] > 0 && data[10] < 0)){
                        failTest = true;
                        if (Verbose){
                                printf("Test fail. NE Mode. Decimate: %d; pre trig value: %d; after trig value: %d\n",dec,data[9],data[10]);
                        }
                }else{
                        if (Verbose){
                                printf("Test OK. NE Mode. Decimate: %d; pre trig value: %d; after trig value: %d\n",dec,data[9],data[10]);
                        }
                }
        }
        rp_Release();
        return failTest;
}