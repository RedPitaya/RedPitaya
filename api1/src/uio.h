#ifndef UIO_H
#define UIO_H

#include <unistd.h>
#include <fcntl.h>
#include <libudev.h>

typedef struct {
    char   *name;
    size_t  addr;
    size_t  offset;
    size_t  size;
    void   *mem;
} rp_uio_map_t;

typedef struct {
    char         *path;
    int           dev;
    struct flock  lock;
    char         *name;
    int unsigned  mapn;
    rp_uio_map_t *map;
} rp_uio_t;

int rp_uio_init        (rp_uio_t *handle, const char *path);
int rp_uio_release     (rp_uio_t *handle);

int rp_uio_irq_enable  (rp_uio_t *handle);
int rp_uio_irq_disable (rp_uio_t *handle);
int rp_uio_irq_wait    (rp_uio_t *handle);

#endif

