/**
* $Id: $
*
* @brief Red Pitaya library I2C module implementation
*
* @Author Red Pitaya
*
* (c) Red Pitaya  http://www.redpitaya.com
*
* This part of code is written in C programming language.
* Please visit http://en.wikipedia.org/wiki/C_(programming_language)
* for more details on the language used herein.
*/


#include "i2c.h"
#include "rp.h"
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>


int fd = -1;

int i2c_Init() {
    char filename[40];

    if (fd == -1) {
        sprintf(filename, I2C_DEVICE_NAME);
        if ((fd = open(filename, O_RDWR)) < 0) {
            return RP_EFOB;
        }
    }
    return RP_OK;
}

int i2c_Release() {
    if (close(fd)) {
        return RP_EFCB;
    }
    fd = -1;
    return RP_OK;
}

int i2c_read(int addr, char *data, int length) {
    if (ioctl(fd, I2C_SLAVE, addr) < 0) {
        return RP_EABA;
    }

    if (read(fd, data, (size_t) length) == -1) {
        return RP_EFRB;
    }
    return RP_OK;
}

int i2c_write(int addr, char *data, int length) {
    if (ioctl(fd, I2C_SLAVE, addr) < 0) {
        return RP_EABA;
    }

    if (write(fd, data, (size_t) length) == -1) {
        printf("%d:\t%s\n", errno, strerror(errno));
        return RP_EFWB;
    }
    return RP_OK;
}