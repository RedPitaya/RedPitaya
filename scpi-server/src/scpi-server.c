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
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <syslog.h>

#include "scpi/scpi.h"
#include "scpi-commands.h"

#include "rp.h"

#define LISTEN_BACKLOG 50
#define LISTEN_PORT 5000
#define MAX_BUFF_SIZE 1024
#define MAX_MESSAGE_SIZE (MAX_BUFF_SIZE * 2)

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

/**
 * This is main method of every child process. Here communication with client is handled.
 * @param connfd The communication port
 * @return
 */
static int handleConnection(int connfd) {
	int read_size;

	char message[MAX_MESSAGE_SIZE + 1];
	char buffer[MAX_BUFF_SIZE];
	size_t msgEnd = 0;

	installTermSignalHandler();

	prctl( 1, SIGTERM );

	while (1) {

	    syslog(LOG_INFO, "Waiting for a message.");

		//Receive a message from client
		while( (read_size = recv(connfd , buffer , MAX_BUFF_SIZE , 0)) > 0 )
		{
			if (app_exit) {
				break;
			}

			char *b = buffer;
			do {
				size_t pos = getNextCommand(b, read_size);

				// Next command not found, just copy all buffer (part of current command) to message...
				if (pos == -1) {
					if (msgEnd + read_size <= MAX_MESSAGE_SIZE)
					{
						memcpy(message + msgEnd, b, read_size);
						msgEnd += read_size;
					}
					else {
						msgEnd = MAX_MESSAGE_SIZE + 1;
					}

					break;
				}
				// Found next command - process the current one
				else {
					if (msgEnd + pos <= MAX_MESSAGE_SIZE)
					{
						memcpy(message + msgEnd, b, pos);
						read_size -= pos;
						b += pos;

						message[msgEnd + pos] = '\0';
						syslog(LOG_ERR, "Processing message %s", message);

						//Parse the message and return response
						SCPI_Input(&scpi_context, message, (pos + msgEnd));
					}
					else {
						message[MAX_MESSAGE_SIZE > 15 ? 15 : MAX_MESSAGE_SIZE] = '\0';
						syslog(LOG_ERR, "Skipping too large message %s...", message);
						msgEnd = 0;
					}
				}
			} while (1);
	    }

	    if(read_size == 0)
	    {
		    syslog(LOG_INFO, "Client is disconnected");
	        return 0;
	    }
	    else if(read_size == -1)
	    {
	    	syslog(LOG_ERR, "Receive message failed (%s)", strerror(errno));
	        perror("Receive message failed");
	        return 1;
	    }
	}
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

	syslog (LOG_NOTICE, "scpi-server started");

	installTermSignalHandler();

    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;

    // Handle close child events
    handleCloseChildEvents();


    int result = rp_Init();
    if (result != RP_OK) {
    	syslog(LOG_ERR, "Failed to initialize RP library: %s", rp_GetError(result));
    	return (EXIT_FAILURE);
    }

    // user_context will be pointer to socket
    scpi_context.user_context = NULL;
    SCPI_Init(&scpi_context);

    // Create a socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1)
    {
    	syslog(LOG_ERR, "Failed to create a socket (%s)", strerror(errno));
    	perror("Failed to create a socket");
    	return (EXIT_FAILURE);
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(LISTEN_PORT);

    if (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
    {
    	syslog(LOG_ERR, "Failed to bind the socket (%s)", strerror(errno));
    	perror("Failed to bind the socket");
    	return (EXIT_FAILURE);
    }

    if (listen(listenfd, LISTEN_BACKLOG) == -1)
    {
    	syslog(LOG_ERR, "Failed to listen on the socket (%s)", strerror(errno));
    	perror("Failed to listen on the socket");
    	return (EXIT_FAILURE);
    }

    syslog(LOG_INFO, "Server is listening on port %d\n", LISTEN_PORT);

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
        	syslog(LOG_ERR, "Failed to accept connection (%s)", strerror(errno));
        	perror("Failed to accept connection\n");
        	return (EXIT_FAILURE);
        }

        // Fork a child process, which will talk to the client
        if (!fork()) {

            syslog(LOG_INFO, "Connection with client ip %s established.", inet_ntoa(cliaddr.sin_addr));

        	// this is the child process
        	close(listenfd); // child doesn't need the listener

        	scpi_context.user_context = &connfd;

        	int result = handleConnection(connfd);

        	close(connfd);

            syslog(LOG_INFO, "Closing connection with client ip %s.", inet_ntoa(cliaddr.sin_addr));

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
    	syslog(LOG_ERR, "Failed to initialize RP library: %s", rp_GetError(result));
    }


    syslog(LOG_INFO, "scpi-server stopped.");

	closelog ();

    return (EXIT_SUCCESS);
}

