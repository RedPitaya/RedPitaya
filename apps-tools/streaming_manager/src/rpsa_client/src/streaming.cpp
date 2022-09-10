#include "streaming.h"
#include "net_lib/asio_net.h"
#include "streaming_lib/streaming_file.h"
#include "streaming_lib/streaming_net_buffer.h"
#include "data_lib/thread_cout.h"
#include "settings_lib/stream_settings.h"
#include "writer_lib/file_helper.h"
#include "converter_lib/converter.h"
#include "config.h"
#include "remote.h"
#include "test_helper.h"

//std::map<std::string,streaming_lib::CStreamingFile::Ptr> g_file_manager;
std::map<std::string,converter_lib::CConverter::Ptr> g_converter;
//std::map<std::string,streaming_lib::CStreamingNetBuffer::Ptr> g_net_buffer;

//std::map<std::string,net_lib::CAsioNet::Ptr> g_asionet;

std::mutex         g_smutex;
std::mutex         g_s_csv_mutex;
ClientOpt::Options g_soption;
std::string        g_filenameDate;
std::atomic<bool>  sig_exit_flag(false);
std::atomic<int>   g_runClientCounter;

std::map<std::string,bool>            g_terminate;

std::vector<std::thread>              clients;

auto stopCSV () -> void;
auto stopStreaming() -> void;
auto stopStreaming(std::string host) -> void;




auto runClient(std::string  host,StateRunnedHosts state) -> void{
    g_terminate[host] = false;    
    auto protocol = net_lib::EProtocol::P_TCP;

    if (g_soption.save_dir == "")
        g_soption.save_dir = ".";

    auto file_type = CStreamSettings::UNDEF;
    switch(g_soption.streamign_type){
        case ClientOpt::StreamingType::TDMS:
            file_type = CStreamSettings::TDMS;
            break;
        case ClientOpt::StreamingType::WAV:
            file_type = CStreamSettings::WAV;
            break;
        case ClientOpt::StreamingType::CSV:
            file_type = CStreamSettings::BIN;
            break;
        case ClientOpt::StreamingType::BIN:
            file_type = CStreamSettings::BIN;
            break;
        default:
            stopStreaming(host);
            return;
    }

    bool convert_v = false;
    switch (g_soption.save_type) {
        case ClientOpt::SaveType::RAW:
            convert_v = false;
            break;
        case ClientOpt::SaveType::VOL:
            convert_v = true;
            break;
        default:
            convert_v = false;
            break;
    }

    if (state == StateRunnedHosts::UDP)
        protocol = net_lib::EProtocol::P_UDP;

    bool testMode = g_soption.testmode == ClientOpt::TestMode::ENABLE;
    auto g_file_manager = streaming_lib::CStreamingFile::create(file_type, g_soption.save_dir, g_soption.samples , convert_v,testMode);
    g_file_manager->run(host + "_" + g_filenameDate);
    auto g_net_buffer = streaming_lib::CStreamingNetBuffer::create();
    g_net_buffer->outMemoryNotify.connect([host](uint64_t ram){
        if (g_soption.verbous)
            aprintf(stderr,"%s Out of memory (%d)\n", getTS(": ").c_str(),host.c_str(),ram);
    });

    auto g_s_file_w = std::weak_ptr<streaming_lib::CStreamingFile>(g_file_manager);
    g_net_buffer->brokenPacksNotify.connect([g_s_file_w,host](uint64_t count){
        auto obj = g_s_file_w.lock();
        if(obj){
            obj->addNetWorkLost(count);
        }
    });


    g_net_buffer->receivedPackNotify.connect([g_s_file_w,host](DataLib::CDataBuffersPack::Ptr pack,uint64_t){
        auto obj = g_s_file_w.lock();
        if (obj){
            if (g_soption.testmode == ClientOpt::TestMode::ENABLE || g_soption.verbous){
                uint64_t sempCh1 = 0;
                uint64_t sempCh2 = 0;
                uint64_t sizeCh1 = 0;
                uint64_t sizeCh2 = 0;
                uint64_t lostRate = 0;
                auto ch1 = pack->getBuffer(DataLib::CH1);
                auto ch2 = pack->getBuffer(DataLib::CH2);
                if (ch1){
                    sempCh1 = ch1->getSamplesCount();
                    sizeCh1 = ch1->getBufferLenght();
                    lostRate += ch1->getLostSamplesAll();
                }

                if (ch2){
                    sempCh2 = ch2->getSamplesCount();
                    sizeCh2 = ch2->getBufferLenght();
                    lostRate += ch2->getLostSamplesAll();
                }

                auto net   = obj->getNetworkLost();
                auto flost = obj->getFileLost();
                int  brokenBuffer = -1;
                if (g_soption.testStreamingMode == ClientOpt::TestSteamingMode::WITH_TEST_DATA){
                    brokenBuffer = testBuffer(ch1 ? ch1->getBuffer().get() : nullptr,ch2 ? ch2->getBuffer().get() : nullptr,sizeCh1,sizeCh2) ? 0 : 1;
                }
                auto h = host;
                addStatisticSteaming(h,sizeCh1 + sizeCh2,sempCh1,sempCh2,lostRate, net, flost,brokenBuffer);
            }
          obj->passBuffers(pack);
        }
    });


    auto g_asionet = net_lib::CAsioNet::create(net_lib::M_CLIENT, protocol ,host , g_soption.ports.streaming_port  != "" ? g_soption.ports.streaming_port : ClientOpt::Ports().streaming_port);

    g_asionet->clientConnectNotify.connect([](std::string host) {
        const std::lock_guard<std::mutex> lock(g_smutex);
        if (g_soption.verbous)
            aprintf(stdout,"%s Connect %s\n",getTS(": ").c_str(),host.c_str());
        g_runClientCounter--;
    });

    g_asionet->clientErrorNotify.connect([host](std::error_code)
    {
        const std::lock_guard<std::mutex> lock(g_smutex);
        if (g_soption.verbous)
            aprintf(stdout,"%s Error %s\n",getTS(": ").c_str(),host.c_str());
        stopStreaming(host);
        g_runClientCounter--;
    });

    auto g_net_buffer_w = std::weak_ptr<streaming_lib::CStreamingNetBuffer>(g_net_buffer);
    g_asionet->reciveNotify.connect([g_net_buffer_w](std::error_code error,uint8_t *buff,size_t _size){
        auto obj = g_net_buffer_w.lock();        
        if (obj){
            if (!error){
                obj->addNewBuffer(buff,_size);
            }
        }
    });
    g_asionet->start();    
    auto beginTime = std::chrono::time_point_cast<std::chrono::milliseconds >(std::chrono::system_clock::now()).time_since_epoch().count();
    auto curTime = beginTime;
    while(g_file_manager->isFileThreadWork() &&  !g_terminate[host]){
        sleepMs(1);
        if (g_soption.timeout >= 0){
            if (curTime - beginTime >= g_soption.timeout) break;
            curTime = std::chrono::time_point_cast<std::chrono::milliseconds >(std::chrono::system_clock::now()).time_since_epoch().count();
        }
        if (g_soption.testmode == ClientOpt::TestMode::ENABLE || g_soption.verbous){
            printStatisitc(false);
        }
    }

    g_file_manager->stop();
    g_asionet->stop();
    if (g_soption.streamign_type == ClientOpt::StreamingType::CSV && g_soption.testmode != ClientOpt::TestMode::ENABLE) {
        const std::lock_guard<std::mutex> lock(g_s_csv_mutex);
        auto fileName = g_file_manager->getCSVFileName();
          g_converter[host] = converter_lib::CConverter::create();
        g_converter[host]->convertToCSV(fileName,host);
    }
}

auto startStreaming(std::shared_ptr<ClientNetConfigManager> cl,ClientOpt::Options &option) -> void{
    g_soption = option;


    char time_str[40];
    struct tm *timenow;
    time_t now = time(nullptr);
    timenow = gmtime(&now);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d_%H-%M-%S", timenow);
    g_filenameDate = time_str;

#ifndef  _WIN32
    auto size = getFreeSpaceDisk(option.save_dir != "" ? option.save_dir : "." );
    if (g_soption.verbous)
        aprintf(stdout,"%s Free disk space:  %d Mb\n", getTS(": ").c_str(), size / (1024 * 1024));
#endif

    resetStreamingCounter();
    if (g_soption.testmode == ClientOpt::TestMode::ENABLE){
        if (!sendCopyToTestConfig(cl,option)){
            return;
        }
    }

    ClientOpt::Options remote_opt = g_soption;
    remote_opt.mode = ClientOpt::Mode::REMOTE;
    remote_opt.remote_mode = ClientOpt::RemoteMode::STOP;
    remote_opt.ports.config_port = g_soption.ports.config_port  != "" ? g_soption.ports.config_port : ClientOpt::Ports().config_port;
    remote_opt.verbous = g_soption.verbous;
    std::map<string,StateRunnedHosts> runned_hosts;
    if (!startRemote(cl,remote_opt,&runned_hosts)){
        aprintf(stdout,"%s Can't stop streaming on remote machines\n", getTS(": ").c_str());
        return;
    }
    runned_hosts.clear();
    remote_opt.remote_mode = ClientOpt::RemoteMode::START;
    if (startRemote(cl,remote_opt,&runned_hosts)){
        for(auto kv:runned_hosts){
            if (kv.second == StateRunnedHosts::TCP || kv.second == StateRunnedHosts::UDP)
                clients.push_back(std::thread(runClient, kv.first,kv.second));
        }
        while (g_runClientCounter>0){
            sleepMs(100);
            if (sig_exit_flag) {
                break;
            }
        }

        remote_opt.remote_mode = ClientOpt::RemoteMode::START_FPGA_ADC;
        std::vector<string> runnedThreads;
        for (const auto& [key, _] : runned_hosts) {
            runnedThreads.push_back(key);
        }
        if (!startRemote(cl,remote_opt,&runned_hosts)){
            aprintf(stdout,"%s Can't start ADC on remote machines\n", getTS(": ").c_str());
            for(auto &h:runnedThreads){
                if (runned_hosts.count(h) == 0)
                    stopStreaming(h);
            }
        }

        cl->errorNofiy.connect([](ClientNetConfigManager::Errors errors,std::string host,error_code err){
            if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
                aprintf(stderr,"%s Error: %s %s\n", getTS(": ").c_str(),host.c_str(),err.message().c_str());
            }
            stopStreaming(host);
        });

        cl->serverStoppedNofiy.connect([](std::string host){
            if (g_soption.verbous)
                aprintf(stderr,"%s Streaming stopped: %s [OK]\n", getTS(": ").c_str(),host.c_str());
            stopStreaming(host);
        });

        cl->serverStoppedSDFullNofiy.connect([](std::string host){
            if (g_soption.verbous)
                aprintf(stderr,"%s Streaming stopped: %s SD is full [OK]\n", getTS(": ").c_str(),host.c_str());
            stopStreaming(host);
        });

        cl->serverStoppedSDDoneNofiy.connect([](std::string host){
            if (g_soption.verbous)
                aprintf(stderr,"%s Streaming stopped: %s Local mode [OK]\n", getTS(": ").c_str(),host.c_str());
            stopStreaming(host);
        });

        for (std::thread &t: clients) {
            if (t.joinable()) {
                t.join();
            }
        }
        

        remote_opt.remote_mode = ClientOpt::RemoteMode::STOP;
        if (!startRemote(cl,remote_opt,&runned_hosts)){
            aprintf(stdout,"%s Can't stop streaming on remote machines\n", getTS(": ").c_str());
            g_converter.clear();
            return;
        }

        if (g_soption.testmode == ClientOpt::TestMode::ENABLE || g_soption.verbous){
            const std::lock_guard<std::mutex> lock(g_smutex);
            printFinalStatisitc();
        }
    }
    g_converter.clear();
}

auto streamingSIGHandler() -> void{
    stopCSV();
    stopStreaming();
    sig_exit_flag = true;
}

auto stopCSV () -> void{
    for(const auto& kv : g_converter){
        kv.second->stopWriteToCSV();
    }
}

auto stopStreaming(std::string host) -> void{
    g_terminate[host] = true;
}

auto stopStreaming() -> void{
    for(auto& kv : g_terminate){
        kv.second = true;
    }
}

