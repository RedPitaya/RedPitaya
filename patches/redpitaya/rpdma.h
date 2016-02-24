/**
 * $Id: $
 *
 * @brief Red Pitaya DMA
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef __RPDMA_H
#define __RPDMA_H


/*
ioctl macro definitions
*/
#define STOP_TX 0
#define STOP_RX 1
#define CYCLIC_TX 2
#define CYCLIC_RX 3
#define SINGLE_RX 10
#define SINGLE_TX 11
#define SIMPLE_RX 18  //blocking until complete
#define SIMPLE_TX 17  //blocking until complete
#define SIMPLE 12 //blocking until complete

#define SET_TX_SGMNT_CNT 14
#define SET_TX_SGMNT_SIZE 13
#define SET_RX_SGMNT_CNT 16
#define SET_RX_SGMNT_SIZE 15

#define SGMNT_CNT 4
#define SGMNT_SIZE 4*1024
#define RX_SGMNT_CNT SGMNT_CNT
#define RX_SGMNT_SIZE SGMNT_SIZE
#define TX_SGMNT_CNT SGMNT_CNT
#define TX_SGMNT_SIZE SGMNT_SIZE

#endif // __RPDMA_H
