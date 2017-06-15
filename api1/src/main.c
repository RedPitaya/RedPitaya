#include <stdio.h>
#include "uio.h"

int main () {
    printf ("DEBUG: start\n");

    rp_uio_t handle;
    char path [] = "/dev/uio/hwid";
    
    rp_uio_init    (&handle, path);
    rp_uio_release (&handle);

    printf ("DEBUG: end\n");
    return(0);
}

