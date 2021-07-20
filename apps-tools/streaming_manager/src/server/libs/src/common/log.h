#pragma once

#ifndef NDEBUG
    #define LOG_P(...) fprintf(stderr,__VA_ARGS__)
#else
    #define LOG_P(...)
#endif