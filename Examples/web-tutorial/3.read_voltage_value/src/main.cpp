#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <vector>

#include "main.h"




//Signal size
#define SIGNAL_SIZE_DEFAULT      1


//Signal
CFloatSignal VOLTAGE("VOLTAGE", SIGNAL_SIZE_DEFAULT, 0.0f);


//Parameter
CBooleanParameter READ_VALUE("READ_VALUE", CBaseParameter::RW, false, 0);






const char *rp_app_desc(void)
{
    return (const char *)"Red Pitaya read voltage.\n";
}


int rp_app_init(void)
{
    fprintf(stderr, "Loading read voltage application\n");

    // Initialization of API
    if (rpApp_Init() != RP_OK) 
    {
        fprintf(stderr, "Red Pitaya API init failed!\n");
        return EXIT_FAILURE;
    }
    else fprintf(stderr, "Red Pitaya API init success!\n");

    return 0;
}


int rp_app_exit(void)
{
    fprintf(stderr, "Unloading read voltage application\n");

    rpApp_Release();

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


void OnNewParams(void)
{
    READ_VALUE.Update();

    if (READ_VALUE.Value() == true)
    {
        float val;
    
        //Read data from pin
        rp_AIpinGetValue(0, &val);

        //Write data to signal
        VOLTAGE[0] = val;

        //Reset READ value
        READ_VALUE.Set(false);
    }
}


void OnNewSignals(void){}


void PostUpdateSignals(void){}