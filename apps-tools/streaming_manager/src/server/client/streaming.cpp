#include "streaming.h"
#include "AsioNet.h"
#include "StreamingManager.h"
#include "thread_cout.h"
#include "config.h"
#include "remote.h"
#include <chrono>

std::map<std::string,CStreamingManager::Ptr> g_manger;
std::map<std::string,asionet::CAsioNet::Ptr> g_asionet;
std::mutex         g_smutex;
std::mutex         g_s_csv_mutex;
ClientOpt::Options g_soption;
std::string        g_filenameDate;
std::atomic<bool>  sig_exit_flag(false);

std::map<std::string,long long int>   g_timeBegin;
std::map<std::string,bool>            g_terminate;
std::map<std::string,uint64_t>        g_BytesCount;
std::map<std::string,uint64_t>        g_lostRate;
std::map<std::string,uint64_t>        g_packCounter_ch1;
std::map<std::string,uint64_t>        g_packCounter_ch2;

std::vector<std::thread>              clients;

auto stopCSV () -> void;
auto stopStreaming() -> void;
auto stopStreaming(std::string host) -> void;


void reciveData(std::error_code error,uint8_t *buff,size_t _size,std::string host){
    //std::cout << "Get data: " <<  _size << "\n";
    g_BytesCount[host] += _size;
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
    g_packCounter_ch1[host] += size_ch1 / (resolution == 16 ? 2 : 1);
    g_packCounter_ch2[host] += size_ch2 / (resolution == 16 ? 2 : 1);
    g_lostRate[host] += lostRate;
    // std::cout << id << " ; " <<  _size  <<  " ; " << resolution << " ; " << size_ch1 << " ; " << size_ch2 << "\n";

    g_manger[host]->passBuffers(lostRate, oscRate , adc_mode , adc_bits, ch1 , size_ch1 ,  ch2 , size_ch2 , resolution, id);


    delete [] ch1;
    delete [] ch2;

    std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
    auto curTime = std::chrono::time_point_cast<std::chrono::milliseconds >(timeNow);
    auto value = curTime.time_since_epoch();
    //     std::cout << value.count() << "\n";
    //     std::cout <<  g_timeBegin << "\n";
    if ((value.count() - g_timeBegin[host]) >= 5000) {
        const std::lock_guard<std::mutex> lock(g_smutex);
        uint64_t bw = g_BytesCount[host];
        std::string pref = " ";
        if (g_BytesCount[host]  > (1024 * 5)) {
            bw = g_BytesCount[host]  / (1024 * 5);
            pref = " ki";
        }

        if (g_BytesCount[host]   > (1024 * 1024 * 5)) {
            bw = g_BytesCount[host]   / (1024 * 1024 * 5);
            pref = " Mi";
        }
        if (g_soption.verbous)
            std::cout << time_point_to_string(timeNow) << "\tHOST IP:" << host << ": Bandwidth:\t" << bw <<  pref <<"B/s\tData count ch1:\t" << g_packCounter_ch1[host]
            << "\tch2:\t" << g_packCounter_ch2[host]  <<  "\tLost:\t" << g_lostRate[host]  << "\n";
        g_BytesCount[host]  = 0;
        g_lostRate[host]  = 0;
        g_timeBegin[host] = value.count();
    }
}

auto runClient(std::string  host,StateRunnedHosts state) -> void{
    std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
    g_timeBegin[host] = std::chrono::time_point_cast<std::chrono::milliseconds >(timeNow).time_since_epoch().count();

    g_packCounter_ch1[host] = 0;
    g_packCounter_ch2[host] = 0;
    g_lostRate[host] = 0;
    g_BytesCount[host] = 0;
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

    g_manger[host] = CStreamingManager::Create(file_type , g_soption.save_dir.c_str(), g_soption.samples , convert_v);
    g_manger[host]->run(host + "_" + g_filenameDate);

    g_asionet[host] = asionet::CAsioNet::Create(asionet::Mode::CLIENT, protocol ,host , g_soption.port != "" ? g_soption.controlPort : "8900");
    g_asionet[host]->addCallClient_Connect([](std::string host) {
        const std::lock_guard<std::mutex> lock(g_smutex);
        if (g_soption.verbous)
            std::cout << getTS(": ") << "Try connect " << host << '\n';
    });
    g_asionet[host]->addCallClient_Error([host](std::error_code error)
    {
        const std::lock_guard<std::mutex> lock(g_smutex);
        if (g_soption.verbous)
            std::cout << getTS(": ") <<"Disconnect;" << '\n';
        stopStreaming(host);
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
    }
    stopStreaming(host);
    if (file_type == Stream_FileType::CSV_TYPE) {
        const std::lock_guard<std::mutex> lock(g_s_csv_mutex);
        g_manger[host]->convertToCSV(host);
    }
}

auto startStreaming(ClientOpt::Options &option) -> void{
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

    ClientOpt::Options remote_opt = g_soption;
    remote_opt.mode = ClientOpt::Mode::REMOTE;
    remote_opt.remote_mode = ClientOpt::RemoteMode::START;
    remote_opt.port = g_soption.controlPort;
    remote_opt.verbous = g_soption.verbous;
    std::map<string,StateRunnedHosts> runned_hosts;
    if (startRemote(remote_opt,&runned_hosts)){

        for(auto &kv:runned_hosts){
            if (kv.second == StateRunnedHosts::TCP || kv.second == StateRunnedHosts::UDP)
                clients.push_back(std::thread(runClient, kv.first,kv.second));
        }

        for (std::thread &t: clients) {
            if (t.joinable()) {
                t.join();
            }
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

