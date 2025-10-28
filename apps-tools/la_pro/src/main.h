#pragma once

#include <CustomParameters.h>
#include <DataManager.h>

#include "rp_la.h"

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

#define INIT8(PREF, SUFF, args...)                                                                                                                                         \
    {                                                                                                                                                                      \
        {PREF "1" SUFF, args}, {PREF "2" SUFF, args}, {PREF "3" SUFF, args}, {PREF "4" SUFF, args}, {PREF "5" SUFF, args}, {PREF "6" SUFF, args}, {PREF "7" SUFF, args}, { \
            PREF "8" SUFF, args                                                                                                                                            \
        }                                                                                                                                                                  \
    }

#define INIT(PREF, SUFF, args...) INIT4(PREF, SUFF, args)

#ifdef __cplusplus
extern "C" {
#endif

//Rp app functions
const char* rp_app_desc(void);
int rp_app_init(void);
int rp_app_exit(void);

void updateFromFront(bool force);

enum controlSettings { NONE = 0, REQUEST_RESET = 1, RESET_DONE = 2, REQUEST_LIST = 3, SAVE = 4, DELETE = 5, LOAD = 6, LOAD_DONE = 7 };

class CLACallbackHandler : public rp_la::CLACallback {
   public:
    ~CLACallbackHandler() override{};
    void captureStatus(rp_la::CLAController* controller, bool isTimeout, uint32_t numBytes, uint64_t numSamples, uint64_t preTriggerSamples, uint64_t postTriggerSamples) override;
    void decodeStatus(rp_la::CLAController* controller, uint32_t numBytes, uint64_t numSamples, uint64_t preTriggerSamples, uint64_t postTriggerSamples) override;
    void decodeDone(rp_la::CLAController* controller, std::string name) override;
};

#ifdef __cplusplus
}
#endif