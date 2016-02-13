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

#define SET_TX_SEGMENT_CNT 14
#define SET_TX_SEGMENT_SIZE 13
#define SET_RX_SEGMENT_CNT 16
#define SET_RX_SEGMENT_SIZE 15

#define SEGMENT_CNT 4
#define SGMNT_SIZE 4*1024
#define RX_SGMNT_CNT SEGMENT_CNT
#define RX_SGMNT_SIZE SGMNT_SIZE
#define TX_SGMNT_CNT SEGMENT_CNT
#define TX_SGMNT_SIZE SGMNT_SIZE
