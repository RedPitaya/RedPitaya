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

}  // namespace rp_websocket