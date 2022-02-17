#include <chrono>
#include <iostream>
#include <fstream>
#include <functional>
#include <cstdlib>
#include "StreamingApplication.h"
#include "AsioNet.h"

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
    m_Osc_ch(_osc_ch),
    m_StreamingManager(_StreamingManager),
    m_OscThread(),
    mtx(),
    m_ReadyToPass(0),
    m_isRun(false),
    m_isRunNonBloking(false),
    m_isRunADC(false),
    m_Resolution(_resolution),
    m_WriteBuffer_ch1(nullptr),
    m_WriteBuffer_ch2(nullptr),
    m_testBuffer_ch1(nullptr),
    m_testBuffer_ch2(nullptr),
    m_oscRate(_oscRate),
    m_channels(_channels),
    m_adc_mode(_adc_mode),
    m_adc_bits(_adc_bits),
    m_BytesCount(0),
    m_testMode(false),
    m_verbMode(false),
    m_printDebugBuffer(false)
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

CStreamingApplication::~CStreamingApplication(){
    stop();
    if (m_WriteBuffer_ch1) {
        free(m_WriteBuffer_ch1);
        m_WriteBuffer_ch1 = nullptr;
    }
    if (m_WriteBuffer_ch2) {
        free(m_WriteBuffer_ch2);
        m_WriteBuffer_ch2 = nullptr;
    }

    if (m_testBuffer_ch1) {
        free(m_testBuffer_ch1);
        m_testBuffer_ch1 = nullptr;
    }

    if (m_testBuffer_ch2) {
        free(m_testBuffer_ch2);
        m_testBuffer_ch2 = nullptr;
    }
}

void CStreamingApplication::run(std::string _file_name_prefix)
{
    mtx.lock();
    m_size_ch1 = 0;
    m_size_ch2 = 0;
    m_lostRate = 0;

    m_isRun = true;
    m_isRunNonBloking = false;
    m_isRunADC = true;
    try {

        m_StreamingManager->run(_file_name_prefix);
        m_OscThread = std::thread(&CStreamingApplication::oscWorker, this);
        if (m_OscThread.joinable()){
            m_OscThread.join();
        }

    }
    catch (const asio::system_error &e)
    {
        std::cerr << "Error: CStreamingApplication::run(), " << e.what() << std::endl;
        PrintDebugInFile( e.what());
    }
    mtx.unlock();
}

void CStreamingApplication::runNonBlock(std::string _file_name_prefix){
    mtx.lock();
    m_size_ch1 = 0;
    m_size_ch2 = 0;
    m_lostRate = 0;
    m_isRun = true;
    m_isRunNonBloking = true;
    m_isRunADC = true;
    try {
        m_StreamingManager->run(_file_name_prefix); // MUST BE INIT FIRST for thread logic
        m_OscThread = std::thread(&CStreamingApplication::oscWorker, this);
    }
    catch (const asio::system_error &e)
    {
        std::cerr << "Error: CStreamingApplication::runNonBlock(), " << e.what() << std::endl;
        PrintDebugInFile( e.what());
    }
    mtx.unlock();
}

auto CStreamingApplication::runNonBlockNoADC(std::string _file_name_prefix) -> void{
    mtx.lock();
    m_size_ch1 = 0;
    m_size_ch2 = 0;
    m_lostRate = 0;
    m_isRun = true;
    m_isRunNonBloking = true;
    m_isRunADC = false;
    try {
        m_StreamingManager->run(_file_name_prefix); // MUST BE INIT FIRST for thread logic
    }
    catch (const asio::system_error &e)
    {
        std::cerr << "Error: CStreamingApplication::runNonBlock(), " << e.what() << std::endl;
        PrintDebugInFile( e.what());
    }
    mtx.unlock();
}

auto CStreamingApplication::runADC() -> void{
    try {
        if (m_isRun && !m_isRunADC){
            m_isRunADC = true;
            m_OscThread = std::thread(&CStreamingApplication::oscWorker, this);
        }
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
        m_StreamingManager->stop();
        m_Osc_ch->stop();
        m_OscThreadRun.clear();
        if (m_isRunADC){
            if (wait) {
                m_OscThread.join();
            }else{
                while(isRun());
            }
        }else{
            m_isRun = false;
        }
        m_Osc_ch = nullptr;
        state = true;
        m_StreamingManager = nullptr;
    }
    mtx.unlock();
    return state;
}

void CStreamingApplication::oscWorker(){
    auto timeNow = std::chrono::system_clock::now();
    auto curTime = std::chrono::time_point_cast<std::chrono::milliseconds >(timeNow);
    auto value = curTime.time_since_epoch();

    long long int timeBegin = value.count();
    // uintmax_t counter = 0;
    // uintmax_t passCounter = 0;
    if (m_testMode) {
        prepareTestBuffers();
    }
    m_Osc_ch->prepare();
    // m_Osc_ch->printReg();

try{
    while (m_OscThreadRun.test_and_set())
    {
        bool state = true;
        uint32_t overFlow = 0;
#ifndef DISABLE_OSC
        state = m_Osc_ch->wait();
        if (state){
            m_size_ch1 = 0;
            m_size_ch2 = 0;
            overFlow = this->passCh(0,m_size_ch1,m_size_ch2);
            if (overFlow > 0) {
//		overFlow += 2;
                m_lostRate += overFlow;
                // ++passCounter;
            }
        }
#endif
        if (state){
            void* passB1 = m_WriteBuffer_ch1;
            void* passB2 = m_WriteBuffer_ch2;

            if (m_testMode){
                passB1 = m_testBuffer_ch1;
                passB2 = m_testBuffer_ch2;
            }
            // printf("SAVE data to file\n");
            oscNotify(overFlow, m_oscRate, m_adc_mode, m_adc_bits, passB1, m_size_ch1, passB2, m_size_ch2);
            if (m_verbMode){
                // ++counter;
                timeNow = std::chrono::system_clock::now();
                curTime = std::chrono::time_point_cast<std::chrono::milliseconds >(timeNow);
                value = curTime.time_since_epoch();

                if ((value.count() - timeBegin) >= 5000) {
                    std::cout << "Lost samples: " << m_lostRate  << "\n";
                    //counter = 0;
                    m_lostRate = 0;
                    // passCounter = 0;
                    timeBegin = value.count();
                }
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
    if (m_printDebugBuffer){
        std::ofstream outfile2;
        outfile2.open("/tmp/test.txt", std::ios_base::app);  
        short *wb2_1 = (short*)buffer_ch1;
        short *wb2_2 = (short*)buffer_ch2;
        for(int i = 0 ;i < 16 ;i ++){            
            acout() << std::hex <<  (wb2_1 ? (static_cast<int>(wb2_1[i]/ 4)) : 0)  << " - " << (wb2_2 ?  (static_cast<int>(wb2_2[i]/ 4)) : 0)  << "\n";
        }
//        exit(1);
    }

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
    return m_StreamingManager->passBuffers(_lostRate,_oscRate, _adc_mode,_adc_bits, _buffer_ch1,_size_ch1,_buffer_ch2,_size_ch2,m_Resolution, 0,m_channels);
}


void CStreamingApplication::signalHandler(const asio::error_code &_error, int _signalNumber)
{
    UNUSED(_error);
    static_cast<void>(_signalNumber);
    stop();
}

auto CStreamingApplication::setTestMode(bool mode) -> void{
    m_testMode = mode;
}

auto CStreamingApplication::setVerbousMode(bool mode) -> void{
    m_verbMode = mode;
}

auto CStreamingApplication::prepareTestBuffers() -> void{
    m_testBuffer_ch1 = aligned_alloc(64, osc_buf_size);

    if (!m_testBuffer_ch1) {
        std::cerr << "CStreamingApplication: aligned_alloc" << std::endl;
        std::terminate();
    }
    

    m_testBuffer_ch2 = aligned_alloc(64, osc_buf_size);

    if (!m_testBuffer_ch2) {
        std::cerr << "CStreamingApplication: aligned_alloc" << std::endl;
        std::terminate();
    }
    uint8_t z = 0;
    for(uint32_t i = 0; i < osc_buf_size;i++,z++){
        ((uint8_t*)m_testBuffer_ch1)[i] = z;
        ((uint8_t*)m_testBuffer_ch2)[i] = z;
    }
}
