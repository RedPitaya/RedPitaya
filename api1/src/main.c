#include <stdio.h>
#include <inttypes.h>

#include "hwid.h"
#include "gen.h"

int main () {
    printf ("DEBUG: start\n");

    rp_hwid_t hwid;
    rp_hwid_init (&hwid);
    printf ("HWID = 0x%0x\n", rp_hwid_get_hwid(&hwid));
    printf ("DNA  = 0x%016" PRIx64 "\n", rp_hwid_get_dna(&hwid));
    rp_hwid_release (&hwid);

    rp_gen_t gen0;
    rp_gen_init (&gen0, 0);
    
//    rp_gen_release (&gen0);

    printf ("DEBUG: end\n");
    return(0);
}

