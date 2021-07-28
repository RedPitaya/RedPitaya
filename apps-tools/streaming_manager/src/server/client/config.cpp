#include "config.h"
#include <mutex>

ClientOpt::Options g_option;

auto startConfig(ClientOpt::Options &option) -> void{
    std::list<std::string> connected_hosts;
    std::atomic<int> connect_counter;
    g_option = option;

    ClientNetConfigManager cl("",false);
    cl.addHandler(ClientNetConfigManager::Events::SERVER_CONNECTED, [&cl,&connected_hosts,&connect_counter](std::string host){
        if (g_option.conf_verbous)
            std::cout << "Connected: " << host << "\n";
        connected_hosts.push_back(host);
        connect_counter--;
    });

    cl.addHandler(ClientNetConfigManager::Events::GET_NEW_SETTING,[&cl](std::string host){
//        CStreamSettings* s = cl->getLocalSettingsOfHost(host);
//        if (s != nullptr){
//            std::cout << "Save config\n";
//            s->writeToFile("./new_jost.conf");
//        }
    });

    cl.addHandlerError([&connect_counter](ClientNetConfigManager::Errors errors,std::string host){
        if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
            std::cerr << "Error: " << host.c_str() << "\n";
            connect_counter--;
        }
        if (errors == ClientNetConfigManager::Errors::CONNECT_TIMEOUT) {
            if (g_option.conf_verbous)
                std::cerr << "Connect timeout: " << host.c_str() << "\n";
            connect_counter--;
        }

    });


    std::copy(option.hosts.begin(),option.hosts.end(),back_inserter(connected_hosts));
    connect_counter = connected_hosts.size();
    cl.connectToServers(option.hosts,g_option.port != "" ? g_option.port : "8901");
    while (connect_counter){
#ifdef _WIN32
        Sleep(100);
#else
        usleep(100000);
#endif // _WIN32
    }
    if (g_option.conf_get != ClientOpt::ConfGet::NONE) {
        for(auto &host:connected_hosts) {
            cl.requestConfig(host);
        }
    }
    sleep(5);
}
