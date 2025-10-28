#pragma once

#include <sys/syslog.h>  //Add custom RP_LCR LOG system

#include <CustomParameters.h>
#include <DataManager.h>

#include <complex.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/syslog.h>
#include <unistd.h>
#include <ctime>
#include <fstream>

#include <algorithm>
#include <mutex>
#include <thread>
#include <vector>

#include <signal.h>
#include <unistd.h>

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

auto isDACAviable() -> bool;

#ifdef __cplusplus
}
#endif
