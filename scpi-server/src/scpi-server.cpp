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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <syslog.h>
#include <termios.h>
#include <unistd.h>
#include <mutex>
#include <thread>
#include <vector>

#include "common.h"
#include "scpi-commands.h"
#include "scpi-parser-ext.h"

#include "api_cmd.h"
#include "common/rp_sweep.h"
#include "rp.h"
#include "scpi/parser.h"

#define LISTEN_BACKLOG 50
#define LISTEN_PORT 5000
#define MAX_BUFF_SIZE 1024
#define UART_RATE 115200
#define UART_ARDUINO_RATE 57600

#define SCPI_ERROR_QUEUE_SIZE 16
#define CONFIG_FILE_UART "/root/.scpi_uart"
#define CONFIG_FILE_ARDUINO "/root/.scpi_arduino"
#define CONFIG_FILE_ARDUINO_TCP "/root/.scpi_arduino_tcp"

enum START_MODE { TCP, UART, ARDUINO, ARDUINO_TCP };

constexpr char id0[] = "REDPITAYA";
constexpr char id1[] = "INSTR2025";
constexpr char id2[] = "";
constexpr char id3[] = "01-21";

constexpr char device[] = "/dev/ttyPS1";

static bool app_exit = false;
static char delimiter[] = "\r\n";

static void handleCloseChildEvents() {
    struct sigaction sigchld_action;
    memset(&sigchld_action, 0, sizeof(struct sigaction));
    sigchld_action.sa_handler = SIG_DFL;
    sigchld_action.sa_flags = SA_NOCLDWAIT;
    sigaction(SIGCHLD, &sigchld_action, NULL);
}

static void termSignalHandler(int signum) {
    app_exit = true;
    syslog(LOG_NOTICE, "Received terminate signal. Exiting...");
}

static void installTermSignalHandler() {
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
static size_t getNextCommand(const char* buffer, size_t bufferLen) {
    size_t delimiterLen = sizeof(delimiter) - 1;  // dont count last null char.
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
                    return i + 1;  // Position of next command
                }

                dist++;
            }
        }
    }

    // No match found
    return 0;
}

void LogMessage(char* m, size_t len) {
    const size_t buff_len = 50;
    char buff[buff_len];

    len = std::min(len, buff_len);
    strncpy(buff, m, len);
    buff[len - 1] = '\0';

    rp_Log(nullptr, LOG_INFO, 0, "Processing command: %s", buff);
}

/**
 * This is main method of every child process. Here communication with client is handled.
 * @param connfd The communication port
 * @return
 */
static int handleConnection(scpi_t* ctx, int connfd) {
    static std::mutex mtx;
    int read_size;

    size_t message_len = MAX_BUFF_SIZE;
    char* message_buff = (char*)malloc(message_len);
    char buffer[MAX_BUFF_SIZE];
    size_t msg_end = 0;

    // installTermSignalHandler();

    // prctl( 1, SIGTERM );

    rp_Log(nullptr, LOG_INFO, 0, "Waiting for first client request.");
    int buf = 1024 * 16;
    if (setsockopt(connfd, SOL_SOCKET, SO_RCVBUF, &buf, sizeof(int)) == -1) {
        rp_Log(nullptr, LOG_ERR, 0, "Error setting socket opts: %s", strerror(errno));
    }

    buf = 1024 * 16;
    if (setsockopt(connfd, SOL_SOCKET, SO_SNDBUF, &buf, sizeof(int)) == -1) {
        rp_Log(nullptr, LOG_ERR, 0, "Error setting socket opts: %s", strerror(errno));
    }
    //Receive a message from client
    while ((read_size = recv(connfd, buffer, MAX_BUFF_SIZE, 0)) > 0) {
        if (app_exit) {
            break;
        }

        // First make sure that message buffer is large enough
        while (msg_end + read_size >= message_len) {
            message_len *= 2;
            message_buff = (char*)realloc(message_buff, message_len);
        }

        // Copy read buffer into message buffer
        memcpy(message_buff + msg_end, buffer, read_size);
        msg_end += read_size;

        // Now try to parse each command out
        char* m = message_buff;
        size_t pos = 0;
        while ((pos = getNextCommand(m, msg_end)) != 0) {
            std::lock_guard<std::mutex> lock(mtx);
            // Log out message
            LogMessage(m, pos);

            //Parse the message and return response
            SCPI_Input(ctx, m, pos);
            m += pos;
            msg_end -= pos;
        }

        // Move the rest of the message to the beginning of the buffer
        if (message_buff != m && msg_end > 0) {
            memmove(message_buff, m, msg_end);
        }

        rp_Log(nullptr, LOG_INFO, 0, "Waiting for next client request.");
    }

    free(message_buff);

    rp_Log(nullptr, LOG_INFO, 0, "Closing client connection...");

    if (read_size == 0) {
        rp_Log(nullptr, LOG_INFO, 0, "Client is disconnected");
        return 0;
    } else if (read_size == -1) {
        rp_Log(nullptr, LOG_ERR, 0, "Receive message failed (%s)", strerror(errno));
        perror("Receive message failed");
        return 1;
    }
    return 0;
}

std::thread* threadConnection(int connId, bool isArduino) {
    auto func = [](int connId) {
        int sockId = connId;
        user_context_t uc;
        scpi_error_t scpi_error_queue_data[SCPI_ERROR_QUEUE_SIZE];
        auto ctx = initContext(isArduino);
        if (ctx == NULL) {
            close(sockId);
            return;
        }

        SCPI_Init(ctx, ctx->cmdlist, ctx->interface, ctx->units, id0, id1, id2, id3, ctx->buffer.data, ctx->buffer.length, scpi_error_queue_data,
                  SCPI_ERROR_QUEUE_SIZE);

        uc.fd = sockId;
        if (isArduino) {
            uc.binary_format = false;
        }
        ctx->user_context = &uc;

        handleConnection(ctx, sockId);

        close(sockId);
        delete[] ctx->buffer.data;
        delete ctx;

        rp_Log(nullptr, LOG_INFO, 0, "Closing connection with client");
    };

    auto th = new std::thread(func, connId);
    return th;
}

auto startTCP(bool isArduino) -> int {
    std::vector<std::thread*> clients;
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;

    // Create a socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        rp_Log(nullptr, LOG_ERR, 0, "Failed to create a socket (%s)", strerror(errno));
        perror("Failed to create a socket");
        return (EXIT_FAILURE);
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(LISTEN_PORT);
    int buf = 1024 * 16;
    if (setsockopt(listenfd, SOL_SOCKET, SO_RCVBUF, &buf, sizeof(int)) == -1) {
        rp_Log(nullptr, LOG_ERR, 0, "Error setting socket opts: %s", strerror(errno));
    }

    buf = 1024 * 16;
    if (setsockopt(listenfd, SOL_SOCKET, SO_SNDBUF, &buf, sizeof(int)) == -1) {
        rp_Log(nullptr, LOG_ERR, 0, "Error setting socket opts: %s", strerror(errno));
    }
    int enable = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) {
        rp_Log(nullptr, LOG_ERR, 0, "Error setting socket opts: %s", strerror(errno));
    }

    if (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        rp_Log(nullptr, LOG_ERR, 0, "Failed to bind the socket (%s)", strerror(errno));
        perror("Failed to bind the socket");
        return (EXIT_FAILURE);
    }

    if (listen(listenfd, LISTEN_BACKLOG) == -1) {
        rp_Log(nullptr, LOG_ERR, 0, "Failed to listen on the socket (%s)", strerror(errno));
        perror("Failed to listen on the socket");
        return (EXIT_FAILURE);
    }

    rp_Log(nullptr, LOG_INFO, 0, "Server is listening on port %d", LISTEN_PORT);

    // Socket is opened and listening on port. Now we can accept connections
    while (1) {
        struct sockaddr_in cliaddr;
        socklen_t clilen;
        clilen = sizeof(cliaddr);

        connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);

        if (app_exit == true) {
            break;
        }

        if (connfd == -1) {
            rp_Log(nullptr, LOG_ERR, 0, "Failed to accept connection (%s)", strerror(errno));
            perror("Failed to accept connection\n");
            return (EXIT_FAILURE);
        }

        auto cth = threadConnection(connfd, isArduino);
        clients.push_back(cth);
    }

    for (auto th : clients) {
        if (th->joinable())
            th->join();
    }

    close(listenfd);

    return (EXIT_SUCCESS);
}

auto startUART() -> int {

    int read_size;
    struct termios g_settings;
    user_context_t uc;

    size_t message_len = MAX_BUFF_SIZE;

    int uart_fd = open(device, O_RDWR | O_NOCTTY);

    if (uart_fd == -1) {
        ERROR_LOG("Failed to open %s.", device);
        return (EXIT_FAILURE);
    }

    tcflush(uart_fd, TCIFLUSH);
    tcflush(uart_fd, TCIOFLUSH);
    tcgetattr(uart_fd, &g_settings);
    g_settings.c_cflag &= ~CSIZE;
    g_settings.c_cflag |= CS8 | CLOCAL | CREAD;             /* 8 bits */
    g_settings.c_cflag &= ~CRTSCTS;                         // Disable flow control
    g_settings.c_iflag &= ~(IXON | IXOFF | IXANY);          // Disable XON/XOFF flow control both input & output
    g_settings.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG);  // Non Cannonical mode
    g_settings.c_iflag &= ~ICRNL;
    g_settings.c_oflag &= ~OPOST; /* raw output */

    g_settings.c_lflag = 0;     //  enable raw input instead of canonical,
    g_settings.c_cc[VMIN] = 1;  // Read at least 1 character
    cfsetspeed(&g_settings, UART_RATE);
    tcsetattr(uart_fd, TCSANOW, &g_settings);

    scpi_error_t scpi_error_queue_data[SCPI_ERROR_QUEUE_SIZE];
    auto ctx = initContext(false);
    if (ctx == NULL) {
        close(uart_fd);
        return (EXIT_FAILURE);
    }

    SCPI_Init(ctx, ctx->cmdlist, ctx->interface, ctx->units, id0, id1, id2, id3, ctx->buffer.data, ctx->buffer.length, scpi_error_queue_data,
              SCPI_ERROR_QUEUE_SIZE);
    uc.fd = uart_fd;
    ctx->user_context = &uc;

    std::vector<uint8_t> message_buff;
    std::vector<uint8_t> buffer;
    message_buff.resize(message_len);
    buffer.resize(message_len);

    size_t msg_end = 0;
    //Receive a message from client
    while ((read_size = read(uart_fd, buffer.data(), buffer.size())) > 0) {
        if (app_exit) {
            break;
        }
        // First make sure that message buffer is large enough
        auto new_message_size = msg_end + read_size;
        if (new_message_size >= message_buff.size()) {
            message_buff.resize(new_message_size);
            if (message_buff.size() != new_message_size) {
                rp_Log(nullptr, LOG_ERR, 0, "Not enough memory for buffer.");
                break;
            }
        }

        // Copy read buffer into message buffer
        memcpy(&(message_buff.data()[msg_end]), buffer.data(), read_size);
        msg_end += read_size;

        // Now try to parse each command out
        char* m = (char*)(message_buff.data());
        size_t pos = 0;
        while ((pos = getNextCommand(m, msg_end)) != 0) {
            // Log out message
            LogMessage(m, pos);

            //Parse the message and return response
            SCPI_Input(ctx, m, pos);
            m += pos;
            msg_end -= pos;
        }

        // Move the rest of the message to the beginning of the buffer
        if ((char*)message_buff.data() != m && msg_end > 0) {
            memmove(message_buff.data(), m, msg_end);
        }

        rp_Log(nullptr, LOG_INFO, 0, "Waiting for next client request.");
    }

    if (read_size == -1) {
        rp_Log(nullptr, LOG_ERR, 0, "Receive message failed (%s)", strerror(errno));
        perror("Receive message failed");
    }

    if (uart_fd != -1) {
        tcflush(uart_fd, TCIFLUSH);
        close(uart_fd);
    }

    delete[] ctx->buffer.data;
    delete ctx;

    return (EXIT_SUCCESS);
}

auto startArduinoApi() -> int {

    int read_size;
    struct termios g_settings;
    user_context_t uc;

    size_t message_len = MAX_BUFF_SIZE;

    int uart_fd = open(device, O_RDWR | O_NOCTTY);

    if (uart_fd == -1) {
        ERROR_LOG("Failed to open %s.", device);
        return (EXIT_FAILURE);
    }

    tcflush(uart_fd, TCIFLUSH);
    tcflush(uart_fd, TCIOFLUSH);
    tcgetattr(uart_fd, &g_settings);
    g_settings.c_cflag &= ~CSIZE;
    g_settings.c_cflag |= CS8 | CLOCAL | CREAD;             /* 8 bits */
    g_settings.c_cflag &= ~CRTSCTS;                         // Disable flow control
    g_settings.c_iflag &= ~(IXON | IXOFF | IXANY);          // Disable XON/XOFF flow control both input & output
    g_settings.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG);  // Non Cannonical mode
    g_settings.c_iflag &= ~ICRNL;
    g_settings.c_oflag &= ~OPOST; /* raw output */

    g_settings.c_lflag = 0;     //  enable raw input instead of canonical,
    g_settings.c_cc[VMIN] = 1;  // Read at least 1 character
    g_settings.c_cc[VTIME] = 0;
    cfsetspeed(&g_settings, UART_ARDUINO_RATE);
    tcsetattr(uart_fd, TCSANOW, &g_settings);

    scpi_error_t scpi_error_queue_data[SCPI_ERROR_QUEUE_SIZE];
    auto ctx = initContext(true);
    if (ctx == NULL) {
        close(uart_fd);
        return (EXIT_FAILURE);
    }

    SCPI_Init(ctx, ctx->cmdlist, ctx->interface, ctx->units, id0, id1, id2, id3, ctx->buffer.data, ctx->buffer.length, scpi_error_queue_data,
              SCPI_ERROR_QUEUE_SIZE);
    uc.fd = uart_fd;
    uc.binary_format = false;
    ctx->user_context = &uc;

    std::vector<uint8_t> message_buff;
    std::vector<uint8_t> buffer;
    message_buff.resize(message_len);
    buffer.resize(message_len);

    // auto uart_protocol = getUARTProtocol();
    // uart_protocol->setTimeout(5000);
    size_t msg_end = 0;
    //Receive a message from client
    while ((read_size = read(uart_fd, buffer.data(), buffer.size())) > 0) {
        if (app_exit) {
            break;
        }
        // First make sure that message buffer is large enough
        auto new_message_size = msg_end + read_size;
        if (new_message_size >= message_buff.size()) {
            message_buff.resize(new_message_size);
            if (message_buff.size() != new_message_size) {
                rp_Log(nullptr, LOG_ERR, 0, "Not enough memory for buffer.");
                break;
            }
        }

        // Copy read buffer into message buffer
        memcpy(&(message_buff.data()[msg_end]), buffer.data(), read_size);
        msg_end += read_size;

        // Now try to parse each command out
        char* m = (char*)(message_buff.data());
        size_t pos = 0;
        while ((pos = getNextCommand(m, msg_end)) != 0) {
            // Log out message
            LogMessage(m, pos);

            //Parse the message and return response
            SCPI_Input(ctx, m, pos);
            m += pos;
            msg_end -= pos;
        }
        // Move the rest of the message to the beginning of the buffer
        if ((char*)message_buff.data() != m && msg_end > 0) {
            memmove(message_buff.data(), m, msg_end);
        }

        rp_Log(nullptr, LOG_INFO, 0, "Waiting for next client request.");
    }

    if (read_size == -1) {
        rp_Log(nullptr, LOG_ERR, 0, "Receive message failed (%s)", strerror(errno));
        perror("Receive message failed");
    }

    if (uart_fd != -1) {
        tcflush(uart_fd, TCIFLUSH);
        close(uart_fd);
    }

    delete[] ctx->buffer.data;
    delete ctx;

    return (EXIT_SUCCESS);
}

/**
 * Main daemon entrance point. Opens a socket and listens for any incoming connection.
 * When client connects, if forks the conversation into a new socket and the daemon (parent process)
 * waits for another connection. It can handle multiple connections simultaneously.
 * @param argc  not used
 * @param argv  not used
 * @return
 */
int main(int argc, char* argv[]) {

    START_MODE mode = TCP;

    if (argc > 1) {
        std::string param = argv[1];
        if (param == "-u") {
            mode = UART;
        }
        if (param == "-a") {
            mode = ARDUINO;
        }

        if (param == "-at") {
            mode = ARDUINO_TCP;
        }
    }

    std::ifstream conf(CONFIG_FILE_UART);
    if (conf.is_open()) {
        mode = UART;
        conf.close();
    } else {
        std::ifstream conf(CONFIG_FILE_ARDUINO);
        if (conf.is_open()) {
            mode = ARDUINO;
            conf.close();
        } else {
            std::ifstream conf(CONFIG_FILE_ARDUINO_TCP);
            if (conf.is_open()) {
                mode = ARDUINO_TCP;
                conf.close();
            }
        }
    }

    // Open logging into "/var/log/messages" or /var/log/syslog" or other configured...
    setlogmask(LOG_UPTO(LOG_INFO));
    openlog("scpi-server", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

    rp_Log(nullptr, LOG_NOTICE, 0, "scpi-server started");

    installTermSignalHandler();

    prctl(1, SIGTERM);

    // Handle close child events
    handleCloseChildEvents();

    int result = rp_Init();
    if (result != RP_OK) {
        rp_Log(nullptr, LOG_ERR, result, "Failed to initialize RP APP library: %s", rp_GetError(result));
        return (EXIT_FAILURE);
    }

    result = rp_Reset();
    if (result != RP_OK) {
        rp_Log(nullptr, LOG_ERR, result, "Failed to reset RP APP: %s", rp_GetError(result));
        return (EXIT_FAILURE);
    }

    result = rp_sweep_api::rp_SWInit();
    if (result != RP_OK) {
        rp_Log(nullptr, LOG_ERR, result, "Failed to initialize RP Sweep library: %s", rp_GetError(result));
        return (EXIT_FAILURE);
    }

    int ret = 0;

    switch (mode) {
        case TCP:
            ret = startTCP(false);
            break;
        case UART:
            ret = startUART();
            break;
        case ARDUINO:
            ret = startArduinoApi();
            break;
        case ARDUINO_TCP:
            ret = startTCP(true);
            break;
        default:
            rp_Log(nullptr, LOG_ERR, 0, "Scpi-server stopped. [Unknown mode]");
            break;
    }

    rp_sweep_api::rp_SWRelease();

    result = rp_Release();
    if (result != RP_OK) {
        rp_Log(nullptr, LOG_ERR, result, "Failed to release RP App library: %s", rp_GetError(result));
    }

    rp_Log(nullptr, LOG_INFO, 0, "scpi-server stopped.");

    closelog();

    return ret;
}
