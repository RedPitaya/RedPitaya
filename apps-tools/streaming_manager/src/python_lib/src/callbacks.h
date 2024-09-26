#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <string>
#include <stdint.h>
#include <vector>
#include "logger_lib/file_logger.h"

struct ADCChannel{
    uint32_t samples = 0;
    uint8_t bitsBySample = 0;
    uint64_t fpgaLost = 0;
    bool attenuator_1_20 = 0;
    uint32_t baseRate = 0;
    uint8_t adcBaseBits = 0;
    uint64_t packId = 0;
    std::vector<int16_t> raw;
};

struct ADCPack{
    std::string host;
    ADCChannel channel1;
    ADCChannel channel2;
    ADCChannel channel3;
    ADCChannel channel4;
};

class ADCStreamClient;

class ADCCallback
{
   public:
      virtual ~ADCCallback(){}
      virtual void recievePack(ADCStreamClient*, ADCPack&){}
};

#endif