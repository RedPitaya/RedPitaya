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
std::mutex         g_rmutex;
std::atomic<bool>  g_rexit_flag;

auto startRemote(ClientOpt::Options &option) -> void{
    std::list<std::string> connected_hosts;
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

    g_rconnect_counter = g_roption.hosts.size();
    cl.connectToServers(option.hosts,g_roption.port != "" ? g_roption.port : "8901");
    while (g_rconnect_counter>0){
        sleepMs(100);
        if (g_rexit_flag) return;
    }


}

auto remoteSIGHandler() -> void{
    g_rexit_flag = true;
}