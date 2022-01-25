#include <vector>
#include "streaming.h"

CStreamingApplication  *g_app = nullptr;
CStreamingManager::Ptr 	g_manger = nullptr;
bool					g_verbMode = false;
COscilloscope::Ptr 		osc = nullptr;
std::shared_ptr<ServerNetConfigManager> g_serverNetConfig = nullptr;


auto calibFullScaleToVoltage(uint32_t fullScaleGain) -> float {
    /* no scale */
    if (fullScaleGain == 0) {
        return 1;
    }
    return (float) ((float)fullScaleGain  * 100.0 / ((uint64_t)1<<32));
}

auto setServer(std::shared_ptr<ServerNetConfigManager> serverNetConfig) -> void{
    g_serverNetConfig = serverNetConfig;
}

auto startServer(bool verbMode,bool testMode) -> void{
	// Search oscilloscope
    if (!g_serverNetConfig) return;
	osc = nullptr;
	g_verbMode = verbMode;
	try{
		CStreamSettings settings = testMode ? g_serverNetConfig->getTempSettings() : g_serverNetConfig->getSettings();
		if (!settings.isSetted()) return;

		auto resolution   = settings.getResolution();
		auto format       = settings.getFormat();
		auto sock_port    = settings.getPort();
		auto use_file     = settings.getSaveType();
		auto protocol     = settings.getProtocol();
		auto channel      = settings.getChannels();
		auto rate         = settings.getDecimation();
		auto ip_addr_host = "127.0.0.1";
		auto samples      = settings.getSamples();
		auto save_mode    = settings.getType();

#ifdef Z20
		auto use_calib    = 0;
		auto attenuator   = 0;
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

		if (g_app!= nullptr){
			g_app->stop();
			delete g_app;
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
			g_manger = CStreamingManager::Create(
					ip_addr_host,
					sock_port,
					protocol == CStreamSettings::TCP ? asionet::Protocol::TCP : asionet::Protocol::UDP);
		}else{
			auto file_type = Stream_FileType::WAV_TYPE;
			if (format == CStreamSettings::TDMS) file_type = Stream_FileType::TDMS_TYPE;
			if (format == CStreamSettings::CSV)  file_type = Stream_FileType::CSV_TYPE;
			g_manger = CStreamingManager::Create(file_type , FILE_PATH, samples , save_mode == CStreamSettings::VOLT, testMode);
			g_manger->notifyStop = [](int status)
								{
									stopNonBlocking(status == 0 ? 2 : 3);
								};
		}

		int resolution_val = (resolution == CStreamSettings::BIT_8 ? 8 : 16);
		g_app = new CStreamingApplication(g_manger, osc, resolution_val, rate, channel , attenuator , 16);
		g_app->setVerbousMode(g_verbMode);
		g_app->setTestMode(testMode);

		char time_str[40];
    	struct tm *timenow;
    	time_t now = time(nullptr);
    	timenow = gmtime(&now);
    	strftime(time_str, sizeof(time_str), "%Y-%m-%d_%H-%M-%S", timenow);
    	std::string filenameDate = time_str;

		g_app->runNonBlock(filenameDate);
		if (!g_manger->isLocalMode()){
			if (g_manger->getProtocol() == asionet::Protocol::TCP){
				g_serverNetConfig->sendServerStartedTCP();
			}
			if (g_manger->getProtocol() == asionet::Protocol::UDP){
				g_serverNetConfig->sendServerStartedUDP();
			}
		}else{
			g_serverNetConfig->sendServerStartedSD();
		}
		if (g_verbMode){
			fprintf(stdout,"[Streaming] Start server %s\n",testMode ? "[Benchmark mode]":"");
        	syslog (LOG_NOTICE, "[Streaming] Start server %s\n",testMode ? "[Benchmark mode]":"");
		}
	}catch (std::exception& e)
	{
		fprintf(stderr, "Error: StartServer() %s\n",e.what());
        syslog (LOG_ERR,"Error: StartServer() %s\n",e.what());
	}
}

auto stopNonBlocking(int x) -> void{
	try{
		std::thread th(stopServer ,x);
		th.detach();
	}catch (std::exception& e)
	{
		fprintf(stderr, "Error: StopNonBlocking() %s\n",e.what());
        syslog (LOG_ERR,"Error: StopNonBlocking() %s\n",e.what());
	}
}


auto stopServer(int x) -> void{
	try{
		if (g_app)
		{
			g_app->stop();
			delete g_app;
			g_app = nullptr;
		}
        if (g_serverNetConfig){
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
        }

		if (g_verbMode){
        	fprintf(stdout,"[Streaming] Stop server\n");
        	syslog (LOG_NOTICE, "[Streaming] Stop server\n");
		}
	}catch (std::exception& e)
	{
		fprintf(stderr, "Error: StopServer() %s\n",e.what());
        syslog (LOG_ERR,"Error: StopServer() %s\n",e.what());
	}
}