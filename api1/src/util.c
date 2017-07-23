#include <stdio.h>

#include "redpitaya/util.h"

char * rp_util_fixp_print (const fixp_t value) {
    static char str[16];
    sprintf (str, "(s=%u,m=%u,f=%u)", value.s, value.m, value.f);
    return str;
}

