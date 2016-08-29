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
#define SIGNAL_SIZE_DEFAULT      1024
#define SIGNAL_UPDATE_INTERVAL      10


//Signal
CFloatSignal VOLTAGE("VOLTAGE", SIGNAL_SIZE_DEFAULT, 0.0f);
std::vector<float> g_data(SIGNAL_SIZE_DEFAULT);


//Parameter
CIntParameter GAIN("GAIN", CBaseParameter::RW, 1, 0, 1, 100);
CFloatParameter OFFSET("OFFSET", CBaseParameter::RW, 0.0, 0, 0.0, 5.0);






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

    //Set signal update interval
    CDataManager::GetInstance()->SetSignalInterval(SIGNAL_UPDATE_INTERVAL);

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








void UpdateSignals(void)
{
    float val;
    
    //Read data from pin
    rp_AIpinGetValue(0, &val);

    //Push it to vector
    g_data.erase(g_data.begin());
    g_data.push_back(val);

    //Write data to signal
    for(int i = 0; i < SIGNAL_SIZE_DEFAULT; i++) 
    {
        VOLTAGE[i] = g_data[i] * GAIN.Value() + OFFSET.Value();
    }
}


void UpdateParams(void){}


void OnNewParams(void) 
{
    GAIN.Update();
    OFFSET.Update();
}


void OnNewSignals(void){}


void PostUpdateSignals(void){}