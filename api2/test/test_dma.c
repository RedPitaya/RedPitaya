
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>

#include "la_acq.h"
#include "rp_api.h"

int suite_la_acq_init(void)
{
    if(rp_OpenUnit()!=RP_OK){
        return -1;
    }
    return 0;
}

int suite_la_acq_cleanup(void)
{
    if(rp_CloseUnit()!=RP_OK){
        return -1;
    }
    return 0;
}

void rpReadyCallback(RP_STATUS status, void * pParameter)
{
    printf("\r\nACQ_CALLBACK");
}

pthread_t tid;

void* trigGen(void *arg)
{
    sleep(2);
    rp_DigSigGenSoftwareControl(1);
    return NULL;
}

int main () {
    pthread_t tid;
    RP_STATUS s;
    RP_DIGITAL_CHANNEL_DIRECTIONS dir[1];
    dir[0].channel=RP_DIGITAL_CHANNEL_7;
    dir[0].direction=RP_DIGITAL_DIRECTION_RISING;

    if(rp_OpenUnit()!=RP_OK){
        return -1;
    }

    for (int n=0; n<1; n++) {
        rp_DigSigGenOuput(true);
        double sample_rate=125e6;
        rp_SetDigSigGenBuiltIn(RP_DIG_SIGGEN_PAT_UP_COUNT_8BIT_SEQ_256,&sample_rate,0,0,RP_TRG_DGEN_SWE_MASK);
        //printf("sample rate %lf",sample_rate);
    
        // start trigger a bit later in a new thread
        int err;
        err = pthread_create(&tid, NULL, &trigGen, NULL);
        if (err != 0)
            printf("\ncan't create thread :[%s]", strerror(err));
        else
            printf("\n Thread created successfully\n");
    
        printf("\r\nTriggers");
        s=rp_SetTriggerDigitalPortProperties(dir,1);
        if(s!=RP_API_OK){
            printf("Failed to set trigger properties.");
        }
    
        // enable RLE
        rp_EnableDigitalPortDataRLE(0);
    
        printf("\r\nRunBlock");
        double timeIndisposedMs;
        uint32_t pre=100;
        uint32_t post=8000;
        s=rp_RunBlock(pre,post,0,&timeIndisposedMs,&rpReadyCallback,NULL);
        if(s!=RP_API_OK){
            printf("Failed to acquire data.");
        }
    
        uint32_t samples=pre+post;
    
        int16_t * buf = malloc(samples * sizeof(int16_t));
        if (NULL == buf) {
            printf("malloc failed");
        }
    
        // set data buffer to which data will be read from memory space
        rp_SetDataBuffer(RP_CH_DIN,buf,samples,RP_RATIO_MODE_NONE);
    
        // get data
        rp_GetValues(0,&samples,1,RP_RATIO_MODE_NONE,NULL);
    
        // verify data
        int first=buf[0];
        for(int i=0;i<samples;i++){
            if(buf[i]!=first){
                printf("\n\r data mismatch @ i=%d buf=%04x exp=%04x",i,buf[i],first);
                break;
            }
            if(first==0xff)
                first=0;
            else
                first++;
        }
    
        // verify trigger position
        printf("\n\r data @ trigger pos \n\r");
        printf("\n\r %04x",buf[pre-1]);
        printf("\n\r ->%04x",buf[pre]);
        printf("\n\r %04x",buf[pre+1]);
    
        free(buf);
    }

    if(rp_CloseUnit()!=RP_OK){
        return -1;
    }

    return (0);
}

