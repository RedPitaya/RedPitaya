
#ifndef PROJECT_PRINTTHREAD_H
#define PROJECT_PRINTTHREAD_H

#include <stdarg.h>
#include <cstdio>

// Asynchronous output

void aprintf(FILE * stream, const char *format, ...);

#endif
