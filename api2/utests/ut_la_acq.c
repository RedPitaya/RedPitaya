
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include <stdlib.h>
#include <stdint.h>

#include "rp2.h"
#include "la_acq.h"


#include "ut_main.h"

int suite_la_acq_init(void)
{
    return 0;
}

int suite_la_acq_cleanup(void)
{
    return 0;
}

void reg_rw_test(void){


    rp_handle_uio_t handle;

    // Open logic analyzer device
    if(rp_LaAcqOpen("/dev/dummy", &handle) != RP_OK) {
        CU_FAIL_FATAL("Could not open the device.");
    }

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
    rp_LaAcqSetConfig(&handle,cfgw);
    rp_LaAcqGetConfig(&handle,&cfgr);
    CU_ASSERT_FALSE(memcmp((char*)&cfgw, (char*)&cfgr, sizeof(rp_la_cfg_regset_t)));
    cfgw.acq=RP_LA_ACQ_CFG_AUTO_MASK;
    cfgw.pre=56456;
    cfgw.pst=45677;
    rp_LaAcqSetConfig(&handle,cfgw);
    rp_LaAcqGetConfig(&handle,&cfgr);
    CU_ASSERT_FALSE(memcmp((char*)&cfgw, (char*)&cfgr, sizeof(rp_la_cfg_regset_t)));
    memset(&cfgw, 0, sizeof(rp_la_cfg_regset_t));
    rp_LaAcqSetConfig(&handle,cfgw);
    rp_LaAcqGetConfig(&handle,&cfgr);
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
    //rp_LaAcqReset(&handle);
    //rp_LaAcqRunAcq(&handle);
    //rp_LaAcqStopAcq(&handle);
    //rp_LaAcqTriggerAcq(&handle);

    // rp_data_ptrs_regset_t dpt
    // TODO:

    // Close log. anal. device
    if(rp_LaAcqClose(&handle) != RP_OK){
        CU_FAIL_FATAL("Could not properly close the device.");
    }
}

