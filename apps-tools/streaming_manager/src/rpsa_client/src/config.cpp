#include <mutex>

#include "data_lib/thread_cout.h"
#include "config.h"

#ifdef _WIN32
#include <dir.h>
#else
#include <sys/stat.h>
#endif

std::mutex         g_mutex;
std::atomic<bool>  g_exit_flag;

auto sleepMs(int ms) -> void{
#ifdef _WIN32
    usleep(ms * 1000);
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

    cl->serverConnectedNofiy.connect([&](std::string host){
        const std::lock_guard<std::mutex> lock(g_mutex);
        if (option.verbous)
            aprintf(stdout,"%s Connected: %s\n",getTS(": ").c_str(),host.c_str());
        connected_hosts.push_back(host);
        connect_counter--;
    });

    cl->errorNofiy.connect([&](ClientNetConfigManager::Errors errors,std::string host,error_code err){
        const std::lock_guard<std::mutex> lock(g_mutex);
        if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
            aprintf(stderr,"%s Error: %s %s\n",getTS(": ").c_str(),host.c_str(),err.message().c_str());
            connect_counter--;
        }

        if (errors == ClientNetConfigManager::Errors::CONNECT_TIMEOUT) {
            if (option.verbous)
                aprintf(stderr,"%s Connect timeout: %s\n",getTS(": ").c_str(),host.c_str());
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
    cl->getNewSettingsNofiy.connect([&](std::string host){
        const std::lock_guard<std::mutex> lock(g_mutex);
        if (option.verbous)
            aprintf(stdout,"%s Get settings from: %s\n",getTS(": ").c_str(),host.c_str());
        CStreamSettings* s = cl->getLocalSettingsOfHost(host);
        if (option.conf_get == ClientOpt::ConfGet::VERBOUS_JSON || option.conf_get == ClientOpt::ConfGet::VERBOUS_JSON_DATA){
            auto str = s->getJson();
            if (option.conf_get == ClientOpt::ConfGet::VERBOUS_JSON){
                aprintf(stdout,"\n===============================\n");
                aprintf(stdout,"CONFIGURATION %s \n",host.c_str());
            }
            std::cout << str;
            if (option.conf_get == ClientOpt::ConfGet::VERBOUS_JSON){
                aprintf(stdout,"\n===============================\n");
            }else{
                aprintf(stdout,"\n");
            }
        }
        if (option.conf_get == ClientOpt::ConfGet::VERBOUS){
            auto str = s->String();
            aprintf(stdout,"\n========================================================\n");
            aprintf(stdout,"Host:\t\t\t%s\n",host.c_str());
            aprintf(stdout,"%s",str.c_str());
            aprintf(stdout,"\n========================================================\n");
        }
        if (option.conf_get == ClientOpt::ConfGet::FILE){
            createDirTreeConfig("configs");
            std::string file = "configs/config_" + host + ".json";
            if (option.verbous)
                aprintf(stdout,"%s Save configuration from %s to %s\n",getTS(": ").c_str(), host.c_str(),file.c_str());
            s->writeToFile(file);
        }
        get_counter--;
    });

    cl->errorNofiy.connect([&](ClientNetConfigManager::Errors errors,std::string host,error_code err){
        const std::lock_guard<std::mutex> lock(g_mutex);
        if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
            aprintf(stderr,"%s Error: %s %s\n",getTS(": ").c_str(),host.c_str(),err.message().c_str());
            get_counter--;
        }

        if (errors == ClientNetConfigManager::Errors::CANNT_SET_DATA_TO_CONFIG){
            if (option.verbous)
                aprintf(stderr,"%s Error get settings from: %s\n",getTS(": ").c_str(),host.c_str());
            get_counter--;
        }
    });


    auto connected_hosts = cl->getHosts();

    get_counter = connected_hosts.size();
    for(auto &host:connected_hosts) {
        if (option.verbous)
            aprintf(stdout,"%s Send configuration request: %s\n",getTS(": ").c_str(),host.c_str());
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
    cl->successSendConfigNofiy.connect([&](std::string host){
        const std::lock_guard<std::mutex> lock(g_mutex);
        if (option.verbous)
            aprintf(stdout,"%s SET: %s [OK]\n",getTS(": ").c_str(),host.c_str());
        if (option.conf_set == ClientOpt::ConfSet::FILE){
            if (option.verbous)
                aprintf(stdout,"%s Send configuration save command to: %s\n",getTS(": ").c_str(),host.c_str());
            cl->sendSaveToFile(host);
        }else{
            set_counter--;
        }
    });

    cl->failSendConfigNofiy.connect([&](std::string host){
        const std::lock_guard<std::mutex> lock(g_mutex);
        if (option.verbous)
            aprintf(stdout,"%s SET: %s [FAIL]\n",getTS(": ").c_str(),host.c_str());
        set_counter--;
    });


    cl->successSaveConfigNofiy.connect([&](std::string host){
        const std::lock_guard<std::mutex> lock(g_mutex);
        if (option.verbous)
            aprintf(stdout,"%s SAVE TO FILE: %s [OK]\n",getTS(": ").c_str(),host.c_str());
        set_counter--;
    });


    cl->failSaveConfigNofiy.connect([&](std::string host){
        const std::lock_guard<std::mutex> lock(g_mutex);
        if (option.verbous)
            aprintf(stdout,"%s SAVE TO FILE: %s [FAIL]\n",getTS(": ").c_str(),host.c_str());
        set_counter--;
    });


    cl->errorNofiy.connect([&](ClientNetConfigManager::Errors errors,std::string host,error_code err){
        const std::lock_guard<std::mutex> lock(g_mutex);
        if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
            aprintf(stderr,"%s Error: %s %s\n",getTS(": ").c_str(),host.c_str(),err.message().c_str());
            set_counter--;
        }

        if (errors == ClientNetConfigManager::Errors::ERROR_SEND_CONFIG){
            if (option.verbous)
                aprintf(stderr,"%s Error send configuration: %s %s\n",getTS(": ").c_str(),host.c_str(),err.message().c_str());
            set_counter--;
        }
    });


    auto file = option.conf_file == "" ? "config.json" : option.conf_file;
    if (cl->readFromFile(file)){
        auto connected_hosts = cl->getHosts();
        set_counter = connected_hosts.size();
        for(auto &host:connected_hosts) {
            if (option.verbous)
                aprintf(stdout,"%s Send configuration to: %s\n",getTS(": ").c_str(),host.c_str());
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
            aprintf(stderr,"%s Error read configuration file: %s\n",getTS(": ").c_str(),file.c_str());
            cl->removeHadlers();
            return false;
        }
    }

    cl->removeHadlers();
    return true;
}

auto sendCopyToTestConfig(std::shared_ptr<ClientNetConfigManager> cl,ClientOpt::Options &option) ->bool{
    std::atomic<int> set_counter;
    cl->serverLoopbackCopySettingsDoneNofiy.connect([&](std::string host){
        const std::lock_guard<std::mutex> lock(g_mutex);
        if (option.verbous)
            aprintf(stdout,"%s Copy config to test: %s [OK]\n",getTS(": ").c_str(),host.c_str());
        set_counter--;
    });

    cl->errorNofiy.connect([&](ClientNetConfigManager::Errors errors,std::string host,error_code err){
        const std::lock_guard<std::mutex> lock(g_mutex);
        if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
            aprintf(stderr,"%s Error: %s %s\n",getTS(": ").c_str(),host.c_str(),err.message().c_str());
            set_counter--;
        }
    });


    auto connected_hosts = cl->getHosts();
    set_counter = connected_hosts.size();
    for(auto &host:connected_hosts) {
        if (option.verbous)
            aprintf(stdout,"%s Send set test config to: %s\n",getTS(": ").c_str(),host.c_str());
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

auto getCurrentStreamingMode(std::shared_ptr<ClientNetConfigManager> cl,ClientOpt::Options &option,bool test_mode,std::map<std::string,StateRunnedHosts> *runned_hosts) ->bool{
    std::atomic<int> set_counter;
    cl->serverLoopbackCopySettingsDoneNofiy.connect([&](std::string host){
        const std::lock_guard<std::mutex> lock(g_mutex);
        if (option.verbous)
            aprintf(stdout,"%s Copy config to test: %s [OK]\n",getTS(": ").c_str(),host.c_str());
        set_counter--;
    });

    cl->errorNofiy.connect([&](ClientNetConfigManager::Errors errors,std::string host,error_code err){
        const std::lock_guard<std::mutex> lock(g_mutex);
        if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
            aprintf(stderr,"%s Error: %s %s\n",getTS(": ").c_str(),host.c_str(),err.message().c_str());
            set_counter--;
        }
    });


    auto connected_hosts = cl->getHosts();
    set_counter = connected_hosts.size();
    for(auto &host:connected_hosts) {
        if (option.verbous)
            aprintf(stdout,"%s Get streaming mode: %s\n",getTS(": ").c_str(),host.c_str());
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

