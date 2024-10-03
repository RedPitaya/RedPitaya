#include "rp.h"
#include "websocket_server.h"

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

websocket_server::websocket_server() {
	m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
	// m_endpoint.set_access_channels(websocketpp::log::alevel::connect);
	// m_endpoint.set_access_channels(websocketpp::log::alevel::disconnect);
	// m_endpoint.set_access_channels(websocketpp::log::alevel::app);
    m_endpoint.init_asio();
    m_endpoint.set_open_handler(bind(&websocket_server::on_open,this,::_1));
    m_endpoint.set_close_handler(bind(&websocket_server::on_close,this,::_1));
    m_endpoint.set_message_handler(bind(&websocket_server::on_message,this,::_1,::_2));
	m_isRun = false;
	sem_init(&m_runSem, 0, 1);
}


websocket_server::~websocket_server() {
	receiveHandle.disconnect_all();
    stop();
}

void websocket_server::run(uint16_t port) {
	TRACE_SHORT("run websocket server")
	m_endpoint.set_reuse_addr(true);
	m_endpoint.listen(boost::asio::ip::tcp::v4(), port);
	try {
		m_endpoint.start_accept();
		m_isRun = true;
		sem_post(&m_runSem);
		m_endpoint.run();
		m_isRun = false;
	} catch (websocketpp::exception const & e) {
		ERROR("%s",e.what())
	}
	sem_post(&m_runSem);
}

void websocket_server::on_open(connection_hdl hdl) {
	TRACE_SHORT("Client connected")
	m_connections.insert(hdl);
}

void websocket_server::on_close(connection_hdl hdl) {
	TRACE_SHORT("Client disconnected")
	m_connections.erase(hdl);
}

void websocket_server::on_message(connection_hdl hdl, server::message_ptr msg) {
	receiveHandle(msg->get_payload());
}

auto websocket_server::send(const char *buffer, size_t size) -> bool{
	if (!m_isRun) return false;
	try{
		if (size) {
			for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
				m_endpoint.send(*it, buffer, size, websocketpp::frame::opcode::binary);
			}
			return true;
		}
	}catch(websocketpp::lib::error_code ec){
		ERROR("%s",ec.message().c_str())
    }
	return false;
}

auto websocket_server::start(uint16_t port) -> void {
	stop();
    std::lock_guard lock(m_mutex);
	m_thread = std::thread(&websocket_server::run,this, port);
	if (sem_wait(&m_runSem)){
		FATAL("Can't lock semaphore")
	}
}

auto websocket_server::stop() -> void {
    std::lock_guard lock(m_mutex);
	if (!m_isRun) return;
	try{
		m_endpoint.stop_listening();
	} catch (websocketpp::exception const & e) {
		ERROR("%s",e.what())
	}
	for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
		connection_hdl hdl = *it;
		try{
            m_endpoint.close(hdl, websocketpp::close::status::normal, "shutdown");
        }catch(websocketpp::lib::error_code ec){
			ERROR("%s",ec.message().c_str())
        }
	}
	try{
		m_connections.clear();
		m_endpoint.stop();
    } catch (websocketpp::exception const & e) {
		ERROR("%s",e.what())
	}
	if (m_thread.joinable())
    	m_thread.join();
}

auto websocket_server::isRun() -> bool{
	return m_isRun;
}

