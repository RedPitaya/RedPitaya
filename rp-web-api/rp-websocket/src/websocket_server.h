#pragma once
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <set>
#include <mutex>
#include <thread>
#include <mutex>
#include <semaphore.h>
#include "signal.hpp"

class websocket_server {
public:
    typedef websocketpp::connection_hdl connection_hdl;
    typedef websocketpp::server<websocketpp::config::asio> server;

    websocket_server();
    ~websocket_server();


    auto start(uint16_t port) -> void;
    auto stop() -> void;
    auto isRun() -> bool;

    auto send(const char *buffer,size_t size) -> bool;
  	sigslot::signal<const std::string&> receiveHandle;

private:
    typedef std::set<connection_hdl,std::owner_less<connection_hdl>> con_list;

    auto run(uint16_t port) -> void;

    void on_open(connection_hdl hdl);
    void on_close(connection_hdl hdl);
    void on_message(connection_hdl hdl, server::message_ptr msg);

    server m_endpoint;
    con_list m_connections;
    std::thread m_thread;
    std::mutex m_mutex;
    sem_t m_runSem;
    bool m_isRun;
};
