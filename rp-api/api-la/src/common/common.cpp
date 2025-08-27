#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "rp.h"
#include "rp_hw-profiles.h"

int common_Open(const std::string dev, rp_handle_uio_t* handle) {
    handle->dev = dev;
    // try opening the device
    handle->fd = open(handle->dev.c_str(), O_RDWR);
    if (handle->fd == -1) {
        ERROR_LOG("Error open device: %s", dev.c_str())
        return -1;
    }
    // get regset pointer
    handle->regset = mmap(NULL, handle->length, PROT_READ | PROT_WRITE, MAP_SHARED, handle->fd, 0x0);
    if (handle->regset == MAP_FAILED) {
        ERROR_LOG("Error map memory")
        close(handle->fd);
        return -1;
    }
    return 0;
}

int common_Close(rp_handle_uio_t* handle) {
    int r = 0;
    // release regset
    if (munmap((void*)handle->regset, handle->length) == -1) {
        ERROR_LOG("Error unmap memory")
        r = -1;
    }
    // close device
    if (close(handle->fd) == -1) {
        ERROR_LOG("Error close device")
        r = -1;
    }
    return r;
}

/**
 *  Check if value x is in range
 *  @return true if value x is in [minval,maxval] range
 */
bool inrangeUint32(uint32_t x, uint32_t minval, uint32_t maxval) {
    return (x >= minval) && (x <= maxval);
}

bool inrangeDouble(double x, double minval, double maxval) {
    return (x >= minval) && (x <= maxval);
}

uint32_t getMaxFreq() {
    uint32_t c = 0;
    if (rp_HPGetBaseSpeedHz(&c) != RP_HP_OK) {
        FATAL("Can't get base speed");
    }
    return c;
}

auto getClockMs() -> double {
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    return ((double)tp.tv_sec * 1000.f) + ((double)tp.tv_nsec / 1000000.f);
}
