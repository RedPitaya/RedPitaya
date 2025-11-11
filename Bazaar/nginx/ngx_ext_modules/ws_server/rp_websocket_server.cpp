#include "rp_websocket_server.h"

#include <websocketpp/common/thread.hpp>
#include <websocketpp/message_buffer/message.hpp>

#include "libjson/JSONOptions.h"
#include "libjson/_internal/Source/JSONGlobals.h"
#include "libjson/libjson.h"

#include <math.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <future>
#include <iostream>
#include <streambuf>
#include <string>

using websocketpp::lib::bind;
using websocketpp::lib::thread;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;

#define MAX_BUFFER_SIZE 1024 * 1024 * 32

enum BinarySignalType { UNDEFINED = 0, INT8 = 1, INT16 = 2, INT32 = 3, UINT8 = 4, UINT16 = 5, UINT32 = 6, FLOAT = 7, DOUBLE = 8 };

struct BinarySignal {
    std::string name = {};
    BinarySignalType type = UNDEFINED;
    size_t byteSize = 0;
    const void* data_vector = NULL;
};

// Creates the directory and subdirectories if needed.
auto rp_WSCreateDirectory(const std::string& _path) -> bool {
    auto isDirectory = [](const std::string& _path) -> bool {
        struct stat st;

        if (stat(_path.c_str(), &st) == 0) {
            return st.st_mode & S_IFDIR;
        }

        return false;
    };

    size_t pos = 0;

    for (;;) {
        pos = _path.find('/', pos);

        if (pos == std::string::npos) {
            // Create the last directory
            if (!isDirectory(_path.c_str())) {
                int mkdir_err = mkdir(_path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                return (mkdir_err == 0) || (mkdir_err == EEXIST);
            } else {
                return true;
            }
        } else {
            ++pos;
            std::string sub_path = _path.substr(0, pos);

            // Create subdirectory
            if (!isDirectory(sub_path.c_str())) {
                int mkdir_err = mkdir(sub_path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

                if (!((mkdir_err == 0) || (mkdir_err == EEXIST))) {
                    return false;
                }
            }

            if (pos >= _path.size()) {
                return true;
            }
        }
    }

    return false;
}

rp_websocket_server::rp_websocket_server() : m_params(NULL), m_OnClosed(false) {}

rp_websocket_server::rp_websocket_server(struct server_parameters* params) : m_params(params) {
    // set up access channels to only log interesting things
    m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
    m_endpoint.set_access_channels(websocketpp::log::alevel::access_core);
    m_endpoint.set_access_channels(websocketpp::log::alevel::app);

    // Initialize the Asio transport policy
    m_endpoint.init_asio();

    // Bind the handlers we are using
    using websocketpp::lib::bind;
    using websocketpp::lib::placeholders::_1;
    m_endpoint.set_open_handler(bind(&rp_websocket_server::on_open, this, ::_1));
    m_endpoint.set_close_handler(bind(&rp_websocket_server::on_close, this, ::_1));
    m_endpoint.set_http_handler(bind(&rp_websocket_server::on_http, this, ::_1));
    m_endpoint.set_message_handler(bind(&rp_websocket_server::on_message, this, ::_1, ::_2));
    if (params->enable_ws_log) {
        rp_WSCreateDirectory("/var/log/redpitaya_nginx");
        m_out.open("/var/log/redpitaya_nginx/ws_server.log", std::ofstream::out | std::ofstream::app);
        m_endpoint.get_alog().set_ostream(&m_out);
    }
    m_endpoint.get_alog().write(websocketpp::log::alevel::app, "ws_server constructor");

    std::stringstream ss;
    ss << "default params: signal_interval = " << params->signal_interval << ", param_interval =" << params->param_interval;
    m_endpoint.get_alog().write(websocketpp::log::alevel::app, ss.str());
    m_OnClosed = true;
}

rp_websocket_server::~rp_websocket_server() {
    if (m_params) {
        free(m_params);
        m_params = NULL;
    }
}

void rp_websocket_server::run(std::string docroot, uint16_t port) {
    m_endpoint.get_alog().write(websocketpp::log::alevel::app, "run");
    std::stringstream ss;
    ss << "Running telemetry server on port " << port << " using docroot=" << docroot;
    m_endpoint.get_alog().write(websocketpp::log::alevel::app, ss.str());
    m_docroot = docroot;

    m_endpoint.set_reuse_addr(true);
    // listen on specified port
    m_endpoint.listen(boost::asio::ip::tcp::v4(), port);
    // m_endpoint.listen(port);
    // Start the server accept loop
    m_endpoint.start_accept();
    // Start the ASIO io_service run loop
    try {
        m_OnClosed = false;
        m_endpoint.run();
    } catch (websocketpp::exception const& e) {
        std::cout << e.what() << std::endl;
        m_endpoint.get_alog().write(websocketpp::log::alevel::app, e.what());
    }
}

void rp_websocket_server::set_signal_timer() {

    if (m_signal_timer != NULL)
        m_signal_timer->cancel();
    int interval = m_params->get_signals_interval_func != 0 ? m_params->get_signals_interval_func() : m_params->signal_interval;
    // fprintf(stderr, "set_signal_timer interval %d\n", interval);
    m_signal_timer = m_endpoint.set_timer(interval, websocketpp::lib::bind(&rp_websocket_server::on_signal_timer, this, websocketpp::lib::placeholders::_1));
}

void rp_websocket_server::set_param_timer() {

    if (m_param_timer != NULL)
        m_param_timer->cancel();
    int interval = m_params->get_params_interval_func != 0 ? m_params->get_params_interval_func() : m_params->param_interval;
    // fprintf(stderr, "set_param_timer interval %d\n", interval);
    m_param_timer = m_endpoint.set_timer(interval, websocketpp::lib::bind(&rp_websocket_server::on_param_timer, this, websocketpp::lib::placeholders::_1));
}

void rp_websocket_server::on_signal_timer(websocketpp::lib::error_code const& ec) {

    if (ec) {
        m_endpoint.get_alog().write(websocketpp::log::alevel::app, "Signal timer Error: " + ec.message());
        return;
    }
    // If the client does not have time to receive all the data, then we skip the next send until the buffer is cleared.
    for (con_list::iterator it = m_connections.begin(); it != m_connections.end(); ++it) {
        server::connection_ptr con = m_endpoint.get_con_from_hdl(*it);
        if (con->get_buffered_amount() > MAX_BUFFER_SIZE) {
            set_signal_timer();
            return;
        }
    }

    auto sendSignals = [&]() {
        con_list::iterator it;
        const char* signals = m_params->get_signals_func();
        if (strlen(signals) == 0)
            return;
        std::string js(signals);
        static std::vector<uint8_t> buffer;
        buffer.clear();
        size_t size;
        const void* dataSend = NULL;
        std::string prefix = "";
        if (m_params->gzip_func(1, js.c_str(), &buffer, &size)) {
            dataSend = js.c_str();
            size = js.length();
            prefix = "NZIA";
        } else {
            dataSend = buffer.data();
            prefix = "EZIA";
        }
        for (it = m_connections.begin(); it != m_connections.end(); ++it) {
            server::connection_ptr con = m_endpoint.get_con_from_hdl(*it);
            auto msg = con->get_message(websocketpp::frame::opcode::binary, 0);
            msg->set_payload("");
            msg->append_payload(prefix);
            msg->append_payload(dataSend, size);
            m_endpoint.send(*it, msg);
        }
    };

    auto sendBinarySignals = [&]() {
        // Header structure with prefix included and 64-byte alignment
        static std::vector<uint8_t> zeroData(64, 0);
        struct BaseHeader {
            char prefix[4];  // Exactly 4 bytes for prefix, no null terminator
            uint32_t dataType = 0;
            uint32_t nameSize = 0;
            uint32_t dataSize = 0;
            uint32_t dataSizeExtra = 0;
            uint32_t headerSize = 0;  // Actual total header size including name
        };

        con_list::iterator it;
        const void* signals = m_params->get_bin_signals_func();
        if (signals == NULL)
            return;

        static std::vector<uint8_t> buffer;
        auto csb = static_cast<const std::vector<BinarySignal>*>(signals);
        for (it = m_connections.begin(); it != m_connections.end(); ++it) {
            server::connection_ptr con = m_endpoint.get_con_from_hdl(*it);
            auto msg = con->get_message(websocketpp::frame::opcode::binary, 0);
            msg->set_payload("");
            bool needSend = false;
            for (auto& item : *csb) {
                buffer.clear();
                std::string prefix = "";
                size_t size = item.byteSize;
                const void* dataSend = NULL;
                if (m_params->gzip_func(2, item.data_vector, &buffer, &size)) {
                    // Without zip
                    dataSend = item.data_vector;
                    prefix = "NZIB";
                } else {
                    dataSend = buffer.data();
                    prefix = "EZIB";
                }

                // Calculate actual header size including prefix and name
                size_t nameLength = item.name.length();
                size_t baseHeaderSize = sizeof(BaseHeader);

                // Total header size includes base header + name
                size_t totalHeaderSize = baseHeaderSize + nameLength;

                // Round up to nearest multiple of 64
                size_t actualHeaderSize = ((totalHeaderSize + 63) / 64) * 64;

                // Prepare complete header buffer
                std::vector<uint8_t> headerBuffer(actualHeaderSize, 0);
                // Create and populate base header directly in the buffer
                BaseHeader* baseHeader = reinterpret_cast<BaseHeader*>(headerBuffer.data());
                memcpy(baseHeader->prefix, prefix.c_str(), 4);
                baseHeader->dataType = item.type;
                baseHeader->nameSize = static_cast<uint32_t>(nameLength);
                baseHeader->dataSize = static_cast<uint32_t>(size);
                baseHeader->dataSizeExtra = static_cast<uint32_t>(size % 64 ? 64 - size % 64 : 0);
                baseHeader->headerSize = static_cast<uint32_t>(actualHeaderSize);
                // Copy name directly after base header
                if (nameLength > 0) {
                    memcpy(headerBuffer.data() + sizeof(BaseHeader), item.name.c_str(), nameLength);
                }

                // Build message - header buffer (with prefix and name) and data
                msg->append_payload(headerBuffer.data(), actualHeaderSize);
                msg->append_payload(dataSend, size);
                msg->append_payload(zeroData.data(), baseHeader->dataSizeExtra);
                needSend = true;
            }
            if (needSend) {
                m_endpoint.send(*it, msg);
            }
        }
    };

    sendSignals();
    sendBinarySignals();
    // set timer for next check
    set_signal_timer();
}

void rp_websocket_server::on_param_timer(websocketpp::lib::error_code const& ec) {

    if (ec) {
        m_endpoint.get_alog().write(websocketpp::log::alevel::app, "Param timer Error: " + ec.message());
        return;
    }
    // If the client does not have time to receive all the data, then we skip the next send until the buffer is cleared.
    for (con_list::iterator it = m_connections.begin(); it != m_connections.end(); ++it) {
        server::connection_ptr con = m_endpoint.get_con_from_hdl(*it);
        if (con->get_buffered_amount() > MAX_BUFFER_SIZE) {
            set_param_timer();
            return;
        }
    }

    con_list::iterator it;
    const char* params = m_params->get_params_func();
    if (strlen(params) == 0)
        return;
    std::string js(params);
    static std::vector<uint8_t> buffer;
    buffer.clear();
    size_t size;
    const void* dataSend = NULL;
    std::string prefix = "";
    if (m_params->gzip_func(0, js.c_str(), &buffer, &size)) {
        dataSend = js.c_str();
        size = js.length();
        prefix = "NZIA";
    } else {
        prefix = "EZIA";
        dataSend = buffer.data();
    }

    if (size) {
        for (it = m_connections.begin(); it != m_connections.end(); ++it) {
            server::connection_ptr con = m_endpoint.get_con_from_hdl(*it);
            auto msg = con->get_message(websocketpp::frame::opcode::binary, 0);
            msg->set_payload("");
            msg->append_payload(prefix);
            msg->append_payload(dataSend, size);
            m_endpoint.send(*it, msg);
        }
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

    m_endpoint.get_alog().write(websocketpp::log::alevel::app, "http request1: " + filename);

    if (filename == "/") {
        filename = m_docroot + "index.html";
    } else {
        filename = m_docroot + filename.substr(1);
    }

    m_endpoint.get_alog().write(websocketpp::log::alevel::app, "http request2: " + filename);

    file.open(filename.c_str(), std::ios::in);
    if (!file) {
        // 404 error
        std::stringstream ss;

        ss << "<!doctype html><html><head>" << "<title>Error 404 (Resource not found)</title><body>" << "<h1>Error 404</h1>" << "<p>The requested URL " << filename
           << " was not found on this server.</p>" << "</body></head></html>";

        con->set_body(ss.str());
        con->set_status(websocketpp::http::status_code::not_found);
        return;
    }

    file.seekg(0, std::ios::end);
    response.reserve(file.tellg());
    file.seekg(0, std::ios::beg);

    response.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    con->set_body(response);
    con->set_status(websocketpp::http::status_code::ok);
}

void rp_websocket_server::on_open(connection_hdl hdl) {
    m_endpoint.get_alog().write(websocketpp::log::alevel::app, "ws server on connection");
    m_connections.insert(hdl);
}

void rp_websocket_server::on_close(connection_hdl hdl) {
    m_endpoint.get_alog().write(websocketpp::log::alevel::app, "ws server connection closed");
    m_connections.erase(hdl);

    if (!m_OnClosed) {
        exit(-1);
        m_OnClosed = true;
    }
}

void rp_websocket_server::on_message(connection_hdl hdl, server::message_ptr msg) {
    //	std::stringstream ss;
    //	ss << "Detected " << msg->get_payload() << " test cases.";
    //	m_endpoint.get_alog().write(websocketpp::log::alevel::app,ss.str());
    //get child, it is always only one: "parameters" or "signals"
    JSONNode n = libjson::parse(msg->get_payload());

    JSONNode child = n.at(0);
    std::string name = child.name();

    std::string data = child.write();
    const char* data_str = data.c_str();
    if (name == "parameters") {
        set_param_timer();
        m_params->set_params_func(data_str);
    } else if (name == "signals") {
        set_signal_timer();
        m_params->set_signals_func(data_str);
    }
}

rp_websocket_server* rp_websocket_server::create(struct server_parameters* params) {
    return new rp_websocket_server(params);
}

void rp_websocket_server::start(std::string docroot, uint16_t port) {
    m_endpoint.get_alog().write(websocketpp::log::alevel::app, "start ws_server");
    m_thread = thread(bind(&rp_websocket_server::run, this, docroot, port));
    set_signal_timer();
    set_param_timer();
    int timeout = 0;
    while (m_OnClosed && timeout < 5000) {
        usleep(1000);
        timeout++;
    }
}

void rp_websocket_server::join() {
    m_thread.join();
}

void rp_websocket_server::stop() {
    auto th = std::thread([]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        exit(-1);
    });
    th.detach();
    m_OnClosed = true;

    m_endpoint.get_alog().write(websocketpp::log::alevel::app, "stop ws_server");

    m_endpoint.stop_listening();
    m_endpoint.stop();
    m_param_timer->cancel();
    m_signal_timer->cancel();
    con_list::iterator it;

    for (it = m_connections.begin(); it != m_connections.end(); ++it) {
        connection_hdl hdl = *it;

        try {
            m_endpoint.close(hdl, websocketpp::close::status::normal, "shutdown");

        } catch (websocketpp::lib::error_code ec) {
            m_endpoint.get_alog().write(websocketpp::log::alevel::app, "Close error: " + ec.message());
        }
    }
    m_connections.clear();
    join();
    m_out.close();
}
