#include <vector>

#ifdef RP_PLATFORM
#include "rp.h"
#include "rp_hw-calib.h"
#include "rp_hw-profiles.h"
#include "api250-12/rp-spi.h"
#include "api250-12/rp-gpio-power.h"
#include "api250-12/rp-i2c-max7311.h"
#endif

#ifndef _WIN32
#include <syslog.h>
#endif

#include "data_lib/thread_cout.h"
#include "uio_lib/generator.h"
#include "dac_streaming_lib/dac_streaming_application.h"
#include "dac_streaming_lib/dac_net_controller.h"
#include "dac_streaming_lib/dac_streaming_manager.h"
#include "dac_streaming.h"


using namespace dac_streaming_lib;
using namespace uio_lib;

CDACStreamingApplication::Ptr g_dac_app = nullptr;
CDACStreamingManager::Ptr     g_dac_manger = nullptr;
CGenerator::Ptr               g_gen = nullptr;
ServerNetConfigManager::Ptr   g_serverDACNetConfig = nullptr;

bool                          g_dac_verbMode = false;
std::atomic_bool              g_dac_serverRun(false);


auto setDACServer(ServerNetConfigManager::Ptr serverNetConfig) -> void{
    g_serverDACNetConfig = serverNetConfig;
}

auto startDACServer(bool verbMode,bool testMode) -> void{
    if (!g_serverDACNetConfig) return;
    g_gen = nullptr;
    g_dac_app = nullptr;
    g_dac_manger = nullptr;
	g_dac_verbMode = verbMode;
	try{
		CStreamSettings settings = testMode ? g_serverDACNetConfig->getTempSettings() : g_serverDACNetConfig->getSettings();

#ifndef RP_PLATFORM
        settings.resetDefault();
#endif
        if (!settings.isSetted()) {
            g_serverDACNetConfig->sendConfigFileMissed();
            printWithLog(LOG_ERR,stderr,"[ERROR] Config file is missing\n");
            return;
        }

		g_dac_serverRun = true;
		auto use_file     =  settings.getDACMode();
		auto sock_port    =  settings.getDACPort();
		auto dac_speed    =  settings.getDACHz();
		auto ip_addr_host = "127.0.0.1";

		auto use_calib    = false;

#ifdef RP_PLATFORM
        use_calib    = settings.getCalibration();
		if (rp_CalibInit() != RP_HW_CALIB_OK){
	        fprintf(stderr,"Error init calibration\n");
    	}
		auto dac_gain = settings.getDACGain();
#endif

		auto channels = ClientOpt::getDACChannels();
		std::vector<UioT> uioList = GetUioList();
		int32_t ch_off[MAX_DAC_CHANNELS] = { 0 , 0 };
		double  ch_gain[MAX_DAC_CHANNELS] = { 1 , 1 };
#ifdef RP_PLATFORM

		if (use_calib) {
			for(uint8_t ch = 0; ch < channels; ++ch){
				rp_gen_gain_calib_t  mode = dac_gain == CStreamSettings::X1 ? RP_GAIN_CALIB_1X : RP_GAIN_CALIB_5X;
				if (rp_CalibGetFastDACCalibValue((rp_channel_calib_t)ch,mode,&ch_gain[ch],&ch_off[ch]) != RP_HW_CALIB_OK){
					fprintf(stderr,"Error get calibration channel: %d\n",ch);
				}
			}
		}

		if (rp_HPGetIsGainDACx5OrDefault()){
        	rp_max7311::rp_setGainOut(RP_MAX7311_OUT1, dac_gain == CStreamSettings::X1 ? RP_GAIN_2V : RP_GAIN_10V);
        	rp_max7311::rp_setGainOut(RP_MAX7311_OUT2, dac_gain == CStreamSettings::X1 ? RP_GAIN_2V : RP_GAIN_10V);
		}

		for (const UioT &uio : uioList)
		{
			 if (uio.nodeName == "rp_dac")
			{
                g_gen = CGenerator::create(uio, true , true ,dac_speed,ClientOpt::getDACRate());
				g_gen->setCalibration(ch_off[0],ch_gain[0],ch_off[1],ch_gain[1]);
				g_gen->setDacHz(dac_speed);
			}
		}
#else
        uio_lib::UioT uio_t;
        g_gen = CGenerator::create(uio_t, true , true ,dac_speed,ClientOpt::getDACRate());
        g_gen->setCalibration(ch_off[0],ch_gain[0],ch_off[1],ch_gain[1]);
        g_gen->setDacHz(dac_speed);
#endif

        if (!g_gen){
            printWithLog(LOG_ERR,stderr,"[Streaming] Error init generator module\n");
			return;
		}


		if (use_file == CStreamSettings::DAC_NET) {
			g_dac_manger = CDACStreamingManager::Create(ip_addr_host,sock_port);
        }

        if (use_file == CStreamSettings::DAC_FILE) {
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
                g_serverDACNetConfig->sendDACServerStoppedSDBroken();
				return;
			}
            g_dac_manger->notifyStop.connect([](CDACStreamingManager::NotifyResult status)
			{
				stopDACNonBlocking(status);
            });

		}

        g_dac_app = std::make_shared<CDACStreamingApplication>(g_dac_manger, g_gen);
		g_dac_app->setVerbousMode(g_dac_verbMode);
		g_dac_app->setTestMode(testMode);

		g_dac_app->runNonBlock();
		if (g_dac_manger->isLocalMode()){
			g_serverDACNetConfig->sendDACServerStartedSD();
		}else{
			g_serverDACNetConfig->sendDACServerStarted();
		}
		if (g_dac_verbMode){
            printWithLog(LOG_ERR,stderr,"[Streaming] Start dac server\n");
		}

	}catch (std::exception& e)
	{
        printWithLog(LOG_ERR,stderr,"Error: startDACServer() %s\n",e.what());
    }
}

auto stopDACNonBlocking(CDACStreamingManager::NotifyResult x) -> void{
	try{
		std::thread th(stopDACServer ,x);
		th.detach();
	}catch (std::exception& e)
	{
        printWithLog(LOG_ERR,stderr,"Error: StopNonBlocking() %s\n",e.what());
    }
}

auto stopDACServer(CDACStreamingManager::NotifyResult x) -> void{
	try{
		if (g_dac_app)
		{
			g_dac_app->stop();
			g_dac_app = nullptr;
		}
        if (g_serverDACNetConfig){
            switch (x)
            {
				case CDACStreamingManager::NR_STOP:
					g_serverDACNetConfig->sendDACServerStopped();
					break;
				case CDACStreamingManager::NR_ENDED:
					g_serverDACNetConfig->sendDACServerStoppedSDDone();
					break;
				case CDACStreamingManager::NR_EMPTY:
					g_serverDACNetConfig->sendDACServerStoppedSDEmpty();
					break;
				case CDACStreamingManager::NR_BROKEN:
					g_serverDACNetConfig->sendDACServerStoppedSDBroken();
					break;
				case CDACStreamingManager::NR_MISSING_FILE:
					g_serverDACNetConfig->sendDACServerStoppedSDMissingFile();
					break;
				default:
					throw runtime_error("Unknown state");
					break;
            }
        }
		if (g_dac_verbMode){
            printWithLog(LOG_NOTICE,stdout,"[Streaming] Stop dac server\n");
        }
	}catch (std::exception& e)
	{
        printWithLog(LOG_NOTICE,stdout,"Error: stopDACServer() %s\n",e.what());
	}
}
