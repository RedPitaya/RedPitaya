#pragma once

#include <string>

class scpi_client {
    public:
        enum State{
            OK,
            TIMEOUT,
            ERROR,
            ERR_BUFFER_IS_FULL,
            ERR_WAS_OPEN
        };

        scpi_client() = default;
        auto open(const std::string &addr, const unsigned short port) -> bool;
        auto write(const std::string &data) const -> State;
        auto read(std::string &data, const unsigned int timeouts_ms) const -> State;
        auto close() -> int;
        bool is_opened() const noexcept;

    private:
        scpi_client(const scpi_client &) = delete;
        scpi_client &operator=(const scpi_client &) = delete;

        const unsigned short _read_line_max = {1024};
        auto write_tcp(const std::string &data) const -> State;
        auto read_tcp(std::string &data, const unsigned int timeouts_ms) const -> State;

        bool m_is_opened;
        int m_socket;
};
