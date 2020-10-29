#pragma once
#pragma GCC diagnostic ignored "-Wreorder"

#include <iostream>
#include <sstream>
#include <cstring>
#include <string>
#include <functional>
#include <system_error>
#include <cstdint>
#include <memory>
#include <deque>

#include "neon_asm.h"
#include "asio.hpp"
#include "EventHandlers.h"
//#include "rpsa/common/messaging/message_factory.h"
//#include "rpsa/common/io/basic_buffer.h"

#define  SOCKET_BUFFER_SIZE 65536
#define  FIFO_BUFFER_SIZE  SOCKET_BUFFER_SIZE * 3

using  namespace std;
using  namespace asio;

namespace  asionet {
    enum Protocol {
        TCP,
        UDP
    };

    enum Mode {
        SERVER,
        CLIENT,
        NONE
    };

    class CAsioSocket {
    public:
        typedef uint8_t* send_buffer;
        enum Events{
            CONNECT_SERVER,
            DISCONNECT_SERVER,
            CONNECT_CLIENT,
            DISCONNECT_CLIENT,
            ERROR_SERVER,
            ERROR_CLIENT,
            SEND_DATA,
            RECIVED_DATA_FROM_SERVER};

        using Ptr = shared_ptr<CAsioSocket>;

        static Ptr Create(asio::io_service &io, Protocol _protocol, string host, string port);
        CAsioSocket(asio::io_service &io, Protocol _protocol, string host, string port);
        ~CAsioSocket();

        void InitServer();
        void InitClient();
        void CloseSocket();
        bool IsConnected();
        void SendBuffer(const void *_buffer, size_t _size);
        bool SendBuffer(bool async,send_buffer _buffer, size_t _size);
        void addHandler(Events _event, std::function<void(string host)> _func);
        void addHandler(Events _event, std::function<void(error_code error)> _func);
        void addHandler(Events _event, std::function<void(error_code error,size_t)> _func);
        void addHandler(Events _event, std::function<void(error_code error,uint8_t*,size_t)> _func);

    private:

        CAsioSocket(const CAsioSocket &) = delete;
        CAsioSocket(CAsioSocket &&) = delete;


        void WaitClient();
        void HandlerReceiveFromClient(const asio::error_code &error);
        void HandlerAcceptFromClient(const asio::error_code &_error);
        void HandlerConnectToServer(const asio::error_code &_error, asio::ip::tcp::resolver::iterator endpoint_iterator);
        void HandlerSend(const asio::error_code &_error, size_t _bytesTransferred);
        void HandlerSend2(const asio::error_code &_error, size_t _bytesTransferred, uint8_t *buffer);
        void HandlerReceiveFromServer(const asio::error_code &ErrorCode, size_t bytes_transferred);

        Mode m_mode;
        Protocol m_protocol;
        string m_host;
        string m_port;
        io_service &m_io_service;

        shared_ptr<asio::ip::udp::udp::socket> m_udp_socket;
        shared_ptr<asio::ip::tcp::socket> m_tcp_socket;
        shared_ptr<asio::ip::tcp::acceptor> m_tcp_acceptor;
        asio::ip::udp::udp::endpoint m_udp_endpoint;
        asio::ip::tcp::endpoint m_tcp_endpoint;

        uint8_t *m_SocketReadBuffer;
        char m_udp_recv_server_buffer[1];
        bool m_is_udp_connected;
        bool m_is_tcp_connected;
        uint8_t  *m_tcp_fifo_buffer;
        uint32_t  m_pos_last_in_fifo;
        uint64_t  m_last_pack_id;


        EventList<std::string> m_callback_Str;
        EventList<std::error_code> m_callback_Error;
        EventList2<std::error_code,size_t> m_callback_ErrorInt;
        EventList3<std::error_code,uint8_t*,size_t > m_callbackErrorUInt8Int;
    };


    class CAsioNet {
    public:

        using Ptr = shared_ptr<CAsioNet>;

        static Ptr Create(Mode _mode,Protocol _protocol,string _host , string _port);
        CAsioNet(asionet::Mode _mode,Protocol _protocol,string _host , string _port);
        ~CAsioNet();

        void Start();
        void Stop();
        void addCallServer_Connect(function<void(string host)> _func);
        void addCallServer_Disconnect(function<void(string host)> _func);
        void addCallServer_Error(function<void(error_code error)> _func);

        void addCallClient_Connect(function<void(string host)> _func);
        void addCallClient_Disconnect(function<void(string host)> _func);
        void addCallClient_Error(function<void(error_code error)> _func);

        void addCallSend(function<void(error_code error,size_t)> _func);
        void addCallReceived(function<void(error_code error,uint8_t*,size_t)> _func);

        bool SendData(bool async,CAsioSocket::send_buffer _buffer,size_t _size);
    Protocol GetProtocol() { return  m_protocol;};
        bool IsConnected();

        static uint8_t *BuildPack(
                uint64_t _id ,
                uint64_t _lostRate ,
                uint32_t _oscRate  ,
                uint32_t _resolution ,
                uint32_t _adc_mode,
                uint32_t _adc_bits,
                const void *_ch1 ,
                size_t _size_ch1 ,
                const void *_ch2 ,
                size_t _size_ch2 ,
                size_t &_buffer_size );

        static void BuildPack(
                CAsioSocket::send_buffer buffer ,
                uint64_t _id ,
                uint64_t _lostRate ,
                uint32_t _oscRate  ,
                uint32_t _resolution ,
                uint32_t _adc_mode,
                uint32_t _adc_bits,
                const void *_ch1 ,
                size_t _size_ch1 ,
                const void  *_ch2 ,
                size_t _size_ch2 ,
                size_t &_buffer_size);

        static bool     ExtractPack(
                CAsioSocket::send_buffer _buffer ,
                size_t _size ,
                uint64_t &_id ,
                uint64_t &_lostRate ,
                uint32_t &_oscRate  ,
                uint32_t &_resolution ,
                uint32_t &_adc_mode,
                uint32_t &_adc_bits,
                CAsioSocket::send_buffer &_ch1 ,
                size_t &_size_ch1 ,
                CAsioSocket::send_buffer  &_ch2 ,
                size_t &_size_ch2);

    private:

        CAsioNet(const CAsioNet &) = delete;
        CAsioNet(CAsioNet &&) = delete;
        void SendServerStop();

        Mode m_mode;
        Protocol m_protocol;
        string m_host;
        string m_port;
        asio::io_service m_Ios;
        asio::io_service::work m_Work;
        asio::thread *m_asio_th;
        bool m_IsRun;
        shared_ptr<CAsioSocket> m_server;

    };


}



