#include "thread_cout.h"
#include <iostream>
#include <mutex>

static std::mutex g_mtx;

void aprintf(FILE * stream, const char *format, ...){
    const std::lock_guard lock(g_mtx);
    va_list args;
    va_start( args, format );
    vfprintf( stream, format, args );
    va_end( args );
}
