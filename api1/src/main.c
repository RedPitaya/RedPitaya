#include <stdio.h>

#include "uio.h"
#include "hwid.h"

int main () {
    printf ("DEBUG: start\n");

    char path [] = "/dev/uio/hwid";
    
    rp_uio_t handle_uio;
    rp_uio_init    (&handle_uio, path);
    rp_uio_release (&handle_uio);

    rp_hwid_t handle_hwid;
    rp_hwid_init    (&handle_hwid);
    printf ("HWID = %0x\n", rp_hwid_get_hwid(&handle_hwid));
    rp_hwid_release (&handle_hwid);

    printf ("DEBUG: end\n");
    return(0);
}

