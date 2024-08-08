#include <vector>
#include <memory>

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

#include "uio_lib/oscilloscope.h"
#include "streaming_lib/streaming_net.h"
#include "streaming_lib/streaming_fpga.h"
#include "streaming_lib/streaming_buffer_cached.h"
#include "streaming_lib/streaming_file.h"

#include "streaming_fpga.h"
#include "streaming_buffer.h"
#include "streaming.h"

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

auto startServer(bool verbMode,bool testMode,bool is_master) -> void{
	// Search oscilloscope
    if (!g_serverNetConfig) return;

    g_s_fpga = {nullptr};
    g_osc = {nullptr};
    g_s_buffer = {nullptr};
    g_s_file = {nullptr};
    g_s_net = {nullptr};

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

		auto use_calib    = settings.getCalibration();
		auto attenuator   = settings.getAttenuator();
        auto ac_dc        = 0xF;
        auto max_channels = ClientOpt::getADCChannels();

#ifdef RP_PLATFORM
        use_calib    = settings.getCalibration();
		if (rp_CalibInit() != RP_HW_CALIB_OK){
	        fprintf(stderr,"Error init calibration\n");
    	}
		ac_dc = settings.getAC_DC();
#endif


        auto uioList = uio_lib::GetUioList();
		int32_t ch_off[MAX_ADC_CHANNELS] = {0,0,0,0};
		double  ch_gain[MAX_ADC_CHANNELS] = {1,1,1,1};
		bool  filterBypass = true;
		uint32_t aa_ch[MAX_ADC_CHANNELS] = {0,0,0,0};
		uint32_t bb_ch[MAX_ADC_CHANNELS] = {0,0,0,0};
		uint32_t kk_ch[MAX_ADC_CHANNELS] = {0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF};
		uint32_t pp_ch[MAX_ADC_CHANNELS] = {0,0,0,0};
        bool is_platform = false;
#ifdef RP_PLATFORM
        is_platform = true;
        filterBypass = !rp_HPGetFastADCIsFilterPresentOrDefault();
		if (use_calib) {
            for(uint8_t ch = 0; ch < max_channels; ++ch){
   				rp_acq_ac_dc_mode_calib_t  mode = (CStreamSettings::checkChannel(ac_dc, ch) ? RP_DC_CALIB : RP_AC_CALIB);
                if (CStreamSettings::checkChannel(attenuator,ch) == false){
                    if (rp_CalibGetFastADCCalibValue((rp_channel_calib_t)ch,mode,&ch_gain[ch],&ch_off[ch]) != RP_HW_CALIB_OK){
                        fprintf(stderr,"Error get calibration channel: %d\n",ch);
                    }

                    if (!filterBypass){
                        channel_filter_t f;
                        if (rp_CalibGetFastADCFilter((rp_channel_calib_t)ch,&f) != RP_HW_CALIB_OK){
                            fprintf(stderr,"Error get filter value: %d\n",ch);
                        }
                        aa_ch[ch] = f.aa;
                        bb_ch[ch] = f.bb;
                        kk_ch[ch] = f.kk;
                        pp_ch[ch] = f.pp;
                    }
                } else {
                    if (rp_CalibGetFastADCCalibValue_1_20((rp_channel_calib_t)ch,mode,&ch_gain[ch],&ch_off[ch]) != RP_HW_CALIB_OK){
                        fprintf(stderr,"Error get calibration channel: %d\n",ch);
                    }

                    if (!filterBypass){
                        channel_filter_t f;
                        if (rp_CalibGetFastADCFilter_1_20((rp_channel_calib_t)ch,&f) != RP_HW_CALIB_OK){
                            fprintf(stderr,"Error get filter value: %d\n",ch);
                        }
                        aa_ch[ch] = f.aa;
                        bb_ch[ch] = f.bb;
                        kk_ch[ch] = f.kk;
                        pp_ch[ch] = f.pp;
                    }
                }
			}
		}

        if (rp_HPGetIsAttenuatorControllerPresentOrDefault()){
		    rp_max7311::rp_setAttenuator(RP_MAX7311_IN1, attenuator & CStreamSettings::CH1 ? RP_ATTENUATOR_1_20 : RP_ATTENUATOR_1_1);
		    rp_max7311::rp_setAttenuator(RP_MAX7311_IN2, attenuator & CStreamSettings::CH2 ? RP_ATTENUATOR_1_20 : RP_ATTENUATOR_1_1);
        }

        if (rp_HPGetFastADCIsAC_DCOrDefault()){
	    	rp_max7311::rp_setAC_DC(RP_MAX7311_IN1, ac_dc & CStreamSettings::CH1 ? RP_DC_MODE : RP_AC_MODE);
	    	rp_max7311::rp_setAC_DC(RP_MAX7311_IN2, ac_dc & CStreamSettings::CH2 ? RP_DC_MODE : RP_AC_MODE);
        }

        for (auto &uio : uioList)
		{
			if (uio.nodeName == "rp_oscilloscope")
			{
                g_osc = COscilloscope::create(uio,rate, is_master,ClientOpt::getADCRate(),!filterBypass,ClientOpt::getADCBits(),max_channels);
				for(uint8_t ch = 0; ch < max_channels; ++ch){
					g_osc->setCalibration(ch,ch_off[ch],ch_gain[ch]);
                	g_osc->setFilterCalibration(ch,aa_ch[ch],bb_ch[ch],kk_ch[ch],pp_ch[ch]);
				}
                g_osc->setFilterBypass(filterBypass);
                g_osc->set8BitMode(resolution == CStreamSettings::BIT_8);
				break;
			}
		}
#else
        uio_lib::UioT uio_t;
        g_osc = COscilloscope::create(uio_t,rate, is_master,ClientOpt::getADCRate(),!filterBypass,16, 2);
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
            g_s_file = streaming_lib::CStreamingFile::create(format,f_path,samples, save_mode == CStreamSettings::VOLT, testMode, is_platform);
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

        for(int i = 0; i < max_channels; i++){
            if(CStreamSettings::checkChannel(channel,i)){
                g_s_fpga->addChannel((DataLib::EDataBuffersPackChannel)i, CStreamSettings::checkChannel(attenuator,i) ? DataLib::CDataBuffer::ATT_1_20 : DataLib::CDataBuffer::ATT_1_1,resolution_val);
                g_s_buffer->addChannel((DataLib::EDataBuffersPackChannel)i,uio_lib::osc_buf_size,resolution_val);
            }
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
        g_s_fpga = nullptr;
        g_s_buffer = nullptr;
        g_s_net = nullptr;
        g_s_file = nullptr;

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
