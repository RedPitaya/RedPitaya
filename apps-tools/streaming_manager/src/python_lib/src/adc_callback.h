#ifndef ADC_CALLBACK_H
#define ADC_CALLBACK_H

#include <string>
#include <stdint.h>
#include <vector>
#include "logger_lib/file_logger.h"

struct ADCPack{
    std::string host;
    uint64_t fpgaLost;
    std::vector<int16_t> ch1_raw;
    std::vector<int16_t> ch2_raw;
    std::vector<int16_t> ch3_raw;
    std::vector<int16_t> ch4_raw;
};

class ADCCallback
{
   public:
      virtual ~ADCCallback(){}
      virtual void recievePack(ADCPack&){}
};

#endif