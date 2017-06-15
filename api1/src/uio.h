#include <unistd.h>
#include <fcntl.h>
#include <libudev.h>

typedef struct {
    char*  name;
    size_t addr;
    size_t offset;
    size_t size;
} rp_uio_map_t;

typedef struct {
    char*              path;
    int                dev;
    struct flock       lock;
} rp_uio_t;

