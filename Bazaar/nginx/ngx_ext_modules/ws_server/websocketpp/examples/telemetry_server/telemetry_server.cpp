#include <websocketpp/config/asio_no_tls.hpp>

#include <websocketpp/server.hpp>

#include <json/json.h>

#include <fstream>
#include <iostream>
#include <set>
#include <streambuf>
#include <string>

/**
 * The telemetry server accepts connections and sends a message every second to
 * each client containing an integer count. This example can be used as the
 * basis for programs that expose a stream of telemetry data for logging,
 * dashboards, etc.
 *
 * This example uses the timer based concurrency method and is self contained
 * and singled threaded. Refer to telemetry client for an example of a similar
 * telemetry setup using threads rather than timers.
 *
 * This example also includes an example simple HTTP server that serves a web
 * dashboard displaying the count. This simple design is suitable for use 
 * delivering a small number of files to a small number of clients. It is ideal
 * for cases like embedded dashboards that don't want the complexity of an extra
 * HTTP server to serve static files.
 *
 * This design *will* fall over under high traffic or DoS conditions. In such
 * cases you are much better off proxying to a real HTTP server for the http
 * requests.
 */
class telemetry_server {
public:
    typedef websocketpp::connection_hdl connection_hdl;
    typedef websocketpp::server<websocketpp::config::asio> server;
    typedef websocketpp::lib::lock_guard<websocketpp::lib::mutex> scoped_lock;

    telemetry_server() : m_count(0) {
        // set up access channels to only log interesting things
        m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
        m_endpoint.set_access_channels(websocketpp::log::alevel::access_core);
        m_endpoint.set_access_channels(websocketpp::log::alevel::app);

        // Initialize the Asio transport policy
        m_endpoint.init_asio();

        // Bind the handlers we are using
        using websocketpp::lib::placeholders::_1;
        using websocketpp::lib::bind;
        m_endpoint.set_open_handler(bind(&telemetry_server::on_open,this,::_1));
        m_endpoint.set_close_handler(bind(&telemetry_server::on_close,this,::_1));
        m_endpoint.set_http_handler(bind(&telemetry_server::on_http,this,::_1));
	m_endpoint.set_message_handler(bind(&telemetry_server::on_message,this,::_1,::_2));

    }

    void run(std::string docroot, uint16_t port) {
        std::stringstream ss;
        ss << "Running telemetry server on port "<< port <<" using docroot=" << docroot;
        m_endpoint.get_alog().write(websocketpp::log::alevel::app,ss.str());
        
        m_docroot = docroot;
        
        // listen on specified port
        m_endpoint.listen(port);

        // Start the server accept loop
        m_endpoint.start_accept();

        // Set the initial timer to start telemetry
        set_timer();

        // Start the ASIO io_service run loop
        try {
            m_endpoint.run();
        } catch (websocketpp::exception const & e) {
            std::cout << e.what() << std::endl;
        }
    }

    void set_timer() {
        m_timer = m_endpoint.set_timer(
            1000,
            websocketpp::lib::bind(
                &telemetry_server::on_timer,
                this,
                websocketpp::lib::placeholders::_1
            )
        );
    }

    void on_timer(websocketpp::lib::error_code const & ec) {
        if (ec) {
            // there was an error, stop telemetry
            m_endpoint.get_alog().write(websocketpp::log::alevel::app,
                    "Timer Error: "+ec.message());
            return;
        }
        
        std::stringstream val;
        val << "count is " << m_count++;
        
        // Broadcast count to all connections
        con_list::iterator it;

	//std::string jc = (std::string) send_obj.write();
	//std::cout << "TXD: " << jc <<std::endl;
	

        for (it = m_connections.begin(); it != m_connections.end(); ++it) {
            m_endpoint.send(*it,val.str(),websocketpp::frame::opcode::text);
//m_endpoint.send(*it, js.str(),websocketpp::frame::opcode::text);
        }
        
        // set timer for next telemetry check
        set_timer();
    }

    void on_http(connection_hdl hdl) {
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

    void on_open(connection_hdl hdl) {
        m_connections.insert(hdl);
    }

    void on_close(connection_hdl hdl) {
        m_connections.erase(hdl);
    }

	void on_message(connection_hdl hdl, server::message_ptr msg) {
 		std::cout << "Detected " << msg->get_payload() << " test cases."
                  << std::endl;
	std::cout << "Detected " << msg->get_payload() << " test cases."
                  << std::endl;

	Json::Value root;
Json::Reader reader;
std::string json_example = msg->get_payload();
bool parsedSuccess = reader.parse(json_example,root,false);
//parce json input
	//Json::Value n = libjson::parse(msg->get_payload());
	//std::string url1 = n.at("url1").as_string();
	//std::string url2 = n.at("url2").as_string();
//send json object
	//send_obj(JSON_NODE);
	//send_obj.push_back(JSONNode("url1", url1));
	//send_obj.push_back(JSONNode("url2", url2));
	
    }


private:
    typedef std::set<connection_hdl,std::owner_less<connection_hdl>> con_list;
    
    server m_endpoint;
    con_list m_connections;
    server::timer_ptr m_timer;
    
    std::string m_docroot;
    //JSONNode send_obj;
    // Telemetry data
    uint64_t m_count;
};

int main(int argc, char* argv[]) {
    telemetry_server s;

    std::string docroot;
    uint16_t port = 9002;

    if (argc == 1) {
        std::cout << "Usage: telemetry_server [documentroot] [port]" << std::endl;
        return 1;
    }
    
    if (argc >= 2) {
        docroot = std::string(argv[1]);
    }
        
    if (argc >= 3) {
        int i = atoi(argv[2]);
        if (i <= 0 || i > 65535) {
            std::cout << "invalid port" << std::endl;
            return 1;
        }
        
        port = uint16_t(i);
    }

    s.run(docroot, port);
    return 0;
}
