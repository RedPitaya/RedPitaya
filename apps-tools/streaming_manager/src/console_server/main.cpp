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
#include <netinet/in.h>
#include <sys/prctl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <regex>
#include <mutex>

#include "rp.h"
#include "StreamingApplication.h"
#include "StreamingManager.h"
#include "ServerNetConfigManager.h"
#include "options.h"
#include "streaming.h"
#include "dac_streaming.h"

#ifdef Z20_250_12
#include "rp-spi.h"
#include "rp-i2c-max7311.h"
#endif




std::mutex g_print_mtx;
std::shared_ptr<ServerNetConfigManager> con_server = nullptr;
std::atomic_bool g_run(true);

const char  *g_argv0 = nullptr;


static void handleCloseChildEvents()
{
    struct sigaction sigchld_action; 
    sigchld_action.sa_handler = SIG_DFL,
    sigchld_action.sa_flags = SA_NOCLDWAIT;
    sigaction(SIGCHLD, &sigchld_action, NULL);
}


static void termSignalHandler(int signum)
{
    fprintf(stdout,"\nReceived terminate signal. Exiting...\n");
    syslog (LOG_NOTICE, "Received terminate signal. Exiting...");
    // StopNonBlocking(0);
    // if (s_manger) s_manger->stopWriteToCSV();
    g_run = false;
}


static void installTermSignalHandler()
{
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = termSignalHandler;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGINT, &action, NULL);
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
     // Open logging into "/var/log/messages" or /var/log/syslog" or other configured...
    setlogmask (LOG_UPTO (LOG_INFO));
    openlog ("streaming-server", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

    g_argv0 = argv[0];
    auto opt = ClientOpt::parse(argc,argv);
    bool verbMode = opt.verbose;
    if (opt.background){
        FILE *fp= NULL;
        pid_t process_id = 0;
        pid_t sid = 0;
        // Create child process
        process_id = fork();
        // Indication of fork() failure
        if (process_id < 0)
        {
            fprintf(stderr,"fork failed!\n");
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


    try{
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
        
       	#ifdef STREAMING_MASTER
			auto mode = asionet_broadcast::CAsioBroadcastSocket::ABMode::AB_SERVER_MASTER;
		#endif
        #ifdef STREAMING_SLAVE
			auto mode = asionet_broadcast::CAsioBroadcastSocket::ABMode::AB_SERVER_SLAVE;
        #endif 

        #ifdef Z10
		asionet_broadcast::CAsioBroadcastSocket::Model model = asionet_broadcast::CAsioBroadcastSocket::Model::RP_125_14;
		#endif

		#ifdef Z20
		asionet_broadcast::CAsioBroadcastSocket::Model model = asionet_broadcast::CAsioBroadcastSocket::Model::RP_122_16;
		#endif

		#ifdef Z20_125
		asionet_broadcast::CAsioBroadcastSocket::Model model = asionet_broadcast::CAsioBroadcastSocket::Model::RP_125_14_Z20;
		#endif

		#ifdef Z20_250_12
		asionet_broadcast::CAsioBroadcastSocket::Model model = asionet_broadcast::CAsioBroadcastSocket::Model::RP_250_12;
		#endif

		con_server = std::make_shared<ServerNetConfigManager>(opt.conf_file,mode,"127.0.0.1",opt.config_port);
        con_server->startBroadcast(model, brchost,opt.broadcast_port);

        con_server->addHandler(ServerNetConfigManager::Events::GET_NEW_SETTING,[con_server,verbMode](){
            std::lock_guard<std::mutex> lock(g_print_mtx);
            if (verbMode){
                fprintf(stdout, "Get new settings\n");
                fprintf(stdout,"%s",con_server->String().c_str());
                RP_LOG (LOG_INFO,"Get new settings\n");
                RP_LOG (LOG_INFO,"%s",con_server->String().c_str());
            }
        });

        con_server->addHandler(ServerNetConfigManager::Events::START_STREAMING,[con_server,verbMode](){
            startServer(con_server,verbMode,false);
        });

        con_server->addHandler(ServerNetConfigManager::Events::START_DAC_STREAMING,[con_server,verbMode](){
            startDACServer(con_server,verbMode,false);
        });

        con_server->addHandler(ServerNetConfigManager::Events::START_STREAMING_TEST,[con_server,verbMode](){
            startServer(con_server,verbMode,true);
        });

        con_server->addHandler(ServerNetConfigManager::Events::START_DAC_STREAMING_TEST,[con_server,verbMode](){
            startDACServer(con_server,verbMode,true);
        });

        con_server->addHandler(ServerNetConfigManager::Events::STOP_STREAMING,[](){
            stopNonBlocking(0);
        });

        con_server->addHandler(ServerNetConfigManager::Events::STOP_DAC_STREAMING,[](){
            stopDACNonBlocking(CDACStreamingManager::NR_STOP);
        });
        
    }catch (std::exception& e)
    {
        fprintf(stderr, "Error: Init ServerNetConfigManager() %s\n",e.what());
        RP_LOG (LOG_ERR,"Error: Init ServerNetConfigManager() %s\n",e.what());
        exit(EXIT_FAILURE);
    }
    if (verbMode){
        fprintf(stdout,"streaming-server started\n");
        RP_LOG (LOG_NOTICE, "streaming-server started");
    }

    installTermSignalHandler();
    // Handle close child events
    handleCloseChildEvents();

    try {
		CStreamingManager::MakeEmptyDir(FILE_PATH);
	}catch (std::exception& e)
	{
        fprintf(stderr,  "Error: Can't create %s dir %s",FILE_PATH,e.what());
        RP_LOG (LOG_ERR, "Error: Can't create %s dir %s",FILE_PATH,e.what());
        exit(EXIT_FAILURE);
	}

    while(g_run){
    }

    try{
        stopServer(0);
        stopDACServer(CDACStreamingManager::NR_STOP);
    }catch (std::exception& e)
    {
        fprintf(stderr, "Error: main() %s\n",e.what());
        RP_LOG(LOG_ERR, "Error: main() %s\n",e.what());
    }
    con_server->stop();
    if (verbMode){
        fprintf(stdout,  "streaming-server stopped.\n");
        RP_LOG(LOG_INFO, "streaming-server stopped.");
    }
    closelog ();
    return (EXIT_SUCCESS);
}