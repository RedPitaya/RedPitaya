
#include <stdio.h>
#include <string.h>

#include "CUnit/Basic.h"
#include "CUnit/Console.h"
#include "CUnit/Automated.h"
#include "CUnit/CUCurses.h"

#include "ut_main.h"

#include "generate.h"
#include "rp2.h"
#include "rp_api.h"

int suite_sig_gen_init(void)
{
    rp_OpenUnit();
    return 0;
}

int suite_sig_gen_cleanup(void)
{
    rp_CloseUnit();
    return 0;
}

void sig_gen_test(void)
{
	rp_DigSigGenOuput(true);
    rp_SetDigSigGenBuiltIn(RP_DIG_SIGGEN_PAT_UP_COUNT_8BIT_SEQ,125e6,RP_TRG_DIG_GEN_SWE_MASK);
    rp_DigSigGenSoftwareControl(1);
}
