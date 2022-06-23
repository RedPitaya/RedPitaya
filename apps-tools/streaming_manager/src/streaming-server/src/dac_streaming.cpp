#include <vector>

#ifdef RP_PLATFORM
#include "rp.h"
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

#ifdef Z20_250_12
#include "api250-12/rp-spi.h"
#include "api250-12/rp-gpio-power.h"
#include "api250-12/rp-i2c-max7311.h"
#endif

using namespace dac_streaming_lib;
using namespace uio_lib;

CDACStreamingApplication::Ptr g_dac_app = nullptr;
CDACStreamingManager::Ptr     g_dac_manger = nullptr;
CGenerator::Ptr               g_gen = nullptr;
ServerNetConfigManager::Ptr   g_serverDACNetConfig = nullptr;

bool                          g_dac_verbMode = false;
std::atomic_bool              g_dac_serverRun(false);


auto calibDACFullScaleToVoltage(uint32_t fullScaleGain) -> float {
    /* no scale */
    if (fullScaleGain == 0) {
        return 1;
    }
    return (float) ((float)fullScaleGain  * 100.0 / ((uint64_t)1<<32));
}

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

#ifdef Z20
		auto use_calib    = false;
		auto attenuator   = 0;
#else
#ifdef RP_PLATFORM
        auto use_calib    = settings.getCalibration();
        rp_CalibInit();
		auto osc_calib_params = rp_GetCalibrationSettings();
#endif
#endif

#ifdef Z20_250_12
		auto dac_gain = settings.getDACGain();
#endif

		std::vector<UioT> uioList = GetUioList();
		uint32_t ch1_off = 0;
		uint32_t ch2_off = 0;
		float ch1_gain = 1;
		float ch2_gain = 1;
#ifdef RP_PLATFORM
		if (use_calib) {
#ifdef Z20_250_12
			if (dac_gain == CStreamSettings::X1) {			
				ch1_gain = calibDACFullScaleToVoltage(osc_calib_params.gen_ch1_g_1);  
				ch2_gain = calibDACFullScaleToVoltage(osc_calib_params.gen_ch2_g_1);  
				ch1_off  = osc_calib_params.gen_ch1_off_1;
				ch2_off  = osc_calib_params.gen_ch2_off_1;
			}else{
				ch1_gain = calibDACFullScaleToVoltage(osc_calib_params.gen_ch1_g_5);  
				ch2_gain = calibDACFullScaleToVoltage(osc_calib_params.gen_ch2_g_5);  
				ch1_off  = osc_calib_params.gen_ch1_off_5;
				ch2_off  = osc_calib_params.gen_ch2_off_5;
			}
#endif

#if defined Z10 || defined Z20_125
			ch1_gain = calibDACFullScaleToVoltage(osc_calib_params.be_ch1_fs);
			ch2_gain = calibDACFullScaleToVoltage(osc_calib_params.be_ch2_fs);
			ch1_off  = osc_calib_params.be_ch1_dc_offs;
			ch2_off  = osc_calib_params.be_ch2_dc_offs;
#endif
		}

#ifdef Z20_250_12

        rp_max7311::rp_setGainOut(RP_MAX7311_OUT1, dac_gain == CStreamSettings::X1 ? RP_GAIN_2V : RP_GAIN_10V);
        rp_max7311::rp_setGainOut(RP_MAX7311_OUT2, dac_gain == CStreamSettings::X1 ? RP_GAIN_2V : RP_GAIN_10V);

#endif

		for (const UioT &uio : uioList)
		{
			 if (uio.nodeName == "rp_dac")
			{
                g_gen = CGenerator::create(uio, true , true ,dac_speed,DAC_FREQUENCY);
				g_gen->setCalibration(ch1_off,ch1_gain,ch2_off,ch2_gain);
				g_gen->setDacHz(dac_speed);
			}
		}

#else
        uio_lib::UioT uio_t;
        g_gen = CGenerator::create(uio_t, true , true ,dac_speed,125e6);
        g_gen->setCalibration(ch1_off,ch1_gain,ch2_off,ch2_gain);
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
