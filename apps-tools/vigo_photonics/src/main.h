#pragma once

#include <CustomParameters.h>
#include <DataManager.h>
#include "rp.h"
#include "rpApp.h"

#define CH_SIGNAL_SIZE_DEFAULT 1024

#define IF_VALUE_CHANGED_FORCE(X, ACTION, FORCE) \
    if (X.Value() != X.NewValue() || FORCE) {    \
        int res = ACTION;                        \
        if (res == RP_OK) {                      \
            X.Update();                          \
        }                                        \
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

#define XSTR(s) STR(s)
#define STR(s) #s

#define INIT2(PREF, SUFF, args...) \
    {                              \
        {PREF "1" SUFF, args}, {   \
            PREF "2" SUFF, args    \
        }                          \
    }

#define INIT4(PREF, SUFF, args...)                                             \
    {                                                                          \
        {PREF "1" SUFF, args}, {PREF "2" SUFF, args}, {PREF "3" SUFF, args}, { \
            PREF "4" SUFF, args                                                \
        }                                                                      \
    }

#define INIT(PREF, SUFF, args...) INIT4(PREF, SUFF, args)

#ifdef __cplusplus
extern "C" {
#endif

const char* rp_app_desc(void);
int rp_app_init(void);
int rp_app_exit(void);

void updateSlowDAC(bool force);

#ifdef __cplusplus
}
#endif
