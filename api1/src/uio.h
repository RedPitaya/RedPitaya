#ifndef UIO_H
#define UIO_H

#include <unistd.h>
#include <fcntl.h>
#include <libudev.h>

//! UIO memory mapped region details.
typedef struct {
    char   *name;    //!< name
    size_t  addr;    //!< address
    size_t  offset;  //!< offset
    size_t  size;    //!< size
    void   *mem;     //!< mmap
} rp_uio_map_t;

//! UIO handle.
typedef struct {
    char         *path;  //!< device path
    int           dev;   //!< device file descriptor
    struct flock  lock;  //!< device file exclusive lock
    char         *name;  //!< device tree node name
    int unsigned  mapn;  //!< number of mamory map regions
    rp_uio_map_t *map;   //!< array of mamory map regions
} rp_uio_t;

int rp_uio_init        (rp_uio_t *handle, const char *path);
int rp_uio_release     (rp_uio_t *handle);

int rp_uio_irq_enable  (rp_uio_t *handle);
int rp_uio_irq_disable (rp_uio_t *handle);
int rp_uio_irq_wait    (rp_uio_t *handle);

#endif

