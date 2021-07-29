#include "config.h"
#include <mutex>

#ifdef _WIN32
#include <dir.h>
#else
#include <sys/stat.h>
#endif

ClientOpt::Options g_option;
std::atomic<int>   g_connect_counter;
std::atomic<int>   g_get_counter;
std::atomic<int>   g_set_counter;
std::mutex         g_mutex;
std::atomic<bool>  g_exit_flag;


#define UNUSED(x) [&x]{}()

auto sleepMs(int ms) -> void{
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(1000 * ms);
#endif // _WIN32
}

auto createDirConfig(const std::string dir) -> bool
{
#ifdef _WIN32
    mkdir(dir.c_str());
#else
    mkdir(dir.c_str(), 0777);
#endif

    return true;
}

auto createDirTreeConfig(const std::string full_path) -> bool
{
    char ch = '/';
#ifdef _WIN32
    ch = '\\';
#endif

    size_t pos = 0;
    bool ret_val = true;

    while(ret_val == true && pos != std::string::npos)
    {
        pos = full_path.find(ch, pos + 1);
        ret_val = createDirConfig(full_path.substr(0, pos));
    }

    return ret_val;
}

auto startConfig(ClientOpt::Options &option) -> void{
    std::list<std::string> connected_hosts;
    g_exit_flag = false;
    g_option = option;

    ClientNetConfigManager cl("",false);
    cl.addHandler(ClientNetConfigManager::Events::SERVER_CONNECTED, [&cl,&connected_hosts](std::string host){
        const std::lock_guard<std::mutex> lock(g_mutex);
        if (g_option.verbous)
            std::cout << "Connected: " << host << "\n";
        connected_hosts.push_back(host);
        g_connect_counter--;
    });

    cl.addHandler(ClientNetConfigManager::Events::GET_NEW_SETTING,[&cl](std::string host){
        const std::lock_guard<std::mutex> lock(g_mutex);
        if (g_option.verbous)
            std::cerr << "Get settings from: " << host.c_str() << "\n";
        CStreamSettings* s = cl.getLocalSettingsOfHost(host);
        if (g_option.conf_get == ClientOpt::ConfGet::VERBOUS_JSON || g_option.conf_get == ClientOpt::ConfGet::VERBOUS_JSON_DATA){
            auto str = s->getJson();
            if (g_option.conf_get == ClientOpt::ConfGet::VERBOUS_JSON){
                std::cout << "\n===============================\n";
                std::cout << "CONFIGURATION " << host.c_str() << "\n";
            }
            std::cout << str;
            std::cout << "\n===============================\n";
        }
        if (g_option.conf_get == ClientOpt::ConfGet::VERBOUS){
            auto str = s->String();
            std::cout << "\n========================================================\n";
            std::cout << "Host:\t\t\t" << host.c_str() << "\n";
            std::cout << str;
            std::cout <<   "========================================================\n";
        }
        if (g_option.conf_get == ClientOpt::ConfGet::FILE){
            createDirTreeConfig("configs");
            std::string file = "configs/config_" + host + ".json";
            if (g_option.verbous)
                std::cout << "Save configuration from " << host << " to " <<  file << "\n";
            s->writeToFile(file);
        }
        g_get_counter--;
    });

    cl.addHandler(ClientNetConfigManager::Events::SUCCESS_SEND_CONFIG, [&cl](std::string host){
        const std::lock_guard<std::mutex> lock(g_mutex);
        if (g_option.verbous)
            std::cout << "SET: " << host << " OK\n";
        if (g_option.conf_set == ClientOpt::ConfSet::FILE){
            if (g_option.verbous)
                std::cout << "Send configuration save command to: " << host.c_str() << "\n";
            cl.sendSaveToFile(host);
        }else{
            g_set_counter--;
        }
    });

    cl.addHandler(ClientNetConfigManager::Events::FAIL_SEND_CONFIG, [&cl](std::string host){
        const std::lock_guard<std::mutex> lock(g_mutex);
        if (g_option.verbous)
            std::cout << "SET: " << host << " FAIL\n";
        g_set_counter--;
    });


    cl.addHandler(ClientNetConfigManager::Events::SUCCESS_SAVE_CONFIG, [&cl](std::string host){
        const std::lock_guard<std::mutex> lock(g_mutex);
        if (g_option.verbous)
            std::cout << "SAVE TO FILE: " << host << " OK\n";
        g_set_counter--;
    });


    cl.addHandler(ClientNetConfigManager::Events::FAIL_SAVE_CONFIG, [&cl](std::string host){
        const std::lock_guard<std::mutex> lock(g_mutex);
        if (g_option.verbous)
            std::cout << "SAVE TO FILE: " << host << " FAIL\n";
        g_set_counter--;
    });

    cl.addHandlerError([](ClientNetConfigManager::Errors errors,std::string host){
        const std::lock_guard<std::mutex> lock(g_mutex);
        if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
            std::cerr << "Error: " << host.c_str() << "\n";
            g_connect_counter--;
            g_get_counter--;
            g_set_counter--;
        }

        if (errors == ClientNetConfigManager::Errors::CANNT_SET_DATA_TO_CONFIG){
            if (g_option.verbous)
                std::cerr << "Error get settings from: " << host.c_str() << "\n";
            g_get_counter--;
        }

        if (errors == ClientNetConfigManager::Errors::CONNECT_TIMEOUT) {
            if (g_option.verbous)
                std::cerr << "Connect timeout: " << host.c_str() << "\n";
            g_connect_counter--;
        }

        if (errors == ClientNetConfigManager::Errors::ERROR_SEND_CONFIG){
            if (g_option.verbous)
                std::cerr << "Error send configuration: " << host.c_str() << "\n";
            g_set_counter--;
        }

    });


    g_connect_counter = g_option.hosts.size();
    cl.connectToServers(option.hosts,g_option.port != "" ? g_option.port : "8901");
    while (g_connect_counter>0){
        sleepMs(100);
        if (g_exit_flag) return;
    }

    if (g_option.conf_get != ClientOpt::ConfGet::NONE) {
        g_get_counter = connected_hosts.size();
        for(auto &host:connected_hosts) {
            if (g_option.verbous)
                std::cerr << "Send configuration request: " << host.c_str() << "\n";
            if (!cl.requestConfig(host)){
                g_get_counter--;
            }
        }
        while (g_get_counter>0){
            sleepMs(100);
            if (g_exit_flag) return;
        }
    }

    if (g_option.conf_set != ClientOpt::ConfSet::NONE) {
        auto file = g_option.conf_file == "" ? "config.json" : g_option.conf_file;
        if (cl.readFromFile(file)){
            g_set_counter = connected_hosts.size();
            for(auto &host:connected_hosts) {
                if (g_option.verbous)
                    std::cout << "Send configuration to: " << host.c_str() << "\n";
                if (!cl.sendConfig(host)){
                    g_set_counter--;
                }
            }
            while (g_set_counter>0){
                sleepMs(100);
                if (g_exit_flag) return;
            }
        }else{
            if (g_option.verbous)
                std::cerr << "Error read configuration file: " << file.c_str() << "\n";
        }
    }
}

auto configSIGHandler() -> void{
    g_exit_flag = true;
}