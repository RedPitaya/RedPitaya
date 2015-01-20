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

#include "dpin.h"
#include "apin.h"
#include "generate.h"
#include "acquire.h"
#include "../3rdparty/libs/scpi-parser/libscpi/inc/scpi/parser.h"

#define LISTEN_BACKLOG 50
#define LISTEN_PORT 5000
#define MAX_BUFF_SIZE 1048576
#define MAX_MESSAGE_SIZE (MAX_BUFF_SIZE * 2)

static bool app_exit = false;
static char delimiter[2] = "\r\n";

// ring buffer
//#include <lcthw/bstrlib.h>

#define RingBuffer_available_data(B) (((B)->end + 1) % (B)->length - (B)->start - 1)

#define RingBuffer_available_space(B) ((B)->length - (B)->end - 1)

#define RingBuffer_full(B) (RingBuffer_available_data((B)) - (B)->length == 0)

#define RingBuffer_empty(B) (RingBuffer_available_data((B)) == 0)

#define RingBuffer_get_all(B) RingBuffer_gets((B), RingBuffer_available_data((B)))

#define RingBuffer_starts_at(B) ((B)->buffer + (B)->start)

#define RingBuffer_ends_at(B) ((B)->buffer + (B)->end)

#define RingBuffer_commit_read(B, A) ((B)->start = ((B)->start + (A)) % (B)->length)

#define RingBuffer_commit_write(B, A) ((B)->end = ((B)->end + (A)) % (B)->length)

typedef struct {
    char *buffer;
    int length;
    int start;
    int end;
} RingBuffer;

RingBuffer *RingBuffer_create(int length)
{
    RingBuffer *buffer = calloc(1, sizeof(RingBuffer));
    buffer->length  = length + 1;
    buffer->start = 0;
    buffer->end = 0;
    buffer->buffer = calloc(buffer->length, 1);

    return buffer;
}

void RingBuffer_destroy(RingBuffer *buffer)
{
    if(buffer) {
        free(buffer->buffer);
        free(buffer);
    }
}

int RingBuffer_write(RingBuffer *buffer, char *data, int length)
{
    if(RingBuffer_available_space(buffer) == 0) {
        buffer->start = buffer->end = 0;
    }

    if(length >= RingBuffer_available_space(buffer)){
    	//printf("Not enough space: %d request, %d available\r\n",
    	//		length, RingBuffer_available_space(buffer));
    	return -1;
    }

    void *result = memcpy(RingBuffer_ends_at(buffer), data, length);
    if(result == NULL){
    	//printf("Failed to write data into buffer.\r\n");
    	return -1;
    }
    RingBuffer_commit_write(buffer, length);
    return length;
}

int RingBuffer_read(RingBuffer *buffer, char *target, int amount)
{
    if(amount > RingBuffer_available_data(buffer)){
    	//printf("Not enough in the buffer: has %d, needs %d\r\n",
    	//		RingBuffer_available_data(buffer), amount);
        return -1;
    }

    void *result = memcpy(target, RingBuffer_starts_at(buffer), amount);
    if(result == NULL){
    	//printf("Failed to write buffer into data.\r\n");
        return -1;
    }

    RingBuffer_commit_read(buffer, amount);

    if(buffer->end == buffer->start) {
        buffer->start = buffer->end = 0;
    }

    return amount;
}

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

size_t RingBuffer_getCmd(RingBuffer *buffer, char* cmd)
{
	char *ptr = strstr(&buffer->buffer[buffer->start],delimiter);
	if(ptr==NULL){
		return 0;
	}
    int cmdLen=ptr-&buffer->buffer[buffer->start]+sizeof(delimiter);
    int len=RingBuffer_read(buffer, cmd, cmdLen);
    if(len!=cmdLen){
		return 0;
    }
    cmd[cmdLen]='\0';
    return len;
}


/**
 * This is main method of every child process. Here communication with client is handled.
 * @param connfd The communication port
 * @return
 */
static int handleConnection(int connfd) {

	RingBuffer * ringBuf;
	ringBuf=RingBuffer_create(MAX_BUFF_SIZE);
	char * buffer = malloc(MAX_BUFF_SIZE);
	char * cmdBuf = malloc(MAX_BUFF_SIZE);

	installTermSignalHandler();

	prctl( 1, SIGTERM );

	while (1) {

	    syslog(LOG_INFO, "Waiting for a message.");

		int read_size;
		//Receive a message from client
		while( (read_size = recv(connfd , buffer , MAX_BUFF_SIZE , 0)) > 0 ){

			if (app_exit) {
				break;
			}

			int len=RingBuffer_write(ringBuf,buffer,read_size);
			if(len!=read_size){
				syslog(LOG_ERR, "Error writing circular buffer %d,%d\r\n",len,read_size);
			}

			do {
				len=RingBuffer_getCmd(ringBuf,cmdBuf);
				if(len){
					//Parse the message and return response
					syslog(LOG_INFO, "Processing message %s", cmdBuf);
					SCPI_Input(&scpi_context, cmdBuf, len);
				}
				else{
			      	// no command available
					break;
				}
			} while (1);
		}

		RingBuffer_destroy(ringBuf);
		free(buffer);
		free(cmdBuf);

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

	result = rp_Reset();
	if (result != RP_OK) {
		syslog(LOG_ERR, "Failed to reset RP: %s", rp_GetError(result));
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

        	result = handleConnection(connfd);

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

