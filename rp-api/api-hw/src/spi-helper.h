/**
 * $Id: $
 *
 * @brief Red Pitaya Helper Module.
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef SPI_HELPER_H
#define SPI_HELPER_H

#include "rp_hw.h"

typedef struct spi_config {
    uint8_t  raw_value;
    rp_spi_mode_t       spi_mode;
    rp_spi_order_bit_t  lsb_first;
    int                 bits_per_word; // [7...]
    int                 spi_speed;
    rp_spi_state_t      spi_ready;
    rp_spi_cs_mode_t    cs_mode;

} spi_config_t;

typedef struct spi_message {
    uint8_t *rx_buffer;
    uint8_t *tx_buffer;
    size_t   size;
    bool     cs_change;
} spi_message_t;

typedef struct spi_data {
    spi_message_t *messages;
    size_t         size;
} spi_data_t;



int read_spi_configuration(int fd, spi_config_t *config);

int write_spi_configuration(int fd, spi_config_t *config);

int read_write_spi_buffers(int fd, spi_data_t *data);


#endif
