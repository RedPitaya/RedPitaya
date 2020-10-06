
#include "main.h"

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
#include "StreamingApplication.h"
#include "StreamingManager.h"


void StartServer();
void StopServer(int x);
void StopNonBlocking(int x);

static std::mutex mut;
static pthread_mutex_t mutex;

#define SS_TCP		1
#define SS_UDP  	2

#define SS_CH1  	1
#define SS_CH2  	2
#define SS_BOTH 	3

#define SS_8BIT		1
#define SS_16BIT	2
//#define DEBUG_MODE


//Parameters

CBooleanParameter 	ss_start(			"SS_START", 	        CBaseParameter::RW, false,0);


CBooleanParameter 	ss_use_localfile(	"SS_USE_FILE", 	        CBaseParameter::RW, false,0);
CIntParameter		ss_port(  			"SS_PORT_NUMBER", 		CBaseParameter::RW, 8900,0,	1,65535);
CStringParameter    ss_ip_addr(			"SS_IP_ADDR",			CBaseParameter::RW, "",0);
CIntParameter		ss_protocol(  		"SS_PROTOCOL", 			CBaseParameter::RW, 1 ,0,	1,2);
CIntParameter		ss_samples(  		"SS_SAMPLES", 			CBaseParameter::RW, 2000000000 ,0,	-1,2000000000);
CIntParameter		ss_channels(  		"SS_CHANNEL", 			CBaseParameter::RW, 1 ,0,	1,3);
CIntParameter		ss_resolution(  	"SS_RESOLUTION", 		CBaseParameter::RW, 1 ,0,	1,2);
CIntParameter		ss_calib( 	 		"SS_USE_CALIB", 		CBaseParameter::RW, 2 ,0,	1,2);
CIntParameter		ss_save_mode(  		"SS_SAVE_MODE", 		CBaseParameter::RW, 1 ,0,	1,2);
CIntParameter		ss_rate(  			"SS_RATE", 				CBaseParameter::RW, 1 ,0,	1,65536);
CIntParameter		ss_format( 			"SS_FORMAT", 			CBaseParameter::RW, 0 ,0,	0,1);
CIntParameter		ss_status( 			"SS_STATUS", 			CBaseParameter::RW, 1 ,0,	0,100);
CIntParameter		ss_acd_max(			"SS_ACD_MAX", 			CBaseParameter::RW, ADC_SAMPLE_RATE ,0,	0, ADC_SAMPLE_RATE);
CStringParameter 	redpitaya_model(	"RP_MODEL_STR", 		CBaseParameter::ROSA, RP_MODEL, 10);

CStreamingManager::Ptr s_manger;
CStreamingApplication  *s_app;


void PrintLogInFile(const char *message){
#ifdef DEBUG_MODE
	std::time_t result = std::time(nullptr);
	std::fstream fs;
  	fs.open ("/tmp/debug.log", std::fstream::in | std::fstream::out | std::fstream::app);
	fs << std::asctime(std::localtime(&result)) << " : " << message << "\n";
	fs.close();
#endif
}

float calibFullScaleToVoltage(uint32_t fullScaleGain) {
    /* no scale */
    if (fullScaleGain == 0) {
        return 1;
    }
    return (float) ((float)fullScaleGain  * 100.0 / ((uint64_t)1<<32));
}

//Application description
const char *rp_app_desc(void)
{
	return (const char *)"Red Pitaya Stream server application.\n";
}



//Application init
int rp_app_init(void)
{
	fprintf(stderr, "Loading stream server version %s-%s.\n", VERSION_STR, REVISION_STR);
	CDataManager::GetInstance()->SetParamInterval(100);

	ss_status.SendValue(0);
	ss_acd_max.SendValue(ADC_SAMPLE_RATE);
	try {
		CStreamingManager::MakeEmptyDir(FILE_PATH);
	}catch (std::exception& e)
	{
		fprintf(stderr, "Error: rp_app_init() %s\n",e.what());
		PrintLogInFile(e.what());
	}
	PrintLogInFile("rp_app_init");

	return 0;
}

//Application exit
int rp_app_exit(void)
{
	StopServer(0);
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

void SaveConfigInFile(){
	char pathtofile[255];
	sprintf(pathtofile,"/root/%s",".streaming_config");
	ofstream file(pathtofile);
	file << "host " << ss_ip_addr.Value() << std::endl;
	file << "port " << ss_port.Value() << std::endl;
	file << "protocol " << ss_protocol.Value() << std::endl;
	file << "rate " << ss_rate.Value() << std::endl;
	file << "channels " << ss_channels.Value() << std::endl;
	file << "resolution " << ss_resolution.Value() << std::endl;
	file << "use_file " << ss_use_localfile.Value() << std::endl;
	file << "format " << ss_format.Value() << std::endl;
	file << "samples " << ss_samples.Value() << std::endl;
	file << "save_mode " << ss_save_mode.Value() << std::endl;
	file << "use_calib " << ss_calib.Value() << std::endl;
}

//Update parameters
void UpdateParams(void)
{
	try{
	if (ss_port.IsNewValue())
	{
		ss_port.Update();
		SaveConfigInFile();
	}

	if (ss_ip_addr.IsNewValue())
	{
		ss_ip_addr.Update();
		SaveConfigInFile();		
	}

	if (ss_use_localfile.IsNewValue())
	{
		ss_use_localfile.Update();
		SaveConfigInFile();
	}

	if (ss_protocol.IsNewValue())
	{
		ss_protocol.Update();
		SaveConfigInFile();
	}

	if (ss_channels.IsNewValue())
	{
		ss_channels.Update();
		SaveConfigInFile();
	}

	if (ss_resolution.IsNewValue())
	{
		ss_resolution.Update();
		SaveConfigInFile();
	}

	if (ss_save_mode.IsNewValue())
	{
		ss_save_mode.Update();
		SaveConfigInFile();
	}

	if (ss_calib.IsNewValue())
	{
		ss_calib.Update();
		SaveConfigInFile();
	}

	if (ss_rate.IsNewValue())
	{
		ss_rate.Update();
		SaveConfigInFile();
	}

	if (ss_format.IsNewValue())
	{
		ss_format.Update();
		SaveConfigInFile();
	}

	if (ss_samples.IsNewValue())
	{
		ss_samples.Update();
		SaveConfigInFile();
	}

	if (ss_start.IsNewValue())
	{
		PrintLogInFile("command");
		ss_start.Update();
		if (ss_start.Value() == 1){
			PrintLogInFile("Start server");
			StartServer();
		}else{
			PrintLogInFile("Stop server");
			StopNonBlocking(0);
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



void StartServer(){
	try{

	auto resolution = ss_resolution.Value();
	auto format = ss_format.Value();
	auto sock_port = ss_port.Value();
	auto use_file = ss_use_localfile.Value();
	auto protocol = ss_protocol.Value();
	auto channel = ss_channels.Value();
	auto rate = ss_rate.Value();
	auto ip_addr_host = ss_ip_addr.Value();
	auto samples = ss_samples.Value();
	auto use_calib = ss_calib.Value();
	auto save_mode = ss_save_mode.Value();

	std::vector<UioT> uioList = GetUioList();
	rp_CalibInit();
    auto osc_calib_params = rp_GetCalibrationSettings();
    uint32_t ch1_off = 0;
    uint32_t ch2_off = 0;
    float ch1_gain = 1;
    float ch2_gain = 1;

if (use_calib == 2) {
	#ifdef Z20_250_12
	// TODO 
		// ch1_gain = osc_calib_params.osc_ch1_g_1_dc;  // 1:1
		// ch2_gain = osc_calib_params.osc_ch2_g_1_dc;  // 1:1
		// ch1_off  = osc_calib_params.osc_ch1_off_1_dc; 
		// ch2_off  = osc_calib_params.osc_ch2_off_1_dc; 
	#else
		ch1_gain = calibFullScaleToVoltage(osc_calib_params.fe_ch1_fs_g_hi);  // 1:1
		ch2_gain = calibFullScaleToVoltage(osc_calib_params.fe_ch2_fs_g_hi);  // 1:1
		ch1_off  = osc_calib_params.fe_ch1_hi_offs; 
		ch2_off  = osc_calib_params.fe_ch2_hi_offs; 

		// ch1_gain = osc_calib_params.fe_ch1_fs_g_lo;  // 1:20
		// ch2_gain = osc_calib_params.fe_ch2_fs_g_lo;  // 1:20
		// ch1_off  = osc_calib_params.fe_ch1_lo_offs; 
		// ch2_off  = osc_calib_params.fe_ch2_lo_offs; 

	#endif    
}

	// Decimation constants

	//static const uint32_t DEC_1     = 1;
	//static const uint32_t DEC_8     = 8;
	//static const uint32_t DEC_64    = 64;
	//static const uint32_t DEC_1024  = 1024;
	//static const uint32_t DEC_8192  = 8192;
	//static const uint32_t DEC_65536 = 65536;

	// Search oscilloscope
	COscilloscope::Ptr osc = nullptr;


	for (const UioT &uio : uioList)
	{
		if (uio.nodeName == "rp_oscilloscope")
		{
			// TODO start server;
			osc = COscilloscope::Create(uio, (channel ==1 || channel == 3) , (channel ==2 || channel == 3) , rate);
			osc->setCalibration(ch1_off,ch1_gain,ch2_off,ch2_gain);
			break;
		}
	}

	CStreamingManager::Ptr s_manger = nullptr;
	if (use_file == false) {
		s_manger = CStreamingManager::Create(
				ip_addr_host,
				std::to_string(sock_port).c_str(),
				protocol == 1 ? asionet::Protocol::TCP : asionet::Protocol::UDP);
	}else{
		s_manger = CStreamingManager::Create((format == 0 ? Stream_FileType::WAV_TYPE: Stream_FileType::TDMS_TYPE) , FILE_PATH, samples , save_mode == 2);
		s_manger->notifyStop = [](int status)
							{
								StopNonBlocking(status == 0 ? 2 : 3);
							};
	}


	if (s_app!= nullptr){
		s_app->stop();
		delete s_app;
	}
	int resolution_val = (resolution == 1 ? 8 : 16);
	s_app = new CStreamingApplication(s_manger, osc, resolution_val, rate, channel , 0 , ADC_BITS);
	ss_status.SendValue(1);
	s_app->runNonBlock();

	}catch (std::exception& e)
	{
		fprintf(stderr, "Error: StopServer() %s\n",e.what());
		PrintLogInFile(e.what());
	}
}

void StopNonBlocking(int x){
	try{
		std::thread th(StopServer ,x);
		th.detach();
	}catch (std::exception& e)
	{
		fprintf(stderr, "Error: StopServer() %s\n",e.what());
		PrintLogInFile(e.what());
	}
}

void StopServer(int x){
	try{
		if (s_app!= nullptr)
		{
			s_app->stop();
			delete s_app;
			s_app = nullptr;
		}
		ss_status.SendValue(x);
	}catch (std::exception& e)
	{
		fprintf(stderr, "Error: StopServer() %s\n",e.what());
		PrintLogInFile(e.what());
	}
}
