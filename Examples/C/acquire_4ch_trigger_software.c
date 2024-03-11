/* Red Pitaya C API example Acquiring a signal from a buffer
 * This application acquires a signal on a specific channel */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "rp.h"
#include "rp_hw-profiles.h"


int main(int argc, char **argv){

        uint8_t c = 0;
        if (rp_HPGetFastADCChannelsCount(&c) != RP_HP_OK){
                fprintf(stderr,"[Error] Can't get fast ADC channels count\n");
                return 1;
        }

        if (c!= 4){
                fprintf(stderr,"[Error] The number of channels is wrong\n");
                return 1;
        }

        /* Print error, if rp_Init() function failed */
        if(rp_Init() != RP_OK){
                fprintf(stderr, "Rp api init failed!\n");
        }

        uint32_t buff_size = 16384;

        rp_AcqReset();
        rp_AcqSetDecimation(RP_DEC_8);
        rp_AcqSetTriggerDelay(0);

        rp_AcqStart();

        /* After acquisition is started some time delay is needed in order to acquire fresh samples in to buffer*/
        /* Here we have used time delay of one second but you can calculate exact value taking in to account buffer*/
        /*length and smaling rate*/

        sleep(1);
        rp_AcqSetTriggerSrc(RP_TRIG_SRC_NOW);
        rp_acq_trig_state_t state = RP_TRIG_STATE_TRIGGERED;

        while(1){
                rp_AcqGetTriggerState(&state);
                if(state == RP_TRIG_STATE_TRIGGERED){
                sleep(1);
                break;
                }
        }

        bool fillState = false;
        while(!fillState){
		rp_AcqGetBufferFillState(&fillState);
	}

        uint32_t pos = 0;
	rp_AcqGetWritePointerAtTrig(&pos);
        buffers_t *b = rp_createBuffer(4,buff_size,false,false,true);

        rp_AcqGetData(pos, b);

        int i;
        for(i = 0; i < buff_size; i++){
                printf("%f %f %f %f\n", b->ch_f[0][i],b->ch_f[1][i],b->ch_f[2][i],b->ch_f[3][i]);
        }
        /* Releasing resources */
        rp_deleteBuffer(b);
        rp_Release();

        return 0;
}
