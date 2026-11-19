/**
 * $Id: $
 *
 * @brief Red Pitaya streaming server implementation
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#ifndef _WIN32
#include <sys/resource.h>
#include <syslog.h>
#endif

#include <signal.h>
#ifndef _WIN32
#include <fcntl.h>
#endif
#include <unistd.h>
#include <atomic>

#include "board_lib/board.h"
#include "options.h"
#include "session_lib/stream_session.h"
#include "uio_lib/hardware.h"
#ifdef STREAMING_WEB
#include "web_page.h"
#endif

static session_lib::StreamSession::Ptr g_session = nullptr;
static std::atomic_bool g_run(true);

// a signal handler may only set a flag: printing or logging from it can deadlock
static void termSignalHandler(int) {
    g_run = false;
}

static void handleCloseChildEvents() {
#ifdef _WIN32
    signal(SIGINT, termSignalHandler);
    signal(SIGTERM, termSignalHandler);
#else
    struct sigaction sigchld_action;
    sigchld_action.sa_handler = SIG_DFL, sigchld_action.sa_flags = SA_NOCLDWAIT;
    sigaction(SIGCHLD, &sigchld_action, NULL);
#endif
}

static void installTermSignalHandler() {
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

int main(int argc, char* argv[]) {
#ifndef _WIN32
    setpriority(PRIO_PROCESS, 0, -20);
#endif
    bool slave = false;  // a daisy chain slave, from the FPGA

    auto opt = ClientOpt::parse(argc, argv);
    bool verbMode = opt.verbose;

#ifndef _WIN32
    // Open logging into "/var/log/messages" or /var/log/syslog" or other configured...
    setlogmask(LOG_UPTO(LOG_INFO));

    openlog("streaming-server", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    if (opt.background) {
        //        FILE *fp= NULL;
        pid_t process_id = 0;
        pid_t sid = 0;
        // Create child process
        process_id = fork();
        // Indication of fork() failure
        if (process_id < 0) {
            aprintf(stderr, "fork failed!\n");
            // Return failure in exit status
            exit(1);
        }

        // PARENT PROCESS. Need to kill it.
        if (process_id > 0) {
            //printf("process_id of child process %d \n", process_id);
            // return success in exit status
            exit(0);
        }

        //unmask the file mode
        umask(0);
        //set new session
        sid = setsid();
        if (sid < 0) {
            // Return failure
            exit(1);
        }

        // not closed: the next sockets or files would take the numbers 0-2 and get every
        // printf of the server
        const int null = open("/dev/null", O_RDWR);
        if (null >= 0) {
            dup2(null, STDIN_FILENO);
            dup2(null, STDOUT_FILENO);
            dup2(null, STDERR_FILENO);
            if (null > STDERR_FILENO)
                close(null);
        }
    }
#endif

#ifdef STREAMING_WEB
    // with the web page its module owns the session (as the nginx module did): the page and the
    // clients of the board drive the same streams
    if (opt.webPort) {
        web_page::start(opt.webPort, opt.conf_file, opt.listen, verbMode);
        printWithLog(LOG_INFO, stdout, "The web page connects on 127.0.0.1:%u\n", (unsigned)opt.webPort);
    } else
#endif
    try {
        // The FPGA blocks for the whole process; the daisy chain role is read from the scope
        auto hw = uio_lib::CHardware::create(board_lib::createBoard());
        if (!hw) {
            FATAL("Can't open the FPGA blocks")
        }
        slave = hw->features().slave;
        printWithLog(LOG_INFO, stdout, "Detected %s mode\n", slave ? "Slave" : "Master");

        session_lib::SessionOptions options;
        options.configFile = opt.conf_file;
        options.host = opt.listen;
        options.verbose = verbMode;
        g_session = session_lib::StreamSession::create(hw, options);
    } catch (std::exception& e) {
        printWithLog(LOG_ERR, stderr, "Error: Init the streaming session %s\n", e.what());
        exit(EXIT_FAILURE);
    }
    if (verbMode) {
        printWithLog(LOG_NOTICE, stdout, "streaming-server started %s\n", slave ? "[Slave]" : "[Master]");
    }

    installTermSignalHandler();
    // Handle close child events
    handleCloseChildEvents();
    while (g_run) {
        sleep(1);
    }
    printWithLog(LOG_INFO, stdout, "\nReceived terminate signal. Exiting...\n");

    try {
#ifdef STREAMING_WEB
        if (opt.webPort)
            web_page::stop();
#endif
        if (g_session)
            g_session->stop();
        g_session = nullptr;
    } catch (std::exception& e) {
        printWithLog(LOG_ERR, stderr, "Error: main() %s\n", e.what());
    }
    if (verbMode) {
        printWithLog(LOG_INFO, stdout, "streaming-server stopped.\n");
    }
#ifndef _WIN32
    closelog();
#endif
    return (EXIT_SUCCESS);
}
