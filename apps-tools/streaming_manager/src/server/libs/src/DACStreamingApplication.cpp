#include <chrono>
#include <iostream>
#include <fstream>
#include <functional>
#include <cstdlib>
#include <deque>
#include "DACStreamingApplication.h"
#include "AsioNet.h"

#define UNUSED(x) [&x]{}()

CDACStreamingApplication::CDACStreamingApplication(CDACStreamingManager::Ptr _streamingManager,CGenerator::Ptr _gen) :
    m_gen(_gen),
    m_streamingManager(_streamingManager),
    m_Thread(),
    mtx(),
    m_ReadyToPass(0),
    m_isRun(false),
    m_isRunNonBloking(false),
    m_testMode(false),
    m_verbMode(false)
{
    m_GenThreadRun.test_and_set();
}

CDACStreamingApplication::~CDACStreamingApplication() {
    stop(true);
}

auto CDACStreamingApplication::run() -> void {
    m_isRun = true;
    m_isRunNonBloking = false;
    try {
        m_streamingManager->run();
        m_Thread = std::thread(&CDACStreamingApplication::genWorker, this);
        if (m_Thread.joinable()){
            m_Thread.join();
        }
    }
    catch (const asio::system_error &e)
    {
        std::cerr << "Error: CDACStreamingApplication::run(), " << e.what() << std::endl;
    }

}

auto CDACStreamingApplication::runNonBlock() -> void {
    m_isRun = true;
    m_isRunNonBloking = true;    
    try {
        m_streamingManager->run(); // MUST BE INIT FIRST for thread logic
        m_Thread = std::thread(&CDACStreamingApplication::genWorker, this);
    }
    catch (const asio::system_error &e)
    {
        std::cerr << "Error: CStreamingApplication::runNonBlock(), " << e.what() << std::endl;
    }
}

auto CDACStreamingApplication::stop(bool wait) -> bool{
    mtx.lock();
    bool state = false;
    if (m_isRun){
        m_GenThreadRun.clear();
        if (wait) {
            m_Thread.join();
        }else{
            while(isRun());
        }
        m_streamingManager->stop();
        m_streamingManager = nullptr;
        m_gen->stop();
        m_gen = nullptr;
        state = true;
    }
    mtx.unlock();
    return state;
}

void CDACStreamingApplication::genWorker()
{
       
    auto timeNow = std::chrono::system_clock::now();
    auto curTime = std::chrono::time_point_cast<std::chrono::milliseconds >(timeNow);
    auto value = curTime.time_since_epoch();

    long long int timeBegin = value.count();
    int64_t counter = 0;
//    uintmax_t passCounter = 0;
    //uint8_t   skipBuffs = 0;
    //bool isFirstBufferInit = false;
    //bool isSecondBufferInit = false;
    m_gen->prepare();
    m_gen->start();
    
    std::deque<CDACAsioNetController::BufferPack> packs;
try{
    while (m_GenThreadRun.test_and_set())
    {
        if (packs.size() < 10){
            auto buf = m_streamingManager->getBuffer();
            packs.push_back(buf);
        }

        if (packs.size() > 0) {
            auto pack = packs[0];
            if (m_gen->write(pack.ch1,pack.ch2,pack.size_ch1,pack.size_ch2)){
                counter++;
                if (pack.ch1){
                    delete[] pack.ch1;
                }

                if (pack.ch2){
                    delete[] pack.ch2;
                }
                packs.pop_front();
            }
        }

        // timeNow = std::chrono::system_clock::now();
        // value = std::chrono::time_point_cast<std::chrono::milliseconds >(timeNow).time_since_epoch();

        // if ((value.count() - timeBegin) >= 5000) {
        //     std::cout << "Get buffers: " << counter  << "\n";
        //     counter = 0;
        //     timeBegin = value.count();
        // }

    }
    
}catch (std::exception& e)
	{
		std::cerr << "Error: oscWorker() " << e.what() << std::endl ;
	}
    m_isRun = false;
}

void CDACStreamingApplication::signalHandler(const asio::error_code &, int _signalNumber)
{
    static_cast<void>(_signalNumber);
    stop(true);
}

auto CDACStreamingApplication::setTestMode(bool mode) -> void{
    m_testMode = mode;
}

auto CDACStreamingApplication::setVerbousMode(bool mode) -> void{
    m_verbMode = mode;
}
