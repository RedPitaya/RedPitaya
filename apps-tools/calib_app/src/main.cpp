#include "main.h"

#include <DataManager.h>
#include <CustomParameters.h>
#include <fstream>  
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/syslog.h>
#include <complex.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <ctime>
#include <stdlib.h>

#include <vector>
#include <algorithm>
#include <thread>
#include <mutex>

#include "rp.h"
#include "version.h"
#include "acq.h"
#include "calib.h"

COscilloscope::Ptr g_acq;
CCalib::Ptr        g_calib;

// #define DEBUG_MODE


//Parameters

CBooleanParameter 	ss_start(			"SS_START", 	        CBaseParameter::RW, false,0);

CFloatParameter 	ch1_min(		"ch1_min", 			CBaseParameter::ROSA, 0, 0, -1e6f, +1e6f);
CFloatParameter 	ch1_max(		"ch1_max", 			CBaseParameter::ROSA, 0, 0, -1e6f, +1e6f);
CFloatParameter 	ch1_avg(		"ch1_avg", 			CBaseParameter::ROSA, 0, 0, -1e6f, +1e6f);
CFloatParameter 	ch2_min(		"ch2_min", 			CBaseParameter::ROSA, 0, 0, -1e6f, +1e6f);
CFloatParameter 	ch2_max(		"ch2_max", 			CBaseParameter::ROSA, 0, 0, -1e6f, +1e6f);
CFloatParameter 	ch2_avg(		"ch2_avg", 			CBaseParameter::ROSA, 0, 0, -1e6f, +1e6f);
CIntParameter		ch1_calib_pass( "ch1_calib_pass", 	CBaseParameter::RW,   0 ,0,	-2147483647, 2147483647);
CIntParameter		ch2_calib_pass( "ch2_calib_pass", 	CBaseParameter::RW,   0 ,0,	-2147483647, 2147483647);
CFloatParameter 	ref_volt(		"ref_volt",			CBaseParameter::RW,   1, 0, 0.001, 20);

CIntParameter       ss_state(		"SS_STATE", 		CBaseParameter::RW ,-1,0,-1,100); // Current completed step
CIntParameter       ss_next_step(	"SS_NEXTSTEP",		CBaseParameter::RW ,-1,0,-2,100); 

CStringParameter 	redpitaya_model("RP_MODEL_STR", 	CBaseParameter::RO, RP_MODEL, 10);



void PrintLogInFile(const char *message){
#ifdef DEBUG_MODE
	std::time_t result = std::time(nullptr);
	std::fstream fs;
  	fs.open ("/tmp/debug.log", std::fstream::in | std::fstream::out | std::fstream::app);
	fs << std::asctime(std::localtime(&result)) << " : " << message << "\n";
	fs.close();
#endif
}


//Application description
const char *rp_app_desc(void)
{
	return (const char *)"Red Pitaya calibration application.\n";
}



//Application init
int rp_app_init(void)
{
	fprintf(stderr, "Loading calibraton app version %s-%s.\n", VERSION_STR, REVISION_STR);
	CDataManager::GetInstance()->SetParamInterval(100);
	PrintLogInFile("rp_app_init");
	rp_Init();
	g_acq = COscilloscope::Create(128);
	g_calib = CCalib::Create(g_acq);
	g_acq->start();
	return 0;
}

//Application exit
int rp_app_exit(void)
{
	g_acq->stop();
	rp_Release();
	fprintf(stderr, "Unloading stream server version %s-%s.\n", VERSION_STR, REVISION_STR);
	PrintLogInFile("Unloading stream server version");
	return 0;
}

//Set parameters
int rp_set_params(rp_app_params_t *p, int len)
{
    return 0;
}

//Get parameters
int rp_get_params(rp_app_params_t **p)
{
    return 0;
}

//Get signals
int rp_get_signals(float ***s, int *sig_num, int *sig_len)
{
    return 0;
}

//Update signals
void UpdateSignals(void)
{

}


//Update parameters
void UpdateParams(void)
{
	try{
		auto x = g_acq->getData();
		ch1_max.Value() = x.ch1_max;
		ch1_min.Value() = x.ch1_min;
		ch1_avg.Value() = x.ch1_avg;
		ch2_max.Value() = x.ch2_max;
		ch2_min.Value() = x.ch2_min;
		ch2_avg.Value() = x.ch2_avg;

		if (ss_next_step.IsNewValue() && ref_volt.IsNewValue())
		{
			ss_next_step.Update();
			ref_volt.Update();
			if (ss_next_step.Value() != -2){
				if (g_calib->calib(ss_next_step.Value(),ref_volt.Value()) == RP_OK){
					auto x = g_calib->getCalibData();
					ch1_calib_pass.SendValue(x.ch1);
					ch2_calib_pass.SendValue(x.ch2);
					ss_state.SendValue(ss_next_step.Value()); 
				}
			}else{
				g_calib->restoreCalib();
			}
		}

	}catch (std::exception& e)
	{
		fprintf(stderr, "Error: UpdateParams() %s\n",e.what());
		PrintLogInFile(e.what());
	}
	
}

void PostUpdateSignals(){}

void OnNewParams(void)
{
	//Update parameters
	UpdateParams();
}

void OnNewSignals(void)
{
	UpdateSignals();
}