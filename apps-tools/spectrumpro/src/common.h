#ifndef COMMON_H
#define COMMON_H

#include <string>
#include "common/rp_arb.h"
#include "rp.h"
#include "rp_hw-profiles.h"

#define MAX_ADC_CHANNELS 4
#define MAX_DAC_CHANNELS 2

#define INIT2(PREF, SUFF, args...) \
    {                              \
        {PREF "1" SUFF, args}, {   \
            PREF "2" SUFF, args    \
        }                          \
    }
#define INIT(PREF, SUFF, args...)                                              \
    {                                                                          \
        {PREF "1" SUFF, args}, {PREF "2" SUFF, args}, {PREF "3" SUFF, args}, { \
            PREF "4" SUFF, args                                                \
        }                                                                      \
    }

#define CURSORS_COUNT 2

#define XSTR(s) STR(s)
#define STR(s) #s

#define RESEND(X) X.SendValue(X.Value());

#define IS_NEW(X) X.Value() != X.NewValue()

#define MAX_FREQ getMaxFreqRate()
#define LEVEL_AMPS_MAX outAmpMax()
#define LEVEL_AMPS_DEF outAmpDef()
#define MAX_DAC_FREQ getMaxDacFreqRate()

auto getADCChannels() -> uint8_t;
auto getDACChannels() -> uint8_t;
auto getDACRate() -> uint32_t;
auto getADCRate() -> uint32_t;
auto getMaxFreqRate() -> float;
auto getMaxDacFreqRate() -> float;

auto getModel() -> rp_HPeModels_t;
auto isZModePresent() -> bool;
auto getModelName() -> std::string;

auto outAmpDef() -> float;
auto outAmpMax() -> float;
auto loadARBList() -> std::string;

auto outFreqMin() -> int;
auto outFreqMax() -> int;

auto getClock() -> int64_t;

#endif
