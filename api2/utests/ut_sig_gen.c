
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "CUnit/Basic.h"
#include "CUnit/Console.h"
#include "CUnit/Automated.h"
#include "CUnit/CUCurses.h"

#include "ut_main.h"
#include "generate.h"
#include "rp_api.h"



int suite_sig_gen_init(void)
{
    if(rp_OpenUnit()!=RP_OK){
        return -1;
    }
    return 0;
}

int suite_sig_gen_cleanup(void)
{
    if(rp_CloseUnit()!=RP_OK){
        return -1;
    }
    return 0;
}

void sig_gen_test(void)
{
    rp_DigSigGenOuput(true);
    double sample_rate=125e6;
    rp_SetDigSigGenBuiltIn(RP_DIG_SIGGEN_PAT_UP_COUNT_8BIT_SEQ_256,&sample_rate,10,0,RP_TRG_DGEN_SWE_MASK);
    //printf("sample rate %lf",sample_rate);
    rp_DigSigGenSoftwareControl(1);
    sleep(5);
}
