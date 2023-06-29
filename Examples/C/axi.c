/* Red Pitaya C API example Acquiring a signal from a buffer
 * This application acquires a signal on a specific channel */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "rp.h"

#define DATA_SIZE 1024

int main(int argc, char **argv)
{
    /* Print error, if rp_Init() function failed */
    if (rp_InitReset(false) != RP_OK) {
        fprintf(stderr, "Rp api init failed!\n");
        return -1;
    }

    uint32_t g_adc_axi_start,g_adc_axi_size;
    rp_AcqAxiGetMemoryRegion(&g_adc_axi_start,&g_adc_axi_size);

    printf("Reserved memory start 0x%X size 0x%X\n",g_adc_axi_start,g_adc_axi_size);
//    rp_AcqResetFpga();

    if (rp_AcqAxiSetDecimationFactor(RP_DEC_1) != RP_OK) {
        fprintf(stderr, "rp_AcqAxiSetDecimationFactor failed!\n");
        return -1;
    }
    if (rp_AcqAxiSetTriggerDelay(RP_CH_1, DATA_SIZE  )  != RP_OK) {
        fprintf(stderr, "rp_AcqAxiSetTriggerDelay RP_CH_1 failed!\n");
        return -1;
    }
    if (rp_AcqAxiSetTriggerDelay(RP_CH_2, DATA_SIZE  ) != RP_OK) {
        fprintf(stderr, "rp_AcqAxiSetTriggerDelay RP_CH_2 failed!\n");
        return -1;
    }
    if (rp_AcqAxiSetBufferSamples(RP_CH_1, g_adc_axi_start, DATA_SIZE) != RP_OK) {
        fprintf(stderr, "rp_AcqAxiSetBuffer RP_CH_1 failed!\n");
        return -1;
    }
    if (rp_AcqAxiSetBufferSamples(RP_CH_2, g_adc_axi_start + g_adc_axi_size / 2, DATA_SIZE) != RP_OK) {
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

    int16_t *buff1 = (uint16_t *)malloc(DATA_SIZE * sizeof(int16_t));
    int16_t *buff2 = (uint16_t *)malloc(DATA_SIZE * sizeof(int16_t));

    uint32_t size1 = DATA_SIZE;
    uint32_t size2 = DATA_SIZE;
    rp_AcqAxiGetDataRaw(RP_CH_1, posChA, &size1, buff1);
    rp_AcqAxiGetDataRaw(RP_CH_2, posChB, &size2, buff2);

    for (int i = 0; i < DATA_SIZE; i++) {
        printf("%d\t%d\n", buff1[i], buff2[i]);
    }

    /* Releasing resources */
    rp_AcqAxiEnable(RP_CH_1, false);
    rp_AcqAxiEnable(RP_CH_2, false);
    rp_Release();
    free(buff1);
    free(buff2);
    return 0;
}
