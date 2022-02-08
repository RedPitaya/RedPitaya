#include "streaming.h"
#include "AsioNet.h"
#include "StreamingManager.h"
#include "thread_cout.h"
#include "config.h"
#include "remote.h"
#include "test_helper.h"

std::map<std::string,CStreamingManager::Ptr> g_manger;
std::map<std::string,asionet::CAsioNet::Ptr> g_asionet;
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


void reciveData(std::error_code error,uint8_t *buff,size_t _size,std::string host){
    //std::cout << "Get data: " <<  _size << "\n";
    uint8_t *ch1 = nullptr;
    uint8_t *ch2 = nullptr;
    size_t   size_ch1 = 0;
    size_t   size_ch2 = 0;
    uint64_t id = 0;
    uint64_t lostRate = 0;
    uint32_t oscRate = 0;
    uint32_t resolution = 0;
    uint32_t adc_mode = 0;
    uint32_t adc_bits = 0;

    asionet::CAsioNet::ExtractPack(buff,_size, id, lostRate,oscRate, resolution, adc_mode , adc_bits, ch1, size_ch1, ch2 , size_ch2);
    
  //  std::cout << id << " ; " <<  _size  <<  " ; " << resolution << " ; " << size_ch1 << " ; " << size_ch2 << "\n";

    g_manger[host]->passBuffers(lostRate, oscRate , adc_mode , adc_bits, ch1 , size_ch1 ,  ch2 , size_ch2 , resolution, id);

    if (g_soption.testmode == ClientOpt::TestMode::ENABLE || g_soption.verbous){
        uint64_t sempCh1 = size_ch1 / (resolution == 16 ? 2 : 1);
        uint64_t sempCh2 = size_ch2 / (resolution == 16 ? 2 : 1);        
        auto net   = g_manger[host]->getNetworkLost();
        auto flost = g_manger[host]->getFileLost();
        int  brokenBuffer = -1;
        if (g_soption.testStreamingMode == ClientOpt::TestSteamingMode::WITH_TEST_DATA){
            brokenBuffer = testBuffer(ch1,ch2,size_ch1,size_ch2) ? 0 : 1;
        }
        addStatisticSteaming(host,_size,sempCh1,sempCh2,lostRate, net, flost,brokenBuffer);
    }
    delete [] ch1;
    delete [] ch2;
}

auto runClient(std::string  host,StateRunnedHosts state) -> void{
    g_terminate[host] = false;
    std::atomic<bool>  err_exit_flag(false);
    std::atomic<bool>  err_local_flag(false);
    std::atomic<bool>  started_flag(false);
    asionet::Protocol protocol = asionet::TCP;


    if (g_soption.save_dir == "")
        g_soption.save_dir = ".";

    auto file_type = Stream_FileType::WAV_TYPE;
    switch(g_soption.streamign_type){
        case ClientOpt::StreamingType::TDMS:
            file_type = Stream_FileType::TDMS_TYPE;
            break;
        case ClientOpt::StreamingType::WAV:
            file_type = Stream_FileType::WAV_TYPE;
            break;
        case ClientOpt::StreamingType::CSV:
            file_type = Stream_FileType::CSV_TYPE;
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
        protocol = asionet::UDP;

    bool testMode = g_soption.testmode == ClientOpt::TestMode::ENABLE;
    g_manger[host] = CStreamingManager::Create(file_type , g_soption.save_dir.c_str(), g_soption.samples , convert_v,testMode);
    g_manger[host]->run(host + "_" + g_filenameDate);

    g_asionet[host] = asionet::CAsioNet::Create(asionet::Mode::CLIENT, protocol ,host , g_soption.ports.streaming_port  != "" ? g_soption.ports.streaming_port : ClientOpt::Ports().streaming_port);
    g_asionet[host]->addCallClient_Connect([](std::string host) {
        const std::lock_guard<std::mutex> lock(g_smutex);
        if (g_soption.verbous)
            std::cout << getTS(": ") << "Connect " << host << '\n';
        g_runClientCounter--;
    });
    g_asionet[host]->addCallClient_Error([host](std::error_code error)
    {
        const std::lock_guard<std::mutex> lock(g_smutex);
        if (g_soption.verbous)
            std::cout << getTS(": ") <<"Disconnect;" << '\n';
        stopStreaming(host);
        g_runClientCounter--;
    });
    g_asionet[host]->addCallReceived(std::bind(&reciveData,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,host));
    g_asionet[host]->Start();
    auto beginTime = std::chrono::time_point_cast<std::chrono::milliseconds >(std::chrono::system_clock::now()).time_since_epoch().count();
    auto curTime = beginTime;
    while(g_manger[host]->isFileThreadWork() &&  !g_terminate[host]){
        sleepMs(1);
        if (g_soption.timeout >= 0){
            if (curTime - beginTime >= g_soption.timeout) break;
            curTime = std::chrono::time_point_cast<std::chrono::milliseconds >(std::chrono::system_clock::now()).time_since_epoch().count();
        }
        if (g_soption.testmode == ClientOpt::TestMode::ENABLE || g_soption.verbous){
            printStatisitc(false);
        }
    }
    
    stopStreaming(host);

    if (file_type == Stream_FileType::CSV_TYPE && g_soption.testmode != ClientOpt::TestMode::ENABLE) {
        const std::lock_guard<std::mutex> lock(g_s_csv_mutex);
        g_manger[host]->convertToCSV(host);
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
    auto size =  FileQueueManager::GetFreeSpaceDisk(option.save_dir != "" ? option.save_dir : "." );
    if (g_soption.verbous)
        std::cout << getTS(": ") << "Free disk space: "<< size / (1024 * 1024) << "Mb \n";
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
        std::cout << getTS(": ") << "Can't stop streaming on remote machines\n";
        return;
    }
    runned_hosts.clear();
    remote_opt.remote_mode = ClientOpt::RemoteMode::START;
    if (startRemote(cl,remote_opt,&runned_hosts)){
        for(auto &kv:runned_hosts){
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
        if (!startRemote(cl,remote_opt,&runned_hosts)){
            std::cout << getTS(": ") << "Can't start ADC on remote machines\n";
        }

        for (std::thread &t: clients) {
            if (t.joinable()) {
                t.join();
            }
        }

        remote_opt.remote_mode = ClientOpt::RemoteMode::STOP;
        if (!startRemote(cl,remote_opt,&runned_hosts)){
            std::cout << getTS(": ") << "Can't stop streaming on remote machines\n";
            return;
        }

        if (g_soption.testmode == ClientOpt::TestMode::ENABLE || g_soption.verbous){
            const std::lock_guard<std::mutex> lock(g_smutex);
            printFinalStatisitc();
        }
    }


}

auto streamingSIGHandler() -> void{
    stopCSV();
    stopStreaming();    
    sig_exit_flag = true;
}

auto stopCSV () -> void{
    for(const auto& kv : g_manger){
        kv.second->stopWriteToCSV();
    }
}

auto stopStreaming(std::string host) -> void{
    if (g_manger.count(host))
        g_manger[host]->stop();
    if (g_asionet.count(host))
        g_asionet[host]->Stop();
    g_terminate[host] = true;
}

auto stopStreaming() -> void{
    for(const auto& kv : g_manger){
        kv.second->stop();
    }
    for(const auto& kv : g_asionet){
        kv.second->Stop();
    }
    for(auto& kv : g_terminate){
        kv.second = true;
    }
}

