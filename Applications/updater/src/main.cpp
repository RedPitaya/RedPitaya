
#include <string>
#include <memory>
#include <unistd.h>
#include <stdio.h>
#include <cstring>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <net/if.h>
#include <unistd.h>
#include <signal.h>

// FILE* logFile;
void signalHandler( int signum )
{
    //fprintf(logFile, "Interrupt signal %d received.\n", signum); fflush(logFile);
}

void StartDaemon()
{
    pid_t pid;

    /* Fork off the parent process */
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* On success: The child process becomes session leader */
    if (setsid() < 0)
        exit(EXIT_FAILURE);

    /* Close all open file descriptors */
    int x;
    for (x = sysconf(_SC_OPEN_MAX); x>0; x--)
    {
        close (x);
    }
    /* Open the log file */
}


int main(int argc, char *argv[])
{


    /* Catch, ignore and handle signals */
    //TODO: Implement a working signal handler */

    StartDaemon();
    signal(SIGCHLD, signalHandler);
    signal(SIGHUP, signalHandler);
    signal(SIGABRT, signalHandler);
    signal(SIGFPE, signalHandler);
    signal(SIGILL, signalHandler);
    signal(SIGINT, signalHandler);
    signal(SIGSEGV, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGUSR1, signalHandler);
    signal(SIGUSR2, signalHandler);

    // logFile = fopen ("/root/daemon.log","a");
    //fprintf (logFile, "----------------------------------------------------\n"); fflush(logFile);
    //fprintf (logFile, "[StartDaemon] Daemon have started\n"); fflush(logFile);
	sleep(1);
    /*int res = 0;*/
	/*res = */system("nohup killall nginx & disown");
    //fprintf (logFile, "> nohup killall nginx : %d\n", res); fflush(logFile);
    sleep(1);

    /*res = */system("nohup killall hostapd & disown");
    //fprintf (logFile, "> nohup killall hostapd : %d\n", res); fflush(logFile);
    sleep(1);

	/*res = */system("nohup mount -o rw,remount /opt/redpitaya & disown");
    //fprintf (logFile, "> nohup mount -o rw,remount /opt/redpitaya : %d\n", res); fflush(logFile);
    sleep(1);

	/*res = */system("nohup /bin/cp -fvr /tmp/build/* /opt/redpitaya/ & disown");
    //fprintf (logFile, "> nohup /bin/cp -fvr /tmp/build/* /opt/redpitaya/ : %d\n", res); fflush(logFile);
    sleep(17);

	/*res = */system("nohup reboot & disown");
    //fprintf (logFile, "> nohup reboot : %d\n", res); fflush(logFile);
    sleep(1);

    // fclose(logFile);
    return 0;
}
