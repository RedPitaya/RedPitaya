#include "config.h"
#include <mutex>

#ifdef _WIN32
#include <dir.h>
#else
#include <sys/stat.h>
#endif

std::mutex         g_mutex;
std::atomic<bool>  g_exit_flag;

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

auto connectConfigServer(std::shared_ptr<ClientNetConfigManager> cl, ClientOpt::Options &option) -> bool{
    std::list<std::string> connected_hosts;
    std::atomic<int>       connect_counter;
    g_exit_flag = false;

    cl->addHandler(ClientNetConfigManager::Events::SERVER_CONNECTED, [&](std::string host){
        const std::lock_guard<std::mutex> lock(g_mutex);
        if (option.verbous)
            std::cout << getTS(": ") << "Connected: " << host << "\n";
        connected_hosts.push_back(host);
        connect_counter--;
    });

    cl->addHandlerError([&](ClientNetConfigManager::Errors errors,std::string host){
        const std::lock_guard<std::mutex> lock(g_mutex);
        if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
            std::cerr << getTS(": ") << "Error: " << host.c_str() << "\n";
            connect_counter--;
        }

        if (errors == ClientNetConfigManager::Errors::CONNECT_TIMEOUT) {
            if (option.verbous)
                std::cerr << getTS(": ") << "Connect timeout: " << host.c_str() << "\n";
            connect_counter--;
        }
    });

    connect_counter = option.hosts.size();
    cl->connectToServers(option.hosts,option.ports.config_port != "" ? option.ports.config_port : ClientOpt::Ports().config_port);
    while (connect_counter>0){
        sleepMs(100);
        if (g_exit_flag) {
            cl->removeHadlers();
            return false;
        }
    }
    cl->removeHadlers();
    return true;
}

auto getConfig(std::shared_ptr<ClientNetConfigManager> cl,ClientOpt::Options &option) ->bool{
    std::atomic<int>   get_counter;
    cl->addHandler(ClientNetConfigManager::Events::GET_NEW_SETTING,[&](std::string host){
        const std::lock_guard<std::mutex> lock(g_mutex);
        if (option.verbous)
            std::cerr << getTS(": ") << "Get settings from: " << host.c_str() << "\n";
        CStreamSettings* s = cl->getLocalSettingsOfHost(host);
        if (option.conf_get == ClientOpt::ConfGet::VERBOUS_JSON || option.conf_get == ClientOpt::ConfGet::VERBOUS_JSON_DATA){
            auto str = s->getJson();
            if (option.conf_get == ClientOpt::ConfGet::VERBOUS_JSON){
                std::cout << "\n===============================\n";
                std::cout << "CONFIGURATION " << host.c_str() << "\n";
            }
            std::cout << str;
            if (option.conf_get == ClientOpt::ConfGet::VERBOUS_JSON){
                std::cout << "\n===============================\n";
            }else{
                std::cout << "\n";
            }
        }
        if (option.conf_get == ClientOpt::ConfGet::VERBOUS){
            auto str = s->String();
            std::cout << "\n========================================================\n";
            std::cout << "Host:\t\t\t" << host.c_str() << "\n";
            std::cout << str;
            std::cout <<   "========================================================\n";
        }
        if (option.conf_get == ClientOpt::ConfGet::FILE){
            createDirTreeConfig("configs");
            std::string file = "configs/config_" + host + ".json";
            if (option.verbous)
                std::cout << getTS(": ") << "Save configuration from " << host << " to " <<  file << "\n";
            s->writeToFile(file);
        }
        get_counter--;
    });

    cl->addHandlerError([&](ClientNetConfigManager::Errors errors,std::string host){
        const std::lock_guard<std::mutex> lock(g_mutex);
        if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
            std::cerr << getTS(": ") << "Error: " << host.c_str() << "\n";
            get_counter--;
        }

        if (errors == ClientNetConfigManager::Errors::CANNT_SET_DATA_TO_CONFIG){
            if (option.verbous)
                std::cerr << getTS(": ") << "Error get settings from: " << host.c_str() << "\n";
            get_counter--;
        }
    });


    auto connected_hosts = cl->getHosts();

    get_counter = connected_hosts.size();
    for(auto &host:connected_hosts) {
        if (option.verbous)
            std::cerr << getTS(": ") << "Send configuration request: " << host.c_str() << "\n";
        if (!cl->requestConfig(host)){
            get_counter--;
        }
    }
    while (get_counter>0){
        sleepMs(100);
        if (g_exit_flag) {
            cl->removeHadlers();
            return false;
        }
    }
    cl->removeHadlers();
    return true;
}

auto setConfig(std::shared_ptr<ClientNetConfigManager> cl,ClientOpt::Options &option) ->bool{
    std::atomic<int> set_counter;
    cl->addHandler(ClientNetConfigManager::Events::SUCCESS_SEND_CONFIG, [&](std::string host){
        const std::lock_guard<std::mutex> lock(g_mutex);
        if (option.verbous)
            std::cout << getTS(": ") <<"SET: " << host << " OK\n";
        if (option.conf_set == ClientOpt::ConfSet::FILE){
            if (option.verbous)
                std::cout << getTS(": ") << "Send configuration save command to: " << host.c_str() << "\n";
            cl->sendSaveToFile(host);
        }else{
            set_counter--;
        }
    });

    cl->addHandler(ClientNetConfigManager::Events::FAIL_SEND_CONFIG, [&](std::string host){
        const std::lock_guard<std::mutex> lock(g_mutex);
        if (option.verbous)
            std::cout << getTS(": ") << "SET: " << host << " FAIL\n";
        set_counter--;
    });


    cl->addHandler(ClientNetConfigManager::Events::SUCCESS_SAVE_CONFIG, [&](std::string host){
        const std::lock_guard<std::mutex> lock(g_mutex);
        if (option.verbous)
            std::cout << getTS(": ") << "SAVE TO FILE: " << host << " OK\n";
        set_counter--;
    });


    cl->addHandler(ClientNetConfigManager::Events::FAIL_SAVE_CONFIG, [&](std::string host){
        const std::lock_guard<std::mutex> lock(g_mutex);
        if (option.verbous)
            std::cout << getTS(": ") << "SAVE TO FILE: " << host << " FAIL\n";
        set_counter--;
    });


    cl->addHandlerError([&](ClientNetConfigManager::Errors errors,std::string host){
        const std::lock_guard<std::mutex> lock(g_mutex);
        if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
            std::cerr << getTS(": ") << "Error: " << host.c_str() << "\n";
            set_counter--;
        }

        if (errors == ClientNetConfigManager::Errors::ERROR_SEND_CONFIG){
            if (option.verbous)
                std::cerr << getTS(": ") << "Error send configuration: " << host.c_str() << "\n";
            set_counter--;
        }
    });


    auto file = option.conf_file == "" ? "config.json" : option.conf_file;
    if (cl->readFromFile(file)){
        auto connected_hosts = cl->getHosts();
        set_counter = connected_hosts.size();
        for(auto &host:connected_hosts) {
            if (option.verbous)
                std::cout << getTS(": ") << "Send configuration to: " << host.c_str() << "\n";
            if (!cl->sendConfig(host)){
                set_counter--;
            }
        }
        while (set_counter>0){
            sleepMs(100);
            if (g_exit_flag) {
                cl->removeHadlers();
                return false;
            }
        }
    }else{
        if (option.verbous){
            std::cerr << getTS(": ") << "Error read configuration file: " << file.c_str() << "\n";
            cl->removeHadlers();
            return false;
        }
    }

    cl->removeHadlers();
    return true;
}

auto sendCopyToTestConfig(std::shared_ptr<ClientNetConfigManager> cl,ClientOpt::Options &option) ->bool{
    std::atomic<int> set_counter;
    cl->addHandler(ClientNetConfigManager::Events::COPY_SETTINGS_TO_TEST_SETTINGS_DONE, [&](std::string host){
        const std::lock_guard<std::mutex> lock(g_mutex);
        if (option.verbous)
            std::cout << getTS(": ") <<"copy config to test: " << host << " [OK]\n";
        set_counter--;
    });

    cl->addHandlerError([&](ClientNetConfigManager::Errors errors,std::string host){
        const std::lock_guard<std::mutex> lock(g_mutex);
        if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
            std::cerr << getTS(": ") << "Error: " << host.c_str() << "\n";
            set_counter--;
        }
    });


    auto connected_hosts = cl->getHosts();
    set_counter = connected_hosts.size();
    for(auto &host:connected_hosts) {
        if (option.verbous)
            std::cout << getTS(": ") << "Send set test config to: " << host.c_str() << "\n";
        if (!cl->sendCopyConfigToTest(host)){
            set_counter--;
        }
    }
    while (set_counter>0){
        sleepMs(100);
        if (g_exit_flag) {
            cl->removeHadlers();
            return false;
        }
    } 

    cl->removeHadlers();
    return true;
}

auto startConfig(std::shared_ptr<ClientNetConfigManager> cl,ClientOpt::Options &option) -> bool {
    g_exit_flag = false;

    if (option.conf_get != ClientOpt::ConfGet::NONE) {
        return getConfig(cl,option);
    }


    if (option.conf_set != ClientOpt::ConfSet::NONE) {
        return setConfig(cl,option);
    }

    return false;
}

auto configSIGHandler() -> void{
    g_exit_flag = true;
}
