#include "remote.h"
#include "config.h"
#include <mutex>

#ifdef _WIN32
#include <dir.h>
#else
#include <sys/stat.h>
#endif

ClientOpt::Options g_roption;
std::atomic<int>   g_rconnect_counter;
std::atomic<int>   g_rstart_counter;
std::atomic<int>   g_rstop_counter;
std::mutex         g_rmutex;
std::atomic<bool>  g_rexit_flag;

auto startRemote(ClientOpt::Options &option) -> void{
    std::list<std::string> connected_hosts;
    std::list<std::string> slaveHosts;
    std::list<std::string> masterHosts;
    g_rexit_flag = false;
    g_roption = option;

    ClientNetConfigManager cl("",false);
    cl.addHandler(ClientNetConfigManager::Events::SERVER_CONNECTED, [&cl,&connected_hosts](std::string host){
        const std::lock_guard<std::mutex> lock(g_rmutex);

        if (g_roption.verbous)
            std::cout << "Connected: " << host << "\n";
        connected_hosts.push_back(host);
        g_rconnect_counter--;
    });

    cl.addHandlerError([&masterHosts,&slaveHosts](ClientNetConfigManager::Errors errors,std::string host){
        const std::lock_guard<std::mutex> lock(g_rmutex);
        if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
            std::cerr << "Error: " << host.c_str() << "\n";
            g_rconnect_counter--;
            g_rstart_counter--;
            g_rstop_counter--;
            masterHosts.remove(host);
            slaveHosts.remove(host);
        }
    });

    cl.addHandler(ClientNetConfigManager::Events::SERVER_STARTED_TCP, [&cl](std::string host){
        const std::lock_guard<std::mutex> lock(g_rmutex);
        if (g_roption.verbous)
            std::cout << "Streaming started: " << host << " TCP mode [OK]\n";
        g_rstart_counter--;
    });

    cl.addHandler(ClientNetConfigManager::Events::SERVER_STARTED_UDP, [&cl](std::string host){
        const std::lock_guard<std::mutex> lock(g_rmutex);
        if (g_roption.verbous)
            std::cout << "Streaming started: " << host << " UDP mode [OK]\n";
        g_rstart_counter--;
    });

    cl.addHandler(ClientNetConfigManager::Events::SERVER_STARTED_SD, [&cl](std::string host){
        const std::lock_guard<std::mutex> lock(g_rmutex);
        if (g_roption.verbous)
            std::cout << "Streaming started: " << host << " Local mode [OK]\n";
        g_rstart_counter--;
    });

    cl.addHandler(ClientNetConfigManager::Events::SERVER_STOPPED, [&cl,&masterHosts,&slaveHosts](std::string host){
        const std::lock_guard<std::mutex> lock(g_rmutex);
        if (g_roption.verbous)
            std::cout << "Streaming stopped: " << host << " [OK]\n";
        masterHosts.remove(host);
        slaveHosts.remove(host);
        g_rstop_counter--;
    });

    cl.addHandler(ClientNetConfigManager::Events::SERVER_STOPPED_SD_FULL, [&cl,&masterHosts,&slaveHosts](std::string host){
        const std::lock_guard<std::mutex> lock(g_rmutex);
        if (g_roption.verbous)
            std::cout << "Streaming stopped: " << host << " SD card is full [OK]\n";
        masterHosts.remove(host);
        slaveHosts.remove(host);
        g_rstop_counter--;
    });

    cl.addHandler(ClientNetConfigManager::Events::SERVER_STOPPED_SD_DONE, [&cl,&masterHosts,&slaveHosts](std::string host){
        const std::lock_guard<std::mutex> lock(g_rmutex);
        if (g_roption.verbous)
            std::cout << "Streaming stopped: " << host << " All samples are recorded on the SD card [OK]\n";
        masterHosts.remove(host);
        slaveHosts.remove(host);
        g_rstop_counter--;
    });

    g_rconnect_counter = g_roption.hosts.size();
    cl.connectToServers(option.hosts,g_roption.port != "" ? g_roption.port : "8901");
    while (g_rconnect_counter>0){
        sleepMs(100);
        if (g_rexit_flag) return;
    }

    for(auto &host:connected_hosts) {
        switch (cl.getModeByHost(host)) {
            case asionet_broadcast::CAsioBroadcastSocket::ABMode::AB_SERVER_MASTER:
                masterHosts.push_back(host);
                break;
            case asionet_broadcast::CAsioBroadcastSocket::ABMode::AB_SERVER_SLAVE:
                slaveHosts.push_back(host);
                break;
            default:
                break;
        }
    }

    if (g_roption.remote_mode == ClientOpt::RemoteMode::START) {
        g_rstart_counter = slaveHosts.size();
        for(auto &host:slaveHosts) {
            if (g_roption.verbous)
                std::cerr << "Send start command to slave board " << host.c_str() << "\n";
            if (!cl.sendStart(host)){
                g_rstart_counter--;
            }
        }
        while (g_rstart_counter>0){
            sleepMs(100);
            if (g_rexit_flag) return;
        }

        g_rstart_counter = masterHosts.size();
        for(auto &host:masterHosts) {
            if (g_roption.verbous)
                std::cerr << "Send start command to master board " << host.c_str() << "\n";
            if (!cl.sendStart(host)){
                g_rstart_counter--;
            }
        }
        while (g_rstart_counter>0){
            sleepMs(100);
            if (g_rexit_flag) return;
        }
    }

    if (g_roption.remote_mode == ClientOpt::RemoteMode::STOP) {
        g_rstop_counter = masterHosts.size();
        for(auto &host:masterHosts) {
            if (g_roption.verbous)
                std::cerr << "Send stop command to master board " << host.c_str() << "\n";
            if (!cl.sendStop(host)){
                g_rstop_counter--;
            }
        }
        while (g_rstop_counter>0){
            sleepMs(100);
            if (g_rexit_flag) return;
        }

        g_rstop_counter = slaveHosts.size();
        for(auto &host:slaveHosts) {
            if (g_roption.verbous)
                std::cerr << "Send stop command to slave board " << host.c_str() << "\n";
            if (!cl.sendStop(host)){
                g_rstop_counter--;
            }
        }
        while (g_rstop_counter>0){
            sleepMs(100);
            if (g_rexit_flag) return;
        }
    }

    if (g_roption.remote_mode == ClientOpt::RemoteMode::START_STOP) {
        g_rstart_counter = slaveHosts.size();
        for(auto &host:slaveHosts) {
            if (g_roption.verbous)
                std::cerr << "Send start command to slave board " << host.c_str() << "\n";
            if (!cl.sendStart(host)){
                g_rstart_counter--;
            }
        }
        while (g_rstart_counter>0){
            sleepMs(100);
            if (g_rexit_flag) return;
        }

        g_rstart_counter = masterHosts.size();
        for(auto &host:masterHosts) {
            if (g_roption.verbous)
                std::cerr << "Send start command to master board " << host.c_str() << "\n";
            if (!cl.sendStart(host)){
                g_rstart_counter--;
            }
        }
        while (g_rstart_counter>0){
            sleepMs(100);
            if (g_rexit_flag) return;
        }

        auto beginTime = std::chrono::time_point_cast<std::chrono::milliseconds >(std::chrono::system_clock::now()).time_since_epoch().count();
        auto curTime = beginTime;
        g_roption.timeout = g_roption.timeout == 0 ? 1000 : g_roption.timeout;
        while ((curTime - beginTime < g_roption.timeout) && (masterHosts.size() > 0 || slaveHosts.size() > 0)){
            sleepMs(1);
            if (g_rexit_flag) break;
            curTime = std::chrono::time_point_cast<std::chrono::milliseconds >(std::chrono::system_clock::now()).time_since_epoch().count();
        }

        g_rstop_counter = masterHosts.size();
        for(auto &host:masterHosts) {
            if (g_roption.verbous)
                std::cerr << "Send stop command to master board " << host.c_str() << "\n";
            if (!cl.sendStop(host)){
                g_rstop_counter--;
            }
        }
        while (g_rstop_counter>0){
            sleepMs(100);
            if (g_rexit_flag) return;
        }

        g_rstop_counter = slaveHosts.size();
        for(auto &host:slaveHosts) {
            if (g_roption.verbous)
                std::cerr << "Send stop command to slave board " << host.c_str() << "\n";
            if (!cl.sendStop(host)){
                g_rstop_counter--;
            }
        }
        while (g_rstop_counter>0){
            sleepMs(100);
            if (g_rexit_flag) return;
        }
    }
}

auto remoteSIGHandler() -> void{
    g_rexit_flag = true;
}