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
#include <vector>

namespace rp_la{

struct OutputPacket
{
    std::string line_name;
    uint8_t control; // 0 when data, elsewise represents specific state
					 // anyway control byte specifies meaning of the “data” byte
    uint32_t data;
	uint32_t length; // RLE, how many counts takes this byte
    float    bitsInPack;  // Number of bits
    uint32_t sampleStart;
};

typedef enum {
    LA_BASIC = 0,
    LA_PRO   = 1
} la_Mode_t;

typedef enum {
    LA_DECODER_NONE = 0,
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
      virtual void captureStatus(CLAController* controller,
                                bool isTimeout,
                                uint32_t numBytes,
                                uint64_t numSamples,
                                uint64_t preTriggerSamples,
                                uint64_t postTriggerSamples){}
      virtual void decodeStatus(CLAController* controller,
                                uint32_t numBytes,
                                uint64_t numSamples,
                                uint64_t preTriggerSamples,
                                uint64_t postTriggerSamples){}
      virtual void decodeDone(CLAController* controller, std::string name){}
};

class CLAController
{

public:
    CLAController();
    ~CLAController();

    auto initFpga() -> bool;

    auto setMode(la_Mode_t mode) -> void;

    auto setTrigger(uint8_t channel, la_Trigger_Mode_t mode) -> void;
    auto getTrigger(uint8_t channel) -> la_Trigger_Mode_t;
    auto isNoTriggers() -> bool;
    auto resetTriggers() -> void;
    auto softwareTrigger() -> bool;

    auto setEnableRLE(bool enable) -> bool;
    auto isRLEEnable() -> bool;

    auto addDecoder(std::string name, la_Decoder_t decoder) -> bool;
    auto setDecoderSettings(std::string name,std::string json) -> bool;
    auto getDecoderSettings(std::string name) -> std::string;

    auto setDecoderSettingsUInt(std::string name, std::string key, uint32_t value) -> bool;
    auto setDecoderSettingsFloat(std::string name, std::string key, float value) -> bool;

    auto getDecoderSettingsUInt(std::string name, std::string key, uint32_t *value) -> bool;
    auto getDecoderSettingsFloat(std::string name, std::string key, float *value) -> bool;

    auto getDefaultSettings(la_Decoder_t decoder) -> std::string;
    auto getDecoders() -> std::vector<std::string>;
    auto getDecodedData(std::string name) -> std::vector<rp_la::OutputPacket>;
    auto setDecoderEnable(std::string name, bool enable) -> void;
    auto getDecoderEnable(std::string name) -> bool;
    auto getDecoderType(std::string name) -> la_Decoder_t;
    auto isDecoderExist(std::string name) -> bool;
    auto removeDecoder(std::string name) -> bool;
    auto removeAllDecoders() -> void;

    auto setDecimation(uint32_t decimation) -> void;
    auto getDecimation() -> uint32_t;

    auto setPreTriggerSamples(uint32_t value) -> void;
    auto getPreTriggerSamples() -> uint32_t;
    auto setPostTriggerSamples(uint32_t value) -> void;
    auto getPostTriggerSamples() -> uint32_t;

    auto getDMAMemorySize() -> uint32_t;
    auto getCapturedDataSize() -> uint32_t;
    auto getCapturedSamples() -> uint64_t;

	auto setDelegate(CLACallback *callbacks) -> void;
	auto removeDelegate() -> void;

    auto isCaptureRun() -> bool;

    // TODO timeout
    // timeout - Timeout in mS. 0 - Disable
    // auto run(uint32_t timeoutMs) -> void;

    // TODO
    // timeout - Timeout in mS. 0 - Disable
    auto runAsync(uint32_t timeoutMs) -> void;
    auto runAsync(std::vector<uint8_t> data, bool isRLE, uint64_t triggerSamplePosition) -> void;
    auto decodeAsync() -> void;
    auto wait(uint32_t timeoutMs, bool *isTimeout) -> void;

    auto decode(std::string name) -> std::vector<rp_la::OutputPacket>;

    // TODO
    // auto wait() -> void;

    auto saveCaptureDataToFile(std::string file) -> bool;
    auto loadFromFile(std::string file, bool isRLE, uint64_t triggerSamplePosition) -> bool;
    auto loadFromFileAndDecode(std::string file, bool isRLE, uint64_t triggerSamplePosition) -> bool;


    auto getDataNP(uint8_t* np_buffer, int size) -> uint32_t;
    auto getUnpackedRLEDataNP(uint8_t* np_buffer, int size) -> uint64_t;

    auto printRLE(bool useHex) -> void;
    auto printRLENP(uint8_t* np_buffer, int size, bool useHex) -> void;
    auto getAnnotationList(la_Decoder_t decoder) -> std::map<uint8_t,std::string>;
    auto getAnnotation(la_Decoder_t decoder, uint8_t control) -> std::string;

    auto decodeNP(la_Decoder_t decoder, std::string json_settings, uint8_t* np_buffer, int size) -> std::vector<rp_la::OutputPacket>;

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
