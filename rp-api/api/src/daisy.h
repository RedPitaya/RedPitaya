/**
 * $Id: $
 *
 * @brief Red Pitaya library daisy module interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef __DAISY_H
#define __DAISY_H

#include <stdbool.h>
#include <stdint.h>
#include "common.h"
#include "rp.h"
#include "rp_hw-profiles.h"

// Base daisy address
static const int DAISY_BASE_ADDR = 0x00500000;
static const int DAISY_BASE_SIZE = 0x20;

typedef enum {
    ZERO = 0,      // data is 0
    USER = 1,      // user data (from logic)
    CUSTOM = 2,    // custom data (from daisy_transmit_t)
    TRAINING = 3,  // training data (0x00FF)
    LOOPBACK = 4,  // transmit received data
    RANDOM = 5

} daisy_data_source_t;

typedef struct daisy_control_s {
    uint32_t tx_enable : 1;
    uint32_t rx_enable : 1;
    uint32_t : 30;  // Reserve
    void print() volatile {
        printRegBit(" - %-39s = 0x%08X (%d)\n", "tx_enable", tx_enable);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "rx_enable", rx_enable);
    };
} daisy_control_t;

typedef struct daisy_transmit_s {
    /*
        Data source
        0 - data is 0
        1 - user data (from logic)
        2 - custom data (from this register)
        3 - training data (0x00FF)
        4 - transmit received data (loop back)
        5 - random data (for testing)
    */
    uint32_t data_source : 4;
    uint32_t : 12;
    uint32_t custom : 16;
    void print() volatile {
        printRegBit(" - %-39s = 0x%08X (%d)\n", "data_source", data_source);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "custom", custom);
    };
} daisy_transmit_t;

typedef struct daisy_receiver_training_s {
    uint32_t enable : 1;
    uint32_t state : 1;
    uint32_t : 30;
    void print() volatile {
        printRegBit(" - %-39s = 0x%08X (%d)\n", "enable", enable);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "state", state);
    };
} daisy_receiver_training_t;

typedef struct daisy_receiver_data_s {
    uint32_t raw : 16;
    uint32_t non_zero_data : 16;
    void print() volatile {
        printRegBit(" - %-39s = 0x%08X (%d)\n", "raw", raw);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "non_zero_data", non_zero_data);
    };
} daisy_receiver_data_t;

typedef struct daisy_testing_control_s {
    uint32_t reset : 1;  // Reset testing conters (error & data)
    uint32_t : 31;
    void print() volatile { printRegBit(" - %-39s = 0x%08X (%d)\n", "reset", reset); };
} daisy_testing_control_t;

typedef struct daisy_regset_s {
    daisy_control_t cotrol;                // 0x0 **Control**
    daisy_transmit_t transmit;             // 0x4 **Transmitter data selector**
    daisy_receiver_training_t r_training;  // 0x8 **Receiver training**
    daisy_receiver_data_t r_data;          // 0xC **Receiver data**
    daisy_testing_control_t t_control;     // 0x10 **Testing control**
    uint32_t t_error_counter;              // 0x14 **Testing error counter**
    uint32_t t_data_counter;               // 0x18 **Testing data counter**
} daisy_regset_t;

typedef struct daisy_regset_uint_s {
    uint32_t cotrol;           // 0x0 **Control**
    uint32_t transmit;         // 0x4 **Transmitter data selector**
    uint32_t r_training;       // 0x8 **Receiver training**
    uint32_t r_data;           // 0xC **Receiver data**
    uint32_t t_control;        // 0x10 **Testing control**
    uint32_t t_error_counter;  // 0x14 **Testing error counter**
    uint32_t t_data_counter;   // 0x18 **Testing data counter**
} daisy_regset_uint_t;

typedef union {
    daisy_regset_t reg;
    daisy_regset_uint_t reg_uint;
} daisy_regset_u_t;

int daisy_Init();
int daisy_Release();

int daisy_printRegset();

int daisy_SetTXEnable(bool enable);
int daisy_GetTXEnable(bool* state);

int daisy_SetRXEnable(bool enable);
int daisy_GetRXEnable(bool* state);

int daisy_SetDataMode(daisy_data_source_t mode);
int daisy_GetDataMode(daisy_data_source_t* mode);

int daisy_SetDataCustom(uint16_t data);
int daisy_GetDataCustom(uint16_t* data);

int daisy_SetReceiverTrainingEnable(bool enable);
int daisy_GetReceiverTrainingEnable(bool* state);

int daisy_GetReceiverTrainingState(bool* state);

int daisy_GetReceiverDataRaw(uint16_t* data);
int daisy_GetReceiverNonZeroData(uint16_t* data);

int daisy_SetResetTesting(bool reset);
int daisy_GetResetTesting(bool* state);

int daisy_GetTestingErrorCounter(uint32_t* counter);
int daisy_GetTestingDataCounter(uint32_t* counter);

#endif  //__DAISY_H
