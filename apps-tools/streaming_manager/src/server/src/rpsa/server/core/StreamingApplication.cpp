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

#define UNUSED(x) [&x]{}()

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

CStreamingApplication::CStreamingApplication(CStreamingManager::Ptr _StreamingManager,COscilloscope::Ptr _osc_ch, unsigned short _resolution,int _oscRate,int _channels, int _adc_mode, uint32_t _adc_bits) :
    m_StreamingManager(_StreamingManager),
    m_Osc_ch(_osc_ch),
    m_OscThread(),
    m_ReadyToPass(0),
    m_Ios(),
    m_WriteBuffer_ch1(nullptr),
    m_WriteBuffer_ch2(nullptr),
    m_Timer(m_Ios),
    m_BytesCount(0),
    m_Resolution(_resolution),
    m_isRun(false),
    m_isRunNonBloking(false),
    m_oscRate(_oscRate),
    m_adc_mode(_adc_mode),
    m_adc_bits(_adc_bits),
    m_channels(_channels),
    mtx()
{
    
    assert(this->m_Resolution == 8 || this->m_Resolution == 16);

    m_size_ch1 = 0;
    m_size_ch2 = 0;
    m_lostRate = 0;

    m_WriteBuffer_ch1 = aligned_alloc(64, osc_buf_size);

    if (!m_WriteBuffer_ch1) {
        std::cerr << "CStreamingApplication: aligned_alloc" << std::endl;
        std::terminate();
    }
    

    m_WriteBuffer_ch2 = aligned_alloc(64, osc_buf_size);

    if (!m_WriteBuffer_ch2) {
        std::cerr << "CStreamingApplication: aligned_alloc" << std::endl;
        std::terminate();
    }
    

    m_OscThreadRun.test_and_set();
}

CStreamingApplication::~CStreamingApplication()
{

    stop();

    if (m_WriteBuffer_ch1) {
        free(m_WriteBuffer_ch1);
        m_WriteBuffer_ch1 = nullptr;
    }

    if (m_WriteBuffer_ch2) {
        free(m_WriteBuffer_ch2);
        m_WriteBuffer_ch2 = nullptr;
    }
}

void CStreamingApplication::run()
{
    m_size_ch1 = 0;
    m_size_ch2 = 0;
    m_lostRate = 0;
  
    m_isRun = true;
    m_isRunNonBloking = false;

    try {

        m_StreamingManager->run();
        m_OscThread = std::thread(&CStreamingApplication::oscWorker, this);
        // OS signal handler
        // asio::signal_set signalSet(m_Ios, SIGINT, SIGTERM);
        // signalSet.async_wait(std::bind(&CStreamingApplication::signalHandler, this, std::placeholders::_1, std::placeholders::_2));

        // asio::io_service::work idle(m_Ios);
        // m_Ios.run();
        if (m_OscThread.joinable()){
            m_OscThread.join();
        }

    }
    catch (const asio::system_error &e)
    {
        std::cerr << "Error: CStreamingApplication::run(), " << e.what() << std::endl;
        PrintDebugInFile( e.what());
    }

}

void CStreamingApplication::runNonBlock(){
    m_size_ch1 = 0;
    m_size_ch2 = 0;
    m_lostRate = 0;
    m_isRun = true;
    m_isRunNonBloking = true;    
    try {
        m_StreamingManager->run(); // MUST BE INIT FIRST for thread logic
        m_OscThread = std::thread(&CStreamingApplication::oscWorker, this);        
    }
    catch (const asio::system_error &e)
    {
        std::cerr << "Error: CStreamingApplication::runNonBlock(), " << e.what() << std::endl;
        PrintDebugInFile( e.what());
    }
}

bool CStreamingApplication::stop(bool wait){
    mtx.lock();   
    bool state = false;
    if (m_isRun){
        m_OscThreadRun.clear();
        if (wait) {
            m_OscThread.join();
        }else{
            while(isRun());
        }
        m_StreamingManager->stop();
        // m_Ios.stop();
        m_Osc_ch->stop();
        state = true;
    }
    mtx.unlock();
    return state;
}

void CStreamingApplication::oscWorker()
{
       
    auto timeNow = std::chrono::system_clock::now();
    auto curTime = std::chrono::time_point_cast<std::chrono::milliseconds >(timeNow);
    auto value = curTime.time_since_epoch();

    long long int timeBegin = value.count();
    uintmax_t counter = 0;
    uintmax_t passCounter = 0;
    uint8_t   skipBuffs = 0;
    m_Osc_ch->prepare();
try{
    while (m_OscThreadRun.test_and_set())
    {
#ifndef DISABLE_OSC
        m_Osc_ch->wait();
        m_size_ch1 = 0;
        m_size_ch2 = 0;
        uint32_t overFlow = this->passCh(0,m_size_ch1,m_size_ch2);
        if (skipBuffs > 0) { skipBuffs--; continue; }
        if (overFlow > 0) {
            m_lostRate += overFlow;
            ++passCounter;
        }
#endif
        LOG_P("Pass\n");
        oscNotify(overFlow, m_oscRate, m_adc_mode, m_adc_bits, m_WriteBuffer_ch1, m_size_ch1, m_WriteBuffer_ch2, m_size_ch2);
        ++counter;

        timeNow = std::chrono::system_clock::now();
        curTime = std::chrono::time_point_cast<std::chrono::milliseconds >(timeNow);
        value = curTime.time_since_epoch();

          
        if ((value.count() - timeBegin) >= 5000) {
            std::cout << "Lost samples: " << m_lostRate  << "\n";
            counter = 0;
            m_lostRate = 0;
            passCounter = 0;
            timeBegin = value.count();
        }

        if (!m_StreamingManager->isFileThreadWork()){
            
            if (m_StreamingManager->notifyStop){
                if (m_StreamingManager->isOutOfSpace())
                    m_StreamingManager->notifyStop(0);
                else 
                    m_StreamingManager->notifyStop(1);
                m_StreamingManager->notifyStop = nullptr;                
            }
        }
    }
    
}catch (std::exception& e)
	{
		std::cerr << "Error: oscWorker() " << e.what() << std::endl ;
        PrintDebugInFile( e.what());
	}
    m_isRun = false;
}


 uint32_t CStreamingApplication::passCh(int _bufferIndex, size_t &_size1, size_t &_size2){
    UNUSED(_bufferIndex);
    uint8_t *buffer_ch1 = nullptr;
    uint8_t *buffer_ch2 = nullptr;
    size_t   size = 0;
    bool success = false;
    uint32_t overFlow = 0;
    
    success = m_Osc_ch->next(buffer_ch1, buffer_ch2, size , overFlow );

    if (!success) {
        std::cerr << "Error: m_Osc->next()" << std::endl;
        return false;
    }

    // for(int i = 0 ;i < 64 ; i++){
    //     buffer_ch1[i] = 0;
    //     buffer_ch2[i] = 0;
    // }

    // std::ofstream outfile2;
    // outfile2.open("/tmp/test.txt", std::ios_base::app);  
    // short *wb2_1 = (short*)buffer_ch1;
    // short *wb2_2 = (short*)buffer_ch2;
    // for(int i = 0 ;i < 16 ;i ++)
    //     acout() << std::hex <<  (static_cast<int>(wb2_1[i]/ 4))  << " - " << (static_cast<int>(wb2_2[i]/ 4))  << "\n";
    // exit(1);

    if (buffer_ch1 != nullptr){
        _size1 = size;
        switch (m_Resolution)
        {
            case 8:
                memcpy_stride_8bit_neon(((void**)m_WriteBuffer_ch1), buffer_ch1, _size1);
                _size1 /= 2;
                break;
            case 16:
                memcpy_neon(((void**)m_WriteBuffer_ch1), buffer_ch1, _size1);
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
                memcpy_stride_8bit_neon(((void**)m_WriteBuffer_ch2), buffer_ch2, _size2);
                _size2 /= 2;
                break;
            case 16:
                memcpy_neon(((void**)m_WriteBuffer_ch2), buffer_ch2, _size2);
                break;
            default:
                break;
        }
    }else{
        _size2 = 0;
    }

    m_Osc_ch->clearBuffer();

    // std::ofstream outfile3;
    // outfile3.open("/tmp/test2.txt", std::ios_base::app);  
    // short *wb2 = ((short*)m_WriteBuffer_ch1);
    // for(int i = 0 ;i < 16000 /2 ;i ++)
    //     outfile3 << std::hex <<  (static_cast<int>(wb2[i]) & 0xFFFF)  << " ";
    // char **wb = (char**)WriteBuffer;
    //   for(int i = 0 ;i < _size ;i ++){
    //      wb[_bufferIndex][i] = ~(-wb[_bufferIndex][i]) + 1;
    //      std::cout << (static_cast<int>(wb[_bufferIndex][i]) & 0xFF)  << " ";
    //   }
 	//  exit(1);
    return overFlow;
}


int CStreamingApplication::oscNotify(uint64_t _lostRate, uint32_t _oscRate, uint32_t _adc_mode, uint32_t _adc_bits, const void *_buffer_ch1, size_t _size_ch1,const void *_buffer_ch2, size_t _size_ch2)
{
    return m_StreamingManager->passBuffers(_lostRate,_oscRate, _adc_mode,_adc_bits, _buffer_ch1,_size_ch1,_buffer_ch2,_size_ch2,m_Resolution, 0);
}


void CStreamingApplication::signalHandler(const asio::error_code &_error, int _signalNumber)
{
    UNUSED(_error);
    static_cast<void>(_signalNumber);
    stop();
}
