#ifndef STREAMING_ROOT_DACASIONETCONTROLLER_H
#define STREAMING_ROOT_DACASIONETCONTROLLER_H

#include <mutex>
#include "net_lib/asio_net_simple.h"
#include "data_lib/signal.hpp"

namespace dac_streaming_lib {

class CDACAsioNetController {
public:
    enum class Errors{
        SOCKET_INTERNAL,
        CONNECT_TIMEOUT
    };

    enum class Events{
        CONNECTED,
        DISCONNECTED,
        DATA_SEND
    };

    struct BufferPack{
        uint8_t* ch1 = nullptr;
        uint8_t* ch2 = nullptr;
        size_t   size_ch1 = 0;
        size_t   size_ch2 = 0;
        uint64_t index = 0;
        bool     empty = true;
    };

    CDACAsioNetController();
    ~CDACAsioNetController();

    CDACAsioNetController(const CDACAsioNetController&) = delete;
    CDACAsioNetController& operator=(const CDACAsioNetController&) = delete;
    CDACAsioNetController(CDACAsioNetController&&) = delete;
    CDACAsioNetController& operator=(CDACAsioNetController&&) = delete;

    auto startAsioNet(net_lib::EMode _mode, std::string _host,std::string _port) -> bool;
    auto stopAsioNet() -> bool;
    auto isConnected() -> bool;
    auto getHost() -> std::string;
    auto getPort() -> std::string;
    auto getBuffer() -> BufferPack;

    auto setReceivedBufferLimit(uint16_t count) -> void;

    // syncSend
    auto sendBuffer(uint8_t *buffer_ch1, size_t size_ch1,uint8_t *buffer_ch2, size_t size_ch2) -> bool;

    sigslot::signal<string&> connectedNotify;
    sigslot::signal<string&> disconnectedNotify;

    sigslot::signal<std::error_code> errorNotify;
    sigslot::signal<std::error_code> timeoutNotify;

    sigslot::signal<> sendNotify;


private:

    auto start() -> bool;
    auto receiveHandler(std::error_code error,uint8_t*,size_t) -> void;
    auto extractBuffer(uint8_t*,size_t) -> void;

    static uint8_t* BuildPack(
            uint64_t _id ,
            const uint8_t *_ch1 ,
            size_t _size_ch1 ,
            const uint8_t *_ch2 ,
            size_t _size_ch2 ,
            size_t &_buffer_size );

    static bool ExtractPack(
            uint8_t* _buffer ,
            size_t _size ,
            uint64_t &_id ,
            uint8_t* &_ch1 ,
            size_t &_size_ch1 ,
            uint8_t*  &_ch2 ,
            size_t &_size_ch2);

    std::string                      m_host;
    std::string                      m_port;
    net_lib::EMode                   m_mode;
    net_lib::CAsioNetSimple*         m_asionet;
//    EventList<std::error_code>       m_errorCallback;
//    EventList<std::string>           m_callbacks;
    uint16_t                         m_bufferLimit;
    uint64_t                         m_index;

    uint8_t*                         m_tcp_fifo_buffer;
    uint32_t                         m_pos_last_in_fifo;
    std::atomic_bool                 m_stopFlag;
    std::deque<BufferPack>           m_bufferdeq;
    std::mutex                       m_recieve_mutex;
};

}

#endif //STREAMING_ROOT_DACASIONETCONTROLLER_H
