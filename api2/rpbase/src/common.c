/**
 * $Id: $
 *
 * @brief Red Pitaya library common module implementation
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <math.h>

#include "common.h"

static int fd = NULL;

int cmn_Map(size_t size, size_t offset, void** mapped)
{
    if(fd == -1) {
        return RP_EMMD;
    }

    *mapped = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);

    if(mapped == (void *) -1) {
        return RP_EMMD;
    }

    return RP_OK;
}

int cmn_Unmap(size_t size, void** mapped)
{
    if(fd == -1) {
        return RP_EUMD;
    }

    if((mapped == (void *) -1) || (mapped == NULL)) {
        return RP_EUMD;
    }

    if((*mapped == (void *) -1) || (*mapped == NULL)) {
        return RP_EUMD;
    }

    if(munmap(*mapped, size) < 0){
        return RP_EUMD;
    }
    *mapped = NULL;
    return RP_OK;
}

