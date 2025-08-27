/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server UART SCPI commands interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */


#ifndef SCPI_UART_PROTOCOL_H_
#define SCPI_UART_PROTOCOL_H_

#include "scpi/types.h"


#define UART_PROTO_BLOCK_SIZE 16

class UARTProtocol{

public:
    auto writeTo(int fd, uint8_t *buffer, size_t size) -> size_t;
private:
    auto getHeaderForBlock(uint8_t *buffer, uint8_t size) -> uint8_t;
    auto uart_read(int fd, unsigned char *_buffer, uint8_t size, uint32_t timeout_ms) -> int;
    auto uart_write(int fd, unsigned char *_buffer, uint8_t size) -> int;
    static auto crc4(uint8_t *data, uint8_t size) -> uint8_t;
};

#endif /* SCPI_UART_PROTOCOL_H_ */
