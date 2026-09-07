#pragma once

#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include <chrono>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

// PTCC_EMULATOR and PYTHON3_EXECUTABLE are injected by tests/CMakeLists.txt.
// The emulator opens a pty, prints the slave path and then answers PTCC
// requests using the upstream Python library.

class EmulatedDevice {
   public:
    explicit EmulatedDevice(const std::string& extraArgs = "") {
        const std::string command =
            std::string(PYTHON3_EXECUTABLE) + " -u " + PTCC_EMULATOR + " " + extraArgs + " 2>/dev/null";

        m_pipe = popen(command.c_str(), "r");
        if (m_pipe == nullptr) {
            throw std::runtime_error("cannot start the PTCC emulator");
        }

        char buffer[256] = {0};
        if (fgets(buffer, sizeof(buffer), m_pipe) == nullptr) {
            pclose(m_pipe);
            m_pipe = nullptr;
            throw std::runtime_error("the PTCC emulator did not report a pty");
        }

        m_port = trim(buffer);

        if (fgets(buffer, sizeof(buffer), m_pipe) == nullptr) {
            pclose(m_pipe);
            m_pipe = nullptr;
            throw std::runtime_error("the PTCC emulator did not report its pid");
        }
        m_pid = std::stoi(trim(buffer));
    }

    // The emulator holds the slave end of the pty open, so closing the port
    // never gives it EOF: it has to be terminated explicitly, otherwise
    // pclose() would block until the test binary is killed.
    ~EmulatedDevice() {
        if (m_pid > 0) {
            kill(m_pid, SIGTERM);
        }
        if (m_pipe != nullptr) {
            pclose(m_pipe);
        }
    }

    EmulatedDevice(const EmulatedDevice&) = delete;
    EmulatedDevice& operator=(const EmulatedDevice&) = delete;

    const std::string& port() const { return m_port; }

   private:
    static std::string trim(const char* text) {
        std::string value(text);
        while (!value.empty() && (value.back() == '\n' || value.back() == '\r')) {
            value.pop_back();
        }
        return value;
    }

    FILE* m_pipe = nullptr;
    std::string m_port;
    pid_t m_pid = -1;
};
