#include "test_helper.h"
#include <chrono>
#include <math.h>

ClientOpt::Options g_hoption;
std::atomic<bool>  g_helper_exit_flag;
std::mutex         g_helper_mutex;

std::mutex                            g_statMutex;
long long int                         g_timeBegin;
std::map<std::string,long long int>   g_timeHostBegin;
std::map<std::string,uint64_t>        g_BytesCount;
std::map<std::string,uint64_t>        g_BytesCountTotal;
std::map<std::string,uint64_t>        g_lostRate;
std::map<std::string,uint64_t>        g_lostRateTotal;

std::map<std::string,uint64_t>        g_packCounter_ch1;
std::map<std::string,uint64_t>        g_packCounter_ch2;

auto setOptions(ClientOpt::Options option) -> void{
    g_hoption = option;
    g_helper_exit_flag = false;
}


auto resetStreamingCounter() -> void{
    const std::lock_guard<std::mutex> lock(g_statMutex);
    g_timeBegin = 0;
    g_timeHostBegin.clear();
    g_BytesCount.clear();
    g_lostRate.clear();
    g_lostRateTotal.clear();
    g_BytesCountTotal.clear();
}

auto addStatisticSteaming(std::string &host,uint64_t bytesCount,uint64_t samp_ch1,uint64_t samp_ch2,uint64_t lost,uint64_t networkLost,uint64_t fileLost) -> void{
    const std::lock_guard<std::mutex> lock(g_statMutex);

    if (g_timeHostBegin.count(host) == 0){
        auto curTime = std::chrono::time_point_cast<std::chrono::milliseconds >(std::chrono::system_clock::now());
        auto value = curTime.time_since_epoch();
        g_timeHostBegin[host] = value.count();
    }

    g_BytesCount[host] += bytesCount;
    g_BytesCountTotal[host] += bytesCount;
}

auto createStr(std::string str,int len) -> std::string {
    std::string s(len, ' ');
    for(int i = 0 ; i < str.length() && i < len; i++){
        s[i] = str[i];
    }
    return s;
}

auto convertBtoS(u_int64_t value) -> std::string {
    double d = value;
    std::string s = "";
    if (value >= 1024 * 1024) {
        d = round(((double)value * 1000.0) / (1024 * 1024)) / 1000;
        s =  to_string_with_precision(d,3) + " Mb";
    } else if (value >= 1024){
        d = round(((double)value * 1000.0) / (1024)) / 1000;
        s =  to_string_with_precision(d,3) + " kb";
    }else  {
        s =  std::to_string(value) + " b";
    }
    return s;
}

auto convertBtoSpeed(u_int64_t value,uint64_t time) -> std::string {
    double d = value;
    double t = time;
    t = t / 1000;
    d = d / t;
    std::string s = "";
    if (value >= 1024 * 1024) {
        d = round(((double)d * 1000.0) / (1024 * 1024)) / 1000;
        s =  to_string_with_precision(d,3) + " MB/s";
    } else if (value >= 1024){
        d = round(((double)d * 1000.0) / (1024)) / 1000;
        s =  to_string_with_precision(d,3) + " kB/s";
    }else  {
        s =  std::to_string(d) + " B/s";
    }
    return s;
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
            std::cout << "\n" << getTS() << "\n";
            std::cout << "===========================================================================================\n";
            std::cout << "Host              | Bytes all         | Bandwidth         |                                \n";

            for(auto const& host: keys){
                auto bw = convertBtoSpeed(g_BytesCount[host],value.count() - g_timeHostBegin[host]);
                std::cout << createStr(host,18) << "|";
                std::cout << " " << createStr(convertBtoS(g_BytesCountTotal[host]),18) << "|";
                std::cout << " " << createStr(bw,18) << "|";
                std::cout << "\n";
                g_BytesCount[host] = 0;
                g_timeHostBegin[host] = value.count();
            }
            std::cout << "===========================================================================================\n";
        }        

        g_timeBegin = value.count();
    }
}
