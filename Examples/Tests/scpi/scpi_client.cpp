#include "scpi_client.h"
#include "socket.h"

auto scpi_client::open(const std::string &ip, const unsigned short port) -> bool {
  if (m_is_opened) return false;
  auto result = createClient(&m_socket, ip.c_str(), 0, port);
  if (result < 0) return false;
  m_is_opened = true;
  return true;
}

auto scpi_client::write(const std::string &data) const -> State {
    if (!m_is_opened) return State::ERR_WAS_OPEN;
    std::string write_data{data};
    write_data.push_back('\r');
    write_data.push_back('\n');
    return write_tcp(write_data);
}

auto scpi_client::read(std::string &data, const unsigned int timeouts_ms) const -> State {
    if (!m_is_opened) return State::ERR_WAS_OPEN;
    if (!data.empty()) data.clear();
    return read_tcp(data, timeouts_ms);
}

auto scpi_client::close() -> int {
    if (m_is_opened && m_socket) {
        if (destroySock(m_socket) < 0) return -1;
        m_is_opened = false;
        m_socket = 0;
        return 0;
    }
    return -1;
}

bool scpi_client::is_opened() const noexcept { return m_is_opened; }


auto scpi_client::write_tcp(const std::string &data) const -> State {
  auto result = sendTcp(m_socket, data.c_str(), data.length(), 0);
  if (result < static_cast<int>(data.size())) return State::ERROR;
  return State::OK;
}

auto scpi_client::read_tcp(std::string &data, const unsigned int timeouts_ms) const -> State {
    char ch{'\0'};
    auto count = _read_line_max;
    do {
        auto result = recvTcp(m_socket, &ch, sizeof(char), timeouts_ms);
        if (result == -2)
            return State::TIMEOUT;
        if (result == -1)
            return State::ERROR;

        if (ch == '\n') break;
        data.push_back(ch);
    } while (--count);
    if (count == 0) return State::ERR_BUFFER_IS_FULL;
    return State::OK;
}
