#include <chrono>
#include <iostream>
#include <fstream>
#include <functional>
#include <cstdlib>
#include "rpsa/server/core/StreamingApplication.h"
#include "AsioNet.h"

#define CH1 1
#define CH2 2

#ifdef OS_MACOS
#   include "rpsa/common/core/aligned_alloc.h"
#endif // OS_MACOS


#ifdef DEBUG_OUT
#define PrintDebugInFile(X) PrintDebugLogInFile(X);
#else
#define PrintDebugInFile(X)
#endif

void PrintDebugLogInFile(const char *message){
	std::time_t result = std::time(nullptr);	
    std::fstream fs;
  	fs.open ("/tmp/debug.log", std::fstream::in | std::fstream::out | std::fstream::app);
	fs << std::asctime(std::localtime(&result)) << " : " << message << "\n";
	fs.close();
}

CStreamingApplication::CStreamingApplication(CStreamingManager::Ptr _StreamingManager,COscilloscope::Ptr _osc_ch, unsigned short _resolution,int _oscRate,int _channels) :
    m_StreamingManager(_StreamingManager),
    m_Osc_ch(_osc_ch),
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
    m_channels(_channels),
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
    m_size_ch1[0] = m_size_ch1[1]  = 0;
    m_size_ch2[0] = m_size_ch2[1]  = 0;
    m_was_send[0] = 0;
    m_was_send[1] = 0;
    m_lostRate[0] = 0;
    m_lostRate[1] = 0;
    m_bufferIndex = 0;

    m_isRun = true;
    m_OscThread = std::thread(&CStreamingApplication::oscWorker, this);
    try {

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
        PrintDebugInFile( e.what());
    }

}

void CStreamingApplication::runNonBlock(){
    m_size_ch1[0] = m_size_ch1[1]  = 0;
    m_size_ch2[0] = m_size_ch2[1]  = 0;
    m_was_send[0] = 0;
    m_was_send[1] = 0;
    m_lostRate[0] = 0;
    m_lostRate[1] = 0;
    m_bufferIndex = 0;
    m_isRun = true;    
    try {
        m_StreamingManager->run(); // MUST BE INIT FIRST for thread logic
        m_OscThread = std::thread(&CStreamingApplication::oscWorker, this);        
    }
    catch (const asio::system_error &e)
    {
        std::cerr << "Error: CStreamingApplication::run(), " << e.what() << std::endl;
        PrintDebugInFile( e.what());
    }
};

bool CStreamingApplication::stop(){
    
    if (m_isRun){
        m_OscThreadRun.clear();
        m_OscThread.join();
        m_StreamingManager->stop();
        m_Ios.stop();
        m_Osc_ch->stop();

        m_isRun = false;
        return true;
    }
    return false;
};

void CStreamingApplication::oscWorker()
{
       
    auto timeNow = std::chrono::system_clock::now();
    auto curTime = std::chrono::time_point_cast<std::chrono::milliseconds >(timeNow);
    auto value = curTime.time_since_epoch();

    long long int timeBegin = value.count();
    uintmax_t counter = 0;
    uintmax_t passCounter = 0;
    m_Osc_ch->prepare();
try{
    while (m_OscThreadRun.test_and_set())
    {
#ifndef DISABLE_OSC
        m_size_ch1[m_bufferIndex] = 0;
        m_size_ch2[m_bufferIndex] = 0;
        bool overFlow = this->passCh(m_bufferIndex,m_size_ch1[m_bufferIndex],m_size_ch2[m_bufferIndex]);
        if (overFlow) {
            m_lostRate[m_bufferIndex]++;
            ++passCounter;
        }

#endif
        oscNotify(m_lostRate[m_bufferIndex], m_oscRate, m_WriteBuffer_ch1[m_bufferIndex], m_size_ch1[m_bufferIndex], m_WriteBuffer_ch2[m_bufferIndex], m_size_ch2[m_bufferIndex]);
        
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

        if (!m_StreamingManager->isFileThreadWork()){
            if (m_StreamingManager->notifyStop){
                m_StreamingManager->notifyStop(0);
                m_StreamingManager->notifyStop = nullptr;                
            }
        }
    }
    
}catch (std::exception& e)
	{
		fprintf(stderr, "Error: oscWorker() -> %s\n",e.what());
        PrintDebugInFile( e.what());
	}

}


 bool CStreamingApplication::passCh(int _bufferIndex, size_t &_size1, size_t &_size2){
    
    uint8_t *buffer_ch1 = nullptr;
    uint8_t *buffer_ch2 = nullptr;
    size_t   size = 0;
    bool success = false;
    void *WriteBuffer_ch1 = nullptr;
    bool  overFlow1 = false;
    bool  overFlow2 = false;
    
    success = m_Osc_ch->next(buffer_ch1, buffer_ch2, size , overFlow1 , overFlow2);

    if (!success) {
        std::cerr << "Error: m_Osc->next()" << std::endl;
        return false;
    }
    // short *wb2 = (short*)buffer;
    // for(int i = 0 ;i < 40 /2 ;i ++)
    //     std::cout << std::hex <<  (static_cast<int>(wb2[i]) & 0xFFFF)  << " ";
    
    if (buffer_ch1 != nullptr){
        _size1 = size;
        switch (m_Resolution)
        {
            case 8:
                memcpy_stride_8bit_neon(((void**)m_WriteBuffer_ch1)[_bufferIndex], buffer_ch1, _size1);
                _size1 /= 2;
                break;
            case 16:
                memcpy_neon(((void**)m_WriteBuffer_ch1)[_bufferIndex], buffer_ch1, _size1);
                break;
            default:
                break;
        }
    }else{
        _size1 = 0;
    }

     if (buffer_ch2 != nullptr){
        _size2 = size;
        switch (m_Resolution)
        {
            case 8:
                memcpy_stride_8bit_neon(((void**)m_WriteBuffer_ch2)[_bufferIndex], buffer_ch2, _size2);
                _size2 /= 2;
                break;
            case 16:
                memcpy_neon(((void**)m_WriteBuffer_ch2)[_bufferIndex], buffer_ch2, _size2);
                break;
            default:
                break;
        }
    }else{
        _size2 = 0;
    }

    //std::ofstream outfile2;
    //outfile2.open("/tmp/test.txt", std::ios_base::app);  
    // char **wb = (char**)WriteBuffer;
    //   for(int i = 0 ;i < _size ;i ++){
    //      wb[_bufferIndex][i] = ~(-wb[_bufferIndex][i]) + 1;
    //      std::cout << (static_cast<int>(wb[_bufferIndex][i]) & 0xFF)  << " ";
    //   }
 	// exit(1);
    return overFlow1 | overFlow2;
}


int CStreamingApplication::oscNotify(uint64_t _lostRate, uint32_t _oscRate,const void *_buffer_ch1, size_t _size_ch1,const void *_buffer_ch2, size_t _size_ch2)
{
    return m_StreamingManager->passBuffers(_lostRate,_oscRate, _buffer_ch1,_size_ch1,_buffer_ch2,_size_ch2,m_Resolution, 0);
}

// void CStreamingApplication::passReadyNotify(int _pass_size)
// {
// //     m_ReadyToPass--;
// //     std::cout << m_ReadyToPass << "\n";
// }

// void CStreamingApplication::passReadyNotifyReset(){
//  //   m_ReadyToPass = 0;
// }

// void CStreamingApplication::performanceCounterHandler(const asio::error_code &_error)
// {
//     if (!_error)
//     {
//         std::cout << "Bandwidth: " << m_BytesCount / (1024 * 1024 * m_PerformanceCounterPeriod) << " MiB/s\n";
//         m_BytesCount = 0;

//         m_Timer.expires_from_now(std::chrono::seconds(m_PerformanceCounterPeriod));
//         m_Timer.async_wait(std::bind(&CStreamingApplication::performanceCounterHandler, this, std::placeholders::_1));
//     }
// }

void CStreamingApplication::signalHandler(const asio::error_code &_error, int _signalNumber)
{
    static_cast<void>(_signalNumber);
    stop();
}
