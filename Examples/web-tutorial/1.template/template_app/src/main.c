/**
 * $Id: main.c 881 2013-12-16 05:37:34Z rp_jmenart $
 *
 * @brief Red Pitaya Oscilloscope main module.
 *
 * @Author Jure Menart <juremenart@gmail.com>
 *         
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>

#include "main.h"

#include <sys/mman.h>

#include <sys/stat.h>
#include <stddef.h>
#include <fcntl.h>




const char *rp_app_desc(void)
{
    return (const char *)"Red Pitaya template application.\n";
}


int rp_app_init(void)
{
    fprintf(stderr, "Loading template application\n");
    return 0;
}


int rp_app_exit(void)
{
    fprintf(stderr, "Unloading template application\n");
    return 0;
}


int rp_set_params(rp_app_params_t *p, int len)
{
    return 0;
}

/* Returned vector must be free'd externally! */
int rp_get_params(rp_app_params_t **p)
{
    return 0;
}

int rp_get_signals(float ***s, int *sig_num, int *sig_len)
{
    return 0;
}

int rp_create_signals(float ***a_signals)
{
    return 0;
}

void rp_cleanup_signals(float ***a_signals)
{

}
