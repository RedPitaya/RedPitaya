#include <fstream>
#include "asio.hpp"
#include "AsioNet.h"

#define UNUSED(x) [&x]{}()



namespace  asionet {

    uint8_t *CAsioNet::BuildPack(
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
            size_t &_buffer_size ){

        size_t  prefix_lenght = sizeof(int8_t) * 16; // ID of pack (16 byte)
        prefix_lenght += sizeof(uint64_t);    // Index (8 byte)
        prefix_lenght += sizeof(uint64_t);    // lostRate  (8 byte)
        prefix_lenght += sizeof(int32_t);     // _oscRate  (4 byte)
        prefix_lenght += sizeof(int32_t);     // pack size (4 byte)
        prefix_lenght += sizeof(int32_t) * 2; // size of channel1 and channel2 (8 byte)
        prefix_lenght += sizeof(int32_t);     // resolution (4 byte)
        prefix_lenght += sizeof(int32_t);     // adc_mode (4 byte)
        prefix_lenght += sizeof(int32_t);     // adc_bits (4 byte)
        size_t  buffer_size = prefix_lenght + _size_ch1 + _size_ch2;
//        printf("buffer_size %d, prefix_size %d\n",buffer_size,prefix_lenght);
        auto buffer = new uint8_t[buffer_size];
        memcpy(buffer,ID_PACK,16);
        ((uint64_t*)buffer)[2] = _id;
        ((uint64_t*)buffer)[3] = _lostRate;
        ((uint32_t*)buffer)[8] = _oscRate;
        ((uint32_t*)buffer)[9] = (uint32_t)buffer_size;
        ((uint32_t*)buffer)[10] = (uint32_t)_size_ch1;
        ((uint32_t*)buffer)[11] = (uint32_t)_size_ch2;
        ((uint32_t*)buffer)[12] = _resolution;
        ((uint32_t*)buffer)[13] = _adc_mode;
        ((uint32_t*)buffer)[14] = _adc_bits;
        if (_size_ch1>0){

            memcpy_neon((&(*buffer)+prefix_lenght), _ch1, _size_ch1);
        }

        if (_size_ch2>0){

            memcpy_neon((&(*buffer)+prefix_lenght + _size_ch1), _ch2, _size_ch2);
        }

        _buffer_size = buffer_size;
        return buffer;
    }

    void CAsioNet::BuildPack(
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
            size_t &_buffer_size){
        size_t  prefix_lenght = sizeof(int8_t) * 16; // ID of pack (16 byte)
        prefix_lenght += sizeof(uint64_t);    // Index (8 byte)
        prefix_lenght += sizeof(uint64_t);    // lostRate  (8 byte)
        prefix_lenght += sizeof(int32_t);     // _oscRate  (4 byte)
        prefix_lenght += sizeof(int32_t);     // pack size (4 byte)
        prefix_lenght += sizeof(int32_t) * 2; // size of channel1 and channel2 (8 byte)
        prefix_lenght += sizeof(int32_t);     // resolution (4 byte)
        prefix_lenght += sizeof(int32_t);     // adc_mode (4 byte)
        size_t  buffer_size = prefix_lenght + _size_ch1 + _size_ch2;
//       printf("buffer_size %d, prefix_size %d\n",buffer_size,prefix_lenght);
        memcpy(buffer,ID_PACK,16);
        ((uint64_t*)buffer)[2] = _id;
        ((uint64_t*)buffer)[3] = _lostRate;
        ((uint32_t*)buffer)[8] = _oscRate;
        ((uint32_t*)buffer)[9] = (uint32_t)buffer_size;
        ((uint32_t*)buffer)[10] = (uint32_t)_size_ch1;
        ((uint32_t*)buffer)[11] = (uint32_t)_size_ch2;
        ((uint32_t*)buffer)[12] = _resolution;
        ((uint32_t*)buffer)[13] = _adc_mode;
        ((uint32_t*)buffer)[14] = _adc_bits;
        
        if (_size_ch1>0){

            memcpy_neon((&(*buffer)+prefix_lenght), _ch1, _size_ch1);
        }

        if (_size_ch2>0){

            memcpy_neon((&(*buffer)+prefix_lenght + _size_ch1), _ch2, _size_ch2);
        }

        _buffer_size = buffer_size;

    }

    bool CAsioNet::ExtractPack(
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
                    size_t &_size_ch2){
        UNUSED(_size);

        if (strncmp((const char*)_buffer,ID_PACK,16) == 0){
            _id = ((uint64_t*)_buffer)[2];
            _lostRate = ((uint64_t*)_buffer)[3];
            _oscRate  = ((uint32_t*)_buffer)[8];
            ASIO_ASSERT(_size == ((uint32_t*)_buffer)[9]);
            _size_ch1 = ((uint32_t*)_buffer)[10];
            _size_ch2 = ((uint32_t*)_buffer)[11];
            _resolution = ((uint32_t*)_buffer)[12];
            _adc_mode = ((uint32_t*)_buffer)[13];     
            _adc_bits = ((uint32_t*)_buffer)[14];            
            uint16_t prefix = 60;

            if (_size_ch1 > 0) {
                _ch1 = new uint8_t[_size_ch1];
                memcpy_neon(_ch1,_buffer + prefix,_size_ch1);
            }else{
                _ch1 = nullptr;
            }

            if (_size_ch2 > 0) {
                _ch2 = new uint8_t[_size_ch2];
                memcpy_neon(_ch2,_buffer + prefix + _size_ch1,_size_ch2);
            }else{
                _ch2 = nullptr;
            }
            return true;
        }
        return false;
    }

    CAsioNet::Ptr CAsioNet::Create(asionet::Mode _mode,asionet::Protocol _protocol,std::string _host , std::string _port) {

        return std::make_shared<CAsioNet>(_mode,_protocol,_host,_port);
    }

    CAsioNet::CAsioNet(asionet::Mode _mode,asionet::Protocol _protocol,std::string _host , std::string _port) :
            m_mode(_mode),
            m_protocol(_protocol),
            m_host(_host),
            m_port(_port),
            m_Ios(),
            m_Work(m_Ios),
            m_asio_th(nullptr),
            m_IsRun(false)
    {
        m_server = CAsioSocket::Create(m_Ios, m_protocol, m_host, m_port);
        auto func = std::bind(static_cast<size_t (asio::io_service::*)()>(&asio::io_service::run), &m_Ios);
        m_asio_th = new asio::thread(func);
    }

    CAsioNet::~CAsioNet() {
        Stop();
        m_Ios.stop();
        if (m_asio_th != nullptr){
            m_asio_th->join();
            delete  m_asio_th;
        }

    }

    bool CAsioNet::IsConnected(){
        return  m_IsRun && m_server->IsConnected();
    }

    void CAsioNet::Start()  {
        if (m_IsRun)
            return;
        if (m_mode == asionet::Mode::SERVER) {
            m_server->InitServer();
        }
        if (m_mode == asionet::Mode::CLIENT){
            m_server->InitClient();
        }
        m_IsRun = true;
    }

    void CAsioNet::Stop() {
        SendServerStop();
        m_IsRun = false;
    }

    void CAsioNet::disconnect(){
        m_server->CloseSocket();
        m_IsRun = false;
    }

    void CAsioNet::SendServerStop(){
        if (IsConnected() && m_mode == asionet::Mode::CLIENT){
            m_server->SendBuffer("\x00",1);
        }
        m_server->CloseSocket();
    }

    void CAsioNet::addCallServer_Connect(std::function<void(std::string host)> _func){
        if (m_server){
            m_server->addHandler(Events::CONNECT_SERVER, _func);
        }
    }

    void CAsioNet::addCallServer_Disconnect(std::function<void(std::string host)> _func){
        if (m_server){
            m_server->addHandler(Events::DISCONNECT_SERVER, _func);
        }
    }

    void CAsioNet::addCallServer_Error(std::function<void(std::error_code error)> _func){
        if (m_server) {
            m_server->addHandler(Events::ERROR_SERVER, _func);
        }
    }

    void CAsioNet::addCallClient_Connect(std::function<void(std::string host)> _func){
        if (m_server)
            m_server->addHandler(Events::CONNECT_CLIENT, _func);
    }

    void CAsioNet::addCallClient_Disconnect(std::function<void(std::string host)> _func){
        if (m_server)
            m_server->addHandler(Events::DISCONNECT_CLIENT, _func);
    }

    void CAsioNet::addCallClient_Error(std::function<void(std::error_code error)> _func){
        if (m_server)
            m_server->addHandler(Events::ERROR_CLIENT, _func);
    }

    void CAsioNet::addCallSend(std::function<void(std::error_code error,size_t)> _func){
        if (m_server)
            m_server->addHandler(Events::SEND_DATA, _func);
    }

    void CAsioNet::addCallReceived(std::function<void(std::error_code error,uint8_t*,size_t)> _func){
        if (m_server)
            m_server->addHandler(Events::RECIVED_DATA_FROM_SERVER, _func);
    }

    bool CAsioNet::SendData(bool async,CAsioSocket::send_buffer _buffer,size_t _size){
        if (m_server){
            return m_server->SendBuffer(async,_buffer,_size);
        }
        return false;
    }
}