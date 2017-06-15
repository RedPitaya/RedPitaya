#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libudev.h>

#include "uio.h"

static struct udev_device * rp_uio_udev (const char *path) {
    struct udev *udev;
    struct stat statbuf;

    // create UDEV structure
    udev = udev_new();
    if (udev == NULL) {
        fprintf( stderr, "UIO: failed to create an UDEV structure.\n");
        return (NULL);
    }
    // get device number from device node path
    if (stat(path, &statbuf) < 0) {
        fprintf( stderr, "UIO: failed to stat device node path '%s'.\n", path);
        return (NULL);
    }
    // get character device ('c') with device number 'st_rdev'
    return udev_device_new_from_devnum(udev, 'c', statbuf.st_rdev);
}

int rp_uio_init (rp_uio_t *handle, char *path) {
    struct udev_device *udev;

    // store UIO device node path
    size_t len = strlen(path)+1;
    handle->path = malloc(len);
    strncpy(handle->path, path, len);
    if (!handle->path) {
        fprintf(stderr, "UIO: failed to allocate memory for device node path string '%s'.\n", path);
        return (-1);
    }
    strcpy(path, handle->path);
    // open device node file
    handle->dev = open(handle->path, O_RDWR);
    if (!handle->dev) {
        fprintf(stderr, "UIO: failed to open device node file '%s'.\n", handle->path);
        return (-1);
    }
    // exclusive lock
    handle->lock.l_type   = F_WRLCK;  // write lock
    handle->lock.l_whence = SEEK_SET; // from beginning of file
    handle->lock.l_start  = 0;        // no offset from beginning
    handle->lock.l_len    = 0;        // till EOF
    handle->lock.l_pid    = getpid();
    int status = fcntl(handle->dev, F_SETLK, &handle->lock);
    if (status == -1) {
        fprintf( stderr, "UIO: failed to obtain lock on device node path '%s'.\n", handle->path);
        return (-1);
    }

    // get UDEV device structure
    udev = rp_uio_udev(path);

    printf("udev_device_get_syspath: %s\n", udev_device_get_syspath(udev));
    printf("udev_device_get_sysattr_value: %s\n", udev_device_get_sysattr_value(udev, "name"));
    
    return 0;
}

int rp_uio_release (rp_uio_t *handle) {
    // close device file
    close(handle->dev);
    // free UIO device node path
    free(handle->path);

    return (0);
}
