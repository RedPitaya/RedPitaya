#ifndef COMMON_H
#define COMMON_H

#include <string>
#include "rp.h"
#include "rp_hw-profiles.h"

#define EXEC_CHECK_MUTEX(x, mutex)          \
    {                                       \
        int retval = (x);                   \
        if (retval != RP_OK) {              \
            pthread_mutex_unlock((&mutex)); \
            return retval;                  \
        }                                   \
    }

#define IF_VALUE_CHANGED(X, ACTION)  \
    if (X.Value() != X.NewValue()) { \
        int res = ACTION;            \
        if (res == RP_OK) {          \
            X.Update();              \
        }                            \
    }

#define IF_VALUE_CHANGED_BOOL(X, ACTION1, ACTION2) \
    if (X.Value() != X.NewValue()) {               \
        if (X.NewValue()) {                        \
            ACTION1;                               \
            X.Update();                            \
        } else {                                   \
            ACTION2;                               \
            X.Update();                            \
        }                                          \
    }

#define IS_NEW(X) X.Value() != X.NewValue()

auto getADCChannels() -> uint8_t;
auto getDACChannels() -> uint8_t;
auto getDACRate() -> uint32_t;
auto getADCRate() -> uint32_t;
auto getModel() -> rp_HPeModels_t;
auto getDACDevider() -> double;
auto getModelName() -> std::string;
auto getADCSamplePeriod(double* value) -> int;

#endif
