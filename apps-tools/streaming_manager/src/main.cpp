
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
#include "ServerNetConfigManager.h"
#include "DACStreamingApplication.h"
#include "DACStreamingManager.h"

#ifdef Z20_250_12
#include "rp-spi.h"
#include "rp-i2c-max7311.h"
#endif



void StartServer(bool testMode);
void StopServer(int x);
void StopNonBlocking(int x);

auto startDACServer(bool testMode) -> void;
auto stopDACNonBlocking(CDACStreamingManager::NotifyResult x) -> void;
auto stopDACServer(CDACStreamingManager::NotifyResult x) -> void;

void setConfig(bool _force);
void updateUI();

static std::mutex mut;
static pthread_mutex_t mutex;
static constexpr char config_file[] = "/root/.streaming_config";

#define SS_TCP		1
#define SS_UDP  	2

#define SS_CH1  	1
#define SS_CH2  	2
#define SS_BOTH 	3

#define SS_8BIT		1
#define SS_16BIT	2

#define SERVER_CONFIG_PORT "8901"
#define SERVER_BROADCAST_PORT "8902"


//#define DEBUG_MODE


//Parameters

CBooleanParameter 	ss_start(			"SS_START", 			CBaseParameter::RW, false,0);
CBooleanParameter 	ss_dac_start(		"SS_DAC_START", 		CBaseParameter::RW, false,0);


CBooleanParameter 	ss_use_localfile(	"SS_USE_FILE", 	        CBaseParameter::RW, false,0);
CIntParameter		ss_port(  			"SS_PORT_NUMBER", 		CBaseParameter::RW, 8900,0,	1,65535);
CStringParameter    ss_ip_addr(			"SS_IP_ADDR",			CBaseParameter::RW, "127.0.0.1",0);
CIntParameter		ss_protocol(  		"SS_PROTOCOL", 			CBaseParameter::RW, 1 ,0,	1,2);
CIntParameter		ss_samples(  		"SS_SAMPLES", 			CBaseParameter::RW, 20000000 ,0,	-1,2000000000);
CIntParameter		ss_channels(  		"SS_CHANNEL", 			CBaseParameter::RW, 1 ,0,	1,3);
CIntParameter		ss_resolution(  	"SS_RESOLUTION", 		CBaseParameter::RW, 1 ,0,	1,2);
CIntParameter		ss_calib( 	 		"SS_USE_CALIB", 		CBaseParameter::RW, 2 ,0,	1,2);
CIntParameter		ss_save_mode(  		"SS_SAVE_MODE", 		CBaseParameter::RW, 1 ,0,	1,2);
CIntParameter		ss_rate(  			"SS_RATE", 				CBaseParameter::RW, 4 ,0,	1,65536);
CIntParameter		ss_format( 			"SS_FORMAT", 			CBaseParameter::RW, 0 ,0,	0, 2);
CIntParameter		ss_status( 			"SS_STATUS", 			CBaseParameter::RW, 1 ,0,	0,100);
CIntParameter		ss_acd_max(			"SS_ACD_MAX", 			CBaseParameter::RO, ADC_SAMPLE_RATE ,0,	0, ADC_SAMPLE_RATE);
CIntParameter		ss_attenuator( 		"SS_ATTENUATOR",		CBaseParameter::RW, 1 ,0,	1, 2);
CIntParameter		ss_ac_dc( 			"SS_AC_DC",				CBaseParameter::RW, 1 ,0,	1, 2);
CStringParameter 	redpitaya_model(	"RP_MODEL_STR", 		CBaseParameter::ROSA, RP_MODEL, 10);

CStringParameter    ss_dac_file(		"SS_DAC_FILE",			CBaseParameter::RW, "", 0);
CIntParameter    	ss_dac_file_type(	"SS_DAC_FILE_TYPE",		CBaseParameter::RW,  0 ,0, 0, 1);
CIntParameter    	ss_dac_gain(		"SS_DAC_GAIN",			CBaseParameter::RW,  0 ,0, 0, 1);
CIntParameter    	ss_dac_mode(		"SS_DAC_MODE",			CBaseParameter::RW,  0 ,0, 0, 1);
CIntParameter		ss_dac_speed(		"SS_DAC_HZ", 			CBaseParameter::RW, DAC_FREQUENCY ,0,	1 / (65536 /DAC_FREQUENCY) + 1, DAC_FREQUENCY);
CIntParameter		ss_dac_max_speed(	"SS_DAC_MAX_HZ",		CBaseParameter::RO, DAC_FREQUENCY ,0,	0, DAC_FREQUENCY);
CIntParameter    	ss_dac_repeat(		"SS_DAC_REPEAT",		CBaseParameter::RW, -1 ,0, -2, 0);
CIntParameter    	ss_dac_rep_count(	"SS_DAC_REPEAT_COUNT",	CBaseParameter::RW, 1 ,0, 1, 2000000000);
CIntParameter		ss_dac_port(		"SS_DAC_PORT_NUMBER", 	CBaseParameter::RW, 8903,0,	1,65535);
CIntParameter		ss_dac_memory(		"SS_DAC_MEMORYCACHE", 	CBaseParameter::RW, 1024 * 1024,0,	0, 1024 * 1024 * 64);
CIntParameter		ss_dac_status( 		"SS_DAC_STATUS",		CBaseParameter::RW, 1 ,0,	0,100);

CIntParameter    	ss_lb_mode(			"SS_LB_MODE",		    CBaseParameter::RW,  0,0,   0,100);
CIntParameter		ss_lb_speed(		"SS_LB_SPEED", 			CBaseParameter::RW, -1,0,	-1,DAC_FREQUENCY);
CIntParameter		ss_lb_timeout(		"SS_LB_TIMEOUT",	 	CBaseParameter::RW,  1,0,	0,1);
CIntParameter		ss_lb_channels(		"SS_LB_CHANNELS",		CBaseParameter::RW,  1,0,	0,1);


CStreamingApplication  *s_app = nullptr;
CStreamingManager::Ptr 	s_manger = nullptr;
COscilloscope::Ptr 		osc = nullptr;

CDACStreamingApplication *g_dac_app = nullptr;
CDACStreamingManager::Ptr g_dac_manger = nullptr;
CGenerator::Ptr 		gen = nullptr;

std::shared_ptr<ServerNetConfigManager> g_serverNetConfig;
std::atomic_bool g_serverRun(false);
std::atomic_bool g_dac_serverRun(false);

// void PrintLogInFile(const char *message){
// #ifdef DEBUG_MODE
// 	std::time_t result = std::time(nullptr);
// 	std::fstream fs;
//   	fs.open ("/tmp/debug.log", std::fstream::in | std::fstream::out | std::fstream::app);
// 	fs << std::asctime(std::localtime(&result)) << " : " << message << "\n";
// 	fs.close();
// #endif
// }

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
	g_serverRun = false;
	try {
		try {
			
			#ifdef STREAMING_MASTER
				auto mode = asionet_broadcast::CAsioBroadcastSocket::ABMode::AB_SERVER_MASTER;
			#endif
			#ifdef STREAMING_SLAVE
				auto mode = asionet_broadcast::CAsioBroadcastSocket::ABMode::AB_SERVER_SLAVE;
			#endif 
			g_serverNetConfig = std::make_shared<ServerNetConfigManager>(config_file,mode,ss_ip_addr.Value(),SERVER_CONFIG_PORT);
			g_serverNetConfig->addHandler(ServerNetConfigManager::Events::GET_NEW_SETTING,[](){
				updateUI();
			});

			g_serverNetConfig->addHandler(ServerNetConfigManager::Events::START_STREAMING,[](){
				StartServer(false);
			});

			g_serverNetConfig->addHandler(ServerNetConfigManager::Events::START_STREAMING_TEST,[](){
				StartServer(true);
			});

			g_serverNetConfig->addHandler(ServerNetConfigManager::Events::STOP_STREAMING,[](){
				StopNonBlocking(0);
			});

			g_serverNetConfig->addHandler(ServerNetConfigManager::Events::START_DAC_STREAMING,[](){
				startDACServer(false);
			});

			g_serverNetConfig->addHandler(ServerNetConfigManager::Events::START_DAC_STREAMING_TEST,[](){
				startDACServer(true);
			});

			g_serverNetConfig->addHandler(ServerNetConfigManager::Events::STOP_DAC_STREAMING,[](){
				stopDACNonBlocking(CDACStreamingManager::NR_STOP);
			});


		}catch (std::exception& e)
			{
				fprintf(stderr, "Error: Init ServerNetConfigManager() %s\n",e.what());
			}

		ss_status.SendValue(0);
		ss_acd_max.SendValue(ADC_SAMPLE_RATE);
		ss_dac_max_speed.SendValue(DAC_FREQUENCY);
		if (g_serverNetConfig->getSettingsRef().isSetted()){
			updateUI();
		}else{
			setConfig(true);
		}
		CStreamingManager::MakeEmptyDir(FILE_PATH);
#ifdef Z20_250_12
	    rp_max7311::rp_initController();
#endif

	}catch (std::exception& e)
	{
		fprintf(stderr, "Error: rp_app_init() %s\n",e.what());
	}
	return 0;
}

//Application exit
int rp_app_exit(void)
{
	g_serverNetConfig->stop();
	StopServer(0);
	fprintf(stderr, "Unloading stream server version %s-%s.\n", VERSION_STR, REVISION_STR);	
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

void saveConfigInFile(){
	if (!g_serverNetConfig->getSettingsRef().writeToFile(config_file)){
		std::cerr << "Error save to file(" << config_file <<")\n";
	}
}

void updateUI(){
	ss_port.SendValue(std::atoi(g_serverNetConfig->getSettingsRef().getPort().c_str()));
	ss_dac_file.SendValue(g_serverNetConfig->getSettingsRef().getDACFile());
	ss_dac_port.SendValue(std::atoi(g_serverNetConfig->getSettingsRef().getDACPort().c_str()));

	switch (g_serverNetConfig->getSettingsRef().getSaveType())
	{
		case CStreamSettings::NET:
			ss_use_localfile.SendValue(0);
			break;
		case CStreamSettings::FILE:
			ss_use_localfile.SendValue(1);
			break;
	}

	switch (g_serverNetConfig->getSettingsRef().getProtocol())
	{
		case CStreamSettings::TCP:
			ss_protocol.SendValue(SS_TCP);
			break;
		case CStreamSettings::UDP:
			ss_protocol.SendValue(SS_UDP);
			break;
	}

	switch (g_serverNetConfig->getSettingsRef().getChannels())
	{
		case CStreamSettings::CH1:
			ss_protocol.SendValue(1);
			break;
		case CStreamSettings::CH2:
			ss_channels.SendValue(2);
			break;
		case CStreamSettings::BOTH:
			ss_channels.SendValue(3);
			break;
	}

	switch (g_serverNetConfig->getSettingsRef().getResolution())
	{
		case CStreamSettings::BIT_8:
			ss_resolution.SendValue(SS_8BIT);
			break;
		case CStreamSettings::BIT_16:
			ss_resolution.SendValue(SS_16BIT);
			break;
	}

	switch (g_serverNetConfig->getSettingsRef().getType())
	{
		case CStreamSettings::RAW:
			ss_save_mode.SendValue(1);
			break;
		case CStreamSettings::VOLT:
			ss_save_mode.SendValue(2);
			break;
	}

	switch (g_serverNetConfig->getSettingsRef().getFormat())
	{
		case CStreamSettings::WAV:
			ss_format.SendValue(0);
			break;
		case CStreamSettings::TDMS:
			ss_format.SendValue(1);
			break;
		case CStreamSettings::CSV:
			ss_format.SendValue(2);
			break;
	}

	switch (g_serverNetConfig->getSettingsRef().getAttenuator())
	{
		case CStreamSettings::A_1_1:
			ss_attenuator.SendValue(1);
			break;
		case CStreamSettings::A_1_20:
			ss_attenuator.SendValue(2);
			break;
	}

	switch (g_serverNetConfig->getSettingsRef().getAC_DC())
	{
		case CStreamSettings::AC:
			ss_ac_dc.SendValue(1);
			break;
		case CStreamSettings::DC:
			ss_ac_dc.SendValue(2);
			break;
	}

	switch (g_serverNetConfig->getSettingsRef().getDACFileType())
	{
		case CStreamSettings::WAV:
			ss_dac_file_type.SendValue(0);
			break;
		case CStreamSettings::TDMS:
			ss_dac_file_type.SendValue(1);
			break;
	}

	switch (g_serverNetConfig->getSettingsRef().getDACGain())
	{
		case CStreamSettings::X1:
			ss_dac_gain.SendValue(0);
			break;
		case CStreamSettings::X5:
			ss_dac_gain.SendValue(1);
			break;
	}

	switch (g_serverNetConfig->getSettingsRef().getDACMode())
	{
		case CStreamSettings::DAC_NET:
			ss_dac_mode.SendValue(0);
			break;
		case CStreamSettings::DAC_FILE:
			ss_dac_mode.SendValue(1);
			break;
	}


	ss_dac_repeat.SendValue(g_serverNetConfig->getSettingsRef().getDACRepeat());
	ss_dac_rep_count.SendValue(g_serverNetConfig->getSettingsRef().getDACRepeatCount());
	ss_dac_memory.SendValue(g_serverNetConfig->getSettingsRef().getDACMemoryUsage());
	ss_dac_speed.SendValue(g_serverNetConfig->getSettingsRef().getDACHz());
	ss_rate.SendValue(g_serverNetConfig->getSettingsRef().getDecimation());
	ss_samples.SendValue(g_serverNetConfig->getSettingsRef().getSamples());
	ss_calib.SendValue(g_serverNetConfig->getSettingsRef().getCalibration() ? 2 : 1);
	
	ss_lb_channels.SendValue(g_serverNetConfig->getSettingsRef().getLoopbackChannels());
	ss_lb_mode.SendValue(g_serverNetConfig->getSettingsRef().getLoopbackMode());
	ss_lb_timeout.SendValue(g_serverNetConfig->getSettingsRef().getLoopbackTimeout());
	ss_lb_speed.SendValue(g_serverNetConfig->getSettingsRef().getLoopbackSpeed());
	
}

void setConfig(bool _force){
	bool needUpdate = false;
	if (ss_port.IsNewValue() || _force)
	{
		ss_port.Update();
		g_serverNetConfig->getSettingsRef().setPort(std::to_string(ss_port.Value()));
		needUpdate = true;
	}

	if (ss_dac_file.IsNewValue() || _force)
	{
		ss_dac_file.Update();
		g_serverNetConfig->getSettingsRef().setDACFile(ss_dac_file.Value());
		needUpdate = true;
	}

	if (ss_dac_port.IsNewValue() || _force)
	{
		ss_dac_port.Update();
		g_serverNetConfig->getSettingsRef().setDACPort(std::to_string(ss_dac_port.Value()));
		needUpdate = true;
	}

	if (ss_ip_addr.IsNewValue() || _force)
	{
		ss_ip_addr.Update();
		g_serverNetConfig->startServer(ss_ip_addr.Value(),SERVER_CONFIG_PORT);

		#ifdef Z10
		asionet_broadcast::CAsioBroadcastSocket::Model model = asionet_broadcast::CAsioBroadcastSocket::Model::RP_125_14;
		#endif

		#ifdef Z20
		asionet_broadcast::CAsioBroadcastSocket::Model model = asionet_broadcast::CAsioBroadcastSocket::Model::RP_122_16;
		#endif

		#ifdef Z20_125
		asionet_broadcast::CAsioBroadcastSocket::Model model = asionet_broadcast::CAsioBroadcastSocket::Model::RP_125_14_Z20;
		#endif

		#ifdef Z20_250_12
		asionet_broadcast::CAsioBroadcastSocket::Model model = asionet_broadcast::CAsioBroadcastSocket::Model::RP_250_12;
		#endif

		g_serverNetConfig->startBroadcast(model, ss_ip_addr.Value(),SERVER_BROADCAST_PORT);
	}

	if (ss_use_localfile.IsNewValue() || _force)
	{
		ss_use_localfile.Update();
		if (ss_use_localfile.Value() == 1)
			g_serverNetConfig->getSettingsRef().setSaveType(CStreamSettings::FILE);
		else
			g_serverNetConfig->getSettingsRef().setSaveType(CStreamSettings::NET);
		needUpdate = true;
	}

	if (ss_protocol.IsNewValue() || _force)
	{
		ss_protocol.Update();
		if (ss_protocol.Value() == SS_TCP)
			g_serverNetConfig->getSettingsRef().setProtocol(CStreamSettings::TCP);
		if (ss_protocol.Value() == SS_UDP)
			g_serverNetConfig->getSettingsRef().setProtocol(CStreamSettings::UDP);
		needUpdate = true;
	}

	if (ss_channels.IsNewValue() || _force)
	{
		ss_channels.Update();
		if (ss_channels.Value() == 1)
			g_serverNetConfig->getSettingsRef().setChannels(CStreamSettings::CH1);
		if (ss_channels.Value() == 2)
			g_serverNetConfig->getSettingsRef().setChannels(CStreamSettings::CH2);
		if (ss_channels.Value() == 3)
			g_serverNetConfig->getSettingsRef().setChannels(CStreamSettings::BOTH);
		needUpdate = true;
	}

	if (ss_resolution.IsNewValue() || _force)
	{
		ss_resolution.Update();
		if (ss_resolution.Value() == SS_8BIT)
			g_serverNetConfig->getSettingsRef().setResolution(CStreamSettings::BIT_8);
		if (ss_resolution.Value() == SS_16BIT)
			g_serverNetConfig->getSettingsRef().setResolution(CStreamSettings::BIT_16);
		needUpdate = true;
	}

	if (ss_save_mode.IsNewValue() || _force)
	{
		ss_save_mode.Update();
		if (ss_save_mode.Value() == 1)
			g_serverNetConfig->getSettingsRef().setType(CStreamSettings::RAW);
		if (ss_save_mode.Value() == 2)
			g_serverNetConfig->getSettingsRef().setType(CStreamSettings::VOLT);
		needUpdate = true;
	}

	if (ss_rate.IsNewValue() || _force)
	{
		ss_rate.Update();
		g_serverNetConfig->getSettingsRef().setDecimation(ss_rate.Value());
		needUpdate = true;
	}

	if (ss_format.IsNewValue() || _force)
	{
		ss_format.Update();
		if (ss_format.Value() == 0) 
			g_serverNetConfig->getSettingsRef().setFormat(CStreamSettings::WAV);
		if (ss_format.Value() == 1) 
			g_serverNetConfig->getSettingsRef().setFormat(CStreamSettings::TDMS);
		if (ss_format.Value() == 2) 
			g_serverNetConfig->getSettingsRef().setFormat(CStreamSettings::CSV);
		needUpdate = true;
	}

	if (ss_samples.IsNewValue() || _force)
	{
		ss_samples.Update();
		g_serverNetConfig->getSettingsRef().setSamples(ss_samples.Value());
		needUpdate = true;
	}

#ifndef Z20
	if (ss_attenuator.IsNewValue() || _force)
	{
		ss_attenuator.Update();
		if (ss_attenuator.Value() == 1)
			g_serverNetConfig->getSettingsRef().setAttenuator(CStreamSettings::A_1_1);
		if (ss_attenuator.Value() == 2)
			g_serverNetConfig->getSettingsRef().setAttenuator(CStreamSettings::A_1_20);
		needUpdate = true;
	}

	if (ss_calib.IsNewValue() || _force)
	{
		ss_calib.Update();
		g_serverNetConfig->getSettingsRef().setCalibration(ss_calib.Value() == 2);
		needUpdate = true;
	}
#else
	g_serverNetConfig->getSettingsRef().setAttenuator(CStreamSettings::A_1_1);
	g_serverNetConfig->getSettingsRef().setCalibration(false);
#endif

#ifdef Z20_250_12
	if (ss_ac_dc.IsNewValue() || _force)
	{
		ss_ac_dc.Update();
		if (ss_ac_dc.Value() == 1)
			g_serverNetConfig->getSettingsRef().setAC_DC(CStreamSettings::AC);
		if (ss_ac_dc.Value() == 2)
			g_serverNetConfig->getSettingsRef().setAC_DC(CStreamSettings::DC);
		needUpdate = true;
	}
#else
	g_serverNetConfig->getSettingsRef().setAC_DC(CStreamSettings::AC);
#endif

	if (ss_dac_file_type.IsNewValue() || _force)
	{
		ss_dac_file_type.Update();
		if (ss_dac_file_type.Value() == 0) 
			g_serverNetConfig->getSettingsRef().setDACFileType(CStreamSettings::WAV);
		if (ss_dac_file_type.Value() == 1) 
			g_serverNetConfig->getSettingsRef().setDACFileType(CStreamSettings::TDMS);
		needUpdate = true;
	}

#ifdef Z20_250_12
	if (ss_dac_gain.IsNewValue() || _force)
	{
		ss_dac_gain.Update();
		if (ss_dac_gain.Value() == 0)
			g_serverNetConfig->getSettingsRef().setDACGain(CStreamSettings::X1);
		if (ss_dac_gain.Value() == 1)
			g_serverNetConfig->getSettingsRef().setDACGain(CStreamSettings::X5);
		needUpdate = true;
	}
#else
	g_serverNetConfig->getSettingsRef().setDACGain(CStreamSettings::X1);
#endif

	if (ss_dac_mode.IsNewValue() || _force)
	{
		ss_dac_mode.Update();
		if (ss_dac_mode.Value() == 0)
			g_serverNetConfig->getSettingsRef().setDACMode(CStreamSettings::DAC_NET);
		if (ss_dac_mode.Value() == 1)
			g_serverNetConfig->getSettingsRef().setDACMode(CStreamSettings::DAC_FILE);
		needUpdate = true;
	}

	if (ss_dac_repeat.IsNewValue() || _force)
	{
		ss_dac_repeat.Update();
		g_serverNetConfig->getSettingsRef().setDACRepeat((CStreamSettings::DACRepeat)ss_dac_repeat.Value());
		needUpdate = true;
	}

	if (ss_dac_rep_count.IsNewValue() || _force)
	{
		ss_dac_rep_count.Update();
		g_serverNetConfig->getSettingsRef().setDACRepeatCount(ss_dac_rep_count.Value());
		needUpdate = true;
	}

	if (ss_dac_memory.IsNewValue() || _force)
	{
		ss_dac_memory.Update();
		g_serverNetConfig->getSettingsRef().setDACMemoryUsage(ss_dac_memory.Value());
		needUpdate = true;
	}

	if (ss_dac_speed.IsNewValue() || _force)
	{
		ss_dac_speed.Update();
		g_serverNetConfig->getSettingsRef().setDACHz(ss_dac_speed.Value());
		needUpdate = true;
	}


	if (ss_lb_channels.IsNewValue() || _force)
	{
		ss_lb_channels.Update();
		g_serverNetConfig->getSettingsRef().setLoopbackChannels((CStreamSettings::LOOPBACKChannels)ss_lb_channels.Value());
		needUpdate = true;
	}

	if (ss_lb_mode.IsNewValue() || _force)
	{
		ss_lb_mode.Update();
		g_serverNetConfig->getSettingsRef().setLoopbackMode((CStreamSettings::LOOPBACKMode)ss_lb_mode.Value());
		needUpdate = true;
	}

	if (ss_lb_speed.IsNewValue() || _force)
	{
		ss_lb_speed.Update();
		g_serverNetConfig->getSettingsRef().setLoopbackSpeed(ss_lb_speed.Value());
		needUpdate = true;
	}

	if (ss_lb_timeout.IsNewValue() || _force)
	{
		ss_lb_timeout.Update();
		g_serverNetConfig->getSettingsRef().setLoopbackTimeout(ss_lb_timeout.Value());
		needUpdate = true;
	}
	
	if (needUpdate){
		saveConfigInFile();
	}
}

//Update parameters
void UpdateParams(void)
{
	try{

		setConfig(false);

		if (ss_start.IsNewValue())
		{
			ss_start.Update();
			if (ss_start.Value() == 1){
				StartServer(false);
			}else{
				StopNonBlocking(0);
			}
		}

		if (ss_dac_start.IsNewValue())
		{
			ss_dac_start.Update();
			if (ss_dac_start.Value() == 1){
				startDACServer(false);
			}else{
				stopDACNonBlocking(CDACStreamingManager::NR_STOP);
			}
		}
	}catch (std::exception& e)
	{
		fprintf(stderr, "Error: UpdateParams() %s\n",e.what());
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



void StartServer(bool testMode){
	// Search oscilloscope

	try{
		CStreamSettings settings = testMode ? g_serverNetConfig->getTempSettings() : g_serverNetConfig->getSettings();
		if (!settings.isSetted()) return;
		// if (g_serverRun) {
		// 	if (s_manger){
		// 		if (!s_manger->isLocalMode()){
		// 			if (s_manger->getProtocol() == asionet::Protocol::TCP){
		// 				g_serverNetConfig->sendServerStartedTCP();
		// 			}
		// 			if (s_manger->getProtocol() == asionet::Protocol::UDP){
		// 				g_serverNetConfig->sendServerStartedUDP();
		// 			}
		// 		}else{
		// 			g_serverNetConfig->sendServerStartedSD();
		// 		}
		// 	}
		// 	return;
		// }
		// g_serverRun = true;

		auto resolution   = settings.getResolution();
		auto format       = settings.getFormat();
		auto sock_port    = settings.getPort();
		auto use_file     = settings.getSaveType();
		auto protocol     = settings.getProtocol();
		auto channel      = settings.getChannels();
		auto rate         = settings.getDecimation();
		auto ip_addr_host = ss_ip_addr.Value();
		auto samples      = settings.getSamples();
		auto save_mode    = settings.getType();

#ifdef Z20
		auto use_calib = 0;
		auto attenuator = 0;
#else
		auto use_calib    = settings.getCalibration();
		auto attenuator   = settings.getAttenuator();
		rp_CalibInit();
		auto osc_calib_params = rp_GetCalibrationSettings();
#endif

#ifdef Z20_250_12
		auto ac_dc = settings.getAC_DC();
#endif

		std::vector<UioT> uioList = GetUioList();
		uint32_t ch1_off = 0;
		uint32_t ch2_off = 0;
		float ch1_gain = 1;
		float ch2_gain = 1;
		bool  filterBypass = true;
		uint32_t aa_ch1 = 0;
		uint32_t bb_ch1 = 0;
		uint32_t kk_ch1 = 0xFFFFFF;
		uint32_t pp_ch1 = 0;
		uint32_t aa_ch2 = 0;
		uint32_t bb_ch2 = 0;
		uint32_t kk_ch2 = 0xFFFFFF;
		uint32_t pp_ch2 = 0;

		if (use_calib == 2) {
#ifdef Z20_250_12
			if (attenuator == CStreamSettings::A_1_1) {
				if (ac_dc == CStreamSettings::AC) {
					ch1_gain = calibFullScaleToVoltage(osc_calib_params.osc_ch1_g_1_ac) / 20.0;  // 1:1
					ch2_gain = calibFullScaleToVoltage(osc_calib_params.osc_ch2_g_1_ac) / 20.0;  // 1:1
					ch1_off  = osc_calib_params.osc_ch1_off_1_ac;
					ch2_off  = osc_calib_params.osc_ch2_off_1_ac;
				}
				else {
					ch1_gain = calibFullScaleToVoltage(osc_calib_params.osc_ch1_g_1_dc) / 20.0;  // 1:1
					ch2_gain = calibFullScaleToVoltage(osc_calib_params.osc_ch2_g_1_dc) / 20.0;  // 1:1
					ch1_off  = osc_calib_params.osc_ch1_off_1_dc;
					ch2_off  = osc_calib_params.osc_ch2_off_1_dc;
				}
			}else{
				if (ac_dc == CStreamSettings::A_1_1) {
					ch1_gain = calibFullScaleToVoltage(osc_calib_params.osc_ch1_g_20_ac);  // 1:20
					ch2_gain = calibFullScaleToVoltage(osc_calib_params.osc_ch2_g_20_ac);  // 1:20
					ch1_off  = osc_calib_params.osc_ch1_off_20_ac;
					ch2_off  = osc_calib_params.osc_ch2_off_20_ac;
				} else {
					ch1_gain = calibFullScaleToVoltage(osc_calib_params.osc_ch1_g_20_dc);  // 1:20
					ch2_gain = calibFullScaleToVoltage(osc_calib_params.osc_ch2_g_20_dc);  // 1:20
					ch1_off  = osc_calib_params.osc_ch1_off_20_dc;
					ch2_off  = osc_calib_params.osc_ch2_off_20_dc;
				}
			}
#endif

#if defined Z10 || defined Z20_125
			filterBypass = false;
			if (attenuator == CStreamSettings::A_1_1) {
				ch1_gain = calibFullScaleToVoltage(osc_calib_params.fe_ch1_fs_g_lo) / 20.0;
				ch2_gain = calibFullScaleToVoltage(osc_calib_params.fe_ch2_fs_g_lo) / 20.0;
				ch1_off  = osc_calib_params.fe_ch1_lo_offs;
				ch2_off  = osc_calib_params.fe_ch2_lo_offs;
				aa_ch1 = osc_calib_params.low_filter_aa_ch1;
				bb_ch1 = osc_calib_params.low_filter_bb_ch1;
				pp_ch1 = osc_calib_params.low_filter_pp_ch1;
				kk_ch1 = osc_calib_params.low_filter_kk_ch1;
				aa_ch2 = osc_calib_params.low_filter_aa_ch2;
				bb_ch2 = osc_calib_params.low_filter_bb_ch2;
				pp_ch2 = osc_calib_params.low_filter_pp_ch2;
				kk_ch2 = osc_calib_params.low_filter_kk_ch2;

			}else{
				ch1_gain = calibFullScaleToVoltage(osc_calib_params.fe_ch1_fs_g_hi);
				ch2_gain = calibFullScaleToVoltage(osc_calib_params.fe_ch2_fs_g_hi);
				ch1_off  = osc_calib_params.fe_ch1_hi_offs;
				ch2_off  = osc_calib_params.fe_ch2_hi_offs;
				aa_ch1 = osc_calib_params.hi_filter_aa_ch1;
				bb_ch1 = osc_calib_params.hi_filter_bb_ch1;
				pp_ch1 = osc_calib_params.hi_filter_pp_ch1;
				kk_ch1 = osc_calib_params.hi_filter_kk_ch1;
				aa_ch2 = osc_calib_params.hi_filter_aa_ch2;
				bb_ch2 = osc_calib_params.hi_filter_bb_ch2;
				pp_ch2 = osc_calib_params.hi_filter_pp_ch2;
				kk_ch2 = osc_calib_params.hi_filter_kk_ch2;
			}
#endif
		}

#ifdef Z20_250_12
		rp_max7311::rp_setAttenuator(RP_MAX7311_IN1, attenuator == CStreamSettings::A_1_1  ? RP_ATTENUATOR_1_1 : RP_ATTENUATOR_1_20);
		rp_max7311::rp_setAttenuator(RP_MAX7311_IN2, attenuator == CStreamSettings::A_1_1  ? RP_ATTENUATOR_1_1 : RP_ATTENUATOR_1_20);
		rp_max7311::rp_setAC_DC(RP_MAX7311_IN1, ac_dc == CStreamSettings::AC ? RP_AC_MODE : RP_DC_MODE);
		rp_max7311::rp_setAC_DC(RP_MAX7311_IN2, ac_dc == CStreamSettings::AC ? RP_AC_MODE : RP_DC_MODE);
#endif

		if (s_app!= nullptr){
			s_app->stop();
			delete s_app;
		}

		for (const UioT &uio : uioList)
		{
			if (uio.nodeName == "rp_oscilloscope")
			{
				#ifdef STREAMING_MASTER
					auto isMaster = true;
				#endif
				#ifdef STREAMING_SLAVE
					auto isMaster = false
				#endif 
				osc = COscilloscope::Create(uio, (channel == CStreamSettings::CH1 || channel == CStreamSettings::BOTH), (channel == CStreamSettings::CH2 || channel == CStreamSettings::BOTH), rate,isMaster);
				osc->setCalibration(ch1_off,ch1_gain,ch2_off,ch2_gain);
				osc->setFilterCalibrationCh1(aa_ch1,bb_ch1,kk_ch1,pp_ch1);
				osc->setFilterCalibrationCh2(aa_ch2,bb_ch2,kk_ch2,pp_ch2);
				osc->setFilterBypass(filterBypass);
				break;
			}
		}


		if (use_file == CStreamSettings::NET) {
			s_manger = CStreamingManager::Create(
					ip_addr_host,
					sock_port,
					protocol == CStreamSettings::TCP ? asionet::Protocol::TCP : asionet::Protocol::UDP);
		}else{
			auto file_type = Stream_FileType::WAV_TYPE;
			if (format == CStreamSettings::TDMS) file_type = Stream_FileType::TDMS_TYPE;
			if (format == CStreamSettings::CSV)  file_type = Stream_FileType::CSV_TYPE;
			s_manger = CStreamingManager::Create(file_type , FILE_PATH, samples , save_mode == CStreamSettings::VOLT,testMode);
			s_manger->notifyStop = [](int status)
								{
									StopNonBlocking(status == 0 ? 2 : 3);
								};
		}


		int resolution_val = (resolution == CStreamSettings::BIT_8 ? 8 : 16);
		s_app = new CStreamingApplication(s_manger, osc, resolution_val, rate, channel , attenuator , 16);
		s_app->setTestMode(testMode);

		ss_status.SendValue(1);
		
		char time_str[40];
    	struct tm *timenow;
    	time_t now = time(nullptr);
    	timenow = gmtime(&now);
    	strftime(time_str, sizeof(time_str), "%Y-%m-%d_%H-%M-%S", timenow);
    	std::string filenameDate = time_str;

		s_app->runNonBlock(filenameDate);
		if (!s_manger->isLocalMode()){
			if (s_manger->getProtocol() == asionet::Protocol::TCP){
				g_serverNetConfig->sendServerStartedTCP();
			}
			if (s_manger->getProtocol() == asionet::Protocol::UDP){
				g_serverNetConfig->sendServerStartedUDP();
			}
		}else{
			g_serverNetConfig->sendServerStartedSD();
		}

		fprintf(stderr,"[Streaming] Start server\n");

	}catch (std::exception& e)
	{
		fprintf(stderr, "Error: StartServer() %s\n",e.what());
	}
}

void StopNonBlocking(int x){
	try{
		std::thread th(StopServer ,x);
		th.detach();
	}catch (std::exception& e)
	{
		fprintf(stderr, "Error: StopServer() %s\n",e.what());
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
		switch (x)
		{
		case 0:
			g_serverNetConfig->sendServerStopped();
			break;
		case 2:
			g_serverNetConfig->sendServerStoppedSDFull();
			break;
		case 3:
			g_serverNetConfig->sendServerStoppedDone();
			break;
		default:
			throw runtime_error("Unknown state");
			break;
		}
		//g_serverRun = false;
		fprintf(stderr,"[Streaming] Stop server\n");
	}catch (std::exception& e)
	{
		fprintf(stderr, "Error: StopServer() %s\n",e.what());
	}
}


auto startDACServer(bool testMode) -> void{
    if (!g_serverNetConfig) return;
	gen = nullptr;
	try{
		CStreamSettings settings = testMode ? g_serverNetConfig->getTempSettings() : g_serverNetConfig->getSettings();
		if (!settings.isSetted()) return;
		// g_dac_serverRun = true;
		auto use_file     =  settings.getDACMode();
		auto sock_port    =  settings.getDACPort();
		auto dac_speed    =  settings.getDACHz();
		auto ip_addr_host = "127.0.0.1";

#ifdef Z20
		auto use_calib    = 0;
		auto attenuator   = 0;
#else
		auto use_calib    = settings.getCalibration();
		rp_CalibInit();
		auto osc_calib_params = rp_GetCalibrationSettings();
#endif

#ifdef Z20_250_12
		auto dac_gain = g_serverNetConfig->getDACGain();
#endif

		std::vector<UioT> uioList = GetUioList();
		uint32_t ch1_off = 0;
		uint32_t ch2_off = 0;
		float ch1_gain = 1;
		float ch2_gain = 1;

		if (use_calib == 1) {
#ifdef Z20_250_12
			if (dac_gain == CStreamSettings::X1) {
				ch1_gain = calibFullScaleToVoltage(osc_calib_params.gen_ch1_g_1);
				ch2_gain = calibFullScaleToVoltage(osc_calib_params.gen_ch2_g_1);
				ch1_off  = osc_calib_params.gen_ch1_off_1;
				ch2_off  = osc_calib_params.gen_ch2_off_1;
			}else{
				ch1_gain = calibFullScaleToVoltage(osc_calib_params.gen_ch1_g_5);
				ch2_gain = calibFullScaleToVoltage(osc_calib_params.gen_ch2_g_5);
				ch1_off  = osc_calib_params.gen_ch1_off_5;
				ch2_off  = osc_calib_params.gen_ch2_off_5;
			}
#endif

#if defined Z10 || defined Z20_125
			ch1_gain = calibFullScaleToVoltage(osc_calib_params.be_ch1_fs);
			ch2_gain = calibFullScaleToVoltage(osc_calib_params.be_ch2_fs);
			ch1_off  = osc_calib_params.be_ch1_dc_offs;
			ch2_off  = osc_calib_params.be_ch2_dc_offs;
#endif
		}

#ifdef Z20_250_12

        rp_max7311::rp_setGainOut(RP_MAX7311_OUT1, dac_gain == CStreamSettings::X1 ? RP_GAIN_2V : RP_GAIN_10V);
        rp_max7311::rp_setGainOut(RP_MAX7311_OUT2, dac_gain == CStreamSettings::X1 ? RP_GAIN_2V : RP_GAIN_10V);

#endif

		if (g_dac_app!= nullptr){
			g_dac_app->stop();
			delete g_dac_app;
		}

		for (const UioT &uio : uioList)
		{
			 if (uio.nodeName == "rp_dac")
			{
				gen = CGenerator::Create(uio, true , true ,dac_speed,DAC_FREQUENCY);
				gen->setCalibration(ch1_off,ch1_gain,ch2_off,ch2_gain);
				gen->setDacHz(dac_speed);
			}
		}

		if (!gen){
			fprintf(stdout,"[Streaming] Error init generator module\n");
        	syslog (LOG_NOTICE, "[Streaming] Error init generator module\n");
			return;
		}


		if (use_file == CStreamSettings::DAC_NET) {
			g_dac_manger = CDACStreamingManager::Create(
					ip_addr_host,
					sock_port);
		}else{

			auto format = settings.getDACFileType();
			auto filePath = settings.getDACFile();
			auto dacRepeatMode = settings.getDACRepeat();
			auto dacRepeatCount = settings.getDACRepeatCount();
			auto dacMemory = settings.getDACMemoryUsage();
			
			if (format == CStreamSettings::WAV) {
				g_dac_manger = CDACStreamingManager::Create(CDACStreamingManager::WAV_TYPE,filePath,dacRepeatMode,dacRepeatCount,dacMemory);
			}else if (format == CStreamSettings::TDMS) {
				g_dac_manger = CDACStreamingManager::Create(CDACStreamingManager::TDMS_TYPE,filePath,dacRepeatMode,dacRepeatCount,dacMemory);
			}else{
				return;
			}
			g_dac_manger->notifyStop = [](CDACStreamingManager::NotifyResult status)
			{
				stopDACNonBlocking(status);
			};
		
		}

		g_dac_app = new CDACStreamingApplication(g_dac_manger, gen);
		g_dac_app->setTestMode(testMode);		

		g_dac_app->runNonBlock();
		if (g_dac_manger->isLocalMode()){
			g_serverNetConfig->sendDACServerStartedSD();
		}else{
			g_serverNetConfig->sendDACServerStarted();
		}

		fprintf(stdout,"[Streaming] Start dac server\n");
        syslog (LOG_NOTICE, "[Streaming] Start dac server\n");
	}catch (std::exception& e)
	{
		fprintf(stderr, "Error: startDACServer() %s\n",e.what());
        syslog (LOG_ERR,"Error: startDACServer() %s\n",e.what());
	}
}

auto stopDACNonBlocking(CDACStreamingManager::NotifyResult x) -> void{
	try{
		std::thread th(stopDACServer ,x);
		th.detach();
	}catch (std::exception& e)
	{
		fprintf(stderr, "Error: StopNonBlocking() %s\n",e.what());
        syslog (LOG_ERR,"Error: StopNonBlocking() %s\n",e.what());
	}
}


auto stopDACServer(CDACStreamingManager::NotifyResult x) -> void{
	try{
		if (g_dac_app)
		{
			g_dac_app->stop();
			delete g_dac_app;
			g_dac_app = nullptr;
		}
        if (g_serverNetConfig){
            switch (x)
            {
				case CDACStreamingManager::NR_STOP:
					g_serverNetConfig->sendDACServerStopped();
					break;
				case CDACStreamingManager::NR_ENDED:
					g_serverNetConfig->sendDACServerStoppedSDDone();
					break;
				case CDACStreamingManager::NR_EMPTY:
					g_serverNetConfig->sendDACServerStoppedSDEmpty();
					break;
				case CDACStreamingManager::NR_BROKEN:
					g_serverNetConfig->sendDACServerStoppedSDBroken();
					break;
				case CDACStreamingManager::NR_MISSING_FILE:
					g_serverNetConfig->sendDACServerStoppedSDMissingFile();
					break;
				default:
					throw runtime_error("Unknown state");
					break;
            }
        }
        fprintf(stdout,"[Streaming] Stop dac server\n");
        syslog (LOG_NOTICE, "[Streaming] Stop dac server\n");
	}catch (std::exception& e)
	{
		fprintf(stderr, "Error: stopDACServer() %s\n",e.what());
        syslog (LOG_ERR,"Error: stopDACServer() %s\n",e.what());
	}
}