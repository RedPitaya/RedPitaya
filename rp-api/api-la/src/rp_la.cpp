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
#include <atomic>
#include <functional>
#include <signal.h>
#include <csignal>
#include <unistd.h>
#include <thread>
#include <future>
#include <filesystem>
#include "decoders/decoder.h"
#include "decoders/can_decoder.h"
#include "decoders/uart_decoder.h"
#include "decoders/spi_decoder.h"
#include "decoders/i2c_decoder.h"
#include "common/common.h"
#include "common/profiler.h"
#include "rp_la.h"
#include "rp_la_api.h"
#include "rp.h"

#ifdef _WIN32
#include <dir.h>
#else
#include <sys/stat.h>
#endif

namespace rp_la{

#define MAX_LINES 8
#define MAX_BUFFER_SIZE 1024 * 1024

#ifdef TRACE_ENABLE
#define TRACE_CODE(X) { X;}
#else
#define TRACE_CODE(X) { }
#endif

std::function<void(int)> shutdown_handler;

void signal_handler(int signal)
{
	shutdown_handler(signal);
}

struct CapturedData {
    std::vector<uint8_t> m_buffer;
    uint32_t m_samplesBeforeTrigger;
    uint32_t m_trigerPosistion;
    uint32_t m_capturedBytes;
    uint64_t m_capturedSamples;
    bool     m_isRLE;
};

struct CLAController::Impl {

    auto open() -> void;
    auto close() -> void;
    auto isNoTriggers() -> bool;
    auto run(uint32_t timeoutMs) -> void;
    auto loadFromBytes(std::vector<uint8_t> data, bool isRLE, uint64_t triggerSamplePosition) -> void;
    auto runDecode() -> void;
    auto resetAllDecoders() -> void;
    auto decode(const uint8_t* _input, uint32_t _size) -> void;
    auto getAnnotation(la_Decoder_t decoder,uint8_t control) -> std::string;
    auto getAnnotationSize(la_Decoder_t decoder) -> uint16_t;

    bool m_isOpen = false;
    std::map<std::string, std::shared_ptr<Decoder>> m_decoders;
    std::mutex m_decoder_mutex;
    std::mutex m_run_mutex;
    std::mutex m_thread_mutex;
    std::mutex m_delegate_mutex;
    std::atomic_bool m_isRun = false;

    // Settings
    RP_DIGITAL_CHANNEL_DIRECTIONS m_triggers[RP_MAX_DIGITAL_CHANNELS];
    uint32_t m_decimation;
    uint32_t m_preTriggerSamples = 0;
    uint32_t m_postTriggerSamples = 0;
    ///

    CapturedData m_data;

    CLACallback *m_delegate = nullptr;
    CLAController *m_parent = nullptr;
    std::thread *m_captureThread = nullptr;
};

auto createDir(const std::string &dir) -> bool{
#ifdef _WIN32
	mkdir(dir.c_str());
#else
	mkdir(dir.c_str(), 0777);
#endif
	return true;
}

auto createDirTree(const std::string &full_path) -> bool{
	char ch = '/';
#ifdef _WIN32
	ch = '\\';
#endif

	size_t pos = 0;
	bool ret_val = true;
	while (ret_val == true && pos != std::string::npos) {
		pos = full_path.find(ch, pos + 1);
		ret_val = createDir(full_path.substr(0, pos));
	}
	return ret_val;
}

CLAController::CLAController()
{
    m_pimpl = new Impl();
    m_pimpl->m_parent = this;
    m_pimpl->open();
    for(int i = 0; i < RP_MAX_DIGITAL_CHANNELS; i++){
        m_pimpl->m_triggers[i].channel = (RP_DIGITAL_CHANNEL)i;
        m_pimpl->m_triggers[i].direction = RP_DIGITAL_DONT_CARE;
    }

    if (m_pimpl->m_isOpen){
        uint32_t dmaSize = 0;
        rp_GetFullBufferSize(&dmaSize);
        m_pimpl->m_data.m_buffer.resize(dmaSize);
        if (m_pimpl->m_data.m_buffer.size() != dmaSize){
            ERROR_LOG("Can't allocate buffer")
            m_pimpl->m_data.m_buffer.resize(0);
        }
    }

  	auto sigInt = [&](int) { rp_Stop(); };
	shutdown_handler = sigInt;
	std::signal(SIGINT, signal_handler);

}

CLAController::~CLAController(){
    std::lock_guard lock_thread(m_pimpl->m_thread_mutex);
    TRACE_SHORT("Start destroy")
    if (m_pimpl->m_captureThread && m_pimpl->m_captureThread->joinable()){
        m_pimpl->m_captureThread->join();
    }
    delete m_pimpl->m_captureThread;
    m_pimpl->m_captureThread = nullptr;
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

auto CLAController::Impl::getAnnotation(la_Decoder_t decoder,uint8_t control) -> std::string{
    switch (decoder)
    {
        case LA_DECODER_CAN:
            return can::CANParameters::getCANAnnotationsString((can::CANAnnotations)control);
        case LA_DECODER_I2C:
            return i2c::I2CParameters::getI2CAnnotationsString((i2c::I2CAnnotations)control);
        case LA_DECODER_SPI:
            return spi::SPIParameters::getSPIAnnotationsString((spi::SPIAnnotations)control);
        case LA_DECODER_UART:
            return uart::UARTParameters::getUARTAnnotationsString((uart::UARTAnnotations)control);
        default:
            break;
    }
    return "";
}

auto CLAController::Impl::getAnnotationSize(la_Decoder_t decoder) -> uint16_t{
    switch (decoder)
    {
        case LA_DECODER_CAN:
            return can::CANAnnotations::ENUM_END;
        case LA_DECODER_I2C:
            return i2c::I2CAnnotations::ENUM_END;
        case LA_DECODER_SPI:
            return spi::SPIAnnotations::ENUM_END;
        case LA_DECODER_UART:
            return uart::UARTAnnotations::ENUM_END;
        default:
            break;
    }
    return 0;
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

auto CLAController::setDecoderEnable(std::string name, bool enable) -> void{
    std::lock_guard lock(m_pimpl->m_decoder_mutex);
    if (m_pimpl->m_decoders.find(name) != m_pimpl->m_decoders.end()){
        m_pimpl->m_decoders[name]->setEnabled(enable);
    }else{
        ERROR_LOG("Decoder %s not found",name.c_str())
    }
}

auto CLAController::getDecoderEnable(std::string name) -> bool{
    std::lock_guard lock(m_pimpl->m_decoder_mutex);
    if (m_pimpl->m_decoders.find(name) != m_pimpl->m_decoders.end()){
        return m_pimpl->m_decoders[name]->getEnabled();
    }else{
        ERROR_LOG("Decoder %s not found",name.c_str())
    }
    return false;
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
            m_pimpl->m_decoders[name] = std::make_shared<can::CANDecoder>(LA_DECODER_CAN);
            break;
        case LA_DECODER_I2C:
            m_pimpl->m_decoders[name] = std::make_shared<i2c::I2CDecoder>(LA_DECODER_I2C);
            break;
        case LA_DECODER_SPI:
            m_pimpl->m_decoders[name] = std::make_shared<spi::SPIDecoder>(LA_DECODER_SPI);
            break;
        case LA_DECODER_UART:
            m_pimpl->m_decoders[name] = std::make_shared<uart::UARTDecoder>(LA_DECODER_UART);
            break;

    default:
        FATAL("Unknown decoder")
        break;
    }
    return true;
}

auto CLAController::setDecoderSettings(std::string name,std::string json) -> bool{
    std::lock_guard lock(m_pimpl->m_decoder_mutex);
    if (m_pimpl->m_decoders.find(name) != m_pimpl->m_decoders.end()){
        m_pimpl->m_decoders[name]->setParametersInJSON(json);
        return true;
    }
    return false;
}

auto CLAController::getDecoderSettings(std::string name) -> std::string{
    std::lock_guard lock(m_pimpl->m_decoder_mutex);
    if (m_pimpl->m_decoders.find(name) != m_pimpl->m_decoders.end()){
        return m_pimpl->m_decoders[name]->getParametersInJSON();
    }
    return "";
}

auto CLAController::setDecoderSettingsUInt(std::string name, std::string key, uint32_t value) -> bool{
    std::lock_guard lock(m_pimpl->m_decoder_mutex);
    if (m_pimpl->m_decoders.find(name) != m_pimpl->m_decoders.end()){
        return m_pimpl->m_decoders[name]->setDecoderSettingsUInt(key,value);
    }
    return false;
}

auto CLAController::setDecoderSettingsFloat(std::string name, std::string key, float value) -> bool{
    std::lock_guard lock(m_pimpl->m_decoder_mutex);
    if (m_pimpl->m_decoders.find(name) != m_pimpl->m_decoders.end()){
        return m_pimpl->m_decoders[name]->setDecoderSettingsFloat(key,value);
    }
    return false;
}

auto CLAController::getDecoderSettingsUInt(std::string name, std::string key, uint32_t *value) -> bool{
    std::lock_guard lock(m_pimpl->m_decoder_mutex);
    if (m_pimpl->m_decoders.find(name) != m_pimpl->m_decoders.end()){
        return m_pimpl->m_decoders[name]->getDecoderSettingsUInt(key,value);
    }
    return false;
}

auto CLAController::getDecoderSettingsFloat(std::string name, std::string key, float *value) -> bool{
    std::lock_guard lock(m_pimpl->m_decoder_mutex);
    if (m_pimpl->m_decoders.find(name) != m_pimpl->m_decoders.end()){
        return m_pimpl->m_decoders[name]->getDecoderSettingsFloat(key,value);
    }
    return false;
}

auto CLAController::isDecoderExist(std::string name) -> bool{
    std::lock_guard lock(m_pimpl->m_decoder_mutex);
    if (m_pimpl->m_decoders.find(name) != m_pimpl->m_decoders.end()){
        return true;
    }
    return false;
}

auto CLAController::getDecoderType(std::string name) -> la_Decoder_t{
    std::lock_guard lock(m_pimpl->m_decoder_mutex);
    if (m_pimpl->m_decoders.find(name) != m_pimpl->m_decoders.end()){
        return (la_Decoder_t)m_pimpl->m_decoders[name]->getDecoderType();
    }
    return LA_DECODER_NONE;
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

auto CLAController::Impl::resetAllDecoders() -> void{
    std::lock_guard lock(m_decoder_mutex);
    for(auto decoder: m_decoders){
        decoder.second->reset();
    }
}

auto CLAController::Impl::decode(const uint8_t* _input, uint32_t _size) -> void{
    std::lock_guard lock(m_decoder_mutex);
    for(auto decoder: m_decoders){
        if(!decoder.second->getEnabled()) continue;
        TRACE_CODE(profiler::clearHistory(decoder.first))
        TRACE_CODE(profiler::setTimePoint(decoder.first))
        decoder.second->decode(_input,_size);
        TRACE_CODE(profiler::saveTimePointuS(decoder.first,"Decoder %s. Data size %d",decoder.first.c_str(),_size))
        TRACE_SHORT("Decoder %s. Memory usage %llu",decoder.first.c_str(),decoder.second->getMemoryUsage())
        std::lock_guard lock_delegate(m_delegate_mutex);
        if (m_delegate){
            m_delegate->decodeDone(m_parent,decoder.first);
        }
    }
    TRACE_CODE(profiler::print())
}

auto CLAController::getDecoders() -> std::vector<std::string>{
    std::lock_guard lock(m_pimpl->m_decoder_mutex);
    std::vector<std::string> list;
    for(auto decoder: m_pimpl->m_decoders){
        list.push_back(decoder.first);
    }
    return list;
}


auto CLAController::getDecodedData(std::string name) -> std::vector<rp_la::OutputPacket>{
    std::lock_guard lock(m_pimpl->m_decoder_mutex);
    if (m_pimpl->m_decoders.find(name) != m_pimpl->m_decoders.end()){
        auto items = m_pimpl->m_decoders[name]->getSignal();
        // auto type = m_pimpl->m_decoders[name]->getDecoderType();
        std::vector<rp_la::OutputPacket> new_vect;
        for(auto &itm:items){
            rp_la::OutputPacket pack;
            pack.line_name = itm.line_name;
            pack.control = itm.control;
            pack.data = itm.data;
            pack.length = itm.length;
            pack.bitsInPack = itm.bitsInPack;
            pack.sampleStart = itm.sampleStart;
            // pack.annotation = m_pimpl->getAnnotation((la_Decoder_t)type,itm.control);
            new_vect.push_back(pack);
        }
        return new_vect;
    }
    return {};
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

    static auto trig = [](uint32_t timeoutMs) {
        usleep(timeoutMs * 1000);
        rp_SoftwareTrigger();
    };


    if (!m_parent) {
        m_isRun = false;
        return;
    }
    std::lock_guard lock(m_run_mutex);

    rp_Stop();

    int trigger_check = 0;

    for(size_t i = 0; i < RP_MAX_DIGITAL_CHANNELS; i++){
        if (m_triggers[i].direction == RP_DIGITAL_DIRECTION_RISING ||
            m_triggers[i].direction == RP_DIGITAL_DIRECTION_FALLING ||
            m_triggers[i].direction == RP_DIGITAL_DIRECTION_RISING_OR_FALLING){
                trigger_check++;
            }
    }

    if (trigger_check > 1){
        ERROR_LOG("Incorrect combination of triggers. You cannot use triggers at the same time: RISING, FALLING, RISING_OR_FALLING")
        m_isRun = false;
        return;
    }

    ECHECK_NO_RET(rp_SetTriggerDigitalPortProperties(m_triggers,RP_MAX_DIGITAL_CHANNELS))

    double timeIndisposedMs;

    auto ret = rp_Run(m_preTriggerSamples, m_postTriggerSamples, m_decimation, &timeIndisposedMs);
    if (ret != RP_OK){
        ERROR_LOG("Error starting data capture")
        m_isRun = false;
        return;
    }
    std::future<void> fut;
    if (isNoTriggers()){
        auto timeoutTrigMs = 1000;
        fut = std::async(std::launch::async, trig, timeoutTrigMs); // Fix for bug in FPGA
    }

    ECHECK_NO_RET(rp_WaitDataRLE(timeoutMs))

    if (fut.valid())
        fut.wait();

    rp_SetDataBuffer((int16_t*)m_data.m_buffer.data(), m_data.m_buffer.size());
    bool isTimeout = false;
    rp_GetIsTimeout(&isTimeout);
    if (!isTimeout){
        uint32_t blockCount = 0;
        uint64_t numSamples = 0;
        ECHECK_NO_RET(rp_GetValuesRLE(&blockCount,&numSamples))
        m_data.m_capturedBytes = blockCount * 2;
        rp_GetTrigBlockPositionRLE(&m_data.m_trigerPosistion);
        m_data.m_samplesBeforeTrigger = m_preTriggerSamples;
        m_data.m_capturedSamples = numSamples;
        m_data.m_isRLE = true;
        decode(m_data.m_buffer.data(),m_data.m_capturedBytes);
    }else{
        m_data.m_trigerPosistion = 0;
        m_data.m_samplesBeforeTrigger = 0;
        m_data.m_capturedSamples = 0;
        m_data.m_isRLE = false;
    }
    m_isRun = false;
    std::lock_guard lock_delegate(m_delegate_mutex);
    if (m_delegate){
        m_delegate->captureStatus(m_parent,
                                  isTimeout,
                                  m_data.m_capturedBytes,
                                  m_data.m_capturedSamples,
                                  m_data.m_samplesBeforeTrigger,
                                  m_data.m_capturedSamples - m_data.m_samplesBeforeTrigger);
    }
    TRACE_SHORT("Done")
}

auto CLAController::Impl::loadFromBytes(std::vector<uint8_t> data, bool isRLE, uint64_t triggerSamplePosition) -> void{
     if (!m_parent) {
        m_isRun = false;
        return;
    }
    std::lock_guard lock(m_run_mutex);

    m_data.m_buffer = data;
    uint64_t numSamples = 0;
    if (isRLE){
        if (data.size() % 2) {
            ERROR_LOG("The buffer must be a multiple of 2")
            m_isRun = false;
            return;
        }
        for(size_t i = 0; i < data.size() / 2; i++){
            numSamples += data[i * 2] + 1;
        }
        m_data.m_capturedBytes = data.size();
        m_data.m_samplesBeforeTrigger = triggerSamplePosition;
        m_data.m_capturedSamples = numSamples;
        m_data.m_isRLE = true;

    }else{
        ERROR_LOG("Unsupported mode")
    }
    decode(m_data.m_buffer.data(),m_data.m_capturedBytes);
    m_isRun = false;
    std::lock_guard lock_delegate(m_delegate_mutex);
    if (m_delegate){
        m_delegate->captureStatus(m_parent,
                                  false,
                                  m_data.m_capturedBytes,
                                  m_data.m_capturedSamples,
                                  m_data.m_samplesBeforeTrigger,
                                  m_data.m_capturedSamples - m_data.m_samplesBeforeTrigger);
    }
    TRACE_SHORT("Done")
}

auto CLAController::Impl::runDecode() -> void{
    if (!m_parent) {
        return;
    }
    std::lock_guard lock(m_run_mutex);
    if (m_data.m_isRLE && m_data.m_capturedBytes)
        decode(m_data.m_buffer.data(),m_data.m_capturedBytes);
    TRACE_SHORT("Decode done")
}


// auto CLAController::run(uint32_t timeoutMs) -> void{
//     if (m_pimpl->m_isRun) return;
//     m_isRun = true;
//     m_pimpl->run(timeoutMs);
// }

auto CLAController::runAsync(uint32_t timeoutMs) -> void{
    std::lock_guard lock_thread(m_pimpl->m_thread_mutex);
    if (m_pimpl->m_isRun) return;

    if (m_pimpl->m_captureThread && m_pimpl->m_captureThread->joinable()){
        m_pimpl->m_captureThread->join();
    }
    delete m_pimpl->m_captureThread;
    timeoutMs = 0;
    m_pimpl->m_isRun = true;
    m_pimpl->m_captureThread = new std::thread(&CLAController::Impl::run, m_pimpl, timeoutMs);
}

auto CLAController::runAsync(std::vector<uint8_t> data, bool isRLE, uint64_t triggerSamplePosition) -> void{
    std::lock_guard lock_thread(m_pimpl->m_thread_mutex);
    if (m_pimpl->m_isRun) return;

    if (m_pimpl->m_captureThread && m_pimpl->m_captureThread->joinable()){
        m_pimpl->m_captureThread->join();
    }
    delete m_pimpl->m_captureThread;
    m_pimpl->m_isRun = true;
    m_pimpl->m_captureThread = new std::thread(&CLAController::Impl::loadFromBytes, m_pimpl, data, isRLE, triggerSamplePosition);
}

auto CLAController::decodeAsync() -> void{
    std::lock_guard lock_thread(m_pimpl->m_thread_mutex);
    if (m_pimpl->m_captureThread && m_pimpl->m_captureThread->joinable()){
        m_pimpl->m_captureThread->join();
    }
    delete m_pimpl->m_captureThread;
    m_pimpl->m_captureThread = new std::thread(&CLAController::Impl::runDecode, m_pimpl);
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
    if (m_pimpl->m_captureThread && m_pimpl->m_captureThread->joinable()){
        m_pimpl->m_captureThread->join();
    }
    delete m_pimpl->m_captureThread;
    m_pimpl->m_captureThread = nullptr;
}

// auto CLAController::wait() -> void{
//     if (m_pimpl->m_catureThread && m_pimpl->m_catureThread->joinable()){
//         m_pimpl->m_catureThread->join();
//         delete m_pimpl->m_catureThread;
//         m_pimpl->m_catureThread = nullptr;
//     }
// }

auto CLAController::saveCaptureDataToFile(std::string file) -> bool{
    std::filesystem::path path(file);
    createDirTree(path.parent_path().string());
	FILE* f = fopen(file.c_str(), "wb");
    if (!f){
        ERROR_LOG("File opening failed");
        return false;
    }
    uint32_t bytes = getCapturedDataSize();
    size_t bytesForWrite = std::min((size_t)bytes,m_pimpl->m_data.m_buffer.size());
	fwrite(m_pimpl->m_data.m_buffer.data(), sizeof(uint8_t), bytesForWrite, f);
	fclose(f);
    TRACE("Data writed to file: %s size: %d", file.c_str(),bytesForWrite);
    return true;
}

auto CLAController::loadFromFile(std::string file, uint64_t triggerSamplePosition) -> bool{
    FILE* f = fopen(file.c_str(), "rb");
    if (!f){
        ERROR_LOG("File opening failed");
        return false;
    }

    fseek(f, 0, SEEK_END);
    int64_t fileSize = ftell(f);
    rewind(f);
    if(fileSize < 0){
        ERROR_LOG("Error getting file size for: %s",file.c_str())
        fclose(f);
        return false;
    }
    std::vector<uint8_t> buffer;
    buffer.resize(fileSize); // Resize the vector to hold the whole file

    int64_t bytesRead = fread(buffer.data(), 1, fileSize, f);
    if (bytesRead != fileSize) {
        ERROR_LOG("Error reading file. Read: %lld expected: %lld", bytesRead, fileSize)
        buffer.clear();
        return false;
    }

    fclose(f);
    if (buffer.size() == 0){
        ERROR_LOG("File is empty");
        return false;
    }
    runAsync(buffer,true,triggerSamplePosition);
    return true;
}


auto CLAController::getDataNP(uint8_t* np_buffer, int size) -> uint32_t{
    uint32_t copySize = (size_t)size < m_pimpl->m_data.m_buffer.size() ? size : m_pimpl->m_data.m_buffer.size();
    memcpy(np_buffer,m_pimpl->m_data.m_buffer.data(),copySize);
    return copySize;
}

auto CLAController::getUnpackedRLEDataNP(uint8_t* np_buffer, int size) -> uint64_t{
    if (!m_pimpl->m_data.m_isRLE){
        ERROR_LOG("Missing RLE data")
        return 0;
    }

    uint64_t samples = 0;
    for(uint32_t index = 0; index < m_pimpl->m_data.m_capturedBytes; index++){
        uint16_t count = m_pimpl->m_data.m_buffer[index++] + 1;
        uint8_t data = m_pimpl->m_data.m_buffer[index];
        for (uint16_t j = 0 ; j < count; j++){
            if ((uint64_t)size < samples){
                WARNING("There was not enough buffer to decompress the data")
                return samples;
            }
            np_buffer[samples++] = data;
        }
    }
    return samples;
}


auto CLAController::getDMAMemorySize() -> uint32_t{
    uint32_t size;
    rp_GetFullBufferSize(&size);
    return size;
}

auto CLAController::getCapturedDataSize() -> uint32_t{
    return m_pimpl->m_data.m_capturedBytes;
}

auto CLAController::getCapturedSamples() -> uint64_t{
    return m_pimpl->m_data.m_capturedSamples;
}

auto CLAController::printRLE(bool useHex) -> void{
    if (!m_pimpl->m_data.m_isRLE){
        ERROR_LOG("Missing RLE data")
        return;
    }
    printRLENP(m_pimpl->m_data.m_buffer.data(),m_pimpl->m_data.m_capturedBytes,useHex);
}

auto CLAController::printRLENP(uint8_t* np_buffer, int size, bool useHex) -> void{
    std::vector<uint64_t> count_vec;
    std::vector<uint8_t> data_vec;
    for(uint32_t index = 0; index < (uint32_t)size - 1; index++){
        uint16_t count = np_buffer[index++] + 1;
        uint8_t data = np_buffer[index];
        if (data_vec.size() == 0){
            count_vec.push_back(count);
            data_vec.push_back(data);
        } else if (data_vec.back() != data){
            count_vec.push_back(count);
            data_vec.push_back(data);
        } else{
            count_vec.back() += count;
        }
    }

    auto binary = [](uint8_t i,int bigEndian){
        int j=0,m = bigEndian ? 1 : 10000000;
        while (i)
        {
            j+=m*(i%2);
            if (bigEndian) m*=10; else m/=10;
            i >>= 1;
        }
        return j;
    };
    uint64_t samples = 0;
    for(size_t i = 0; i < count_vec.size(); i++){
        if (useHex)
            fprintf(stdout, "%06llu/%06llu\t: 0x%X\n",samples,count_vec[i],data_vec[i]);
        else
            fprintf(stdout, "%06llu/%06llu\t: %08d\n",samples,count_vec[i],binary(data_vec[i],0));
        samples += count_vec[i];
    }
}

auto CLAController::getAnnotationList(la_Decoder_t decoder) -> std::map<uint8_t,std::string>{
    uint8_t count = m_pimpl->getAnnotationSize(decoder);
    std::map<uint8_t,std::string> map;
    for(uint8_t i = 0; i < count; i++){
        map[i] = getAnnotation(decoder,i);
    }
    return map;
}

auto CLAController::getAnnotation(la_Decoder_t decoder, uint8_t control) -> std::string{
    return m_pimpl->getAnnotation(decoder,control);
}

auto CLAController::getDefaultSettings(la_Decoder_t decoder) -> std::string{
    std::shared_ptr<Decoder> ptr = nullptr;
    switch (decoder)
    {
        case LA_DECODER_CAN:
            ptr = std::make_shared<can::CANDecoder>(LA_DECODER_CAN);
            break;
        case LA_DECODER_I2C:
            ptr = std::make_shared<i2c::I2CDecoder>(LA_DECODER_I2C);
            break;
        case LA_DECODER_SPI:
            ptr = std::make_shared<spi::SPIDecoder>(LA_DECODER_SPI);
            break;
        case LA_DECODER_UART:
            ptr = std::make_shared<uart::UARTDecoder>(LA_DECODER_UART);
            break;

    default:
        FATAL("Unknown decoder")
        break;
    }
    return ptr->getParametersInJSON();
}


auto CLAController::decodeNP(la_Decoder_t decoder, std::string json_settings, uint8_t* np_buffer, int size) -> std::vector<rp_la::OutputPacket>{
    std::shared_ptr<Decoder> ptr = nullptr;
    switch (decoder)
    {
        case LA_DECODER_CAN:
            ptr = std::make_shared<can::CANDecoder>(LA_DECODER_CAN);
            break;
        case LA_DECODER_I2C:
            ptr = std::make_shared<i2c::I2CDecoder>(LA_DECODER_I2C);
            break;
        case LA_DECODER_SPI:
            ptr = std::make_shared<spi::SPIDecoder>(LA_DECODER_SPI);
            break;
        case LA_DECODER_UART:
            ptr = std::make_shared<uart::UARTDecoder>(LA_DECODER_UART);
            break;

    default:
        FATAL("Unknown decoder")
        break;
    }

    ptr->setParametersInJSON(json_settings);
    TRACE_CODE(profiler::clearHistory("decodeNP"))
    TRACE_CODE(profiler::setTimePoint("decodeNP"))
    ptr->decode(np_buffer,size);
    TRACE_CODE(profiler::saveTimePointuS("decodeNP","Data size %d",size))
    TRACE_CODE(profiler::printuS("decodeNP","Decoder %s",ptr->name().c_str()))

    TRACE_SHORT("Memory usage %llu",ptr->getMemoryUsage())
    auto items = ptr->getSignal();
    std::vector<rp_la::OutputPacket> new_vect;
    for(auto &itm:items){
        rp_la::OutputPacket pack;
        pack.line_name = itm.line_name;
        pack.control = itm.control;
        pack.data = itm.data;
        pack.length = itm.length;
        pack.bitsInPack = itm.bitsInPack;
        pack.sampleStart = itm.sampleStart;
        //pack.annotation = m_pimpl->getAnnotation(decoder,itm.control);
        new_vect.push_back(pack);
    }
    return new_vect;
}


}