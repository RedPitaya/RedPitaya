/**
 * $Id: $
 *
 * @brief Red Pitaya streaming server implementation
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <sys/stat.h>
#include <errno.h>
#ifndef _WIN32
#include <netinet/in.h>
#include <arpa/inet.h>
#include <syslog.h>
#endif
#include <signal.h>
#include <unistd.h>
#include <regex>
#include <mutex>
#include <unistd.h>
#include <signal.h>

//#ifndef __APPLE__
//#include <sys/prctl.h>
//#endif

#include "broadcast_lib/asio_broadcast_socket.h"
#include "config_net_lib/server_net_config_manager.h"
#include "data_lib/thread_cout.h"
#include "streaming_lib/streaming_file.h"

#include "options.h"
#include "streaming.h"
#include "dac_streaming.h"
#include "streaming_lib/streaming_file.h"
#include "streaming.h"
#include "uio_lib/oscilloscope.h"


std::mutex g_print_mtx;
std::shared_ptr<ServerNetConfigManager> con_server = nullptr;
std::atomic_bool g_run(true);

const char  *g_argv0 = nullptr;

static void termSignalHandler(int)
{
    printWithLog(LOG_INFO,stdout,"\nReceived terminate signal. Exiting...\n");
    g_run = false;
}

static void handleCloseChildEvents()
{
#ifdef _WIN32
    signal(SIGINT, termSignalHandler);
    signal(SIGTERM, termSignalHandler);
#else
    struct sigaction sigchld_action;
    sigchld_action.sa_handler = SIG_DFL,
    sigchld_action.sa_flags = SA_NOCLDWAIT;
    sigaction(SIGCHLD, &sigchld_action, NULL);
#endif
}


static void installTermSignalHandler()
{
#ifdef _WIN32
    signal(SIGINT, termSignalHandler);
    signal(SIGTERM, termSignalHandler);
#else
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = termSignalHandler;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGINT, &action, NULL);
#endif
}

std::string exec(const char* cmd) {
    char buffer[128];
    std::string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    try {
        while (fgets(buffer, sizeof buffer, pipe) != NULL) {
            result += buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;
}

int main(int argc, char *argv[])
{
    uio_lib::BoardMode isMaster = uio_lib::BoardMode::UNKNOWN;

    g_argv0 = argv[0];
    auto opt = ClientOpt::parse(argc,argv);
    bool verbMode = opt.verbose;


#ifndef _WIN32
     // Open logging into "/var/log/messages" or /var/log/syslog" or other configured...
    setlogmask (LOG_UPTO (LOG_INFO));

    openlog ("streaming-server", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    if (opt.background){
//        FILE *fp= NULL;
        pid_t process_id = 0;
        pid_t sid = 0;
        // Create child process
        process_id = fork();
        // Indication of fork() failure
        if (process_id < 0)
        {
            aprintf(stderr,"fork failed!\n");
            // Return failure in exit status
            exit(1);
        }

        // PARENT PROCESS. Need to kill it.
        if (process_id > 0)
        {
            //printf("process_id of child process %d \n", process_id);
            // return success in exit status
            exit(0);
        }

        //unmask the file mode
        umask(0);
        //set new session
        sid = setsid();
        if(sid < 0)
        {
            // Return failure
            exit(1);
        }

        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
    }
#endif

    try{
#ifdef RP_PLATFORM
        auto hosts = exec("ip addr show eth0 2> /dev/null");
        auto whosts = exec("ip addr show wlan0 2> /dev/null");
        std::regex pattern("[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}");
        auto v_hosts = ClientOpt::split(hosts,'\n');
        auto v_whosts = ClientOpt::split(whosts,'\n');
        v_hosts.insert(std::end(v_hosts), std::begin(v_whosts), std::end(v_whosts));
        std::string brchost;
        for(auto h: v_hosts){
            std::smatch match;
            if(std::regex_search(h, match, pattern)) {
                if (match.size() > 0){
                    if (brchost != "") brchost += ";";
                    brchost += match[match.size()-1];
                }
            }
        }
        auto uioList = uio_lib::GetUioList();
        for (auto &uio : uioList){
            if (uio.nodeName == "rp_oscilloscope"){
                auto osc = uio_lib::COscilloscope::create(uio,1,true,ClientOpt::getADCRate(),false,ClientOpt::getADCBits(),2);
                isMaster = osc->isMaster();
                printWithLog(LOG_INFO,stdout,"Detected %s mode\n",isMaster == uio_lib::BoardMode::MASTER ? "Master" : (isMaster == uio_lib::BoardMode::SLAVE ? "Slave" : "Unknown"));
                break;
            }
        }
#else
        auto brchost = "127.0.0.1";
#endif
        auto mode = isMaster != uio_lib::BoardMode::SLAVE ? broadcast_lib::EMode::AB_SERVER_MASTER : broadcast_lib::EMode::AB_SERVER_SLAVE;
        auto model = ClientOpt::getBroadcastModel();

		con_server = std::make_shared<ServerNetConfigManager>(opt.conf_file,mode,"127.0.0.1",opt.config_port);
        setServer(con_server);
        setDACServer(con_server);
        con_server->startBroadcast(model, brchost,opt.broadcast_port);
        con_server->getNewSettingsNofiy.connect([verbMode](){
            std::lock_guard<std::mutex> lock(g_print_mtx);
            if (verbMode){
                printWithLog(LOG_INFO,stdout,"Get new settings\n");
                printWithLog(LOG_INFO,stdout,"%s",con_server->getSettingsRef().String().c_str());
            }
        });

//        con_server->clientConnectedNofiy.connect([verbMode](){
//            aprintf(stderr, "clientConnectedNofiy\n");
//        });

        con_server->startStreamingNofiy.connect([verbMode,isMaster](){
            aprintf(stderr, "startStreamingNofiy\n");
            startServer(verbMode,false,isMaster != uio_lib::BoardMode::SLAVE);
        });

        con_server->startDacStreamingNofiy.connect([verbMode](){
            startDACServer(verbMode,false);
        });

        con_server->startStreamingTestNofiy.connect([verbMode,isMaster](){
            startServer(verbMode,true,isMaster != uio_lib::BoardMode::SLAVE);
        });

        con_server->startDacStreamingTestNofiy.connect([verbMode](){
            startDACServer(verbMode,true);
        });

        con_server->stopStreamingNofiy.connect([](){
            stopNonBlocking(ServerNetConfigManager::NORMAL);
        });

        con_server->stopDacStreamingNofiy.connect([](){
            stopDACNonBlocking(dac_streaming_lib::CDACStreamingManager::NR_STOP);
        });

        con_server->startADCNofiy.connect([](){
            startADC();
            con_server->sendADCStarted();
        });

    }catch (std::exception& e)
    {
        printWithLog(LOG_ERR,stderr,"Error: Init ServerNetConfigManager() %s\n",e.what());
        exit(EXIT_FAILURE);
    }
    if (verbMode){
        printWithLog(LOG_NOTICE,stdout,"streaming-server started %s\n", isMaster != uio_lib::BoardMode::SLAVE ? "[Master]" : "[Slave]");
    }

    installTermSignalHandler();
    // Handle close child events
    handleCloseChildEvents();
    try {
        streaming_lib::CStreamingFile::makeEmptyDir(FILE_PATH);
	}catch (std::exception& e)
	{
        printWithLog(LOG_ERR,stderr,"Error: Can't create %s dir %s",FILE_PATH,e.what());
        exit(EXIT_FAILURE);
	}

    while(g_run){
        sleep(1);
    }

    try{
        stopServer(ServerNetConfigManager::NORMAL);
        stopDACServer(dac_streaming_lib::CDACStreamingManager::NR_STOP);
    }catch (std::exception& e)
    {
        printWithLog(LOG_ERR,stderr,"Error: main() %s\n",e.what());
    }
    con_server->stop();
    if (verbMode){
        printWithLog(LOG_INFO,stdout,"streaming-server stopped.\n");
    }
#ifndef _WIN32
    closelog ();
#endif
    return (EXIT_SUCCESS);
}
