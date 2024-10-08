/**
 * $Id: $
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef RP_LOG_H
#define RP_LOG_H

#include <stdio.h>
#include <string.h>

#define __SHORT_FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define FATAL(...) \
	{ \
		char error_msg[1024]; \
		snprintf(error_msg, 1024, __VA_ARGS__); \
		fprintf(stderr, "Fatal error at line %d, file %s:%s %s\n", __LINE__, __SHORT_FILENAME__, __PRETTY_FUNCTION__, error_msg); \
		exit(1); \
	}
#define ERROR_LOG(...) \
	{ \
		char error_msg[1024]; \
		snprintf(error_msg, 1024, __VA_ARGS__); \
		fprintf(stderr,"[E] {%s:%s}(%d) %s\n", __SHORT_FILENAME__, __PRETTY_FUNCTION__, __LINE__, error_msg); \
	}
#define WARNING(...) \
	{ \
		char error_msg[1024]; \
		snprintf(error_msg, 1024, __VA_ARGS__); \
		fprintf(stderr, "[W] {%s:%s}(%d) %s\n", __SHORT_FILENAME__, __PRETTY_FUNCTION__, __LINE__, error_msg); \
	}

#ifdef TRACE_ENABLE
#define TRACE(...) \
	{ \
		char error_msg[1024]; \
		snprintf(error_msg, 1024, __VA_ARGS__); \
		fprintf(stderr, "[T] {%s:%s}(%d) %s\n", __SHORT_FILENAME__, __PRETTY_FUNCTION__, __LINE__, error_msg); \
	}
#define TRACE_SHORT(...) \
	{ \
		char error_msg[1024]; \
		snprintf(error_msg, 1024, __VA_ARGS__); \
		fprintf(stderr, "[T] %s\n", error_msg); \
	}
#else
#define TRACE(...)
#define TRACE_SHORT(...)
#endif

#define ECHECK(x) { \
    int retval = (x); \
    if (retval != RP_OK) { \
        ERROR_LOG("%s returned \"%s\"\n", #x, rp_GetError(retval)); \
        return retval; \
    } \
}

#define ECHECK_NO_RET(x) { \
    int retval = (x); \
    if (retval != RP_OK) { \
        ERROR_LOG("%s returned \"%s\"", #x, rp_GetError(retval)); \
    } \
}

#endif /* RP_LOG_H */