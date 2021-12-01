#include "dac_streaming.h"
#include "DACAsioNetController.h"
#include "DACStreamingManager.h"
#include "common/dac_settings.h"
#include "thread_cout.h"
#include "config.h"
#include "remote.h"
#include <chrono>

std::map<std::string,CDACStreamingManager::Ptr> g_dac_manger;
std::map<std::string,CDACAsioNetController::Ptr> g_dac_asionet;

std::mutex         g_dac_smutex;
ClientOpt::Options g_dac_soption;
std::atomic<bool>  dac_sig_exit_flag(false);

std::map<std::string,long long int>   g_dac_timeBegin;
std::map<std::string,bool>            g_dac_terminate;
std::map<std::string,bool>            g_dac_connected;
std::map<std::string,uint64_t>        g_dac_BytesCount;
std::map<std::string,uint64_t>        g_dac_packCounter_ch1;
std::map<std::string,uint64_t>        g_dac_packCounter_ch2;

std::vector<std::thread>              dac_clients;

auto stopDACStreaming() -> void;
auto stopDACStreaming(std::string host) -> void;



auto dac_runClient(DacSettings conf) -> void{
    std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
    g_dac_timeBegin[conf.host] = std::chrono::time_point_cast<std::chrono::milliseconds >(timeNow).time_since_epoch().count();

    g_dac_packCounter_ch1[conf.host] = 0;
    g_dac_packCounter_ch2[conf.host] = 0;
    g_dac_BytesCount[conf.host] = 0;
    g_dac_terminate[conf.host] = false;
    std::atomic<bool>  err_exit_flag(false);
    std::atomic<bool>  err_local_flag(false);
    std::atomic<bool>  started_flag(false);
    asionet::Protocol protocol = asionet::TCP;




    CDACStreamingManager::DACStream_FileType file_type = CDACStreamingManager::WAV_TYPE;

    switch(conf.file_type){
        case CStreamSettings::TDMS:
            file_type = CDACStreamingManager::TDMS_TYPE;
            break;
        case CStreamSettings::WAV:
            file_type = CDACStreamingManager::WAV_TYPE;
            break;
        default:
            stopDACStreaming(conf.host);
            return;
    }

    CStreamSettings::DACRepeat rep_mode = CStreamSettings::DACRepeat::DAC_REP_OFF;

    if (conf.dac_repeat == (int)ClientOpt::RepeatDAC::INF){
        rep_mode = CStreamSettings::DACRepeat::DAC_REP_INF;
    }
    if (conf.dac_repeat >= 0){
        rep_mode = CStreamSettings::DACRepeat::DAC_REP_ON;
    }

    int dac_rep = conf.dac_repeat >= 0 ? conf.dac_repeat : 0;


    g_dac_manger[conf.host] = CDACStreamingManager::Create(file_type ,conf.dac_file,rep_mode,dac_rep,conf.dac_memory);

    g_dac_manger[conf.host]->notifyStop = [=](CDACStreamingManager::NotifyResult res){
        const std::lock_guard<std::mutex> lock(g_dac_smutex);
        switch(res){
            case CDACStreamingManager::NotifyResult::NR_BROKEN:{
                if (conf.verbous)
                    std::cout << getTS(": ") << "File" << conf.dac_file << " is broken\n" ;
                break;
            }
            case CDACStreamingManager::NotifyResult::NR_EMPTY:{
                if (conf.verbous)
                    std::cout << getTS(": ") << "File" << conf.dac_file << " is empty\n" ;
                break;
            }
            case CDACStreamingManager::NotifyResult::NR_MISSING_FILE:{
                if (conf.verbous)
                    std::cout << getTS(": ") << "File" << conf.dac_file << " is missing\n" ;
                break;
            }
            case CDACStreamingManager::NotifyResult::NR_STOP:{
                if (conf.verbous)
                    std::cout << getTS(": ") << "Get stop command from data controller\n" ;
                break;
            }
            case CDACStreamingManager::NotifyResult::NR_ENDED:{
                if (conf.verbous)
                    std::cout << getTS(": ") << "All data sended " << conf.dac_file << "\n" ;
                break;
            }
            default:
            break;
        }
        g_dac_terminate[conf.host] = true;
    };

    g_dac_connected[conf.host] = false;
    g_dac_asionet[conf.host] = std::make_shared<CDACAsioNetController>();
    g_dac_asionet[conf.host]->addHandler(CDACAsioNetController::Events::CONNECTED, [=](std::string host){
        const std::lock_guard<std::mutex> lock(g_dac_smutex);
        std::cout << getTS(": ") << "CLIENT CONNECTED " << host << "\n" ;
        g_dac_connected[conf.host] = true;
    });

    g_dac_asionet[conf.host]->addHandler(CDACAsioNetController::Events::DISCONNECTED, [=](std::string host){
        const std::lock_guard<std::mutex> lock(g_dac_smutex);
        std::cout << getTS(": ") << "CLIENT DISCONNECTED " << host << "\n" ;
        g_dac_terminate[conf.host] = true;
    });

    g_dac_asionet[conf.host]->startAsioNet(asionet_simple::CAsioSocketSimple::ASMode::AS_CLIENT,conf.host,conf.port != "" ? conf.port : "8903");

    auto beginTime = std::chrono::time_point_cast<std::chrono::milliseconds >(std::chrono::system_clock::now()).time_since_epoch().count();
    auto curTime = beginTime;
    while(!g_dac_connected[conf.host]){
        if (curTime - beginTime >= 5000) break;
        curTime = std::chrono::time_point_cast<std::chrono::milliseconds >(std::chrono::system_clock::now()).time_since_epoch().count();
    }


    if (g_dac_connected[conf.host]){
        while(1){
            uint8_t *ch1 = nullptr;
            uint8_t *ch2 = nullptr;
            size_t size1 = 0;
            size_t size2 = 0;
            auto res = g_dac_manger[conf.host]->getBuffer();
            if (!res.empty){
                ch1 = res.ch1;
                ch2 = res.ch2;
                size1 = res.size_ch1;
                size2 = res.size_ch2;
                g_dac_asionet[conf.host]->sendBuffer(ch1,size1,ch2,size2);
            }
            if (ch1){
                g_dac_packCounter_ch1[conf.host]++;
                g_dac_BytesCount[conf.host] += size1;
                delete[](ch1);
            }
            if (ch2){
                g_dac_packCounter_ch2[conf.host]++;
                g_dac_BytesCount[conf.host] += size2;
                delete[](ch2);
            }

            if (g_dac_terminate[conf.host]){
                break;
            }

            std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
            auto curTime = std::chrono::time_point_cast<std::chrono::milliseconds >(timeNow);
            auto value = curTime.time_since_epoch();
            if ((value.count() - g_dac_timeBegin[conf.host]) >= 5000) {
                const std::lock_guard<std::mutex> lock(g_dac_smutex);
                uint64_t bw = g_dac_BytesCount[conf.host];
                std::string pref = " ";
                if (g_dac_BytesCount[conf.host]  > (1024 * 5)) {
                    bw = g_dac_BytesCount[conf.host]  / (1024 * 5);
                    pref = " ki";
                }

                if (g_dac_BytesCount[conf.host]   > (1024 * 1024 * 5)) {
                    bw = g_dac_BytesCount[conf.host]   / (1024 * 1024 * 5);
                    pref = " Mi";
                }
                std::cout << getTS(": ") << "\tHOST IP:" << conf.host << ": Bandwidth:\t" << bw <<  pref <<"B/s\tData count ch1:\t" << g_dac_packCounter_ch1[conf.host]
                << "\tch2:\t" << g_dac_packCounter_ch2[conf.host]  << "\n";
                g_dac_BytesCount[conf.host]  = 0;
                g_dac_timeBegin[conf.host] = value.count();
            }
        }
    }

    stopDACStreaming(conf.host);
}

auto startDACStreaming(std::string &conf) -> void{
    auto settings = DacSettings::readFromFile(conf);
    if (settings.size() > 0){
        ClientOpt::Options remote_opt;
        remote_opt.mode = ClientOpt::Mode::REMOTE;
        remote_opt.remote_mode = ClientOpt::RemoteMode::START_DAC;
        bool verbous = false;
        for (auto &item : settings){
            remote_opt.hosts.push_back(item.host);
            remote_opt.port = item.config_port;
            verbous |= item.verbous;
        }
        remote_opt.verbous = verbous;
        std::map<string,StateRunnedHosts> runned_hosts;
        if (startRemote(remote_opt,&runned_hosts)){

            for(auto &kv:runned_hosts){
                if (kv.second == StateRunnedHosts::TCP){
                    auto find = std::find_if(std::begin(settings),std::end(settings),[&kv](const DacSettings &c){
                        return c.host == kv.first;
                    });
                    if (find != std::end(settings)){
                        dac_clients.push_back(std::thread(dac_runClient, *find));
                    }else{
                        continue;
                    }
                }
            }

            for (std::thread &t: dac_clients) {
                if (t.joinable()) {
                    t.join();
                }
            }
        }
    }
}

auto startDACStreaming(ClientOpt::Options &option) -> void{
    g_dac_soption = option;

    ClientOpt::Options remote_opt = g_dac_soption;
    remote_opt.mode = ClientOpt::Mode::REMOTE;
    remote_opt.remote_mode = ClientOpt::RemoteMode::START_DAC;
    remote_opt.port = g_dac_soption.controlPort;
    remote_opt.verbous = g_dac_soption.verbous;
    std::map<string,StateRunnedHosts> runned_hosts;
    if (startRemote(remote_opt,&runned_hosts)){

        for(auto &kv:runned_hosts){
            if (kv.second == StateRunnedHosts::TCP){
                DacSettings conf;
                conf.host = kv.first;
                conf.port = g_dac_soption.dac_port;
                switch(g_dac_soption.streamign_type){
                    case ClientOpt::StreamingType::TDMS:
                        conf.file_type = CStreamSettings::TDMS;
                        break;
                    case ClientOpt::StreamingType::WAV:
                        conf.file_type = CStreamSettings::WAV;
                        break;
                    default:
                        conf.file_type = CStreamSettings::UNDEF;
                        return;
                }
                conf.dac_file = g_dac_soption.dac_file;
                conf.dac_repeat = g_dac_soption.dac_repeat;
                conf.dac_memory = g_dac_soption.dac_memory;
                conf.verbous = g_dac_soption.verbous;
                dac_clients.push_back(std::thread(dac_runClient, conf));
            }
        }

        for (std::thread &t: dac_clients) {
            if (t.joinable()) {
                t.join();
            }
        }
    }
}

auto dac_streamingSIGHandler() -> void{
    stopDACStreaming();
    dac_sig_exit_flag = true;
}

auto stopDACStreaming(std::string host) -> void{
    if (g_dac_manger.count(host))
        g_dac_manger[host]->stop();
    if (g_dac_asionet.count(host))
        g_dac_asionet[host]->stopAsioNet();
    g_dac_terminate[host] = true;
}

auto stopDACStreaming() -> void{
    for(const auto& kv : g_dac_manger){
        kv.second->stop();
    }
    for(const auto& kv : g_dac_asionet){
        kv.second->stopAsioNet();
    }
    for(auto& kv : g_dac_terminate){
        kv.second = true;
    }
}
