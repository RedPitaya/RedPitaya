#include "test_helper.h"
#include <chrono>

ClientOpt::Options g_hoption;
std::atomic<bool>  g_helper_exit_flag;
std::mutex         g_helper_mutex;

std::mutex                            g_statMutex;
long long int                         g_timeBegin;
std::map<std::string,long long int>   g_timeHostBegin;
std::map<std::string,uint64_t>        g_BytesCount;
std::map<std::string,uint64_t>        g_lostRate;
std::map<std::string,uint64_t>        g_packCounter_ch1;
std::map<std::string,uint64_t>        g_packCounter_ch2;

auto setOptions(ClientOpt::Options option) -> void{
    g_hoption = option;
    g_helper_exit_flag = false;
}


auto getHostConfig(std::string host) -> bool{

    std::list<std::string> connected_hosts;
    ClientNetConfigManager cl("",false);
    std::atomic<int>   connect_counter;
    std::atomic<int>   get_counter;

    cl.addHandler(ClientNetConfigManager::Events::SERVER_CONNECTED, [&](std::string host){
        const std::lock_guard<std::mutex> lock(g_helper_mutex);
        connected_hosts.push_back(host);
        connect_counter--;
    });

    cl.addHandler(ClientNetConfigManager::Events::GET_NEW_SETTING,[&](std::string host){
        const std::lock_guard<std::mutex> lock(g_helper_mutex);

        CStreamSettings* s = cl.getLocalSettingsOfHost(host);
        auto str = s->StringStreaming();
        std::cout << "========================================================\n";
        std::cout << "Host:\t\t\t" << host.c_str() << "\n";
        std::cout << str;
        std::cout <<   "========================================================\n";

        get_counter--;
    });


    cl.addHandlerError([&](ClientNetConfigManager::Errors errors,std::string host){
        const std::lock_guard<std::mutex> lock(g_helper_mutex);
        if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
            std::cerr << getTS(": ") << "Error: " << host.c_str() << "\n";
            connect_counter--;
            get_counter--;
        }

        if (errors == ClientNetConfigManager::Errors::CONNECT_TIMEOUT) {
            std::cerr << getTS(": ") << "Connect timeout: " << host.c_str() << "\n";
            connect_counter--;
        }
    });

    std::vector<std::string> hosts;
    hosts.push_back(host);

    connect_counter = hosts.size();
    cl.connectToServers(hosts,g_hoption.ports.config_port != "" ? g_hoption.ports.config_port : ClientOpt::Ports().config_port);

    while (connect_counter>0){
        sleepMs(100);
        if (g_helper_exit_flag) return false;
    }

    
    get_counter = connected_hosts.size();
    for(auto &host:connected_hosts) {
        if (!cl.requestConfig(host)){
            get_counter--;
        }
    }

    while (get_counter>0){
        sleepMs(100);
        if (g_helper_exit_flag) return false;
    }
    return true;
}

auto helperSIGHandler() -> void{
    g_helper_exit_flag = true;
}


auto resetStreamingCounter() -> void{
    const std::lock_guard<std::mutex> lock(g_statMutex);
    g_timeBegin = 0;
    g_timeHostBegin.clear();
    g_BytesCount.clear();
}

auto addStatisticSteaming(std::string &host,uint64_t bytesCount) -> void{
    const std::lock_guard<std::mutex> lock(g_statMutex);

    if (g_timeHostBegin.count(host) == 0){
        auto curTime = std::chrono::time_point_cast<std::chrono::milliseconds >(std::chrono::system_clock::now());
        auto value = curTime.time_since_epoch();
        g_timeHostBegin[host] = value.count();
    }

    g_BytesCount[host] += bytesCount;
}

auto printStatisitc(bool force) -> void{
    const std::lock_guard<std::mutex> lock(g_statMutex);

    auto curTime = std::chrono::time_point_cast<std::chrono::milliseconds >(std::chrono::system_clock::now());
    auto value = curTime.time_since_epoch();
    if ((value.count() - g_timeBegin) >= 5000 || force) {

        std::vector<std::string> keys;
        std::transform(
            g_timeHostBegin.begin(),
            g_timeHostBegin.end(),
            std::back_inserter(keys),
            [](const std::map<std::string,long long int>::value_type &pair){return pair.first;});


        // uint64_t bw = g_BytesCount[host];
        // std::string pref = " ";
        // if (g_BytesCount[host]  > (1024 * 5)) {
        //     bw = g_BytesCount[host]  / (1024 * 5);
        //     pref = " ki";
        // }

        // if (g_BytesCount[host]   > (1024 * 1024 * 5)) {
        //     bw = g_BytesCount[host]   / (1024 * 1024 * 5);
        //     pref = " Mi";
        // }
        // if (g_soption.verbous)
        //     std::cout << time_point_to_string(timeNow) << "\tHOST IP:" << host << ": Bandwidth:\t" << bw <<  pref <<"B/s\tData count ch1:\t" << g_packCounter_ch1[host]
        //     << "\tch2:\t" << g_packCounter_ch2[host]  <<  "\tLost:\t" << g_lostRate[host]  << "\n";
        // g_BytesCount[host]  = 0;
        // g_lostRate[host]  = 0;
        if (keys.size() > 0){
            std::cout << "===========================================================================================\n";
            std::cout << "Host              |                                                                        \n";

            for(auto const& host: keys){
                std::cout << host << "\n";
                g_timeHostBegin[host] = value.count();
            }
            std::cout << "===========================================================================================\n";
        }        

        g_timeBegin = value.count();
    }
}