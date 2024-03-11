/* Red Pitaya C API example Acquiring a signal from a buffer
 * This application acquires a signal on a specific channel */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "rp.h"

#define DATA_SIZE 64

int main(int argc, char **argv)
{
    int dsize = DATA_SIZE;
    uint32_t dec = 1;
    if (argc >= 3){
        dsize = atoi(argv[1]);
        dec = atoi(argv[2]);
    }

    /* Print error, if rp_Init() function failed */
    if (rp_InitReset(false) != RP_OK) {
        fprintf(stderr, "Rp api init failed!\n");
        return -1;
    }

    uint32_t g_adc_axi_start,g_adc_axi_size;
    rp_AcqAxiGetMemoryRegion(&g_adc_axi_start,&g_adc_axi_size);

    printf("Reserved memory start 0x%X size 0x%X\n",g_adc_axi_start,g_adc_axi_size);
//    rp_AcqResetFpga();

    if (rp_AcqAxiSetDecimationFactor(dec) != RP_OK) {
        fprintf(stderr, "rp_AcqAxiSetDecimationFactor failed!\n");
        return -1;
    }
    if (rp_AcqAxiSetTriggerDelay(RP_CH_1, dsize  )  != RP_OK) {
        fprintf(stderr, "rp_AcqAxiSetTriggerDelay RP_CH_1 failed!\n");
        return -1;
    }
    if (rp_AcqAxiSetTriggerDelay(RP_CH_2, dsize  ) != RP_OK) {
        fprintf(stderr, "rp_AcqAxiSetTriggerDelay RP_CH_2 failed!\n");
        return -1;
    }
    if (rp_AcqAxiSetBufferSamples(RP_CH_1, g_adc_axi_start, dsize) != RP_OK) {
        fprintf(stderr, "rp_AcqAxiSetBuffer RP_CH_1 failed!\n");
        return -1;
    }
    if (rp_AcqAxiSetBufferSamples(RP_CH_2, g_adc_axi_start + g_adc_axi_size / 2, dsize) != RP_OK) {
        fprintf(stderr, "rp_AcqAxiSetBuffer RP_CH_2 failed!\n");
        return -1;
    }
    if (rp_AcqAxiEnable(RP_CH_1, true)) {
        fprintf(stderr, "rp_AcqAxiEnable RP_CH_1 failed!\n");
        return -1;
    }
    if (rp_AcqAxiEnable(RP_CH_2, true)) {
        fprintf(stderr, "rp_AcqAxiEnable RP_CH_2 failed!\n");
        return -1;
    }

    rp_AcqSetTriggerLevel(RP_T_CH_1,0);

    if (rp_AcqStart() != RP_OK) {
        fprintf(stderr, "rp_AcqStart failed!\n");
        return -1;
    }

    rp_AcqSetTriggerSrc(RP_TRIG_SRC_CHA_PE);
    rp_acq_trig_state_t state = RP_TRIG_STATE_TRIGGERED;

    while(1){
        rp_AcqGetTriggerState(&state);
        if(state == RP_TRIG_STATE_TRIGGERED){
            sleep(1);
            break;
        }
    }

    bool fillState = false;
    while (!fillState) {
        if (rp_AcqAxiGetBufferFillState(RP_CH_1, &fillState) != RP_OK) {
            fprintf(stderr, "rp_AcqAxiGetBufferFillState RP_CH_1 failed!\n");
            return -1;
        }
    }
    rp_AcqStop();

    uint32_t posChA,posChB;
    rp_AcqAxiGetWritePointerAtTrig(RP_CH_1,&posChA);
    rp_AcqAxiGetWritePointerAtTrig(RP_CH_2,&posChB);

    fprintf(stderr,"Tr pos1: 0x%X pos2: 0x%X\n",posChA,posChB);

    int16_t *buff1 = (int16_t *)malloc(dsize * sizeof(int16_t));
    int16_t *buff2 = (int16_t *)malloc(dsize * sizeof(int16_t));

    uint32_t size1 = dsize;
    uint32_t size2 = dsize;
    rp_AcqAxiGetDataRaw(RP_CH_1, posChA, &size1, buff1);
    rp_AcqAxiGetDataRaw(RP_CH_2, posChB, &size2, buff2);

    for (int i = 0; i < dsize; i++) {
        printf("[%d]\t%d\t%d\n",i,buff1[i], buff2[i]);
    }

    /* Releasing resources */
    rp_AcqAxiEnable(RP_CH_1, false);
    rp_AcqAxiEnable(RP_CH_2, false);
    rp_Release();
    free(buff1);
    free(buff2);
    return 0;
}
