#include "DACAsioNetController.h"
#define UNUSED(x) [&x]{}()
constexpr char DAC_ID_PACK[] = "#DAC_STREAM_PACK";

#define  SOCKET_BUFFER_SIZE 65536
#define  FIFO_BUFFER_SIZE  SOCKET_BUFFER_SIZE * 3


CDACAsioNetController::CDACAsioNetController():
    m_host(""),
    m_port(""),
    m_mode(CAsioSocketSimple::ASMode::AS_CLIENT),
    m_asionet(nullptr),
    m_bufferLimit(10),
    m_index(0),
    m_stopFlag(false),
    m_bufferdeq(),
    m_recieve_mutex()
{
    m_tcp_fifo_buffer = new uint8_t[FIFO_BUFFER_SIZE];
}

CDACAsioNetController::~CDACAsioNetController() {
    stopAsioNet();
    delete [] m_tcp_fifo_buffer;
    for (auto obj:m_bufferdeq) {
        delete[] obj.ch1;
        delete[] obj.ch2;
    }
    m_bufferdeq.clear();
}

auto CDACAsioNetController::addHandlerError(CDACAsioNetController::Errors event,std::function<void(std::error_code)> _func) -> void{
    m_errorCallback.addListener(static_cast<int>(event),_func);
}

auto CDACAsioNetController::addHandler(CDACAsioNetController::Events event, std::function<void(std::string)> _func) -> void{
    m_callbacks.addListener(static_cast<int>(event),_func);
}

bool CDACAsioNetController::startAsioNet(CAsioSocketSimple::ASMode _mode, std::string _host,std::string _port){
    m_mode = _mode;
    m_host = _host;
    m_port = _port;
    m_index = 0;
    m_stopFlag = false;
    if (m_host == ""  || m_port == "")
        return false;
    return start();
}

bool CDACAsioNetController::stopAsioNet(){
    m_stopFlag = true;
    if (m_asionet) {
        m_asionet->disconnect();
        delete m_asionet;
        m_asionet = nullptr;
        return true;
    }
    return false;
}

auto CDACAsioNetController::start() -> bool{
    if (m_host == ""  || m_port == "")
        return false;

    if (m_asionet){
        delete m_asionet;
        m_asionet = nullptr;
    }
    m_asionet = new CAsioNetSimple(m_mode, m_host, m_port);
    m_asionet->addCall_Connect([this](std::string host){
        this->m_callbacks.emitEvent((int)CDACAsioNetController::Events::CONNECTED,host);
    });
    m_asionet->addCall_Disconnect([this](std::string host)
    {
        this->m_callbacks.emitEvent((int)CDACAsioNetController::Events::DISCONNECTED,host);
    });
    m_asionet->addCall_Error([this](std::error_code error)
    {
        this->m_errorCallback.emitEvent((int)CDACAsioNetController::Errors::SOCKET_INTERNAL,error);
    });
    m_asionet->addCall_TimeoutError([this](std::error_code error)
    {
        this->m_errorCallback.emitEvent((int)CDACAsioNetController::Errors::CONNECT_TIMEOUT,error);
    });
    m_asionet->addCall_Send([this](std::error_code error,size_t size)
    {
        UNUSED(error);
        UNUSED(size);
        this->m_callbacks.emitEvent((int)CDACAsioNetController::Events::DATA_SEND,getHost());
    });

    m_asionet->addCall_Received(std::bind(&CDACAsioNetController::receiveHandler, this, std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));

    m_asionet->start();
    return true;
}

void CDACAsioNetController::receiveHandler(std::error_code error,uint8_t* _buff,size_t _size){
    if (!error) {
        if (_size + m_pos_last_in_fifo > FIFO_BUFFER_SIZE) {
            fprintf(stderr,"[CDACAsioNetController] TCP received buffer overflow\n");
            exit(5);
        }

        memcpy(m_tcp_fifo_buffer + m_pos_last_in_fifo, _buff, _size);
        m_pos_last_in_fifo += _size;

        uint8_t  size_id = sizeof(DAC_ID_PACK) - 1;
        bool find_all_flag = false;

        do{

            for (uint32_t i = 0; i < m_pos_last_in_fifo - size_id; ++i) {
                bool find_flag = false;
                for (int j = 0; j < size_id; ++j) {
                    if (m_tcp_fifo_buffer[i + j] == DAC_ID_PACK[j]) {
                        find_flag = true;
                    } else {
                        find_flag = false;
                        break;
                    }
                }
                //                   std::cout << i << " pos " <<  m_pos_last_in_fifo << "\n";

                if (find_flag) {
                    uint32_t pack_size = ((uint32_t *) (m_tcp_fifo_buffer + i))[6];
                    if ((pack_size + i) <= m_pos_last_in_fifo) {
                        extractBuffer(m_tcp_fifo_buffer + i, (size_t)pack_size);

                        memcpy(m_tcp_fifo_buffer, m_tcp_fifo_buffer + i + pack_size,
                               m_pos_last_in_fifo - pack_size - i);
                        m_pos_last_in_fifo = m_pos_last_in_fifo - pack_size - i;
                        find_all_flag = true;
                    }
                    else{
                        find_all_flag = false;
                        //                            std::cout << "Header persent " << i << " size " << m_pos_last_in_fifo <<" \n";
                    }
                    break;
                } else{
                    find_all_flag = false;
                }
            }
            if (m_pos_last_in_fifo <= size_id)
                break;
        } while (find_all_flag);
    }else{
        fprintf(stderr,"[CDACAsioNetController] TCP received buffer overflow: %s (%d)\n",error.message().c_str(),error.value());
    }
}

auto CDACAsioNetController::extractBuffer(uint8_t* buff,size_t size) -> void{
    if (m_stopFlag) return;
    while(m_bufferdeq.size() > m_bufferLimit){
        if (m_stopFlag) return;
    }
    const std::lock_guard<std::mutex> lock(m_recieve_mutex);
    BufferPack obj;
    if (ExtractPack(buff,size,obj.index,obj.ch1,obj.size_ch1,obj.ch2,obj.size_ch2)){
        obj.empty = false;
        m_bufferdeq.push_front(obj);
    }
}

auto CDACAsioNetController::getBuffer() -> BufferPack{
    const std::lock_guard<std::mutex> lock(m_recieve_mutex);
    if (m_bufferdeq.size() > 0){
        auto buf = m_bufferdeq.back();
        m_bufferdeq.pop_back();
        return buf;
    }
    return BufferPack();
}

bool CDACAsioNetController::isConnected(){
    if (!m_asionet) return false;
    return m_asionet->isConnected();
}

auto CDACAsioNetController::getHost() -> std::string{
    return m_host;
}

auto CDACAsioNetController::getPort() -> std::string{
    return m_port;
}

auto CDACAsioNetController::setReceivedBufferLimit(uint16_t count) -> void{
    const std::lock_guard<std::mutex> lock(m_recieve_mutex);
    m_bufferLimit = count;
}

auto CDACAsioNetController::sendBuffer(uint8_t *buffer_ch1,size_t size_ch1,uint8_t *buffer_ch2,size_t size_ch2) -> bool{
    if (!m_asionet) return false;
    if (m_asionet->isConnected()){
        size_t size = 0;
        auto buf = BuildPack(m_index++,buffer_ch1,size_ch1,buffer_ch2,size_ch2,size);
        auto ret = m_asionet->sendData(false,buf,size);
        delete[] buf;
        return ret;
    }
    return false;
}

uint8_t* CDACAsioNetController::BuildPack(
        uint64_t _id ,
        const uint8_t *_ch1 ,
        size_t _size_ch1 ,
        const uint8_t *_ch2 ,
        size_t _size_ch2 ,
        size_t &_buffer_size ){
    size_t  prefix_lenght = sizeof(int8_t) * 16; // ID of pack must be 16 byte
    prefix_lenght += sizeof(uint64_t);    // Index (8 byte)
    prefix_lenght += sizeof(int32_t);     // pack size (4 byte)
    prefix_lenght += sizeof(int32_t) * 2; // size of channel1 and channel2 (8 byte)
    size_t  buffer_size = prefix_lenght + _size_ch1 + _size_ch2;
    //       printf("buffer_size %d, prefix_size %d\n",buffer_size,prefix_lenght);
    auto buffer = new uint8_t[buffer_size];
    memcpy(buffer,DAC_ID_PACK,16);
    ((uint64_t*)buffer)[2]  = _id;
    ((uint32_t*)buffer)[6]  = (uint32_t)buffer_size;
    ((uint32_t*)buffer)[7] = (uint32_t)_size_ch1;
    ((uint32_t*)buffer)[8] = (uint32_t)_size_ch2;

    if (_size_ch1>0){

        memcpy((&(*buffer)+prefix_lenght), _ch1, _size_ch1);
    }

    if (_size_ch2>0){

        memcpy((&(*buffer)+prefix_lenght + _size_ch1), _ch2, _size_ch2);
    }

    _buffer_size = buffer_size;
    return buffer;
}

bool CDACAsioNetController::ExtractPack(
        uint8_t* _buffer ,
        size_t _size ,
        uint64_t &_id ,
        uint8_t* &_ch1 ,
        size_t &_size_ch1 ,
        uint8_t*  &_ch2 ,
        size_t &_size_ch2){
    UNUSED(_size);

    if (strncmp((const char*)_buffer,DAC_ID_PACK,16) == 0){
        _id = ((uint64_t*)_buffer)[2];
        ASIO_ASSERT(_size == ((uint32_t*)_buffer)[6]);
        _size_ch1 = ((uint32_t*)_buffer)[7];
        _size_ch2 = ((uint32_t*)_buffer)[8];
        uint16_t prefix = 36;

        if (_size_ch1 > 0) {
            _ch1 = new uint8_t[_size_ch1];
            memcpy(_ch1,_buffer + prefix,_size_ch1);
        }else{
            _ch1 = nullptr;
        }

        if (_size_ch2 > 0) {
            _ch2 = new uint8_t[_size_ch2];
            memcpy(_ch2,_buffer + prefix + _size_ch1,_size_ch2);
        }else{
            _ch2 = nullptr;
        }
        return true;
    }
    return false;
}
