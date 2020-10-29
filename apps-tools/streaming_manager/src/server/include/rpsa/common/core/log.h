#pragma once

#ifndef NDEBUG
    #define LOG_P(...) printf(__VA_ARGS__)
#else
    #define LOG_P(...)
#endif