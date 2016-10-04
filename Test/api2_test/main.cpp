#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>

extern "C" {
#include "rp_api.h"
#include "la_acq.h"
}

pthread_t tid;
pthread_t tid2;
pthread_t tid3;

void rpReadyCallback(RP_STATUS status, void * pParameter)
{
    printf("\r\nACQ_CALLBACK");
}

void* trigAcq(void *arg)
{
    printf("\r\nTrigAcqThreadStarted");
    sleep(2);
    printf("\r\nSW trigger");
    if (rp_SoftwareTrigger() != RP_API_OK) {
    	printf("\r\nCannot trigger acq.");
    	rp_Stop();
    }
    return NULL;
}

void* stopAcq(void *arg)
{
    printf("\r\nStopAcqThreadStarted");
    sleep(2);
    printf("\r\nStopAcq");
    rp_Stop();
    return NULL;
}

int main()
{
    if (rp_OpenUnit() != RP_OK) {
		puts("open unit fail");
        return -1;
    }

    rp_DigSigGenOuput(true);
    double sample_rate=125e6;
    rp_SetDigSigGenBuiltIn(RP_DIG_SIGGEN_PAT_UP_COUNT_8BIT_SEQ_256,&sample_rate,0,0,RP_TRG_DGEN_SWE_MASK);

    RP_STATUS s;
    RP_DIGITAL_CHANNEL_DIRECTIONS dir[1];
    dir[0].channel = RP_DIGITAL_CHANNEL_7;
    dir[0].direction = RP_DIGITAL_DIRECTION_RISING;

    printf("\r\nTriggers");
    s = rp_SetTriggerDigitalPortProperties(dir, 1);
	if (s) {
        puts("Failed to set trigger properties.");
    }

    // enable RLE
    rp_EnableDigitalPortDataRLE(1);

    // stop acq.
    if (pthread_create(&tid3, NULL, &stopAcq, NULL) != 0) {
		puts("can't create thread.");
    }

    printf("\r\nRunBlock");
    double timeIndisposedMs;
    uint32_t pre = 100;
    uint32_t post = 100;
    s = rp_RunBlock(pre, post, 8, &timeIndisposedMs, &rpReadyCallback, NULL);
    if (s != RP_API_OK) {
        puts("Failed to acquire data.");
    }

    uint32_t samples = pre + post;

    int16_t* buf = new int16_t[samples];

    // set data buffer to which data will be read from memory space
    rp_SetDataBuffer(RP_CH_DIN, buf, samples, RP_RATIO_MODE_NONE);

    // get data
    rp_GetValues(0, &samples, 1, RP_RATIO_MODE_NONE, NULL);

    // verify data
    int first=buf[0];
    for (size_t i = 0; i < samples; ++i) {
        if (buf[i] != first) {
            printf("\n\r data mismatch @ i=%d buf=%04x exp=%04x", i, buf[i], first);
            break;
        }
        if (first == 0xff)
            first = 0;
        else
            ++first;
    }

    uint32_t trig_pos=0;
    rp_GetTrigPosition(&trig_pos);

    // verify trigger position
    printf("\n\r data @ trigger pos \n\r");
    printf("\n\r %04x", buf[trig_pos-3]);
    printf("\n\r %04x", buf[trig_pos-2]);
    printf("\n\r %04x", buf[trig_pos-1]);
    printf("\n\r ->%04x", buf[trig_pos]);
    printf("\n\r %04x", buf[trig_pos+1]);
    printf("\n\r %04x", buf[trig_pos+2]);
    printf("\n\r %04x", buf[trig_pos+3]);

    free(buf);

    if (rp_CloseUnit() != RP_OK) {
		puts("close unit fail");
        return -1;
    }
}
