/**
 * $Id$
 *
 * @brief Red Pitaya Web module
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

#pragma once

#include "signal.hpp"

namespace rp_websocket {

class CWEBServer {

   public:
    using Ptr = std::shared_ptr<CWEBServer>;

    CWEBServer();
    ~CWEBServer();

    auto startServer(uint16_t port) -> void;
    auto startServerBinaray(uint16_t port) -> void;

    auto send(std::string_view key, bool value) -> bool;
    auto send(std::string_view key, int value) -> bool;
    auto send(std::string_view key, uint32_t value) -> bool;
    auto send(std::string_view key, float value) -> bool;
    auto send(std::string_view key, std::string_view value) -> bool;
    auto send(std::string_view json) -> bool;
    auto sendInBinarayMode(const char* data, size_t size) -> bool;

    auto sendRequest(std::string_view key, bool value, bool reset_cache = false) -> void;
    auto sendRequest(std::string_view key, int value, bool reset_cache = false) -> void;
    auto sendRequest(std::string_view key, uint32_t value, bool reset_cache = false) -> void;
    auto sendRequest(std::string_view key, float value, bool reset_cache = false) -> void;
    auto sendRequest(std::string_view key, std::string_view value, bool reset_cache = false) -> void;
    auto sendCache() -> bool;

    sigslot::signal<const std::string_view, const bool> receiveBool;
    sigslot::signal<const std::string_view, const int> receiveInt;
    sigslot::signal<const std::string_view, const uint32_t> receiveUInt;
    sigslot::signal<const std::string_view, const double> receiveDouble;
    sigslot::signal<const std::string_view, const std::string_view> receiveStr;

   private:
    CWEBServer(const CWEBServer&) = delete;
    CWEBServer(CWEBServer&&) = delete;
    CWEBServer& operator=(const CWEBServer&) = delete;
    CWEBServer& operator=(const CWEBServer&&) = delete;

    auto resetCache() -> void;

    class Impl;
    // Pointer to the internal implementation
    Impl* m_pimpl;
};

/**
 * Client side of the same protocol.
 *
 * An application that talks to a background service on the board links this
 * instead of hosting the hardware access itself: the service owns the device,
 * the application owns the page. Messages and signals are the ones CWEBServer
 * uses, so both ends speak the same {"key": {"type", "value"}} objects.
 */
class CWEBClient {

   public:
    using Ptr = std::shared_ptr<CWEBClient>;

    CWEBClient();
    ~CWEBClient();

    /** Starts connecting to a server, usually on 127.0.0.1. False means the
     *  address itself is wrong; a server that is not up yet simply never
     *  raises connected. */
    auto start(std::string_view host, uint16_t port) -> bool;
    auto stop() -> void;
    auto isConnected() -> bool;

    auto send(std::string_view key, bool value) -> bool;
    auto send(std::string_view key, int value) -> bool;
    auto send(std::string_view key, uint32_t value) -> bool;
    auto send(std::string_view key, float value) -> bool;
    auto send(std::string_view key, std::string_view value) -> bool;
    auto send(std::string_view json) -> bool;

    /** Collects values and sends them as one message, like the server side. */
    auto sendRequest(std::string_view key, bool value, bool reset_cache = false) -> void;
    auto sendRequest(std::string_view key, int value, bool reset_cache = false) -> void;
    auto sendRequest(std::string_view key, uint32_t value, bool reset_cache = false) -> void;
    auto sendRequest(std::string_view key, float value, bool reset_cache = false) -> void;
    auto sendRequest(std::string_view key, std::string_view value, bool reset_cache = false) -> void;
    auto sendCache() -> bool;

    sigslot::signal<const std::string_view, const bool> receiveBool;
    sigslot::signal<const std::string_view, const int> receiveInt;
    sigslot::signal<const std::string_view, const uint32_t> receiveUInt;
    sigslot::signal<const std::string_view, const double> receiveDouble;
    sigslot::signal<const std::string_view, const std::string_view> receiveStr;

    /** Raised from the client thread when the link comes up or goes down. */
    sigslot::signal<> connected;
    sigslot::signal<> disconnected;

   private:
    CWEBClient(const CWEBClient&) = delete;
    CWEBClient(CWEBClient&&) = delete;
    CWEBClient& operator=(const CWEBClient&) = delete;
    CWEBClient& operator=(const CWEBClient&&) = delete;

    auto resetCache() -> void;

    class Impl;
    Impl* m_pimpl;
};

}  // namespace rp_websocket