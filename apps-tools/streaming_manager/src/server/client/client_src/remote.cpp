#include "remote.h"
#include "config.h"
#include <mutex>

#ifdef _WIN32
#include <dir.h>
#else
#include <sys/stat.h>
#endif

ClientOpt::Options g_roption;
//std::atomic<int>   g_rstart_counter;
//std::atomic<int>   g_rstop_counter;
std::mutex         g_rmutex;
std::atomic<bool>  g_rexit_flag;

auto startStreaming(std::shared_ptr<ClientNetConfigManager> cl,std::list<std::string> &masterHosts,std::list<std::string> &slaveHosts,bool test_mode,std::map<std::string,StateRunnedHosts> *runned_hosts) -> bool;
auto startDACStreaming(std::shared_ptr<ClientNetConfigManager> cl,std::list<std::string> &masterHosts,std::list<std::string> &slaveHosts,bool test_mode,std::map<std::string,StateRunnedHosts> *runned_hosts) -> bool;
auto stopStreaming(std::shared_ptr<ClientNetConfigManager> cl,std::list<std::string> &masterHosts,std::list<std::string> &slaveHosts,std::map<std::string,StateRunnedHosts> *runned_hosts) -> bool;
auto stopDACStreaming(std::shared_ptr<ClientNetConfigManager> cl,std::list<std::string> &masterHosts,std::list<std::string> &slaveHosts,std::map<std::string,StateRunnedHosts> *runned_hosts) -> bool;
auto startStopStreaming(std::shared_ptr<ClientNetConfigManager> cl,std::list<std::string> &masterHosts,std::list<std::string> &slaveHosts,bool test_mode,std::map<std::string,StateRunnedHosts> *runned_hosts) -> bool;
auto startStopDACStreaming(std::shared_ptr<ClientNetConfigManager> cl,std::list<std::string> &masterHosts,std::list<std::string> &slaveHosts,bool test_mode,std::map<std::string,StateRunnedHosts> *runned_hosts) -> bool;
auto startADC(std::shared_ptr<ClientNetConfigManager> cl,std::list<std::string> &masterHosts,std::list<std::string> &slaveHosts,bool test_mode,std::map<std::string,StateRunnedHosts> *runned_hosts) -> bool;

auto startRemote(std::shared_ptr<ClientNetConfigManager> cl,ClientOpt::Options &option,std::map<std::string,StateRunnedHosts> *runned_hosts) -> bool{
    std::list<std::string> connected_hosts;
    std::list<std::string> slaveHosts;
    std::list<std::string> masterHosts;
    g_rexit_flag = false;
    g_roption = option;

//    cl.addHandlerError([&masterHosts,&slaveHosts](ClientNetConfigManager::Errors errors,std::string host){
//        const std::lock_guard<std::mutex> lock(g_rmutex);
//        if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
//            std::cerr << getTS(": ") << "Error: " << host.c_str() << "\n";
//            g_rstart_counter--;
//            g_rstop_counter--;
//            masterHosts.remove(host);
//            slaveHosts.remove(host);
//        }
//    });

//    cl.addHandler(ClientNetConfigManager::Events::SERVER_STARTED_TCP, [&cl,runned_hosts](std::string host){
//        const std::lock_guard<std::mutex> lock(g_rmutex);
//        if (g_roption.verbous)
//            std::cout << getTS(": ") << "Streaming started: " << host << " TCP mode [OK]\n";
//        g_rstart_counter--;
//        if (runned_hosts) (*runned_hosts)[host] = StateRunnedHosts::TCP;
//    });

//    cl.addHandler(ClientNetConfigManager::Events::SERVER_STARTED_UDP, [&cl,runned_hosts](std::string host){
//        const std::lock_guard<std::mutex> lock(g_rmutex);
//        if (g_roption.verbous)
//            std::cout << getTS(": ") << "Streaming started: " << host << " UDP mode [OK]\n";
//        g_rstart_counter--;
//        if (runned_hosts) (*runned_hosts)[host] = StateRunnedHosts::UDP;
//    });

//    cl.addHandler(ClientNetConfigManager::Events::SERVER_STARTED_SD, [&cl,runned_hosts](std::string host){
//        const std::lock_guard<std::mutex> lock(g_rmutex);
//        if (g_roption.verbous)
//            std::cout << getTS(": ") << "Streaming started: " << host << " Local mode [OK]\n";
//        g_rstart_counter--;
//        if (runned_hosts) (*runned_hosts)[host] = StateRunnedHosts::LOCAL;
//    });

//    cl.addHandler(ClientNetConfigManager::Events::SERVER_DAC_STARTED, [&cl,runned_hosts](std::string host){
//        const std::lock_guard<std::mutex> lock(g_rmutex);
//        if (g_roption.verbous)
//            std::cout << getTS(": ") << "DAC streaming started: " << host << " TCP mode [OK]\n";
//        g_rstart_counter--;
//        if (runned_hosts) (*runned_hosts)[host] = StateRunnedHosts::TCP;
//    });

//    cl.addHandler(ClientNetConfigManager::Events::SERVER_DAC_STARTED_SD, [&cl,runned_hosts](std::string host){
//        const std::lock_guard<std::mutex> lock(g_rmutex);
//        if (g_roption.verbous)
//            std::cout << getTS(": ") << "DAC streaming started: " << host << " Local mode [OK]\n";
//        g_rstart_counter--;
//        if (runned_hosts) (*runned_hosts)[host] = StateRunnedHosts::LOCAL;
//    });


//    cl.addHandler(ClientNetConfigManager::Events::SERVER_STOPPED, [&cl,&masterHosts,&slaveHosts](std::string host){
//        const std::lock_guard<std::mutex> lock(g_rmutex);
//        if (g_roption.verbous)
//            std::cout << getTS(": ") << "Streaming stopped: " << host << " [OK]\n";
//        masterHosts.remove(host);
//        slaveHosts.remove(host);
//        g_rstop_counter--;
//    });

//    cl.addHandler(ClientNetConfigManager::Events::SERVER_STOPPED_SD_FULL, [&cl,&masterHosts,&slaveHosts](std::string host){
//        const std::lock_guard<std::mutex> lock(g_rmutex);
//        if (g_roption.verbous)
//            std::cout << getTS(": ") << "Streaming stopped: " << host << " SD card is full [OK]\n";
//        masterHosts.remove(host);
//        slaveHosts.remove(host);
//        g_rstop_counter--;
//    });

//    cl.addHandler(ClientNetConfigManager::Events::SERVER_STOPPED_SD_DONE, [&cl,&masterHosts,&slaveHosts](std::string host){
//        const std::lock_guard<std::mutex> lock(g_rmutex);
//        if (g_roption.verbous)
//            std::cout << getTS(": ") << "Streaming stopped: " << host << " All samples are recorded on the SD card [OK]\n";
//        masterHosts.remove(host);
//        slaveHosts.remove(host);
//        g_rstop_counter--;
//    });

//    cl.addHandler(ClientNetConfigManager::Events::SERVER_DAC_STOPPED, [&cl,&masterHosts,&slaveHosts](std::string host){
//        const std::lock_guard<std::mutex> lock(g_rmutex);
//        if (g_roption.verbous)
//            std::cout << getTS(": ") << "DAC streaming stopped: " << host << " [OK]\n";
//        masterHosts.remove(host);
//        slaveHosts.remove(host);
//        g_rstop_counter--;
//    });

//    cl.addHandler(ClientNetConfigManager::Events::SERVER_DAC_STOPPED_SD_DONE, [&cl,&masterHosts,&slaveHosts](std::string host){
//        const std::lock_guard<std::mutex> lock(g_rmutex);
//        if (g_roption.verbous)
//            std::cout << getTS(": ") << "DAC streaming stopped: " << host << " [OK]\n";
//        masterHosts.remove(host);
//        slaveHosts.remove(host);
//        g_rstop_counter--;
//    });

//    cl.addHandler(ClientNetConfigManager::Events::SERVER_DAC_STOPPED_SD_BROKEN, [&cl,&masterHosts,&slaveHosts](std::string host){
//        const std::lock_guard<std::mutex> lock(g_rmutex);
//        if (g_roption.verbous)
//            std::cout << getTS(": ") << "DAC streaming stopped: " << host << " [File broken]\n";
//        masterHosts.remove(host);
//        slaveHosts.remove(host);
//        g_rstop_counter--;
//    });

//    cl.addHandler(ClientNetConfigManager::Events::SERVER_DAC_STOPPED_SD_EMPTY, [&cl,&masterHosts,&slaveHosts](std::string host){
//        const std::lock_guard<std::mutex> lock(g_rmutex);
//        if (g_roption.verbous)
//            std::cout << getTS(": ") << "DAC streaming stopped: " << host << " [File empty]\n";
//        masterHosts.remove(host);
//        slaveHosts.remove(host);
//        g_rstop_counter--;
//    });

//    cl.addHandler(ClientNetConfigManager::Events::SERVER_DAC_STOPPED_SD_MISSING, [&cl,&masterHosts,&slaveHosts](std::string host){
//        const std::lock_guard<std::mutex> lock(g_rmutex);
//        if (g_roption.verbous)
//            std::cout << getTS(": ") << "DAC streaming stopped: " << host << " [File missing]\n";
//        masterHosts.remove(host);
//        slaveHosts.remove(host);
//        g_rstop_counter--;
//    });

    connected_hosts = cl->getHosts();
    for(auto &host:connected_hosts) {
        switch (cl->getModeByHost(host)) {
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
    switch(g_roption.remote_mode){
        case ClientOpt::RemoteMode::START:{
            return startStreaming(cl,masterHosts,slaveHosts, g_roption.testmode == ClientOpt::TestMode::ENABLE,runned_hosts);
        }

        case ClientOpt::RemoteMode::START_DAC:{
            return startStreaming(cl,masterHosts,slaveHosts, g_roption.testmode == ClientOpt::TestMode::ENABLE,runned_hosts);
        }

        case ClientOpt::RemoteMode::START_FPGA_ADC:{
            return startADC(cl,masterHosts,slaveHosts, g_roption.testmode == ClientOpt::TestMode::ENABLE,runned_hosts);
        }

        case ClientOpt::RemoteMode::STOP:{
            return stopStreaming(cl,masterHosts,slaveHosts,runned_hosts);
        }

        case ClientOpt::RemoteMode::STOP_DAC:{
            return stopDACStreaming(cl,masterHosts,slaveHosts,runned_hosts);
        }

        case ClientOpt::RemoteMode::START_STOP:{
            return startStopStreaming(cl,masterHosts,slaveHosts,g_roption.testmode == ClientOpt::TestMode::ENABLE,runned_hosts);
        }

        case ClientOpt::RemoteMode::START_STOP_DAC:{
            return startStopDACStreaming(cl,masterHosts,slaveHosts,g_roption.testmode == ClientOpt::TestMode::ENABLE,runned_hosts);
        }

        default: {
            std::cerr << getTS(": ") << "[Fatal] Error mode\n";
            return false;
        }
    }
    return true;
}

auto remoteSIGHandler() -> void{
    g_rexit_flag = true;
}


auto startStreaming(std::shared_ptr<ClientNetConfigManager> cl,std::list<std::string> &masterHosts,std::list<std::string> &slaveHosts,bool test_mode,std::map<std::string,StateRunnedHosts> *runned_hosts) -> bool{
    std::atomic<int>   rstart_counter;

    cl->addHandlerError([&](ClientNetConfigManager::Errors errors,std::string host){
        const std::lock_guard<std::mutex> lock(g_rmutex);
        if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
            std::cerr << getTS(": ") << "Error: " << host.c_str() << "\n";
            rstart_counter--;
            masterHosts.remove(host);
            slaveHosts.remove(host);
        }
    });

    cl->addHandler(ClientNetConfigManager::Events::SERVER_STARTED_TCP, [&](std::string host){
        const std::lock_guard<std::mutex> lock(g_rmutex);
        if (g_roption.verbous)
            std::cout << getTS(": ") << "Streaming started: " << host << " TCP mode [OK]\n";
        rstart_counter--;
        if (runned_hosts) (*runned_hosts)[host] = StateRunnedHosts::TCP;
    });

    cl->addHandler(ClientNetConfigManager::Events::SERVER_STARTED_UDP, [&](std::string host){
        const std::lock_guard<std::mutex> lock(g_rmutex);
        if (g_roption.verbous)
            std::cout << getTS(": ") << "Streaming started: " << host << " UDP mode [OK]\n";
        rstart_counter--;
        if (runned_hosts) (*runned_hosts)[host] = StateRunnedHosts::UDP;
    });

    cl->addHandler(ClientNetConfigManager::Events::SERVER_STARTED_SD, [&](std::string host){
        const std::lock_guard<std::mutex> lock(g_rmutex);
        if (g_roption.verbous)
            std::cout << getTS(": ") << "Streaming started: " << host << " Local mode [OK]\n";
        rstart_counter--;
        if (runned_hosts) (*runned_hosts)[host] = StateRunnedHosts::LOCAL;
    });


    rstart_counter = slaveHosts.size();
    for(auto &host:slaveHosts) {
        if (g_roption.verbous)
            std::cerr << getTS(": ") << "Send start command to slave board " << host.c_str() << (test_mode ? " [Benchmark mode]":"") << "\n";
        if (!cl->sendStart(host,test_mode)){
            rstart_counter--;
        }
    }
    while (rstart_counter>0){
        sleepMs(100);
        if (g_rexit_flag) {
            cl->removeHadlers();
            return false;
        }
    }

    rstart_counter = masterHosts.size();
    for(auto &host:masterHosts) {
        if (g_roption.verbous)
            std::cerr << getTS(": ") << "Send start command to master board " << host.c_str() << (test_mode ? " [Benchmark mode]":"") << "\n";
        if (!cl->sendStart(host,test_mode)){
            rstart_counter--;
        }
    }

    while (rstart_counter>0){
        sleepMs(100);
        if (g_rexit_flag) {
            cl->removeHadlers();
            return false;
        }
    }
    cl->removeHadlers();
    return true;
}

auto startADC(std::shared_ptr<ClientNetConfigManager> cl,std::list<std::string> &masterHosts,std::list<std::string> &slaveHosts,bool test_mode,std::map<std::string,StateRunnedHosts> *runned_hosts) -> bool{
    std::atomic<int>   rstart_counter;

    cl->addHandlerError([&](ClientNetConfigManager::Errors errors,std::string host){
        const std::lock_guard<std::mutex> lock(g_rmutex);
        if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
            std::cerr << getTS(": ") << "Error: " << host.c_str() << "\n";
            rstart_counter--;
            masterHosts.remove(host);
            slaveHosts.remove(host);
        }
    });

    cl->addHandler(ClientNetConfigManager::Events::START_ADC_DONE, [&](std::string host){
        const std::lock_guard<std::mutex> lock(g_rmutex);
        if (g_roption.verbous)
            std::cout << getTS(": ") << "ADC is run: " << host << "\n";
        rstart_counter--;
        if (runned_hosts) (*runned_hosts)[host] = StateRunnedHosts::NONE;
    });

    rstart_counter = slaveHosts.size();

    for(auto &host:slaveHosts) {
        if (g_roption.verbous)
            std::cerr << getTS(": ") << "Send start ADC command to slave board " << host.c_str() << (test_mode ? " [Benchmark mode]":"") << "\n";
        if (!cl->sendStartADC(host)){
            rstart_counter--;
        }
    }
    while (rstart_counter>0){
        sleepMs(100);
        if (g_rexit_flag) {
            cl->removeHadlers();
            return false;
        }
    }

    rstart_counter = masterHosts.size();
    for(auto &host:masterHosts) {
        if (g_roption.verbous)
            std::cerr << getTS(": ") << "Send start ADC command to master board " << host.c_str() << (test_mode ? " [Benchmark mode]":"") << "\n";
        if (!cl->sendStartADC(host)){
            rstart_counter--;
        }
    }
    while (rstart_counter>0){
        sleepMs(100);
        if (g_rexit_flag) {
            cl->removeHadlers();
            return false;
        }
    }
    cl->removeHadlers();
    return true;
}

auto startDACStreaming(std::shared_ptr<ClientNetConfigManager> cl,std::list<std::string> &masterHosts,std::list<std::string> &slaveHosts,bool test_mode,std::map<std::string,StateRunnedHosts> *runned_hosts) -> bool{
    std::atomic<int>   rstart_counter;

    cl->addHandlerError([&](ClientNetConfigManager::Errors errors,std::string host){
        const std::lock_guard<std::mutex> lock(g_rmutex);
        if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
            std::cerr << getTS(": ") << "Error: " << host.c_str() << "\n";
            rstart_counter--;
            masterHosts.remove(host);
            slaveHosts.remove(host);
        }
    });

    cl->addHandler(ClientNetConfigManager::Events::SERVER_DAC_STARTED, [&](std::string host){
        const std::lock_guard<std::mutex> lock(g_rmutex);
        if (g_roption.verbous)
            std::cout << getTS(": ") << "DAC streaming started: " << host << " TCP mode [OK]\n";
        rstart_counter--;
        if (runned_hosts) (*runned_hosts)[host] = StateRunnedHosts::TCP;
    });

    cl->addHandler(ClientNetConfigManager::Events::SERVER_DAC_STARTED_SD, [&](std::string host){
        const std::lock_guard<std::mutex> lock(g_rmutex);
        if (g_roption.verbous)
            std::cout << getTS(": ") << "DAC streaming started: " << host << " Local mode [OK]\n";
        rstart_counter--;
        if (runned_hosts) (*runned_hosts)[host] = StateRunnedHosts::LOCAL;
    });

    rstart_counter = slaveHosts.size();

    for(auto &host:slaveHosts) {
        if (g_roption.verbous)
            std::cerr << getTS(": ") << "Send start DAC command to slave board " << host.c_str() << (test_mode ? " [Benchmark mode]":"") << "\n";
        if (!cl->sendDACStart(host,test_mode)){
            rstart_counter--;
        }
    }
    while (rstart_counter>0){
        sleepMs(100);
        if (g_rexit_flag) {
            cl->removeHadlers();
            return false;
        }
    }

    rstart_counter = masterHosts.size();
    for(auto &host:masterHosts) {
        if (g_roption.verbous)
            std::cerr << getTS(": ") << "Send start DAC command to master board " << host.c_str() << (test_mode ? " [Benchmark mode]":"") << "\n";
        if (!cl->sendDACStart(host,test_mode)){
            rstart_counter--;
        }
    }
    while (rstart_counter>0){
        sleepMs(100);
        if (g_rexit_flag) {
            cl->removeHadlers();
            return false;
        }
    }
    cl->removeHadlers();
    return true;
}

auto stopStreaming(std::shared_ptr<ClientNetConfigManager> cl,std::list<std::string> &masterHosts,std::list<std::string> &slaveHosts,std::map<std::string,StateRunnedHosts> *runned_hosts) -> bool{
    std::atomic<int>   rstop_counter;

    cl->addHandlerError([&](ClientNetConfigManager::Errors errors,std::string host){
        const std::lock_guard<std::mutex> lock(g_rmutex);
        if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
            std::cerr << getTS(": ") << "Error: " << host.c_str() << "\n";
            rstop_counter--;
            masterHosts.remove(host);
            slaveHosts.remove(host);
        }
    });

    cl->addHandler(ClientNetConfigManager::Events::SERVER_STOPPED, [&](std::string host){
        const std::lock_guard<std::mutex> lock(g_rmutex);
        if (g_roption.verbous)
            std::cout << getTS(": ") << "Streaming stopped: " << host << " [OK]\n";
        masterHosts.remove(host);
        slaveHosts.remove(host);
        rstop_counter--;
    });

    cl->addHandler(ClientNetConfigManager::Events::SERVER_STOPPED_SD_FULL, [&](std::string host){
        const std::lock_guard<std::mutex> lock(g_rmutex);
        if (g_roption.verbous)
            std::cout << getTS(": ") << "Streaming stopped: " << host << " SD card is full [OK]\n";
        masterHosts.remove(host);
        slaveHosts.remove(host);
        rstop_counter--;
    });

    cl->addHandler(ClientNetConfigManager::Events::SERVER_STOPPED_SD_DONE, [&](std::string host){
        const std::lock_guard<std::mutex> lock(g_rmutex);
        if (g_roption.verbous)
            std::cout << getTS(": ") << "Streaming stopped: " << host << " All samples are recorded on the SD card [OK]\n";
        masterHosts.remove(host);
        slaveHosts.remove(host);
        rstop_counter--;
    });

    rstop_counter = masterHosts.size();
    for(auto &host:masterHosts) {
        if (g_roption.verbous)
            std::cerr << getTS(": ") << "Send stop command to master board " << host.c_str() << "\n";
        if (!cl->sendStop(host)){
            rstop_counter--;
        }
    }
    while (rstop_counter>0){
        sleepMs(100);
        if (g_rexit_flag) {
            cl->removeHadlers();
            return false;
        }
    }

    rstop_counter = slaveHosts.size();
    for(auto &host:slaveHosts) {
        if (g_roption.verbous)
            std::cerr << getTS(": ") << "Send stop command to slave board " << host.c_str() << "\n";
        if (!cl->sendStop(host)){
            rstop_counter--;
        }
    }
    while (rstop_counter>0){
        sleepMs(100);
        if (g_rexit_flag) {
            cl->removeHadlers();
            return false;
        }
    }
    cl->removeHadlers();
    return true;
}


auto stopDACStreaming(std::shared_ptr<ClientNetConfigManager> cl,std::list<std::string> &masterHosts,std::list<std::string> &slaveHosts,std::map<std::string,StateRunnedHosts> *runned_hosts) -> bool{
    std::atomic<int>   rstop_counter;

    cl->addHandlerError([&](ClientNetConfigManager::Errors errors,std::string host){
        const std::lock_guard<std::mutex> lock(g_rmutex);
        if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
            std::cerr << getTS(": ") << "Error: " << host.c_str() << "\n";
            rstop_counter--;
            masterHosts.remove(host);
            slaveHosts.remove(host);
        }
    });

    cl->addHandler(ClientNetConfigManager::Events::SERVER_DAC_STOPPED, [&](std::string host){
        const std::lock_guard<std::mutex> lock(g_rmutex);
        if (g_roption.verbous)
            std::cout << getTS(": ") << "DAC streaming stopped: " << host << " [OK]\n";
        masterHosts.remove(host);
        slaveHosts.remove(host);
        rstop_counter--;
    });

    cl->addHandler(ClientNetConfigManager::Events::SERVER_DAC_STOPPED_SD_DONE, [&](std::string host){
        const std::lock_guard<std::mutex> lock(g_rmutex);
        if (g_roption.verbous)
            std::cout << getTS(": ") << "DAC streaming stopped: " << host << " [OK]\n";
        masterHosts.remove(host);
        slaveHosts.remove(host);
        rstop_counter--;
    });

    cl->addHandler(ClientNetConfigManager::Events::SERVER_DAC_STOPPED_SD_BROKEN, [&](std::string host){
        const std::lock_guard<std::mutex> lock(g_rmutex);
        if (g_roption.verbous)
            std::cout << getTS(": ") << "DAC streaming stopped: " << host << " [File broken]\n";
        masterHosts.remove(host);
        slaveHosts.remove(host);
        rstop_counter--;
    });

    cl->addHandler(ClientNetConfigManager::Events::SERVER_DAC_STOPPED_SD_EMPTY, [&](std::string host){
        const std::lock_guard<std::mutex> lock(g_rmutex);
        if (g_roption.verbous)
            std::cout << getTS(": ") << "DAC streaming stopped: " << host << " [File empty]\n";
        masterHosts.remove(host);
        slaveHosts.remove(host);
        rstop_counter--;
    });

    cl->addHandler(ClientNetConfigManager::Events::SERVER_DAC_STOPPED_SD_MISSING, [&](std::string host){
        const std::lock_guard<std::mutex> lock(g_rmutex);
        if (g_roption.verbous)
            std::cout << getTS(": ") << "DAC streaming stopped: " << host << " [File missing]\n";
        masterHosts.remove(host);
        slaveHosts.remove(host);
        rstop_counter--;
    });

    rstop_counter = masterHosts.size();
    for(auto &host:masterHosts) {
        if (g_roption.verbous)
            std::cerr << getTS(": ") << "Send stop DAC command to master board " << host.c_str() << "\n";
        if (!cl->sendDACStop(host)){
            rstop_counter--;
        }
    }
    while (rstop_counter>0){
        sleepMs(100);
        if (g_rexit_flag) {
            cl->removeHadlers();
            return false;
        }
    }

    rstop_counter = slaveHosts.size();
    for(auto &host:slaveHosts) {
        if (g_roption.verbous)
            std::cerr << getTS(": ") << "Send stop DAC command to slave board " << host.c_str() << "\n";
        if (!cl->sendDACStop(host)){
            rstop_counter--;
        }
    }
    while (rstop_counter>0){
        sleepMs(100);
        if (g_rexit_flag) {
            cl->removeHadlers();
            return false;
        }
    }
    cl->removeHadlers();
    return true;
}


auto startStopStreaming(std::shared_ptr<ClientNetConfigManager> cl,std::list<std::string> &masterHosts,std::list<std::string> &slaveHosts,bool test_mode,std::map<std::string,StateRunnedHosts> *runned_hosts) -> bool{

    if (!startStreaming(cl,masterHosts,slaveHosts,test_mode,runned_hosts)){
        return false;
    }

    auto beginTime = std::chrono::time_point_cast<std::chrono::milliseconds >(std::chrono::system_clock::now()).time_since_epoch().count();
    auto curTime = beginTime;
    g_roption.timeout = g_roption.timeout == 0 ? 1000 : g_roption.timeout;
    while ((curTime - beginTime <= g_roption.timeout) && (masterHosts.size() > 0 || slaveHosts.size() > 0)){
        sleepMs(1);
        if (g_rexit_flag) break;
        curTime = std::chrono::time_point_cast<std::chrono::milliseconds >(std::chrono::system_clock::now()).time_since_epoch().count();
    }

    g_rexit_flag = false;

    if (!stopStreaming(cl,masterHosts,slaveHosts,runned_hosts)){
        return false;
    }

    return true;
}


auto startStopDACStreaming(std::shared_ptr<ClientNetConfigManager> cl,std::list<std::string> &masterHosts,std::list<std::string> &slaveHosts,bool test_mode,std::map<std::string,StateRunnedHosts> *runned_hosts) -> bool{

    if (!startDACStreaming(cl,masterHosts,slaveHosts,test_mode,runned_hosts)){
        return false;
    }

    auto beginTime = std::chrono::time_point_cast<std::chrono::milliseconds >(std::chrono::system_clock::now()).time_since_epoch().count();
    auto curTime = beginTime;
    g_roption.timeout = g_roption.timeout == 0 ? 1000 : g_roption.timeout;
    while ((curTime - beginTime <= g_roption.timeout) && (masterHosts.size() > 0 || slaveHosts.size() > 0)){
        sleepMs(1);
        if (g_rexit_flag) break;
        curTime = std::chrono::time_point_cast<std::chrono::milliseconds >(std::chrono::system_clock::now()).time_since_epoch().count();
    }

    if (!stopDACStreaming(cl,masterHosts,slaveHosts,runned_hosts)){
        return false;
    }

    return true;
}
