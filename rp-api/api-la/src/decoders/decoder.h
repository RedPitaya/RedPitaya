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


#ifndef __DECODER_API_H
#define __DECODER_API_H

#include <stdint.h>
#include <string>
#include <vector>

struct OutputPacket
{
    std::string line_name;
    uint8_t control;                // 0 when data, elsewise represents specific state
					                // anyway control byte specifies meaning of the “data” byte
    uint32_t data;
    float    bitsInPack;            // How many bits detected
    double   sampleStart;
    double   length;
};

class Decoder
{
public:
    virtual void decode(const uint8_t* _input, uint32_t _size) = 0;
    virtual ~Decoder() {}
    virtual auto getParametersInJSON() -> std::string { return ""; };
    virtual auto setParametersInJSON(const std::string &) -> void {};
    virtual auto getMemoryUsage() -> uint64_t { return 0;};
    virtual auto getSignal() -> std::vector<OutputPacket> {return {};};
    virtual auto reset() -> void {};

    virtual auto setDecoderSettingsUInt(std::string&, uint32_t ) -> bool {return false;};
    virtual auto setDecoderSettingsFloat(std::string&, float ) -> bool {return false;};
    virtual auto getDecoderSettingsUInt(std::string&, uint32_t *) -> bool {return false;};
    virtual auto getDecoderSettingsFloat(std::string&, float *) -> bool {return false;};

    auto name() -> std::string { return m_name;};
    auto getDecoderType() -> int {return m_decoderType;};
    auto getEnabled() -> bool {return m_enabled;};
    auto setEnabled(bool enabled) -> void { m_enabled = enabled;};

protected:
    int m_decoderType = 0;
    std::string m_name = "";
    bool m_enabled = true;
};

class DecoderParameters
{
public:
    virtual std::string toJson() = 0;
    virtual bool fromJson(const std::string &) = 0;
    virtual ~DecoderParameters() {}

    virtual auto setDecoderSettingsUInt(std::string& , uint32_t ) -> bool {return false;};
    virtual auto setDecoderSettingsFloat(std::string& , float ) -> bool {return false;};

    virtual auto getDecoderSettingsUInt(std::string& , uint32_t *) -> bool {return false;};
    virtual auto getDecoderSettingsFloat(std::string& , float *) -> bool {return false;};
};

#endif // __DECODER_API_H
