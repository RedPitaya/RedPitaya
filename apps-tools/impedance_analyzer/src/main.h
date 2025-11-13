#pragma once

#include <CustomParameters.h>
#include <DataManager.h>
#include <sys/syslog.h>  //Add custom RP_LCR LOG system
#include <thread>
#include "rp.h"

#define CH_SIGNAL_SIZE_DEFAULT 1024

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

#ifdef __cplusplus
extern "C" {
#endif

//Rp app functions
const char* rp_app_desc(void);
int rp_app_init(void);
int rp_app_exit(void);

auto getModelS() -> std::string;
auto getMaxADC() -> uint32_t;

void updateParametersByConfig();

#ifdef __cplusplus
}
#endif
