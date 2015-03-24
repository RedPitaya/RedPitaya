#include "rp_websocket_server.h"

#include <websocketpp/common/thread.hpp>

#include "libjson/libjson.h"
#include "libjson/_internal/Source/JSONGlobals.h"
#include "libjson/JSONOptions.h"

#include <fstream>
#include <iostream>
#include <streambuf>
#include <string>

#include <math.h>

using websocketpp::lib::thread;

rp_websocket_server::rp_websocket_server()
    : m_params(NULL)
{
}

rp_websocket_server::rp_websocket_server(const struct server_parameters* params)
    : m_params(params) 
{
    // set up access channels to only log interesting things
    m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
    m_endpoint.set_access_channels(websocketpp::log::alevel::access_core);
    m_endpoint.set_access_channels(websocketpp::log::alevel::app);

    // Initialize the Asio transport policy
    m_endpoint.init_asio();

    // Bind the handlers we are using
    using websocketpp::lib::placeholders::_1;
    using websocketpp::lib::bind;
    m_endpoint.set_open_handler(bind(&rp_websocket_server::on_open,this,::_1));
    m_endpoint.set_close_handler(bind(&rp_websocket_server::on_close,this,::_1));
    m_endpoint.set_http_handler(bind(&rp_websocket_server::on_http,this,::_1));
    m_endpoint.set_message_handler(bind(&rp_websocket_server::on_message,this,::_1,::_2));
    m_out.open("ws_server.log");
    m_endpoint.get_alog().set_ostream(&m_out);
    m_endpoint.get_alog().write(websocketpp::log::alevel::app, "ws_server constructor");
}

rp_websocket_server::~rp_websocket_server()
{
    if(m_params)
    {
	free(m_params);
	m_params = NULL;
    }
}

void rp_websocket_server::run(std::string docroot, uint16_t port) {
	m_endpoint.get_alog().write(websocketpp::log::alevel::app,"run");
	std::stringstream ss;
	ss << "Running telemetry server on port "<< port <<" using docroot=" << docroot;
	m_endpoint.get_alog().write(websocketpp::log::alevel::app,ss.str());
	m_docroot = docroot;
	
	// listen on specified port
	m_endpoint.listen(boost::asio::ip::tcp::v4(), port);

	// Start the server accept loop
	m_endpoint.start_accept();

	// Start the ASIO io_service run loop
	try {
		m_endpoint.run();
	} catch (websocketpp::exception const & e) {
		std::cout << e.what() << std::endl;
		m_endpoint.get_alog().write(websocketpp::log::alevel::app, e.what());
	}
}

void rp_websocket_server::set_signal_timer() {
	
	int interval = m_params->get_signals_interval_func != 0 ?  m_params->get_signals_interval_func() : m_params->signal_interval;
	m_signal_timer = m_endpoint.set_timer(
		interval,
		websocketpp::lib::bind(
			&rp_websocket_server::on_signal_timer,
			this,
			websocketpp::lib::placeholders::_1
		)
	);
}

void rp_websocket_server::set_param_timer() {
	
	int interval = m_params->get_params_interval_func != 0 ?  m_params->get_params_interval_func() : m_params->param_interval;
	m_param_timer = m_endpoint.set_timer(
		interval,
		websocketpp::lib::bind(
			&rp_websocket_server::on_param_timer,
			this,
			websocketpp::lib::placeholders::_1
		)
	);
}

void rp_websocket_server::on_signal_timer(websocketpp::lib::error_code const & ec) {
	if (ec) {
		m_endpoint.get_alog().write(websocketpp::log::alevel::app,
				"Timer Error: "+ec.message());
		return;
	}
	
	con_list::iterator it;
	const char* signals = m_params->get_signals_func();

	static int once = 1;
	if(once)
	{
		once = 0;
		m_endpoint.get_alog().write(websocketpp::log::alevel::app, "on_signal_timer");
		m_endpoint.get_alog().write(websocketpp::log::alevel::app, signals);
	}

	std::string js(signals);
	for (it = m_connections.begin(); it != m_connections.end(); ++it) {
		m_endpoint.send(*it, js, websocketpp::frame::opcode::text);
	}
	
	// set timer for next check
	set_signal_timer();
}

void rp_websocket_server::on_param_timer(websocketpp::lib::error_code const & ec) {
	if (ec) {
		m_endpoint.get_alog().write(websocketpp::log::alevel::app,
				"Timer Error: "+ec.message());
		return;
	}
	
	con_list::iterator it;
	const char* params = m_params->get_params_func();

	static int once = 1;
	if(once)
	{
		once = 0;
		m_endpoint.get_alog().write(websocketpp::log::alevel::app, "on_param_timer");
		m_endpoint.get_alog().write(websocketpp::log::alevel::app, params);
	}

	std::string js(params);
	for (it = m_connections.begin(); it != m_connections.end(); ++it) {
		m_endpoint.send(*it, js, websocketpp::frame::opcode::text);
	}
	
	// set timer for next check
	set_param_timer();
}

void rp_websocket_server::on_http(connection_hdl hdl) {
	// Upgrade our connection handle to a full connection_ptr
	server::connection_ptr con = m_endpoint.get_con_from_hdl(hdl);

	std::ifstream file;
	std::string filename = con->get_uri()->get_resource();
	std::string response;

	m_endpoint.get_alog().write(websocketpp::log::alevel::app,
		"http request1: "+filename);

	if (filename == "/") {
		filename = m_docroot+"index.html";
	} else {
		filename = m_docroot+filename.substr(1);
	}
	
	m_endpoint.get_alog().write(websocketpp::log::alevel::app,
		"http request2: "+filename);

	file.open(filename.c_str(), std::ios::in);
	if (!file) {
		// 404 error
		std::stringstream ss;
	
		ss << "<!doctype html><html><head>"
		   << "<title>Error 404 (Resource not found)</title><body>"
		   << "<h1>Error 404</h1>"
		   << "<p>The requested URL " << filename << " was not found on this server.</p>"
		   << "</body></head></html>";
	
		con->set_body(ss.str());
		con->set_status(websocketpp::http::status_code::not_found);
		return;
	}

	file.seekg(0, std::ios::end);
	response.reserve(file.tellg());
	file.seekg(0, std::ios::beg);

	response.assign((std::istreambuf_iterator<char>(file)),
					std::istreambuf_iterator<char>());

	con->set_body(response);
	con->set_status(websocketpp::http::status_code::ok);
}

void rp_websocket_server::on_open(connection_hdl hdl) 
{
	m_endpoint.get_alog().write(websocketpp::log::alevel::app, "ws server on connection");
	m_connections.insert(hdl);
}

void rp_websocket_server::on_close(connection_hdl hdl) {
	m_connections.erase(hdl);
}

void rp_websocket_server::on_message(connection_hdl hdl, server::message_ptr msg) {
	std::stringstream ss;	
	ss << "Detected " << msg->get_payload() << " test cases.";
	m_endpoint.get_alog().write(websocketpp::log::alevel::app,ss.str());
	
	JSONNode n = libjson::parse(msg->get_payload());
	
	std::string name = n.name();
	if(name == "parameters")
	{
		set_param_timer();
		m_params->set_params_func((msg->get_payload()).c_str());
	}
	
	if(name == "signals")
	{
		set_signal_timer();
		m_params->set_signals_func((msg->get_payload()).c_str());
	}

}

rp_websocket_server* rp_websocket_server::create(const struct server_parameters* params) {
  return new rp_websocket_server(params);
}

void rp_websocket_server::start(std::string docroot, uint16_t port)
{
	m_endpoint.get_alog().write(websocketpp::log::alevel::app, "start ws_server");
	m_thread = thread(bind(&rp_websocket_server::run,this, docroot,  port));
	set_signal_timer();
	set_param_timer();
}

void rp_websocket_server::join()
{
	m_thread.join();
} 

void rp_websocket_server::stop()
{	
	m_endpoint.get_alog().write(websocketpp::log::alevel::app, "stop ws_server");
	m_endpoint.stop();
	m_out.close();
	join();
}
