#pragma once
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/common/thread.hpp>
//#include <websocketpp/extensions/permessage_deflate/enabled.hpp>
#include <set>
#include <fstream>

#include "libjson/_internal/Source/JSONNode.h"
#include "ws_server.h"

//class config2{};

extern "C"{

class rp_websocket_server {
public:
    typedef websocketpp::connection_hdl connection_hdl;
    typedef websocketpp::server<websocketpp::config::asio> server;
    typedef websocketpp::lib::lock_guard<websocketpp::lib::mutex> scoped_lock;

    rp_websocket_server();
    rp_websocket_server(struct server_parameters* params);
    ~rp_websocket_server();

    static rp_websocket_server* create(struct server_parameters* params);

    void run(std::string docroot, uint16_t port);

    void start(std::string docroot, uint16_t port);
    void join();
    void stop();

    void set_signal_timer();
    void set_param_timer();

    void on_signal_timer(websocketpp::lib::error_code const & ec);
    void on_param_timer(websocketpp::lib::error_code const & ec);
    void on_http(connection_hdl hdl);
    void on_open(connection_hdl hdl);
    void on_close(connection_hdl hdl);
    void on_message(connection_hdl hdl, server::message_ptr msg);

private:
    typedef std::set<connection_hdl,std::owner_less<connection_hdl>> con_list;

    struct server_parameters* m_params;
    server m_endpoint;
    con_list m_connections;
    server::timer_ptr m_signal_timer;
    server::timer_ptr m_param_timer;
    websocketpp::lib::thread m_thread;
    std::string m_docroot;
	std::ofstream m_out;
	volatile bool m_OnClosed;
};

}
