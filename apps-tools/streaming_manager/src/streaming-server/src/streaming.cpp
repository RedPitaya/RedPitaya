#include <vector>
#include <memory>

#ifdef RP_PLATFORM
#include "rp.h"
#else
#define ADC_SAMPLE_RATE 100000000
#endif

#ifndef _WIN32
#include <syslog.h>
#endif

#include "uio_lib/oscilloscope.h"
#include "streaming_lib/streaming_net.h"
#include "streaming_lib/streaming_fpga.h"
#include "streaming_lib/streaming_buffer_cached.h"
#include "streaming_lib/streaming_file.h"

#include "streaming_fpga.h"
#include "streaming_buffer.h"
#include "streaming.h"

#ifdef Z20_250_12
#include "api250-12/rp-spi.h"
#include "api250-12/rp-gpio-power.h"
#include "api250-12/rp-i2c-max7311.h"
#endif

using namespace streaming_lib;
using namespace uio_lib;

COscilloscope::Ptr          g_osc = nullptr;
CStreamingFPGA::Ptr         g_s_fpga = nullptr;
CStreamingBufferCached::Ptr g_s_buffer = nullptr;
CStreamingNet::Ptr          g_s_net = nullptr;
CStreamingFile::Ptr         g_s_file = nullptr;

bool                                    g_verbMode = false;
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

    g_s_file = nullptr;
    g_s_net = nullptr;
    g_s_buffer = nullptr;
    g_s_fpga = nullptr;
    g_osc = nullptr;

    g_verbMode = verbMode;
	try{
		CStreamSettings settings = testMode ? g_serverNetConfig->getTempSettings() : g_serverNetConfig->getSettings();
#ifndef RP_PLATFORM
        settings.resetDefault();
#endif
        if (!settings.isSetted()) {
            g_serverNetConfig->sendConfigFileMissed();
            printWithLog(LOG_ERR,stderr,"[ERROR] Config file is missing\n");
            return;
        }

		auto resolution   = settings.getResolution();
		auto format       = settings.getFormat();
		auto sock_port    = settings.getPort();
		auto use_file     = settings.getSaveType();
		auto protocol     = settings.getProtocol();
		auto channel      = settings.getChannels();
		auto rate         = settings.getDecimation();
        auto ip_addr_host = std::string("127.0.0.1");
		auto samples      = settings.getSamples();
		auto save_mode    = settings.getType();

#ifdef Z20
		auto use_calib    = false;
		auto attenuator   = 0;
#else
		auto use_calib    = settings.getCalibration();
		auto attenuator   = settings.getAttenuator();
#ifdef RP_PLATFORM
		rp_CalibInit();
		auto osc_calib_params = rp_GetCalibrationSettings();
#endif
#endif

#ifdef Z20_250_12
		auto ac_dc = settings.getAC_DC();
#endif

        auto uioList = uio_lib::GetUioList();
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
#ifdef RP_PLATFORM
		if (use_calib) {
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
				if (ac_dc == CStreamSettings::AC) {
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

        for (auto &uio : uioList)
		{
			if (uio.nodeName == "rp_oscilloscope")
			{
				#ifdef STREAMING_MASTER
					auto isMaster = true;
				#endif
				#ifdef STREAMING_SLAVE
					auto isMaster = false;
				#endif
                g_osc = COscilloscope::create(uio,rate, isMaster,ADC_SAMPLE_RATE);
                g_osc->setCalibration(ch1_off,ch1_gain,ch2_off,ch2_gain);
                g_osc->setFilterCalibrationCh1(aa_ch1,bb_ch1,kk_ch1,pp_ch1);
                g_osc->setFilterCalibrationCh2(aa_ch2,bb_ch2,kk_ch2,pp_ch2);
                g_osc->setFilterBypass(filterBypass);
                g_osc->set8BitMode(resolution == CStreamSettings::BIT_8);
				break;
			}
		}
#else
#ifdef STREAMING_MASTER
        auto isMaster = true;
#endif
#ifdef STREAMING_SLAVE
        auto isMaster = false;
#endif
        uio_lib::UioT uio_t;
        g_osc = COscilloscope::create(uio_t,rate, isMaster,ADC_SAMPLE_RATE);
#endif

        g_s_buffer = streaming_lib::CStreamingBufferCached::create();
        auto g_s_buffer_w = std::weak_ptr<CStreamingBufferCached>(g_s_buffer);

		if (use_file == CStreamSettings::NET) {
            auto proto = protocol == CStreamSettings::TCP ? net_lib::EProtocol::P_TCP : net_lib::EProtocol::P_UDP;
            g_s_net = streaming_lib::CStreamingNet::create(ip_addr_host,sock_port,proto);
            
            g_s_net->getBuffer = [g_s_buffer_w]() -> DataLib::CDataBuffersPack::Ptr{
                auto obj = g_s_buffer_w.lock();
                if (obj) {
                    return obj->readBuffer();
                }
                return nullptr;
            };

            g_s_net->unlockBufferF = [g_s_buffer_w](){
				auto obj = g_s_buffer_w.lock();
				if (obj){
					obj->unlockBufferRead();
				}
				return nullptr;
        	};
        }

        if (use_file == CStreamSettings::FILE) {
            auto f_path = std::string(FILE_PATH);
            g_s_file = streaming_lib::CStreamingFile::create(format,f_path,samples, save_mode == CStreamSettings::VOLT, testMode);
            g_s_file->stopNotify.connect([](CStreamingFile::EStopReason r){
                switch (r) {
                    case CStreamingFile::EStopReason::NORMAL:{
                        stopNonBlocking(ServerNetConfigManager::EStopReason::NORMAL);
                        break;
                    }

                    case CStreamingFile::EStopReason::OUT_SPACE:{
                        stopNonBlocking(ServerNetConfigManager::EStopReason::SD_FULL);
                        break;
                    }

                    case CStreamingFile::EStopReason::REACH_LIMIT:{
                        stopNonBlocking(ServerNetConfigManager::EStopReason::DONE);
                        break;
                    }
                }
            });
		}

        g_s_fpga = std::make_shared<streaming_lib::CStreamingFPGA>(g_osc,16);
        uint8_t resolution_val = (resolution == CStreamSettings::BIT_8 ? 8 : 16);
        auto att = attenuator == CStreamSettings::A_1_1 ? DataLib::CDataBuffer::ATT_1_1 :  DataLib::CDataBuffer::ATT_1_20;
        if(channel == CStreamSettings::CH1 || channel == CStreamSettings::BOTH){
            g_s_fpga->addChannel(DataLib::CH1,att,resolution_val);
            g_s_buffer->addChannel(DataLib::CH1,uio_lib::osc_buf_size,resolution_val);
        }
        if(channel == CStreamSettings::CH2 || channel == CStreamSettings::BOTH){
            g_s_fpga->addChannel(DataLib::CH2,att,resolution_val);
            g_s_buffer->addChannel(DataLib::CH2,uio_lib::osc_buf_size,resolution_val);
        }
        g_s_buffer->generateBuffers();
        g_s_fpga->setVerbousMode(g_verbMode);
        g_s_fpga->setTestMode(testMode);

        auto weak_obj = std::weak_ptr<CStreamingBufferCached>(g_s_buffer);
        g_s_fpga->getBuffF = [weak_obj](uint64_t lostFPGA) -> DataLib::CDataBuffersPack::Ptr {
            auto obj = weak_obj.lock();
            if (obj){
                return obj->getFreeBuffer(lostFPGA);
            }
            return nullptr;
        };

        g_s_fpga->unlockBuffF = [weak_obj](){
            auto obj = weak_obj.lock();
            if (obj){
                obj->unlockBufferWrite();
            }
            return nullptr;
        };

        auto g_s_file_w = std::weak_ptr<CStreamingFile>(g_s_file);
        g_s_fpga->oscNotify.connect([g_s_file_w,g_s_buffer_w](DataLib::CDataBuffersPack::Ptr) {
            auto f_obj = g_s_file_w.lock();
			auto b_obj = g_s_buffer_w.lock();
            if (f_obj && b_obj){
				auto p = b_obj->readBuffer();
				if (p){
                	f_obj->passBuffers(p);
					b_obj->unlockBufferRead();
				}
            }
        });

		char time_str[40];
    	struct tm *timenow;
    	time_t now = time(nullptr);
    	timenow = gmtime(&now);
    	strftime(time_str, sizeof(time_str), "%Y-%m-%d_%H-%M-%S", timenow);
    	std::string filenameDate = time_str;

        if (g_s_net){
            g_s_net->run();

            if (g_s_net->getProtocol() == net_lib::EProtocol::P_TCP){
                g_serverNetConfig->sendServerStartedTCP();
            }
            if (g_s_net->getProtocol() == net_lib::EProtocol::P_UDP){
                g_serverNetConfig->sendServerStartedUDP();
            }
        }

        if (g_s_file){
            g_s_file->run(filenameDate);
            g_serverNetConfig->sendServerStartedSD();
        }

		if (g_verbMode){
            printWithLog(LOG_NOTICE,stdout,"[Streaming] Start server %s\n",testMode ? "[Benchmark mode]":"");
        }
	}catch (std::exception& e)
	{
        printWithLog(LOG_ERR,stderr, "Error: StartServer() %s\n",e.what());
    }
}

auto stopNonBlocking(ServerNetConfigManager::EStopReason x) -> void{
	try{
        std::thread th(stopServer,x);
		th.detach();
	}catch (std::exception& e)
	{
        printWithLog(LOG_ERR,stderr, "Error: StopNonBlocking() %s\n",e.what());
    }
}


auto stopServer(ServerNetConfigManager::EStopReason reason) -> void{
	try{
        if (g_s_buffer) g_s_buffer->notifyToDestory();
        g_s_net = nullptr;
        g_s_file = nullptr;
        g_s_buffer = nullptr;
        g_s_fpga = nullptr;

        if (g_serverNetConfig){
            switch (reason)
            {
            case ServerNetConfigManager::EStopReason::NORMAL:
                g_serverNetConfig->sendServerStopped();
                break;
            case ServerNetConfigManager::EStopReason::SD_FULL:
                g_serverNetConfig->sendServerStoppedSDFull();
                break;
            case ServerNetConfigManager::EStopReason::DONE:
                g_serverNetConfig->sendServerStoppedDone();
                break;
            default:
                throw runtime_error("Unknown state");
                break;
            }
        }

		if (g_verbMode){
            aprintf(stdout,"[Streaming] Stop server\n");
#ifndef _WIN32
            syslog (LOG_NOTICE, "[Streaming] Stop server\n");
#endif
        }
	}catch (std::exception& e)
	{
        printWithLog(LOG_ERR,stderr, "Error: StopServer() %s\n",e.what());
    }
}

auto startADC() -> void{
	try{
        if (g_s_fpga){
            g_s_fpga->runNonBlock();
		}
	}catch (std::exception& e)
	{
        printWithLog(LOG_ERR,stderr, "Error: startADC() %s\n",e.what());
    }
}
