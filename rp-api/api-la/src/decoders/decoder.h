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
    uint8_t control; // 0 when data, elsewise represents specific state
					 // anyway control byte specifies meaning of the “data” byte
    uint32_t data;
	uint32_t length; // RLE, how many counts takes this byte
	// std::string annotation;
};

class Decoder
{
public:
    virtual void decode(const uint8_t* _input, uint32_t _size) = 0;
    virtual ~Decoder() {}
    virtual std::string getParametersInJSON() { return ""; };
    virtual void setParametersInJSON(const std::string &) {};
    virtual uint64_t getMemoryUsage() { return 0;};
    virtual std::vector<OutputPacket> getSignal() {return {};};
    virtual void reset() {};
    std::string name() { return m_name;};
    int  getDecoderType() {return m_decoderType;};
    bool getEnabled() {return m_enabled;};
    void setEnabled(bool enabled) { m_enabled = enabled;};

protected:
    int m_decoderType = 0;
    std::string m_name;
    bool m_enabled = true;
};

class DecoderParameters
{
public:
    virtual std::string toJson() = 0;
    virtual bool fromJson(const std::string &) = 0;
    virtual ~DecoderParameters() {}
};

#endif // __DECODER_API_H
