#include <chrono>
#include <iostream>
#include <fstream>
#include <functional>
#include <cstdlib>
#include <unistd.h>
#include <signal.h>

#include "streaming_fpga.h"
#include "data_lib/neon_asm.h"
#include "logger_lib/file_logger.h"

#define UNUSED(x) [&x]{}()

using namespace streaming_lib;

constexpr int quit_signal = SIGINT;

CStreamingFPGA::CStreamingFPGA(uio_lib::COscilloscope::Ptr _osc, uint8_t _adc_bits) :
    m_Osc_ch(_osc),
    m_OscThread(),
    mtx(),
    m_isRun(false),
    m_adc_bits(_adc_bits),
    m_BytesCount(0),
    m_testMode(false),
    m_verbMode(false),
    m_printDebugBuffer(false),
    m_adcSettings()
{
    m_passRate = 0;
    m_OscThreadRun = false;
    getBuffF = nullptr;
    unlockBuffF = nullptr;
    m_testBuffer = nullptr;
    if (!m_Osc_ch){
        FATAL("Register controller is not initialized")
    }
}

CStreamingFPGA::~CStreamingFPGA(){
    stop();
    delete[] m_testBuffer;
    TRACE("Exit")
}

auto CStreamingFPGA::isRun() -> bool {
    return m_isRun;
}

auto CStreamingFPGA::setPrintDebugBuffer(bool mode) -> void {
    m_printDebugBuffer = mode;
}

auto CStreamingFPGA::runNonBlock() -> void {
    std::lock_guard<std::mutex> lock(mtx);
    try {
        m_OscThreadRun = true;
        setIsRun(true);
        m_OscThread = std::thread(&CStreamingFPGA::oscWorker, this);

#ifdef RP_PLATFORM
        pthread_attr_t thAttr;
        int policy = 0;
        int max_prio_for_policy = 0;
        pthread_attr_init(&thAttr);
        pthread_attr_getschedpolicy(&thAttr, &policy);
        max_prio_for_policy = sched_get_priority_max(policy);
        pthread_setschedprio(m_OscThread.native_handle(), max_prio_for_policy);
        pthread_attr_destroy(&thAttr);
#endif

    }
    catch (const std::system_error &e)
    {
        aprintf(stderr,"Error: CStreamingApplication::runNonBlock() %s\n",e.what());
    }
}

auto CStreamingFPGA::stop() -> bool {
    std::lock_guard<std::mutex> lock(mtx);
    TRACE("Stop")
    m_OscThreadRun = false;
    if (m_OscThread.joinable()) {
        m_OscThread.join();
    }
    m_Osc_ch->stop();
    return true;
}

auto CStreamingFPGA::setIsRun(bool state) -> void {
    if (m_isRun != state){
        m_isRun = state;
        isRunNotify(m_isRun);
    }
}

auto CStreamingFPGA::addChannel(DataLib::EDataBuffersPackChannel _channel,DataLib::CDataBuffer::ADC_MODE _adc_mode, uint8_t _bitsBySample) -> void {
    SADCsettings settings;
    settings.m_bits = _bitsBySample;
    settings.m_mode = _adc_mode;
    m_adcSettings[_channel] = settings;
}


void CStreamingFPGA::oscWorker(){

    auto timeNow = std::chrono::system_clock::now();
    auto curTime = std::chrono::time_point_cast<std::chrono::milliseconds >(timeNow);
    auto value = curTime.time_since_epoch();
    long long int timeBegin = value.count();

    m_passRate = 0;

    if (m_testMode) {
        m_testBuffer = new uint8_t[uio_lib::osc_buf_size];

        uint8_t z = 0;
        for(uint32_t i = 0; i < uio_lib::osc_buf_size;i++,z++){
            m_testBuffer[i] = z;
        }
    }
    m_Osc_ch->prepare();

    try{

        uint64_t dataSize = 0;
        uint64_t lostSize = 0;
        uint64_t totalPassRate = 0;
        m_overFlowSumm = 0;
        m_overFlowSummCount = 0;
        while (m_OscThreadRun)
        {
            bool state = true;
            DataLib::CDataBuffersPack::Ptr pack(nullptr);

            state = m_Osc_ch->wait();
            if (state){
                pack = this->passCh();
                m_passRate++;
                totalPassRate++;
            }
            if (state){

#ifndef RP_PLATFORM
                 usleep(3000);
#endif
                oscNotify(pack);
                if (pack){
                    dataSize += pack->getLenghtAllBuffers();
                    lostSize += pack->getLostAllBuffers();
                }
                if (m_verbMode){
                    timeNow = std::chrono::system_clock::now();
                    curTime = std::chrono::time_point_cast<std::chrono::milliseconds >(timeNow);
                    value = curTime.time_since_epoch();

                    if ((value.count() - timeBegin) >= 5000) {
                        aprintf(stdout,"Pass buffers: %d\n", m_passRate);
                        m_passRate = 0;
                        timeBegin = value.count();
                    }
                }
            }
        }
        auto timeNowEnd = std::chrono::system_clock::now();
        auto p1 = std::chrono::time_point_cast<std::chrono::milliseconds>(timeNow).time_since_epoch();
        auto p2 = std::chrono::time_point_cast<std::chrono::milliseconds>(timeNowEnd).time_since_epoch();
        aprintf(stderr,"Loop %lld ms FPGA size %lld FPGA lost: %lld totalPassRate %lld\n",p2.count() - p1.count(),dataSize,lostSize,totalPassRate);
    }
    catch (std::exception& e)
    {
        aprintf(stderr,"Error: CStreamingApplication::oscWorker() %s\n",e.what());
    }
    setIsRun(false);
}


 auto CStreamingFPGA::passCh() -> DataLib::CDataBuffersPack::Ptr {
    uint8_t *buffer_ch1 = nullptr;
    uint8_t *buffer_ch2 = nullptr;
    uint8_t *buffer_ch3 = nullptr;
    uint8_t *buffer_ch4 = nullptr;
    size_t   size = 0;
    bool success = false;
    uint32_t overFlow = 0;
    success = m_Osc_ch->next(buffer_ch1, buffer_ch2, buffer_ch3,buffer_ch4, size , overFlow );

    if (!success) {
        return nullptr;
    }

    if (m_testMode) {
        buffer_ch1 = m_testBuffer;
        buffer_ch2 = m_testBuffer;
        buffer_ch3 = m_testBuffer;
        buffer_ch4 = m_testBuffer;
    }

    if (m_printDebugBuffer){
        short *wb2_1 = (short*)buffer_ch1;
        short *wb2_2 = (short*)buffer_ch2;
        short *wb2_3 = (short*)buffer_ch3;
        short *wb2_4 = (short*)buffer_ch4;

        for(int i = 0 ;i < 16 ;i ++){
            aprintf(stdout,"%X - %X - %X - %X \n",(wb2_1 ? (static_cast<int>(wb2_1[i]/ 4)) : 0) , (wb2_2 ?  (static_cast<int>(wb2_2[i]/ 4)) : 0), (wb2_3 ?  (static_cast<int>(wb2_3[i]/ 4)) : 0), (wb2_4 ?  (static_cast<int>(wb2_4[i]/ 4)) : 0));
        }
        exit(1);
    }

    if (!getBuffF || !unlockBuffF) {
        return nullptr;
    }

    auto pack = getBuffF(overFlow);

    if (pack){
        pack->setOSCRate(m_Osc_ch->getOSCRate());
        pack->setADCBits(m_adc_bits);
        overFlow += m_overFlowSumm;

        if (m_adcSettings.find(DataLib::EDataBuffersPackChannel::CH1) != m_adcSettings.end()){
            auto settings = m_adcSettings.at(DataLib::EDataBuffersPackChannel::CH1);
            auto bCh1 = pack->getBuffer(DataLib::CH1);
            if (bCh1){

                bCh1->setADCMode(settings.m_mode);
                bCh1->setLostSamples(DataLib::FPGA,overFlow + bCh1->getSamplesCount() * m_overFlowSummCount);
                memcpy_neon(bCh1->getBuffer().get(),buffer_ch1,size);
            }
        }


        if (m_adcSettings.find(DataLib::EDataBuffersPackChannel::CH2) != m_adcSettings.end()){
            auto settings = m_adcSettings.at(DataLib::EDataBuffersPackChannel::CH2);
            auto bCh2 = pack->getBuffer(DataLib::CH2);
            if (bCh2){
                bCh2->setADCMode(settings.m_mode);
                bCh2->setLostSamples(DataLib::FPGA,overFlow + bCh2->getSamplesCount() * m_overFlowSummCount);
                memcpy_neon(bCh2->getBuffer().get(),buffer_ch2,size);
            }
        }

        if (m_adcSettings.find(DataLib::EDataBuffersPackChannel::CH3) != m_adcSettings.end()){
            auto settings = m_adcSettings.at(DataLib::EDataBuffersPackChannel::CH3);
            auto bCh3 = pack->getBuffer(DataLib::CH3);
            if (bCh3){
                bCh3->setADCMode(settings.m_mode);
                bCh3->setLostSamples(DataLib::FPGA,overFlow + bCh3->getSamplesCount() * m_overFlowSummCount);
                memcpy_neon(bCh3->getBuffer().get(),buffer_ch3,size);
            }
        }

        if (m_adcSettings.find(DataLib::EDataBuffersPackChannel::CH4) != m_adcSettings.end()){
            auto settings = m_adcSettings.at(DataLib::EDataBuffersPackChannel::CH4);
            auto bCh4 = pack->getBuffer(DataLib::CH4);
            if (bCh4){
                bCh4->setADCMode(settings.m_mode);
                bCh4->setLostSamples(DataLib::FPGA,overFlow + bCh4->getSamplesCount() * m_overFlowSummCount);
                memcpy_neon(bCh4->getBuffer().get(),buffer_ch4,size);
            }
        }
        m_overFlowSumm = 0;
        m_overFlowSummCount = 0;
        unlockBuffF();
    }else{
        m_overFlowSumm += overFlow;
        m_overFlowSummCount++;
    }
    m_Osc_ch->clearBuffer();
    return pack;
}

auto CStreamingFPGA::setTestMode(bool mode) -> void{
    m_testMode = mode;
}

auto CStreamingFPGA::setVerbousMode(bool mode) -> void{
    m_verbMode = mode;
}
