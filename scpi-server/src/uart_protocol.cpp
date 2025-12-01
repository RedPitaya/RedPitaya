/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server SCPI commands implementation
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <chrono>

#include "common.h"
#include "uart_protocol.h"

auto UARTProtocol::crc4(uint8_t* data, uint8_t size) -> uint8_t {
    const uint8_t CRC4_POLY = 0x13;
    uint8_t crc = 0;
    for (uint8_t z = 0; z < size; z++) {
        uint8_t byte = data[z];
        for (int i = 7; i >= 0; --i) {
            bool bit = (byte >> i) & 1;
            bool crc_msb = crc & 0x08;
            crc <<= 1;
            if (crc_msb ^ bit) {
                crc ^= CRC4_POLY;
            }
            crc &= 0x0F;
        }
    }
    return crc;
}

auto UARTProtocol::uart_read(int fd, unsigned char* _buffer, uint8_t size, uint32_t timeout_ms) -> int {
    struct pollfd fds;
    fds.fd = fd;
    fds.events = POLLIN;
    fds.revents = 0;

    int result = poll(&fds, 1, timeout_ms);

    if (result == -1) {
        ERROR_LOG("Error in poll: %s", strerror(errno))
    } else if (result == 0) {
        return 0;
    } else {
        int bytes_read = read(fd, _buffer, size);
        if (bytes_read == -1) {
            ERROR_LOG("Error reading from serial port: %s", strerror(errno))
        }
        return bytes_read;
    }
    return -1;
}

auto UARTProtocol::uart_write(int fd, unsigned char* _buffer, uint8_t size) -> int {
    int count = 0;
    if (fd != -1) {
        count = write(fd, _buffer, size);
    } else {
        ERROR_LOG("Failed write to UART.");
        return -1;
    }

    if (count < 0) {
        ERROR_LOG("Failed write to UART.");
        return -1;
    }

    return count;
}

auto UARTProtocol::writeTo(int fd, uint8_t* buffer, size_t size) -> size_t {
    size_t pos = 0;
    uint8_t sendSize = size < UART_PROTO_BLOCK_SIZE ? size : UART_PROTO_BLOCK_SIZE;
    while (pos < size) {
        uint8_t header = getHeaderForBlock(buffer + pos, sendSize);
        uint8_t ret = uart_write(fd, &header, 1);  // Send header at first
        if (ret != 1) {
            return pos;
        }
        ret = uart_write(fd, buffer + pos, sendSize);  // Send data block
        if (ret != sendSize) {
            return pos + ret;
        }
        uint8_t headerResponse = 0;
        ret = uart_read(fd, &headerResponse, 1, 2000);
        if (headerResponse != header) {
            ERROR_LOG("The response does not match the packet header 0x%X != 0x%X", header, headerResponse);
            return pos;
        }
        pos += sendSize;
        sendSize = pos + sendSize < size ? sendSize : size - pos;
    }
    return size;
}

auto UARTProtocol::getHeaderForBlock(uint8_t* buffer, uint8_t size) -> uint8_t {
    if (size == 0) {
        ERROR_LOG("Buffer size cannot be 0");
        return 0;
    }
    if (size > UART_PROTO_BLOCK_SIZE) {
        ERROR_LOG("The size %d is larger than the allowed %d", size, UART_PROTO_BLOCK_SIZE);
        return 0;
    }
    uint8_t value = crc4(buffer, size);
    value = value << 4;
    value = (value & 0xF0) | ((size - 1) & 0x0F);
    return value;
}
