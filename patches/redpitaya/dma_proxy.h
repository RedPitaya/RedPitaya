/* This header file is shared between the DMA Proxy test application and the DMA Proxy device driver. It defines the 
 * shared interface to allow DMA transfers to be done from user space. 
 *
 * Note: the buffer in the data structure should be 1st in the channel interface so that the buffer is cached aligned,
 * otherwise there may be issues when using cached memory. The issues were typically the 1st 32 bytes of the buffer
 * not working in the driver test.
 */

#define TEST_SIZE (3 * 1024 * 1024)

struct dma_proxy_channel_interface {
	unsigned char buffer[TEST_SIZE];
	enum proxy_status { PROXY_NO_ERROR = 0, PROXY_BUSY = 1, PROXY_TIMEOUT = 2, PROXY_ERROR = 3 } status;
	unsigned int length;
};

