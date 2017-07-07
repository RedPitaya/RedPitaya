/**
 * @file uio.c 
 * @author Iztok Jeras
 * @brief User space access to UIO devices.
 * 
 * Functions are provided for initializing and releasing UIO devices.
 * Initialization uses UDEV tools to read details from UIO device tree nodes.
 * Additionaly functions for handling interrupts are provided.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libudev.h>
#include <dirent.h>
#include <stdint.h>
#include <sys/mman.h>

#include "redpitaya/uio.h"

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

/**
 * @brief UIO device initialization.
 * 
 * Steps for initilizing an UIO device:
 * 
 * 1. The provided UIO device file is first opened.
 * 2. An attempt is made to exclusively lock the device file.
 *    If another process has already locked this file
 *    an error will be rised. This prevents multiple
 *    applications from accessing the same HW module.
 * 3. If locking is sucessfull sysfs arguments will be read
 *    to determine the maps listed in the device tree.
 *    All maps will be `mmap`ed into memory and provided
 *    as a an array of structures.
 * 
 * @param[in,out] handle Handle structure describing an UIO device.
 * @param[in] path Full UIO device file system path.
 */

int rp_uio_init (rp_uio_t *handle, const char *path) {
    struct udev_device *udev;
    size_t len;

    // store UIO device node path
    len = strlen(path)+1;
    handle->path = malloc(len);
    strncpy(handle->path, path, len);
    if (!handle->path) {
        fprintf(stderr, "UIO: failed to allocate memory for device node path string '%s'.\n", path);
        return (-1);
    }
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
        fprintf(stderr, "UIO: failed to obtain lock on device node path '%s'.\n", handle->path);
        return (-1);
    }

    // get UDEV device structure
    udev = rp_uio_udev(path);

    const char *path_sys = NULL;
    path_sys = udev_device_get_syspath(udev);
    if (!path_sys) {
        fprintf(stderr, "UIO: failed to obtainsysfs path from device node path.\n");
        return (-1);
    }
    len = strlen(udev_device_get_sysattr_value(udev, "name"))+1;
    handle->name = malloc(len * sizeof(char));
    strncpy(handle->name, udev_device_get_sysattr_value(udev, "name"), len);

    const char path_maps[] = "/maps";
    int unsigned len_maps = strlen(path_sys);
    int unsigned len_sys  = strlen(path_maps);
    char path_sys_maps[len_sys+len_maps+1];
    strcpy(path_sys_maps, path_sys);
    strcat(path_sys_maps, path_maps);

    DIR *dir;
    struct dirent *entry;

    if ((dir = opendir(path_sys_maps)) == NULL) {
        fprintf(stderr, "UIO: failed to open 'maps' directory.\n");
        return (-1);
    } else {
        handle->mapn = 0;
        while ((entry = readdir(dir)) != NULL) {
            if (!strncmp(entry->d_name, "map", 3)) {
                handle->mapn++;
            }
        }
        rewinddir(dir);
        handle->map = malloc(handle->mapn * sizeof(rp_uio_map_t));

        while ((entry = readdir(dir)) != NULL) {
            if (!strncmp(entry->d_name, "map", 3)) {
                int unsigned i = atoi(entry->d_name+strlen("map"));

                char path_map [strlen("maps/")+strlen(entry->d_name)+1];
                strcpy(path_map, "maps/");
                strcat(path_map, entry->d_name);

                char path_map_name [strlen(path_map)+strlen("/name")+1];
                strcpy(path_map_name, path_map);
                strcat(path_map_name, "/name");
                len = strlen(udev_device_get_sysattr_value(udev, path_map_name))+1;
                handle->map[i].name = malloc(len * sizeof(char));
                strcpy(handle->map[i].name, udev_device_get_sysattr_value(udev, path_map_name));

                char path_map_addr [strlen(path_map)+strlen("/addr")+1];
                strcpy(path_map_addr, path_map);
                strcat(path_map_addr, "/addr");
                handle->map[i].addr = strtol(udev_device_get_sysattr_value(udev, path_map_addr), NULL, 0);

                char path_map_offset [strlen(path_map)+strlen("/offset")+1];
                strcpy(path_map_offset, path_map);
                strcat(path_map_offset, "/offset");
                handle->map[i].offset = strtol(udev_device_get_sysattr_value(udev, path_map_offset), NULL, 0);

                char path_map_size [strlen(path_map)+strlen("/size")+1];
                strcpy(path_map_size, path_map);
                strcat(path_map_size, "/size");
                handle->map[i].size = strtol(udev_device_get_sysattr_value(udev, path_map_size), NULL, 0);
            }
        }
        closedir(dir);
    }

    // memory map
    for (int unsigned i=0; i<handle->mapn; i++) {
        handle->map[i].mem = mmap(NULL, handle->map[i].size, PROT_READ | PROT_WRITE, MAP_SHARED, handle->dev, i*sysconf(_SC_PAGESIZE));
        if (handle->map[i].mem == MAP_FAILED) {
            fprintf(stderr, "UIO: failed to perform mmap.\n");
            return (-1);
        }
    }

    return 0;
}

/**
 * @brief UIO device release.
 * 
 * Steps for releasing an UIO device:
 * 
 * 1. Close all memory mappings.
 * 2. Close the UIO device file, which also releases the exclusive lock.
 * 
 * @param[in,out] handle Handle structure describing an UIO device.
 */

int rp_uio_release (rp_uio_t *handle) {

    // destroy array of UIO map structures
    for (int unsigned i=0; i<handle->mapn; i++) {
        free(handle->map[i].name);
    }
    free(handle->map);
    free(handle->name);
    // close device file
    close(handle->dev);
    // free UIO device node path
    free(handle->path);

    return (0);
}

/**
 * @brief Enable UIO interrupt.
 * 
 * @param[in] handle Handle structure describing an UIO device.
 */

int rp_uio_irq_enable (rp_uio_t *handle) {
    uint32_t cnt = 1;

    int status = write (handle->dev, &cnt, sizeof(cnt));
    if (status != sizeof(cnt)) {
        fprintf(stderr, "UIO: failed to enable IRQ.\n");
        return (-1);
    }

    return (0);
}

/**
 * @brief Disable UIO interrupt.
 * 
 * @param[in] handle Handle structure describing an UIO device.
 */

int rp_uio_irq_disable (rp_uio_t *handle) {
    uint32_t cnt = 0;

    int status = write (handle->dev, &cnt, sizeof(cnt));
    if (status != sizeof(cnt)) {
        fprintf(stderr, "UIO: failed to disable IRQ.\n");
        return (-1);
    }

    return (0);
}

/**
 * @brief Wait for UIO interrupt.
 * 
 * @param[in] handle Handle structure describing an UIO device.
 */

int rp_uio_irq_wait (rp_uio_t *handle) {
    uint32_t cnt;

    int status = read(handle->dev, &cnt, sizeof(cnt));
    if (status != sizeof(cnt)) {
        fprintf(stderr, "UIO: failed to wait for IRQ.\n");
        return (-1);
    }

    return (0);
}

