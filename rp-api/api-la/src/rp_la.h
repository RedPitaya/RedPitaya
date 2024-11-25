/**
 * $Id: $
 *
 * @brief Red Pitaya library logic analizer api
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */


#ifndef __RP_LA_H
#define __RP_LA_H

#include <stdint.h>
#include <string>

namespace rp_la{

typedef enum {
    LA_BASIC = 0,
    LA_PRO   = 1
} la_Mode_t;

typedef enum {
    LA_DECODER_CAN = 1,
    LA_DECODER_I2C = 2,
    LA_DECODER_SPI = 3,
    LA_DECODER_UART = 4
} la_Decoder_t;

typedef enum {
    LA_ERROR               = -1,
    LA_NONE                = 0,
    LA_LOW                 = 1,
    LA_HIGH                = 2,
    LA_RISING              = 3,
    LA_FALLING             = 4,
    LA_RISING_OR_FALLING   = 5
} la_Trigger_Mode_t;

class CLAController;

class CLACallback
{
   public:
      virtual ~CLACallback(){}
      virtual void captureStatus(CLAController* controller,bool isTimeout){}
};

class CLAController
{

public:
    CLAController();
    ~CLAController();

    auto setMode(la_Mode_t mode) -> void;

    auto setTrigger(uint8_t channel, la_Trigger_Mode_t mode) -> void;
    auto getTrigger(uint8_t channel) -> la_Trigger_Mode_t;
    auto isNoTriggers() -> bool;
    auto resetTriggers() -> void;
    auto softwareTrigger() -> bool;

    auto setEnableRLE(bool enable) -> bool;
    auto isRLEEnable() -> bool;

    auto addDecoder(std::string name, la_Decoder_t decoder) -> bool;
    auto removeDecoder(std::string name) -> bool;
    auto removeAllDecoders() -> void;

    auto setDecimation(uint32_t decimation) -> void;
    auto getDecimation() -> uint32_t;

    auto setPreTriggerSamples(uint32_t value) -> void;
    auto getPreTriggerSamples() -> uint32_t;
    auto setPostTriggerSamples(uint32_t value) -> void;
    auto getPostTriggerSamples() -> uint32_t;


	auto setDelegate(CLACallback *callbacks) -> void;
	auto removeDelegate() -> void;

    auto isCaptureRun() -> bool;

    // TODO timeout
    // timeout - Timeout in mS. 0 - Disable
    // auto run(uint32_t timeoutMs) -> void;

    // TODO
    // timeout - Timeout in mS. 0 - Disable
    auto runAsync(uint32_t timeoutMs) -> void;

    auto wait(uint32_t timeoutMs, bool *isTimeout) -> void;

    // TODO
    // auto wait() -> void;

    auto saveCaptureDataToFile(std::string file) -> bool;

private:

    CLAController(const CLAController &) = delete;
    CLAController(CLAController &&) = delete;
    CLAController& operator=(const CLAController&) = delete;
    CLAController& operator=(const CLAController&&) = delete;

    struct Impl;
    // Pointer to the internal implementation
    Impl *m_pimpl;
};



}

#endif // __RP_LA_H
