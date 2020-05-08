/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server implementation
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

#include <netinet/in.h>
#include <sys/prctl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <syslog.h>

#include "scpi-commands.h"
#include "common.h"

#include "scpi/parser.h"
#include "rp.h"
#include "api_cmd.h"

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

#define LISTEN_BACKLOG 50
#define LISTEN_PORT 5000
#define MAX_BUFF_SIZE 1024

static bool app_exit = false;
static char delimiter[] = "\r\n";


static void handleCloseChildEvents()
{
    struct sigaction sigchld_action = {
            .sa_handler = SIG_DFL,
            .sa_flags = SA_NOCLDWAIT
    };
    sigaction(SIGCHLD, &sigchld_action, NULL);
}


static void termSignalHandler(int signum)
{
    app_exit = true;
    syslog (LOG_NOTICE, "Received terminate signal. Exiting...");
}


static void installTermSignalHandler()
{
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = termSignalHandler;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGINT, &action, NULL);
}

/**
 * Helper method which returns next command position from the buffer.
 * @param buffer     Input buffer
 * @param bufferLen  Input buffer length
 * @return Position of next command within buffer, or -1 if not found.
 */
static size_t getNextCommand(const char* buffer, size_t bufferLen)
{
    size_t delimiterLen = sizeof(delimiter) - 1; // dont count last null char.
    size_t i = 0;
    for (i = 0; i < bufferLen; i++) {

        // Find match for end of delimiter
        if (buffer[i] == delimiter[delimiterLen - 1]) {

            // Now go back checking if all delimiter character matches
            size_t dist = 0;
            while (i - dist >= 0 && delimiterLen - dist > 0) {
                if (buffer[i - dist] != delimiter[delimiterLen - dist - 1]) {
                    break;
                }
                if (delimiterLen - dist - 1 == 0) {
                    return i + 1; // Position of next command
                }

                dist++;
            }
        }
    }

    // No match found
    return -1;
}

void LogMessage(char *m, size_t len) {
    const size_t buff_len = 50;
    char buff[buff_len];

    len = MIN(len, buff_len);
    strncpy(buff, m, len);
    buff[len - 1] = '\0';

    RP_LOG(LOG_INFO, "Processing command: %s\n", buff);
}

/**
 * This is main method of every child process. Here communication with client is handled.
 * @param connfd The communication port
 * @return
 */
static int handleConnection(int connfd) {
    int read_size;

    size_t message_len = MAX_BUFF_SIZE;
    char *message_buff = malloc(message_len);
    char buffer[MAX_BUFF_SIZE];
    size_t msg_end = 0;

    installTermSignalHandler();

    prctl( 1, SIGTERM );

    RP_LOG(LOG_INFO, "Waiting for first client request.");

    //Receive a message from client
    while( (read_size = recv(connfd , buffer , MAX_BUFF_SIZE , 0)) > 0 )
    {
        if (app_exit) {
            break;
        }

        // First make sure that message buffer is large enough
        while (msg_end + read_size >= message_len) {
            message_len *= 2;
            message_buff = realloc(message_buff, message_len);
        }

        // Copy read buffer into message buffer
        memcpy(message_buff + msg_end, buffer, read_size);
        msg_end += read_size;

        // Now try to parse each command out
        char *m = message_buff;
        size_t pos = -1;
        while ((pos = getNextCommand(m, msg_end)) != -1) {

            // Log out message
            LogMessage(m, pos);

            //Parse the message and return response
            SCPI_Input(&scpi_context, m, pos);
            m += pos;
            msg_end -= pos;
        }

        // Move the rest of the message to the beginning of the buffer
        if (message_buff != m && msg_end > 0) {
            memmove(message_buff, m, msg_end);
        }

        RP_LOG(LOG_INFO, "Waiting for next client request.\n");
    }

    free(message_buff);

    RP_LOG(LOG_INFO, "Closing client connection...");

    if(read_size == 0)
    {
        RP_LOG(LOG_INFO, "Client is disconnected");
        return 0;
    }
    else if(read_size == -1)
    {
        RP_LOG(LOG_ERR, "Receive message failed (%s)", strerror(errno));
        perror("Receive message failed");
        return 1;
    }
    return 0;
}


/**
 * Main daemon entrance point. Opens a socket and listens for any incoming connection.
 * When client connects, if forks the conversation into a new socket and the daemon (parent process)
 * waits for another connection. It can handle multiple connections simultaneously.
 * @param argc  not used
 * @param argv  not used
 * @return
 */
int main(int argc, char *argv[])
{

    // Open logging into "/var/log/messages" or /var/log/syslog" or other configured...
    setlogmask (LOG_UPTO (LOG_INFO));
    openlog ("scpi-server", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

    RP_LOG (LOG_NOTICE, "scpi-server started");

    installTermSignalHandler();

    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;

    // Handle close child events
    handleCloseChildEvents();


    int result = rp_Init();
    if (result != RP_OK) {
        RP_LOG(LOG_ERR, "Failed to initialize RP APP library: %s", rp_GetError(result));
        return (EXIT_FAILURE);
    }

    result = rp_Reset();
    if (result != RP_OK) {
        RP_LOG(LOG_ERR, "Failed to reset RP APP: %s", rp_GetError(result));
        return (EXIT_FAILURE);
    }



    // user_context will be pointer to socket
    scpi_context.user_context = NULL;
    scpi_context.binary_output = false;
    SCPI_Init(&scpi_context);
    RP_ResetAll(&scpi_context); // need for set default values of scpi

    // Create a socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1)
    {
        RP_LOG(LOG_ERR, "Failed to create a socket (%s)", strerror(errno));
        perror("Failed to create a socket");
        return (EXIT_FAILURE);
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(LISTEN_PORT);

    if (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
    {
        RP_LOG(LOG_ERR, "Failed to bind the socket (%s)", strerror(errno));
        perror("Failed to bind the socket");
        return (EXIT_FAILURE);
    }

    if (listen(listenfd, LISTEN_BACKLOG) == -1)
    {
        RP_LOG(LOG_ERR, "Failed to listen on the socket (%s)", strerror(errno));
        perror("Failed to listen on the socket");
        return (EXIT_FAILURE);
    }

    RP_LOG(LOG_INFO, "Server is listening on port %d\n", LISTEN_PORT);

    // Socket is opened and listening on port. Now we can accept connections
    while(1)
    {
        struct sockaddr_in cliaddr;
        socklen_t clilen;
        clilen = sizeof(cliaddr);

        connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);

        if (app_exit == true) {
            break;
        }

        if (connfd == -1) {
            RP_LOG(LOG_ERR, "Failed to accept connection (%s)", strerror(errno));
            perror("Failed to accept connection\n");
            return (EXIT_FAILURE);
        }

        // Fork a child process, which will talk to the client
        if (!fork()) {

            RP_LOG(LOG_INFO, "Connection with client ip %s established.", inet_ntoa(cliaddr.sin_addr));

            // this is the child process
            close(listenfd); // child doesn't need the listener

            scpi_context.user_context = &connfd;

            result = handleConnection(connfd);

            close(connfd);

            RP_LOG(LOG_INFO, "Closing connection with client ip %s.", inet_ntoa(cliaddr.sin_addr));

            if (result == 0) {
                return(EXIT_SUCCESS);
            }
            else {
                return(EXIT_FAILURE);
            }
        }

        close(connfd);
    }

    close(listenfd);

    result = rp_Release();
    if (result != RP_OK) {
        RP_LOG(LOG_ERR, "Failed to release RP App library: %s", rp_GetError(result));
    }


    RP_LOG(LOG_INFO, "scpi-server stopped.");

    closelog ();

    return (EXIT_SUCCESS);
}
