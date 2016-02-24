
#ifndef _RP_DMA_H_
#define _RP_DMA_H_

#include "rp_dma.h"

#include <stdint.h>
#include <stdbool.h>

/*
typedef struct {
  char          *name;
  char          *dev;
  size_t         length;
  int            fd;
 // volatile void *regset;
 //          void *context;
 // int            struct_size; ///< only used to reserve dummy memory space for testing
} rp_handle_uio_t;
*/

typedef enum
{
    RP_DMA_SINGLE,
    RP_DMA_CYCLIC,
    RP_DMA_STOP_RX
} RP_DMA_CTRL;


int rp_DmaOpen(const char *dev, rp_handle_uio_t *handle);
int rp_DmaCtrl(rp_handle_uio_t *handle, RP_DMA_CTRL ctrl);
int rp_DmaRead(rp_handle_uio_t *handle);
int rp_DmaMemDump(rp_handle_uio_t *handle);
int rp_DmaClose(rp_handle_uio_t *handle);

#endif // _RP_DMA_H_
