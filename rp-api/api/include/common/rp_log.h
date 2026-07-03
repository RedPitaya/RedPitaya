/**
 * $Id: $
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#ifndef RP_LOG_H
#define RP_LOG_H

#include <stdio.h>
#include <cstring>
#include <string>

#ifdef __cplusplus
#define __SHORT_FUNCTION__ __PRETTY_FUNCTION__
#else
#define __SHORT_FUNCTION__ __func__
#endif

#define __SHORT_FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define FATAL(...)                                                                                                               \
    {                                                                                                                            \
        char error_msg[1024];                                                                                                    \
        snprintf(error_msg, 1024, __VA_ARGS__);                                                                                  \
        fprintf(stderr, "Fatal error at line %d, file %s:%s %s\n", __LINE__, __SHORT_FILENAME__, __SHORT_FUNCTION__, error_msg); \
        exit(1);                                                                                                                 \
    }
#define ERROR_LOG(...)                                                                                        \
    {                                                                                                         \
        char error_msg[1024];                                                                                 \
        snprintf(error_msg, 1024, __VA_ARGS__);                                                               \
        fprintf(stderr, "[E] {%s:%s}(%d) %s\n", __SHORT_FILENAME__, __SHORT_FUNCTION__, __LINE__, error_msg); \
    }
#define WARNING(...)                                                                                          \
    {                                                                                                         \
        char error_msg[1024];                                                                                 \
        snprintf(error_msg, 1024, __VA_ARGS__);                                                               \
        fprintf(stderr, "[W] {%s:%s}(%d) %s\n", __SHORT_FILENAME__, __SHORT_FUNCTION__, __LINE__, error_msg); \
    }

#ifdef TRACE_ENABLE
#define TRACE(...)                                                                                            \
    {                                                                                                         \
        char error_msg[1024];                                                                                 \
        snprintf(error_msg, 1024, __VA_ARGS__);                                                               \
        fprintf(stderr, "[T] {%s:%s}(%d) %s\n", __SHORT_FILENAME__, __SHORT_FUNCTION__, __LINE__, error_msg); \
    }
#define TRACE_SHORT(...)                        \
    {                                           \
        char error_msg[1024];                   \
        snprintf(error_msg, 1024, __VA_ARGS__); \
        fprintf(stderr, "[T] %s\n", error_msg); \
    }

enum class StackTraceFormat {
    SHORT,        // func1 <- func2 <- func3
    WITH_PARAMS,  // func1(int, char*) <- func2() <- main
    FULL          // namespace::Class::func(...) +0x123 [0x555...]
};

std::string getStackTrace(int skipFrames = 2, int maxFrames = 32, StackTraceFormat format = StackTraceFormat::SHORT);

#define TRACE_FUNC() \
    { TRACE("Stack: %s", getStackTrace(2, 5, StackTraceFormat::SHORT).c_str()); }

#define TRACE_FUNC_PARAMS() \
    { TRACE("Stack: %s", getStackTrace(2, 5, StackTraceFormat::WITH_PARAMS).c_str()); }

#define TRACE_FUNC_FULL() \
    { TRACE("Stack trace:\n%s", getStackTrace(0, 32, StackTraceFormat::FULL).c_str()); }

#else
#define TRACE(...)
#define TRACE_SHORT(...)
#define TRACE_FUNC(...)
#define TRACE_FUNC_PARAMS(...)
#define TRACE_FUNC_FULL(...)
#endif

#define ECHECK(x)                                                       \
    {                                                                   \
        int retval = (x);                                               \
        if (retval != RP_OK) {                                          \
            ERROR_LOG("%s returned \"%s\"\n", #x, rp_GetError(retval)); \
            return retval;                                              \
        }                                                               \
    }

#define ECHECK_NO_RET(x)                                              \
    {                                                                 \
        int retval = (x);                                             \
        if (retval != RP_OK) {                                        \
            ERROR_LOG("%s returned \"%s\"", #x, rp_GetError(retval)); \
        }                                                             \
    }

#endif /* RP_LOG_H */