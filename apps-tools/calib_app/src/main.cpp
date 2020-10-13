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

COscilloscope::Ptr g_acq;

#define DEBUG_MODE


//Parameters

CBooleanParameter 	ss_start(			"SS_START", 	        CBaseParameter::RW, false,0);


// CBooleanParameter 	ss_use_localfile(	"SS_USE_FILE", 	        CBaseParameter::RW, false,0);
// CIntParameter		ss_port(  			"SS_PORT_NUMBER", 		CBaseParameter::RW, 8900,0,	1,65535);
// CStringParameter    ss_ip_addr(			"SS_IP_ADDR",			CBaseParameter::RW, "",0);
// CIntParameter		ss_protocol(  		"SS_PROTOCOL", 			CBaseParameter::RW, 1 ,0,	1,2);
// CIntParameter		ss_samples(  		"SS_SAMPLES", 			CBaseParameter::RW, 2000000000 ,0,	-1,2000000000);
// CIntParameter		ss_channels(  		"SS_CHANNEL", 			CBaseParameter::RW, 1 ,0,	1,3);
// CIntParameter		ss_resolution(  	"SS_RESOLUTION", 		CBaseParameter::RW, 1 ,0,	1,2);
// CIntParameter		ss_calib( 	 		"SS_USE_CALIB", 		CBaseParameter::RW, 2 ,0,	1,2);
// CIntParameter		ss_save_mode(  		"SS_SAVE_MODE", 		CBaseParameter::RW, 1 ,0,	1,2);
// CIntParameter		ss_rate(  			"SS_RATE", 				CBaseParameter::RW, 1 ,0,	1,65536);
// CIntParameter		ss_format( 			"SS_FORMAT", 			CBaseParameter::RW, 0 ,0,	0,1);
// CIntParameter		ss_status( 			"SS_STATUS", 			CBaseParameter::RW, 1 ,0,	0,100);
// CIntParameter		ss_acd_max(			"SS_ACD_MAX", 			CBaseParameter::RW, ADC_SAMPLE_RATE ,0,	0, ADC_SAMPLE_RATE);
// CIntParameter		ss_attenuator( 		"SS_ATTENUATOR",		CBaseParameter::RW, 0 ,0,	0,1);
CFloatParameter 	ch1_min("ch1_min", CBaseParameter::ROSA, 0, 0, -1e6f, +1e6f);
CFloatParameter 	ch1_max("ch1_max", CBaseParameter::ROSA, 0, 0, -1e6f, +1e6f);
CFloatParameter 	ch1_avg("ch1_avg", CBaseParameter::ROSA, 0, 0, -1e6f, +1e6f);
CFloatParameter 	ch2_min("ch2_min", CBaseParameter::ROSA, 0, 0, -1e6f, +1e6f);
CFloatParameter 	ch2_max("ch2_max", CBaseParameter::ROSA, 0, 0, -1e6f, +1e6f);
CFloatParameter 	ch2_avg("ch2_avg", CBaseParameter::ROSA, 0, 0, -1e6f, +1e6f);


CStringParameter 	redpitaya_model(	"RP_MODEL_STR", 		CBaseParameter::RO, RP_MODEL, 10);



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

		// if (ss_port.IsNewValue())
		// {
		// 	ss_port.Update(););
		// }

		// if (ss_ip_addr.IsNewValue())
		// {
		// 	ss_ip_addr.Update();	
		// }

		// if (ss_use_localfile.IsNewValue())
		// {
		// 	ss_use_localfile.Update();
		// }
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