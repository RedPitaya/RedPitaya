#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysinfo.h>

#include "main.h"






const char *rp_app_desc(void)
{
    return (const char *)"Red Pitaya NGINX requests.\n";
}


int rp_app_init(void)
{
    fprintf(stderr, "Loading NGINX requests application\n");
    return 0;
}


int rp_app_exit(void)
{
    fprintf(stderr, "Unloading NGINX requests application\n");
    return 0;
}


int rp_set_params(rp_app_params_t *p, int len)
{
    return 0;
}


int rp_get_params(rp_app_params_t **p)
{
    return 0;
}


int rp_get_signals(float ***s, int *sig_num, int *sig_len)
{
    return 0;
}








void UpdateSignals(void){}


void UpdateParams(void){}


void OnNewParams(void) {}


void OnNewSignals(void){}


void PostUpdateSignals(void){}