
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>

#include "ut_main.h"
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
pthread_t tid2;
pthread_t tid3;

void* trigGen(void *arg)
{
    sleep(1);
    rp_DigSigGenSoftwareControl(1);
    return NULL;
}

void* trigAcq(void *arg)
{
    printf("\r\nTrigAcqThreadStarted");
    sleep(2);
    printf("\r\nSW trigger");
    if(rp_SoftwareTrigger()!=RP_API_OK){
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



void la_acq_trig_test(void)
{

   // for(int i=0; i<1; i++){

    //sleep(1);

    RP_STATUS s;
    RP_DIGITAL_CHANNEL_DIRECTIONS dir[1];
    dir[0].channel=RP_DIGITAL_CHANNEL_7;
    dir[0].direction=RP_DIGITAL_DIRECTION_RISING;

    rp_DigSigGenOuput(true);
    double sample_rate=125e6;
    rp_SetDigSigGenBuiltIn(RP_DIG_SIGGEN_PAT_UP_COUNT_8BIT_SEQ_256,&sample_rate,0,0,RP_TRG_DGEN_SWE_MASK);
    //printf("sample rate %lf",sample_rate);

    printf("\r\nTriggers");
    s=rp_SetTriggerDigitalPortProperties(dir,1);
    if(s!=RP_API_OK){
        CU_FAIL("Failed to set trigger properties.");
    }

    // enable RLE
    rp_EnableDigitalPortDataRLE(1);


    // start gen a bit later in a new thread
    /*
    if(pthread_create(&tid, NULL, &trigGen, NULL)!=0){
    	CU_FAIL("can't create thread.");
    }
    */

    // software trig. acq.
    /*
    if(pthread_create(&tid2, NULL, &trigAcq, NULL)!=0){
		CU_FAIL("can't create thread.");
    }
    */

    // stop acq.
    if(pthread_create(&tid3, NULL, &stopAcq, NULL)!=0){
		CU_FAIL("can't create thread.");
    }

    printf("\r\nRunBlock");
    double timeIndisposedMs;
    uint32_t pre=100;
    uint32_t post=100;
    s=rp_RunBlock(pre,post,8,&timeIndisposedMs,&rpReadyCallback,NULL);
    if(s!=RP_API_OK){
        CU_FAIL("Failed to acquire data.");
    }

    uint32_t samples=pre+post;

    int16_t * buf = malloc(samples * sizeof(int16_t));
    if (NULL == buf) {
        CU_FAIL("malloc failed");
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

    uint32_t trig_pos=0;
    rp_GetTrigPosition(&trig_pos);

    // verify trigger position
    printf("\n\r data @ trigger pos \n\r");
    printf("\n\r %04x",buf[trig_pos-1]);
    printf("\n\r ->%04x",buf[trig_pos]);
    printf("\n\r %04x",buf[trig_pos+1]);

    free(buf);
    //}
}

void reg_rw_test(void){

/*
    rp_handle_uio_t handle;



    // once device is opened, acq. should be stopped
    bool status;
    rp_LaAcqAcqIsStopped(&handle, &status);
    CU_ASSERT_TRUE(status);

    // test register access

    // rp_la_cfg_regset_t
    rp_la_cfg_regset_t cfgw, cfgr;
    cfgw.acq=RP_LA_ACQ_CFG_AUTO_MASK|RP_LA_ACQ_CFG_CONT_MASK;
    cfgw.pre=UINT32_MAX;
    cfgw.pst=UINT32_MAX;
    rp_LaAcqSetCntConfig(&handle,cfgw);
    rp_LaAcqGetCntConfig(&handle,&cfgr);
    CU_ASSERT_FALSE(memcmp((char*)&cfgw, (char*)&cfgr, sizeof(rp_la_cfg_regset_t)));
    cfgw.acq=RP_LA_ACQ_CFG_AUTO_MASK;
    cfgw.pre=56456;
    cfgw.pst=45677;
    rp_LaAcqSetCntConfig(&handle,cfgw);
    rp_LaAcqGetCntConfig(&handle,&cfgr);
    CU_ASSERT_FALSE(memcmp((char*)&cfgw, (char*)&cfgr, sizeof(rp_la_cfg_regset_t)));
    memset(&cfgw, 0, sizeof(rp_la_cfg_regset_t));
    rp_LaAcqSetCntConfig(&handle,cfgw);
    rp_LaAcqGetCntConfig(&handle,&cfgr);
    CU_ASSERT_FALSE(memcmp((char*)&cfgw, (char*)&cfgr, sizeof(rp_la_cfg_regset_t)));

    // rp_la_trg_regset_t trg
    rp_la_trg_regset_t trgw, trgr;
    trgw.cmp_msk=UINT32_MAX;
    trgw.cmp_val=UINT32_MAX;
    trgw.edg_pos=UINT32_MAX;
    trgw.edg_neg=UINT32_MAX;
    rp_LaAcqSetTrigSettings(&handle,trgw);
    rp_LaAcqGetTrigSettings(&handle,&trgr);
    CU_ASSERT_FALSE(memcmp((char*)&trgw, (char*)&trgr, sizeof(rp_la_trg_regset_t)));
    trgw.cmp_msk=2355;
    trgw.cmp_val=6345;
    trgw.edg_pos=7567;
    trgw.edg_neg=8567;
    rp_LaAcqSetTrigSettings(&handle,trgw);
    rp_LaAcqGetTrigSettings(&handle,&trgr);
    CU_ASSERT_FALSE(memcmp((char*)&trgw, (char*)&trgr, sizeof(rp_la_trg_regset_t)));

    rp_LaAcqSetTrigSettings(&handle,trgw);
    rp_LaAcqGetTrigSettings(&handle,&trgr);
    CU_ASSERT_FALSE(memcmp((char*)&trgw, (char*)&trgr, sizeof(rp_la_trg_regset_t)));

    // rp_la_decimation_regset_t dec
    rp_la_decimation_regset_t decw, decr;
    decw.avg=UINT32_MAX;
    decw.dec=UINT32_MAX;
    decw.shr=UINT32_MAX;
    rp_LaAcqSetDecimation(&handle,decw);
    rp_LaAcqGetDecimation(&handle,&decr);
    CU_ASSERT_FALSE(memcmp((char*)&decw, (char*)&decr, sizeof(rp_la_decimation_regset_t)));
    decw.avg=23423;
    decw.dec=34534;
    decw.shr=63456;
    rp_LaAcqSetDecimation(&handle,decw);
    rp_LaAcqGetDecimation(&handle,&decr);
    CU_ASSERT_FALSE(memcmp((char*)&decw, (char*)&decr, sizeof(rp_la_decimation_regset_t)));
    memset(&decw, 0, sizeof(rp_la_decimation_regset_t));
    rp_LaAcqSetDecimation(&handle,decw);
    rp_LaAcqGetDecimation(&handle,&decr);
    CU_ASSERT_FALSE(memcmp((char*)&decw, (char*)&decr, sizeof(rp_la_decimation_regset_t)));

    // test control bits
    // TODO:
    rp_LaAcqReset(&handle);
    rp_LaAcqRunAcq(&handle);
    rp_LaAcqStopAcq(&handle);
    rp_LaAcqTriggerAcq(&handle);

    rp_LaAcqFpgaRegDump(&handle);

    // rp_data_ptrs_regset_t dpt
    // TODO:

    // Close log. anal. device
    if(rp_LaAcqClose(&handle) != RP_OK){
        CU_FAIL_FATAL("Could not properly close the device.");
    }
    */
}

