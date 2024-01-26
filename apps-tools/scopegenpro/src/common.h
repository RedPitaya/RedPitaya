#ifndef COMMON_H
#define COMMON_H

#include <string>
#include "rp.h"
#include "rpApp.h"
#include "rp_hw-profiles.h"

#define __SHORT_FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define FATAL(X)  {fprintf(stderr, "Error at line %d, file %s errno %d [%s] %s\n", __LINE__, __SHORT_FILENAME__, errno, strerror(errno),X); exit(1);}
#define WARNING(...) { char error_msg[512]; snprintf(error_msg,512,__VA_ARGS__);fprintf(stderr,"[W] %s:%d %s\n",__SHORT_FILENAME__,__LINE__,error_msg);}

#ifdef TRACE_ENABLE
#define TRACE(...) { char error_msg[512]; snprintf(error_msg,512,__VA_ARGS__);fprintf(stderr,"[T] %s:%d %s\n",__SHORT_FILENAME__,__LINE__,error_msg);}
#define TRACE_SHORT(...) { char error_msg[512]; snprintf(error_msg,512,__VA_ARGS__);fprintf(stderr,"[T] %s\n",error_msg);}
#else
#define TRACE(...)
#define TRACE_SHORT(...)
#endif

#define MAX_ADC_CHANNELS 4
#define MAX_DAC_CHANNELS 2

auto getADCChannels() -> uint8_t;
auto getDACChannels() -> uint8_t;
auto getDACRate() -> uint32_t;
auto getADCRate() -> uint32_t;
auto getMaxFreqRate() -> float;
auto getMaxTriggerLevel() -> float;

auto getModel() -> rp_HPeModels_t;
auto isZModePresent() -> bool;
auto getModelName() -> std::string;

auto outAmpDef() -> float;
auto outAmpMax() -> float;

auto getMeasureValue(int measure) -> float;

#endif
