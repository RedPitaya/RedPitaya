/**
 * @file ptcc_service.cpp
 * @brief See ptcc_service.h.
 */

#include "ptcc_service.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <atomic>
#include <cerrno>
#include <chrono>
#include <memory>
#include <string>

#include "main.h"
#include "ptcc_params.h"
#include "web/rp_websocket.h"

#define PTCC_SERVICE_PORT 50001
#define PTCC_SERVICE_BIN "/opt/redpitaya/bin/ptcc_control"
#define PTCC_SERVICE_LOG "/tmp/vigo_photonics/ptcc_service.log"

/** Published to the page: the link to the controller service. */
CIntParameter ptccServiceState("PTCC_SERVICE_STATE", CBaseParameter::RO, PTCC_SERVICE_STOPPED, 0,
                               PTCC_SERVICE_STOPPED, PTCC_SERVICE_ALIVE);
CStringParameter ptccServiceInfo("PTCC_SERVICE_INFO", CBaseParameter::RO, "", 0);

namespace {

rp_websocket::CWEBClient::Ptr g_client = nullptr;

/** Only set when this application started the process, so an externally run
 *  service is never killed on exit. */
pid_t g_service_pid = -1;

std::atomic_bool g_want_refresh{false};
std::atomic_int g_pong{-1};
int g_ping = 0;
std::chrono::steady_clock::time_point g_last_ping{};
std::chrono::steady_clock::time_point g_last_answer{};
std::chrono::steady_clock::time_point g_last_connect{};

auto setState(ptcc_service_state_t state, const std::string &info) -> void {
    if (ptccServiceState.Value() != state) {
        ptccServiceState.SendValue(state);
    }
    if (ptccServiceInfo.Value() != info) {
        ptccServiceInfo.SendValue(info);
    }
}

/** True when something already listens on the service port. */
auto isPortInUse() -> bool {
    const int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        return false;
    }

    sockaddr_in address = {};
    address.sin_family = AF_INET;
    address.sin_port = htons(PTCC_SERVICE_PORT);
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    const bool open = connect(sock, reinterpret_cast<sockaddr *>(&address), sizeof(address)) == 0;
    close(sock);
    return open;
}

auto spawnService() -> pid_t {
    const pid_t pid = fork();
    if (pid < 0) {
        ERROR_LOG("Can't fork the PTCC service: %s", strerror(errno))
        return -1;
    }

    if (pid > 0) {
        return pid;
    }

    // Child. The worker may disappear without unloading us, so the service is
    // told to follow it rather than stay behind holding the serial port.
    prctl(PR_SET_PDEATHSIG, SIGTERM);
    setpgid(0, 0);

    const int log = open(PTCC_SERVICE_LOG, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    const int null_fd = open("/dev/null", O_RDONLY);
    if (null_fd >= 0) {
        dup2(null_fd, STDIN_FILENO);
    }
    if (log >= 0) {
        dup2(log, STDOUT_FILENO);
        dup2(log, STDERR_FILENO);
    }

    execl(PTCC_SERVICE_BIN, "ptcc_control", "--service", static_cast<char *>(nullptr));
    _exit(127);
}

auto connectClient() -> void {
    if (!g_client) {
        g_client = std::make_shared<rp_websocket::CWEBClient>();
        g_client->receiveStr.connect([](auto key, auto value) {
            if (key == "PTCC_PONG") {
                g_pong = std::stoi(std::string(value));
                return;
            }
            ptccParamsOnString(key, value);
        });
        g_client->receiveDouble.connect([](auto key, auto value) { ptccParamsOnDouble(key, value); });
        g_client->receiveInt.connect([](auto key, auto value) { ptccParamsOnInt(key, value); });
        g_client->receiveUInt.connect(
            [](auto key, auto value) { ptccParamsOnInt(key, static_cast<int>(value)); });
        g_client->receiveBool.connect([](auto key, auto value) { ptccParamsOnBool(key, value); });

        // The service publishes only what changed, so a page that arrives
        // later would see nothing until the next change. Asking for the whole
        // state on every connection covers both the first link and a restart.
        g_client->connected.connect([]() { g_want_refresh = true; });
        g_client->disconnected.connect([]() { ptccParamsOnDisconnect(); });
    }

    // The service needs a moment to open its port, and it may be restarted
    // underneath us, so a refused connection is retried rather than final.
    g_last_connect = std::chrono::steady_clock::now();
    if (!g_client->start("127.0.0.1", PTCC_SERVICE_PORT)) {
        ERROR_LOG("Can't start the websocket client for the PTCC service")
    }
}

}  // namespace

auto startPtccService() -> void {
    ptccParamsInit();
    g_pong = -1;
    g_ping = 0;
    g_last_answer = std::chrono::steady_clock::time_point{};

    if (isPortInUse()) {
        // Someone runs it already, for instance a second instance of this
        // application or a service started by hand. Use it, do not own it.
        setState(PTCC_SERVICE_STARTING, "attached to a service already running");
    } else {
        g_service_pid = spawnService();
        if (g_service_pid < 0) {
            setState(PTCC_SERVICE_STOPPED, "cannot start " PTCC_SERVICE_BIN);
            return;
        }
        setState(PTCC_SERVICE_STARTING, "started " PTCC_SERVICE_BIN);
    }

    connectClient();
}

auto stopPtccService() -> void {
    if (g_client) {
        g_client->stop();
        g_client = nullptr;
    }

    if (g_service_pid <= 0) {
        setState(PTCC_SERVICE_STOPPED, "");
        return;
    }

    kill(g_service_pid, SIGTERM);

    // Give it a moment to close the serial port, then insist.
    for (int waited = 0; waited < 30; ++waited) {
        if (waitpid(g_service_pid, nullptr, WNOHANG) == g_service_pid) {
            g_service_pid = -1;
            setState(PTCC_SERVICE_STOPPED, "");
            return;
        }
        usleep(100000);
    }

    kill(g_service_pid, SIGKILL);
    waitpid(g_service_pid, nullptr, 0);
    g_service_pid = -1;
    setState(PTCC_SERVICE_STOPPED, "");
}

auto updatePtccService() -> void {
    const auto now = std::chrono::steady_clock::now();

    // A service we started and that died on its own is brought back: it has to
    // run for as long as the application is loaded. Liveness is asked of the
    // kernel rather than of waitpid(), because the worker reaps children with
    // its own SIGCHLD handler and our pid may already be gone from there.
    if (g_service_pid > 0) {
        waitpid(g_service_pid, nullptr, WNOHANG);
        if (kill(g_service_pid, 0) != 0 && errno == ESRCH) {
            ERROR_LOG("The PTCC service exited, restarting it")
            g_service_pid = spawnService();
            g_last_connect = now;
        }
    }

    if (!g_client) {
        connectClient();
        if (!g_client) {
            return;
        }
    }

    if (!g_client->isConnected()) {
        if (now - g_last_connect >= std::chrono::seconds(2)) {
            connectClient();
        }
        setState(PTCC_SERVICE_STARTING, "waiting for the service");
        return;
    }

    if (g_want_refresh.exchange(false)) {
        g_client->send("PTCC_REFRESH", true);
    }

    if (now - g_last_ping >= std::chrono::seconds(2)) {
        g_last_ping = now;
        g_client->send("PTCC_PING", ++g_ping);
    }

    if (g_pong.load() == g_ping) {
        g_last_answer = now;
    }

    // Two missed pings in a row mean the process is there but not answering.
    const bool answering =
        g_last_answer != std::chrono::steady_clock::time_point{} &&
        now - g_last_answer < std::chrono::seconds(6);

    setState(answering ? PTCC_SERVICE_ALIVE : PTCC_SERVICE_CONNECTED,
             answering ? "service answers on port " + std::to_string(PTCC_SERVICE_PORT)
                       : "connected, waiting for an answer");
}

auto ptccServiceSend(const char *key, float value) -> bool {
    return g_client && g_client->isConnected() ? g_client->send(key, value) : false;
}

auto ptccServiceSend(const char *key, int value) -> bool {
    return g_client && g_client->isConnected() ? g_client->send(key, value) : false;
}
