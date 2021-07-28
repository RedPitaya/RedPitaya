#include "config.h"
#include <mutex>

ClientOpt::Options g_option;
std::atomic<int> g_connect_counter;
std::atomic<int> g_get_counter;

auto startConfig(ClientOpt::Options &option) -> void{
    std::list<std::string> connected_hosts;

    g_option = option;

    ClientNetConfigManager cl("",false);
    cl.addHandler(ClientNetConfigManager::Events::SERVER_CONNECTED, [&cl,&connected_hosts](std::string host){
        if (g_option.conf_verbous)
            std::cout << "Connected: " << host << "\n";
        connected_hosts.push_back(host);
        g_connect_counter--;
    });

    cl.addHandler(ClientNetConfigManager::Events::GET_NEW_SETTING,[&cl](std::string host){
        if (g_option.conf_verbous)
            std::cerr << "Get settings from: " << host.c_str() << "\n";
//        CStreamSettings* s = cl->getLocalSettingsOfHost(host);
//        if (s != nullptr){
//            std::cout << "Save config\n";
//            s->writeToFile("./new_jost.conf");
//        }
        g_get_counter--;
    });

    cl.addHandlerError([](ClientNetConfigManager::Errors errors,std::string host){
        if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
            std::cerr << "Error: " << host.c_str() << "\n";
            g_connect_counter--;
            g_get_counter--;
        }

        if (errors == ClientNetConfigManager::Errors::CANNT_SET_DATA_TO_CONFIG){
            if (g_option.conf_verbous)
                std::cerr << "Error get settings from: " << host.c_str() << "\n";
            g_get_counter--;
        }
        if (errors == ClientNetConfigManager::Errors::CONNECT_TIMEOUT) {
            if (g_option.conf_verbous)
                std::cerr << "Connect timeout: " << host.c_str() << "\n";
            g_connect_counter--;
        }

    });


    g_connect_counter = g_option.hosts.size();
    cl.connectToServers(option.hosts,g_option.port != "" ? g_option.port : "8901");
    while (g_connect_counter>0){
#ifdef _WIN32
        Sleep(100);
#else
        usleep(100000);
#endif // _WIN32
    }
    if (g_option.conf_get != ClientOpt::ConfGet::NONE) {
        g_get_counter = connected_hosts.size();
        for(auto &host:connected_hosts) {
            if (g_option.conf_verbous)
                std::cerr << "Send config request: " << host.c_str() << "\n";
            cl.requestConfig(host);
        }
        while (g_get_counter>0){
#ifdef _WIN32
            Sleep(100);
#else
            usleep(100000);
#endif // _WIN32
        }
    }
}
