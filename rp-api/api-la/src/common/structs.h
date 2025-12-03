/**
 * $Id: $
 *
 * @brief Red Pitaya DMA library
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#ifndef __RP_LA_STRUCTS_H
#define __RP_LA_STRUCTS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string>

typedef struct {
    std::string name;
    std::string dev;
    size_t length;
    int fd;
    volatile void* regset = NULL;
    void* context;
    int struct_size;  ///< only used to reserve dummy memory space for testing

    std::string dma_dev;
    size_t dma_size;
    int dma_fd;
    //volatile void *dma_mem;
} rp_handle_uio_t;

typedef enum { RP_DMA_SINGLE, RP_DMA_CYCLIC, RP_DMA_STOP_RX } RP_DMA_CTRL;

typedef struct {
    bool inProcess;
    bool isCapture;
    bool isTimeout;
    int16_t* buf = NULL;
    size_t buf_size;
    uint32_t pre_samples;
    uint32_t post_samples;
    uint32_t trig_blockIndexRLE;  // RLE: index of pair (count,value).
    uint32_t trig_indexNonRLE;
    uint32_t last_blockIndexRLE;
    uint32_t last_indexNonRLE;

    auto reset() {
        inProcess = false;
        isCapture = false;
        isTimeout = false;
        buf = NULL;
        buf_size = 0;
        pre_samples = 0;
        post_samples = 0;
        trig_blockIndexRLE = 0;
        trig_indexNonRLE = 0;
        last_blockIndexRLE = 0;
        last_indexNonRLE = 0;
    }
} rp_acq_data_t;

#define STOP_TX 0
#define STOP_RX 1
#define CYCLIC_TX 2
#define CYCLIC_RX 3
#define SINGLE_RX 10
#define SINGLE_TX 11
#define SIMPLE_RX 18  //blocking until complete
#define SIMPLE_TX 17  //blocking until complete
#define SIMPLE 12     //blocking until complete
#define STATUS 20
#define TIMEOUT 21

#define STATUS_STOPPED 0
#define STATUS_READY 1
#define STATUS_BUSSY 2
#define STATUS_ERROR 3

#define SET_TX_SGMNT_CNT 14
#define SET_TX_SGMNT_SIZE 13
#define SET_RX_SGMNT_CNT 16
#define SET_RX_SGMNT_SIZE 15

/*
 * SGMNT_CNT*SGMNT_SIZE should never be larger than second reg argument
 * in DT
 * dma_region: buffer@1000000 {
 *  	reg = <0x1000000 0x2000000>;
 * };
 * and SGMNT_SIZE should not be larger then 0x400000
 * */

#define SGMNT_CNT 8
#define SGMNT_SIZE 4 * 1024 * 1024
#define RX_SGMNT_CNT SGMNT_CNT
#define RX_SGMNT_SIZE SGMNT_SIZE
#define TX_SGMNT_CNT 0
#define TX_SGMNT_SIZE SGMNT_SIZE

#define RP_SGMNT_CNT 8  // 240/RP_SGMNT_CNT must be int
#define RP_SGMNT_SIZE (256 * 1024)

#define RP_TRG_LOA_PAT_MASK (1 << 1)  ///< logic analyzer pattern trigger
#define RP_TRG_LOA_SWE_MASK (1 << 0)  ///< logic analyzer software trigger

#define RP_TRG_ALL_MASK 0

// /** control register masks  */
#define RP_CTL_STO_MASK (1 << 3)  ///< 1 - ACQ: stops / aborts the acq. ; returns 1 when acq. is stopped
#define RP_CTL_STA_MASK (1 << 2)  ///< 1 - ACQ: starts acq.
#define RP_CTL_SWT_MASK (1 << 1)  ///< 1 - sw trigger bit (sw trigger must be enabled)
#define RP_CTL_RST_MASK (1 << 0)  ///< 1 - reset state machine so that it is in known state

#endif