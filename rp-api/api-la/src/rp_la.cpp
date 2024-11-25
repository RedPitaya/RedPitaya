/**
 * $Id: $
 *
 * @brief Red Pitaya library - LA api
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <map>
#include <memory>
#include <mutex>
#include <unistd.h>
#include <thread>
#include "decoders/decoder.h"
#include "decoders/can_decoder.h"
#include "decoders/uart_decoder.h"
#include "decoders/spi_decoder.h"
#include "decoders/i2c_decoder.h"
#include "common/common.h"
#include "rp_la.h"
#include "rp_la_api.h"
#include "rp.h"

namespace rp_la{

#define MAX_LINES 8
#define MAX_BUFFER_SIZE 1024 * 1024

struct CapturedData {
    std::vector<uint8_t> m_buffer;
    uint32_t m_samplesBeforeTrigger;
    uint32_t m_trigerPosistion;
};

struct CLAController::Impl {

    auto open() -> void;
    auto close() -> void;
    auto isNoTriggers() -> bool;
    auto run(uint32_t timeoutMs) -> void;

    bool m_isOpen = false;
    std::map<std::string, std::shared_ptr<Decoder>> m_decoders;
    std::mutex m_decoder_mutex;
    std::mutex m_run_mutex;
    std::mutex m_delegate_mutex;
    bool m_isRun = false;

    // Settings
    RP_DIGITAL_CHANNEL_DIRECTIONS m_triggers[RP_MAX_DIGITAL_CHANNELS];
    uint32_t m_decimation;
    uint32_t m_preTriggerSamples = 0;
    uint32_t m_postTriggerSamples = 0;
    ///

    CapturedData m_data;

    CLACallback *m_delegate = nullptr;
    CLAController *m_parent = nullptr;
    std::thread *m_catureThread = nullptr;
};

CLAController::CLAController()
{
    m_pimpl = new Impl();
    m_pimpl->m_parent = this;
    m_pimpl->open();
    for(int i = 0; i < RP_MAX_DIGITAL_CHANNELS; i++){
        m_pimpl->m_triggers[i].channel = (RP_DIGITAL_CHANNEL)i;
        m_pimpl->m_triggers[i].direction = RP_DIGITAL_DONT_CARE;
    }
}

CLAController::~CLAController(){
    TRACE_SHORT("Start destroy")
    if (m_pimpl->m_catureThread && m_pimpl->m_catureThread->joinable()){
        m_pimpl->m_catureThread->join();
        delete m_pimpl->m_catureThread;
        m_pimpl->m_catureThread = nullptr;
    }
    std::lock_guard lock(m_pimpl->m_run_mutex);
    removeDelegate();
    m_pimpl->m_parent = NULL;
    m_pimpl->close();
    delete m_pimpl;
    TRACE_SHORT("End destroy")
}

auto CLAController::setMode(la_Mode_t mode) -> void{
    if (!m_pimpl->m_isOpen){
        ERROR_LOG("The LA device is not open")
        return;
    }

    switch (mode)
    {
        case LA_BASIC:
            rp_SetPolarity(0);
            break;

        case LA_PRO:
            rp_SetPolarity(0xffff);
            break;

        default:
            ERROR_LOG("Unknown mode")
            break;
    }
}

auto CLAController::setTrigger(uint8_t channel, la_Trigger_Mode_t mode) -> void{
    if (channel > MAX_LINES) {
        ERROR_LOG("The line is larger than acceptable")
        return;
    }
    switch (mode)
    {
        case LA_NONE:
            m_pimpl->m_triggers[channel].direction = RP_DIGITAL_DONT_CARE;
            break;

        case LA_LOW:
            m_pimpl->m_triggers[channel].direction = RP_DIGITAL_DIRECTION_LOW;
            break;

        case LA_HIGH:
            m_pimpl->m_triggers[channel].direction = RP_DIGITAL_DIRECTION_HIGH;
            break;

        case LA_RISING:
            m_pimpl->m_triggers[channel].direction = RP_DIGITAL_DIRECTION_RISING;
            break;

        case LA_FALLING:
            m_pimpl->m_triggers[channel].direction = RP_DIGITAL_DIRECTION_FALLING;
            break;

        case LA_RISING_OR_FALLING:
            m_pimpl->m_triggers[channel].direction = RP_DIGITAL_DIRECTION_RISING_OR_FALLING;
            break;

        default:
            ERROR_LOG("Undefined trigger mode")
            break;
    }
}

auto CLAController::getTrigger(uint8_t channel) -> la_Trigger_Mode_t{
    if (channel > MAX_LINES) {
        ERROR_LOG("The line is larger than acceptable")
        return LA_ERROR;
    }

    switch (m_pimpl->m_triggers[channel].direction)
    {
        case RP_DIGITAL_DONT_CARE: return LA_NONE;
        case RP_DIGITAL_DIRECTION_LOW: return LA_LOW;
        case RP_DIGITAL_DIRECTION_HIGH: return LA_HIGH;
        case RP_DIGITAL_DIRECTION_RISING: return LA_RISING;
        case RP_DIGITAL_DIRECTION_FALLING: return LA_FALLING;
        case RP_DIGITAL_DIRECTION_RISING_OR_FALLING: return LA_RISING_OR_FALLING;
        default:
            ERROR_LOG("Undefined trigger")
            break;
    }
    return LA_ERROR;
}

auto CLAController::Impl::isNoTriggers() -> bool{
    for(int i = 0; i < RP_MAX_DIGITAL_CHANNELS; i++){
        if (m_triggers[i].direction != RP_DIGITAL_DONT_CARE){
            return false;
        }
    }
    return true;
}

auto CLAController::isNoTriggers() -> bool{
    return m_pimpl->isNoTriggers();
}

auto CLAController::resetTriggers() -> void{
    for(int i = 0; i < RP_MAX_DIGITAL_CHANNELS; i++){
        m_pimpl->m_triggers[i].direction = RP_DIGITAL_DONT_CARE;
    }
}

auto CLAController::softwareTrigger() -> bool{
    if (!m_pimpl->m_isOpen){
        ERROR_LOG("The LA device is not open")
        return false;
    }
    return rp_SoftwareTrigger();
}

auto CLAController::setEnableRLE(bool enable) -> bool{
    if (!m_pimpl->m_isOpen){
        ERROR_LOG("The LA device is not open")
        return false;
    }
    return rp_EnableDigitalPortDataRLE(enable);
}

auto CLAController::isRLEEnable() -> bool{
    bool enable;
    rp_IsRLEEnable(&enable);
    return enable;
}

auto CLAController::addDecoder(std::string name, la_Decoder_t decoder) -> bool{
    std::lock_guard lock(m_pimpl->m_decoder_mutex);
    switch (decoder)
    {
        case LA_DECODER_CAN:
            m_pimpl->m_decoders[name] = std::make_shared<can::CANDecoder>();
            break;
        case LA_DECODER_I2C:
            m_pimpl->m_decoders[name] = std::make_shared<i2c::I2CDecoder>();
            break;
        case LA_DECODER_SPI:
            m_pimpl->m_decoders[name] = std::make_shared<spi::SPIDecoder>();
            break;
        case LA_DECODER_UART:
            m_pimpl->m_decoders[name] = std::make_shared<uart::UARTDecoder>();
            break;

    default:
        FATAL("Unknown decoder")
        break;
    }
    return true;
}

auto CLAController::removeDecoder(std::string name) -> bool{
    std::lock_guard lock(m_pimpl->m_decoder_mutex);
    return m_pimpl->m_decoders.erase(name);
}

auto CLAController::removeAllDecoders() -> void{
    std::lock_guard lock(m_pimpl->m_decoder_mutex);
    m_pimpl->m_decoders.clear();
}

auto CLAController::Impl::open() -> void{
    if (rp_OpenUnit() == RP_OK){
        m_isOpen = true;
    }
}

auto CLAController::Impl::close() -> void{
    if (rp_CloseUnit() == RP_OK){
        m_isOpen = false;
    }
}

auto CLAController::setDecimation(uint32_t decimation) -> void{
    m_pimpl->m_decimation = decimation;
}

auto CLAController::getDecimation() -> uint32_t{
    return m_pimpl->m_decimation;
}

auto CLAController::setPreTriggerSamples(uint32_t value) -> void{
    m_pimpl->m_preTriggerSamples = value;
}

auto CLAController::getPreTriggerSamples() -> uint32_t{
    return m_pimpl->m_preTriggerSamples;
}

auto CLAController::setPostTriggerSamples(uint32_t value) -> void{
    m_pimpl->m_postTriggerSamples = value;
}

auto CLAController::getPostTriggerSamples() -> uint32_t{
    return m_pimpl->m_postTriggerSamples;
}

auto CLAController::setDelegate(CLACallback *callbacks) -> void{
    std::lock_guard lock(m_pimpl->m_delegate_mutex);
    m_pimpl->m_delegate = callbacks;
}

auto CLAController::removeDelegate() -> void{
    std::lock_guard lock(m_pimpl->m_delegate_mutex);
    m_pimpl->m_delegate = nullptr;
}

auto CLAController::isCaptureRun() -> bool{
    return m_pimpl->m_isRun;
}

auto CLAController::Impl::run(uint32_t timeoutMs) -> void{
    if (!m_parent) {
        m_isRun = false;
        return;
    }
    std::lock_guard lock(m_run_mutex);
    auto samplesCount = (m_preTriggerSamples + m_postTriggerSamples) * 2;
    m_data.m_buffer.resize(samplesCount);
    if (m_data.m_buffer.size() != samplesCount){
        ERROR_LOG("Can't allocate buffer")
    }

    rp_Stop();

    ECHECK_NO_RET(rp_SetTriggerDigitalPortProperties(m_triggers,RP_MAX_DIGITAL_CHANNELS))

    double timeIndisposedMs;

    auto ret = rp_Run(m_preTriggerSamples, m_postTriggerSamples, m_decimation, &timeIndisposedMs);
    if (ret != RP_OK){
        ERROR_LOG("Error starting data capture")
        m_isRun = false;
        return;
    }

    if (isNoTriggers()){
        rp_SoftwareTrigger();
    }

    ECHECK_NO_RET(rp_WaitData(timeoutMs))

    rp_SetDataBuffer((int16_t*)m_data.m_buffer.data(), m_data.m_buffer.size());
    uint32_t samples = m_data.m_buffer.size();
    bool isTimeout = false;
    rp_GetIsTimeout(&isTimeout);
    if (!isTimeout){
        ECHECK_NO_RET(rp_GetValues(&samples))
        rp_GetTrigPosition(&m_data.m_trigerPosistion);
        m_data.m_samplesBeforeTrigger = 0;
        for (size_t i = 0; i < samples; ++i){
            if(i < m_data.m_trigerPosistion)
                m_data.m_samplesBeforeTrigger += m_data.m_buffer[i * 2] + 1;
            else
                break;
        }
    }else{
        samples = 0;
        m_data.m_buffer.resize(0);
        m_data.m_trigerPosistion = 0;
        m_data.m_samplesBeforeTrigger = 0;
    }
    m_isRun = false;

    std::lock_guard lock_delegate(m_delegate_mutex);
    if (m_delegate){
        m_delegate->captureStatus(m_parent,isTimeout);
    }

    TRACE_SHORT("Done")
}


// auto CLAController::run(uint32_t timeoutMs) -> void{
//     if (m_pimpl->m_isRun) return;
//     m_isRun = true;
//     m_pimpl->run(timeoutMs);
// }

auto CLAController::runAsync(uint32_t timeoutMs) -> void{
    if (m_pimpl->m_isRun) return;
    delete m_pimpl->m_catureThread;
    timeoutMs = 0;
    m_pimpl->m_isRun = true;
    m_pimpl->m_catureThread = new std::thread(&CLAController::Impl::run, m_pimpl, timeoutMs);
}

auto CLAController::wait(uint32_t timeoutMs, bool *isTimeout) -> void{

    *isTimeout = false;
    auto startTime = getClockMs();
    while(m_pimpl->m_isRun){
        if (timeoutMs != 0){
            if (getClockMs() - startTime > timeoutMs){
                rp_Stop();
                TRACE_SHORT("Exit by timeout")
                *isTimeout = true;
                break;
            }
        }
    }
    TRACE_SHORT("Stop wait")
    if (m_pimpl->m_catureThread && m_pimpl->m_catureThread->joinable()){
        m_pimpl->m_catureThread->join();
        delete m_pimpl->m_catureThread;
        m_pimpl->m_catureThread = nullptr;
    }
}

// auto CLAController::wait() -> void{
//     if (m_pimpl->m_catureThread && m_pimpl->m_catureThread->joinable()){
//         m_pimpl->m_catureThread->join();
//         delete m_pimpl->m_catureThread;
//         m_pimpl->m_catureThread = nullptr;
//     }
// }

auto CLAController::saveCaptureDataToFile(std::string file) -> bool{
	FILE* f = fopen(file.c_str(), "wb");
    if (!f){
        ERROR_LOG("File opening failed");
        return false;
    }
	fwrite(m_pimpl->m_data.m_buffer.data(), sizeof(uint8_t), m_pimpl->m_data.m_buffer.size(), f);
	fclose(f);
    TRACE("Data writed to file: %s size: %d", file.c_str(),m_pimpl->m_data.m_buffer.size());
    return true;
}

}