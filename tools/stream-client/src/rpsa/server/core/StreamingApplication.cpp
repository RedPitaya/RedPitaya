#include <chrono>
#include <iostream>
#include <fstream>
#include <functional>
#include <cstdlib>
#include "rpsa/server/core/StreamingApplication.h"
#include "AsioNet.h"

#define CHECK_CH1(X) if (m_Osc_ch1!=nullptr) X;
#define CHECK_CH2(X) if (m_Osc_ch2!=nullptr) X;
#define CH1 1
#define CH2 2

#ifdef OS_MACOS
#   include "rpsa/common/core/aligned_alloc.h"
#endif // OS_MACOS




CStreamingApplication::CStreamingApplication(CStreamingManager::Ptr _StreamingManager,COscilloscope::Ptr _osc_ch1, COscilloscope::Ptr _osc_ch2, unsigned short _resolution,int _oscRate) :
    m_StreamingManager(_StreamingManager),
    m_Osc_ch1(_osc_ch1),
    m_Osc_ch2(_osc_ch2),
    m_OscThread(),
    m_ReadyToPass(0),
    m_Ios(),
    m_WriteBuffer_ch1{nullptr, nullptr},
    m_WriteBuffer_ch2{nullptr, nullptr},
    m_Timer(m_Ios),
    m_BytesCount(0),
    m_Resolution(_resolution),
    m_isRun(false),
    m_oscRate(_oscRate),
    mtx()
{
    
    assert(this->m_Resolution == 8 || this->m_Resolution == 16);

    m_size_ch1[0] = m_size_ch1[1]  = 0;
    m_size_ch2[0] = m_size_ch2[1]  = 0;
    m_was_send[0] = 0;
    m_was_send[1] = 0;

    for (void *&data : m_WriteBuffer_ch1) {
        data = aligned_alloc(64, osc_buf_size);

        if (!data) {
            std::cerr << "CStreamingApplication: aligned_alloc" << std::endl;
            std::terminate();
        }
    }

    for (void *&data : m_WriteBuffer_ch2) {
        data = aligned_alloc(64, osc_buf_size);

        if (!data) {
            std::cerr << "CStreamingApplication: aligned_alloc" << std::endl;
            std::terminate();
        }
    }

    m_OscThreadRun.test_and_set();
    m_SockThreadRun.test_and_set();
}

CStreamingApplication::~CStreamingApplication()
{
    stop();
    for (void *&data : m_WriteBuffer_ch1) {
        if (data) {
            free(data);
            data = nullptr;
        }
    }

    for (void *&data : m_WriteBuffer_ch2) {
        if (data) {
            free(data);
            data = nullptr;
        }
    }

}

void CStreamingApplication::run()
{
    m_isRun = true;
    m_OscThread = std::thread(&CStreamingApplication::oscWorker, this);
    m_SocketThread = std::thread(&CStreamingApplication::socketWorker, this);
    try {

    //    m_StreamingManager->notifyPassData = std::bind(&CStreamingApplication::passReadyNotify, this, std::placeholders::_1);
    //    m_StreamingManager->notifyPassDataReset = std::bind(&CStreamingApplication::passReadyNotifyReset, this);
        m_StreamingManager->run();

        // OS signal handler
        asio::signal_set signalSet(m_Ios, SIGINT, SIGTERM);
        signalSet.async_wait(std::bind(&CStreamingApplication::signalHandler, this, std::placeholders::_1, std::placeholders::_2));

        asio::io_service::work idle(m_Ios);
        m_Ios.run();
        m_OscThread.join();

    }
    catch (const asio::system_error &e)
    {
        std::cerr << "Error: CStreamingApplication::run(), " << e.what() << std::endl;
    }

}

void CStreamingApplication::runNonBlock(){
    m_isRun = true;    
    try {

     //   m_StreamingManager->notifyPassData = std::bind(&CStreamingApplication::passReadyNotify, this, std::placeholders::_1);
     //   m_StreamingManager->notifyPassDataReset = std::bind(&CStreamingApplication::passReadyNotifyReset, this);
        m_StreamingManager->run();
        m_OscThread = std::thread(&CStreamingApplication::oscWorker, this);
        m_SocketThread = std::thread(&CStreamingApplication::socketWorker, this);
        // OS signal handler
        //asio::signal_set signalSet(m_Ios, SIGINT, SIGTERM);
        //signalSet.async_wait(std::bind(&CStreamingApplication::signalHandler, this, std::placeholders::_1, std::placeholders::_2));

        //asio::io_service::work idle(m_Ios);
        //m_Ios.run();
        //m_OscThread.join();

    }
    catch (const asio::system_error &e)
    {
        std::cerr << "Error: CStreamingApplication::run(), " << e.what() << std::endl;
    }
};

bool CStreamingApplication::stop(){
    
    if (m_isRun){
        m_OscThreadRun.clear();
        m_OscThread.join();
        m_SockThreadRun.clear();
        m_SocketThread.join();
        m_StreamingManager->stop();
        m_Ios.stop();
        CHECK_CH1(m_Osc_ch1->stop())
        CHECK_CH2(m_Osc_ch2->stop())

        m_isRun = false;
        return true;
    }
    return false;
};

void CStreamingApplication::oscWorker()
{

    m_bufferIndex = 0;
    m_size_ch1[0] = m_size_ch1[1]  = 0;
    m_size_ch2[0] = m_size_ch2[1]  = 0;
    m_was_send[0] = 0;
    m_was_send[1] = 0;
    m_lostRate[0] = 0;
    m_lostRate[1] = 0;

    auto timeNow = std::chrono::system_clock::now();
    auto curTime = std::chrono::time_point_cast<std::chrono::milliseconds >(timeNow);
    auto value = curTime.time_since_epoch();

    long long int timeBegin = value.count();
    uintmax_t counter = 0;
    uintmax_t passCounter = 0;

    CHECK_CH1(m_Osc_ch1->prepare())
    CHECK_CH2(m_Osc_ch2->prepare())
try{
    while (m_OscThreadRun.test_and_set())
    {
        usleep(1000);
#ifndef DISABLE_OSC
        m_size_ch1[m_bufferIndex] = 0;
        m_size_ch2[m_bufferIndex] = 0;
        CHECK_CH1(this->passCh(CH1,m_bufferIndex,m_size_ch1[m_bufferIndex]))
        CHECK_CH2(this->passCh(CH2,m_bufferIndex,m_size_ch2[m_bufferIndex]))
#endif
        if (m_was_send[m_bufferIndex ? 0 : 1 ] == 0) {
            m_was_send[m_bufferIndex] = 1;
            mtx.lock();
            m_bufferIndex = m_bufferIndex ? 0 : 1;
            mtx.unlock();
        } else {
            ++passCounter;
            m_lostRate[m_bufferIndex]++;
        }

        ++counter;

        timeNow = std::chrono::system_clock::now();
        curTime = std::chrono::time_point_cast<std::chrono::milliseconds >(timeNow);
        value = curTime.time_since_epoch();


        if ((value.count() - timeBegin) >= 5000) {
            std::cout << "Lost rate: " << passCounter << " / " << counter << " (" << (100. * static_cast<double>(passCounter) / counter) << " %)\n";
            counter = 0;
            passCounter = 0;
            timeBegin = value.count();
        }
    }
    
}catch (std::exception& e)
	{
		fprintf(stderr, "Error: oscWorker() -> %s\n",e.what());
	}


    
}

void CStreamingApplication::socketWorker(){

#ifdef  DISABLE_OSC
    char *buffer_ch1 = new char[osc_buf_size];
    char *buffer_ch2 = new char[osc_buf_size];
#endif

try{
    while (m_SockThreadRun.test_and_set()) {

        mtx.lock();
        auto Index = m_bufferIndex ? 0 : 1;
        mtx.unlock();
        if (m_was_send[Index] == 1) {

#ifdef  DISABLE_OSC
            for (size_t i = 0; i < osc_buf_size; i++) {
                ((char *) buffer_ch1)[i] = val;
                ((char *) buffer_ch2)[i] = val++;
            }

            oscNotify(m_lostRate[Index],m_oscRate, buffer_ch1, osc_buf_size, buffer_ch2, osc_buf_size);
#else

            auto w_bufferIndex_ch = m_bufferIndex ? 0: 1;

            if (!m_was_send[w_bufferIndex_ch1]){
                oscNotify(m_WriteBuffer_ch1[w_bufferIndex_ch], m_size_ch1[w_bufferIndex_ch], m_WriteBuffer_ch2[w_bufferIndex_ch], m_size_ch2[w_bufferIndex_ch]);
                m_was_send[w_bufferIndex_ch1]=true;
            }

#endif
            m_lostRate[Index] = 0;
            m_was_send[Index] = 0;
        }

        if (!m_StreamingManager->isFileThreadWork()){
            if (m_StreamingManager->notifyStop){
                m_StreamingManager->notifyStop(0);
                m_StreamingManager->notifyStop = nullptr;                
            }
        }
    }
}catch (std::exception& e)
	{
		fprintf(stderr, "Error: socketWorker() -> %s\n",e.what());
	}


#ifdef  DISABLE_OSC
    delete [] buffer_ch1;
    delete [] buffer_ch2;
#endif
}

 void CStreamingApplication::passCh(int _index_ch,int &_bufferIndex, size_t &_size){
    
    uint8_t *buffer;
    bool success = false;
    void *WriteBuffer;
    

    if (_index_ch == CH1){
        success = m_Osc_ch1->next(buffer, _size);
        WriteBuffer = m_WriteBuffer_ch1;
    }

    if (_index_ch == CH2){
        success = m_Osc_ch2->next(buffer, _size);
        WriteBuffer = m_WriteBuffer_ch2;
    }

    if (!success) {
        std::cerr << "Error: m_Osc->next()" << std::endl;
        _bufferIndex = -1;
        return;
    }
    
    switch (m_Resolution)
    {
        case 8:
            memcpy_stride_8bit_neon(((void**)WriteBuffer)[_bufferIndex], buffer, _size);
            _size /= 2;
            break;
        case 16:
            memcpy_neon(((void**)WriteBuffer)[_bufferIndex], buffer, _size);
            break;
        default:
            break;
    }
       
    
}


int CStreamingApplication::oscNotify(uint64_t _lostRate, uint32_t _oscRate,const void *_buffer_ch1, size_t _size_ch1,const void *_buffer_ch2, size_t _size_ch2)
{
    return m_StreamingManager->passBuffers(_lostRate,_oscRate, _buffer_ch1,_size_ch1,_buffer_ch2,_size_ch2,m_Resolution, 0);
}

void CStreamingApplication::passReadyNotify(int _pass_size)
{
//     m_ReadyToPass--;
//     std::cout << m_ReadyToPass << "\n";
}

void CStreamingApplication::passReadyNotifyReset(){
 //   m_ReadyToPass = 0;
}

void CStreamingApplication::performanceCounterHandler(const asio::error_code &_error)
{
    if (!_error)
    {
        std::cout << "Bandwidth: " << m_BytesCount / (1024 * 1024 * m_PerformanceCounterPeriod) << " MiB/s\n";
        m_BytesCount = 0;

        m_Timer.expires_from_now(std::chrono::seconds(m_PerformanceCounterPeriod));
        m_Timer.async_wait(std::bind(&CStreamingApplication::performanceCounterHandler, this, std::placeholders::_1));
    }
}

void CStreamingApplication::signalHandler(const asio::error_code &_error, int _signalNumber)
{
    static_cast<void>(_signalNumber);
    stop();
}
