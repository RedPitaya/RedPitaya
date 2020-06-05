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
#include <syslog.h>


#include "rp.h"
#include "StreamingApplication.h"
#include "StreamingManager.h"

#define DEBUG

#ifdef DEBUG
#define RP_LOG(...) \
syslog(__VA_ARGS__);
#else
#define RP_LOG(...)
#endif


CStreamingApplication *s_app = nullptr; 
void StopNonBlocking(int x);

static void handleCloseChildEvents()
{
    struct sigaction sigchld_action; 
    sigchld_action.sa_handler = SIG_DFL,
    sigchld_action.sa_flags = SA_NOCLDWAIT;
    
    sigaction(SIGCHLD, &sigchld_action, NULL);
}


static void termSignalHandler(int signum)
{
    syslog (LOG_NOTICE, "Received terminate signal. Exiting...");
    StopNonBlocking(0);
}


static void installTermSignalHandler()
{
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = termSignalHandler;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGINT, &action, NULL);
}

int main(int argc, char *argv[])
{

   
    FILE *fp= NULL;
    pid_t process_id = 0;
    pid_t sid = 0;
    // Create child process
    process_id = fork();
    // Indication of fork() failure
    if (process_id < 0)
    {
        printf("fork failed!\n");
        // Return failure in exit status
        exit(1);
    }

    // PARENT PROCESS. Need to kill it.
    if (process_id > 0)
    {
        printf("process_id of child process %d \n", process_id);
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

    // close(STDIN_FILENO);
    // close(STDOUT_FILENO);
    // close(STDERR_FILENO);


     // Open logging into "/var/log/messages" or /var/log/syslog" or other configured...
    setlogmask (LOG_UPTO (LOG_INFO));

    openlog ("streaming-server", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

    RP_LOG (LOG_NOTICE, "streaming-server started");

    installTermSignalHandler();
    // Handle close child events
    handleCloseChildEvents();

    try {
		CStreamingManager::MakeEmptyDir(FILE_PATH);
	}catch (std::exception& e)
	{
        RP_LOG(LOG_INFO, "Error: rp_app_init() %s\n",e.what());
	}

    try{
        int resolution = 1;
        int channel = 3;
        int protocol = 1;
        int sock_port = 666;
        std::string ip_addr_host = "127.0.0.1";
        int format = 0;
        int rate = 1;
        bool use_file = false;
   
        std::vector<UioT> uioList = GetUioList();
        // Search oscilloscope
        COscilloscope::Ptr osc = nullptr;
        CStreamingManager::Ptr s_manger = nullptr;

        for (const UioT &uio : uioList)
        {
            if (uio.nodeName == "rp_oscilloscope")
            {
                // TODO start server;
                osc = COscilloscope::Create(uio, (channel ==1 || channel == 3) , (channel ==2 || channel == 3) , rate);
                break;
            }
        }

        
        if (use_file == false) {
            s_manger = CStreamingManager::Create(
                    ip_addr_host,
                    std::to_string(sock_port).c_str(),
                    protocol == 1 ? asionet::Protocol::TCP : asionet::Protocol::UDP);
        }else{
            s_manger = CStreamingManager::Create((format == 0 ? Stream_FileType::WAV_TYPE: Stream_FileType::TDMS_TYPE) , FILE_PATH);
            s_manger->notifyStop = [](int status)
                                {
                                    StopNonBlocking(0);
                                };
        }
        int resolution_val = (resolution == 1 ? 8 : 16);
        s_app = new CStreamingApplication(s_manger, osc, 16, rate, channel);
        s_app->run();
        delete s_app;
    }catch (std::exception& e)
    {
        RP_LOG(LOG_INFO, "Error: StopServer() %s\n",e.what());
        fprintf(stderr, "Error: StopServer() %s\n",e.what());
    }
    
    RP_LOG(LOG_INFO, "streaming-server stopped.");

    closelog ();

    return (EXIT_SUCCESS);
}

void StopServer(int x){
	try{
		if (s_app!= nullptr) s_app->stop();
	}catch (std::exception& e){
		fprintf(stderr, "Error: StopServer() %s\n",e.what());
	}
}

void StopNonBlocking(int x){
	try{
		std::thread th(StopServer ,x);
		th.detach();
	}catch (std::exception& e){
		fprintf(stderr, "Error: StopServer() %s\n",e.what());
	}
}


