/**
 * $Id: $
 *
 * @brief Red Pitaya library Generate module interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */
#ifndef __GENERATE_H
#define __GENERATE_H

#include "common.h"
#include "rp.h"
#include "rp_hw-profiles.h"

#define PHASE_MIN -360      // deg
#define PHASE_MAX 360       // deg
#define DUTY_CYCLE_MIN 0    // %
#define DUTY_CYCLE_MAX 100  // %
#define BURST_COUNT_MIN 1
#define BURST_COUNT_MAX 50000
#define BURST_REPETITIONS_MIN 0x1
#define BURST_REPETITIONS_MAX 0x10000  // Used as value-1  0x10000 => 0xFFFF (inf mode)
#define BURST_PERIOD_MIN 1             // us
#define BURST_PERIOD_MAX 500000000     // us

#define CHA_DATA_OFFSET 0x10000
#define CHB_DATA_OFFSET 0x20000
// #define DATA_BIT_LENGTH         14
#define MICRO 1e6

// Base Generate address
#define GENERATE_BASE_ADDR 0x00200000
#define GENERATE_BASE_SIZE 0x00030000

#define GEN_DEBAUNCER_MASK 0xFFFFF  // (20 bit)

typedef struct {
    unsigned int amplitudeScale : 14;
    unsigned int : 2;
    unsigned int amplitudeOffset : 14;
    unsigned int : 2;
    void print() volatile {
        printRegBit(" - %-39s = 0x%08X (%d)\n", "amplitudeScale", amplitudeScale);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "amplitudeOffset", amplitudeOffset);
    };
} asg_ch_amp_scale_t;

typedef union {
    uint32_t reg_full = 0;
    asg_ch_amp_scale_t reg;
} asg_ch_amp_scale_u_t;

typedef struct asg_axi_state {
    uint32_t trig_recive_read_req_ChA : 1;
    uint32_t first_read_out_ChA : 1;
    uint32_t fifo_read_enable_ChA : 1;
    uint32_t fifo_being_reset_ChA : 1;
    uint32_t : 12;
    uint32_t trig_recive_read_req_ChB : 1;
    uint32_t first_read_out_ChB : 1;
    uint32_t fifo_read_enable_ChB : 1;
    uint32_t fifo_being_reset_ChB : 1;
    uint32_t : 12;

    void print() volatile {
        printRegBit(" - %-39s = 0x%08X (%d)\n", "trig_recive_read_req_ChA", trig_recive_read_req_ChA);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "first_read_out_ChA", first_read_out_ChA);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "fifo_read_enable_ChA", fifo_read_enable_ChA);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "fifo_being_reset_ChA", fifo_being_reset_ChA);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "trig_recive_read_req_ChB", trig_recive_read_req_ChB);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "first_read_out_ChB", first_read_out_ChB);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "fifo_read_enable_ChB", fifo_read_enable_ChB);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "fifo_being_reset_ChB", fifo_being_reset_ChB);
    };
} asg_axi_state_t;

typedef union {
    uint32_t reg_full = 0;
    asg_axi_state_t reg;
} asg_axi_state_u_t;

typedef struct {
    uint16_t triggerSelector : 4;
    uint16_t SM_WrapPointer : 1;
    uint16_t : 1;
    uint16_t SM_reset : 1;
    uint16_t setOutputTo0 : 1;
    uint16_t gatedBursts : 1;
    // Work only 250-12 else return 0
    uint16_t tempProtected : 1;
    uint16_t latchedTempAlarm : 1;
    uint16_t runtimeTempAlarm : 1;
    //
    uint16_t : 4;
    void print() volatile {
        printRegBit(" - %-39s = 0x%08X (%d)\n", "triggerSelector", triggerSelector);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "SM_WrapPointer", SM_WrapPointer);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "SM_reset", SM_reset);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "setOutputTo0", setOutputTo0);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "gatedBursts", gatedBursts);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "tempProtected", tempProtected);
        printRegBit(" - %-39s = 0x%08X (%d)\n", "latchedTempAlarm", latchedTempAlarm);
    };
} asg_config_control_t;

typedef union {
    uint32_t reg_full = 0;
    asg_config_control_t reg[2];
} asg_config_control_u_t;

typedef struct {
    uint32_t trig_dbc_t : 20;
    uint32_t : 12;
    void print() volatile { printRegBit(" - %-39s = 0x%08X (%d)\n", "trig_dbc_t", trig_dbc_t); };
} asg_trig_dbc_t;

typedef union {
    uint32_t reg_full = 0;
    asg_trig_dbc_t reg;
} asg_trig_dbc_u_t;

typedef struct generate_control_s {
    uint32_t config;

    uint32_t ampAndScale_ch1;
    uint32_t counterWrap_ch1;
    uint32_t startOffset_ch1;
    uint32_t counterStep_ch1;
    uint32_t counterStepLower_ch1;
    uint32_t cyclesInOneBurst_ch1;
    uint32_t burstRepetitions_ch1;
    uint32_t delayBetweenBurstRepetitions_ch1;  // 1 uS

    uint32_t ampAndScale_ch2;
    uint32_t counterWrap_ch2;
    uint32_t startOffset_ch2;
    uint32_t counterStep_ch2;
    uint32_t counterStepLower_ch2;
    uint32_t cyclesInOneBurst_ch2;
    uint32_t burstRepetitions_ch2;
    uint32_t delayBetweenBurstRepetitions_ch2;  // 1 uS

    uint32_t BurstFinalValue_ch1;       // 0x44
    uint32_t BurstFinalValue_ch2;       // 0x48
    uint32_t reserv1;                   // 0x4C
    uint32_t reserv2;                   // 0x50
                                        /**@brief Trigger debuncer time
    * bits [19:0] Number of ADC clock periods
    * trigger is disabled after activation
    * reset value is decimal 62500
    * or equivalent to 0.5ms
    */
    uint32_t trig_dbc;                  // 0x54
    uint32_t reserv3;                   // 0x58
    uint32_t reserv4;                   // 0x5C
    uint32_t reserv5;                   // 0x60
    uint32_t reserv6;                   // 0x64
    uint32_t initGenValue_ch1;          // 0x68
    uint32_t initGenValue_ch2;          // 0x6C
    uint32_t lengthLastValueState_ch1;  // 0x70
    uint32_t lengthLastValueState_ch2;  // 0x74

    uint32_t randomSeed_ch1;  // 0x78
    uint32_t randomSeed_ch2;  // 0x7C

    uint32_t enableNoise_ch1;  // 0x80
    uint32_t enableNoise_ch2;  // 0x84

    uint32_t reserved[30];

    uint32_t axi_state;  // 0x100 (R)

    uint32_t enableAXI_Ch1;  // 0x104 (R/W)

    // Buffer start address
    // Reads are performed in chunks of 16*64 bit.
    // The buffer size must therefore be N*0x80.

    uint32_t axi_start_address_Ch1;  // 0x108 (R/W)

    // Buffer end address
    // Where the read pointer must pass no further.
    // The last read is performed at
    // [VALUE of this reg]-8 before wrapping around

    uint32_t axi_end_address_Ch1;  // 0x10C (R/W)

    uint32_t reserv7;  // 0x110

    uint32_t enableAXI_Ch2;  // 0x114 (R/W)

    // Buffer start address
    // Reads are performed in chunks of 16*64 bit.
    // The buffer size must therefore be N*0x80.

    uint32_t axi_start_address_Ch2;  // 0x118 (R/W)

    // Buffer end address
    // Where the read pointer must pass no further.
    // The last read is performed at
    // [VALUE of this reg]-8 before wrapping around

    uint32_t axi_end_address_Ch2;  // 0x11C

    uint32_t axi_error_read_count_Ch1;  // 0x120 (R)
    uint32_t axi_transfer_count_Ch1;    // 0x124 (R)

    uint32_t axi_error_read_count_Ch2;  // 0x128 (R)
    uint32_t axi_transfer_count_Ch2;    // 0x12C (R)

    uint32_t axi_decimation_Ch1;  // 0x130 (R/W)
    uint32_t axi_decimation_Ch2;  // 0x134 (R/W)

} generate_control_t;

int generate_Init();
int generate_Release();

int generate_printRegset();

int generate_setOutputDisable(rp_channel_t channel, bool disable);
int generate_getOutputEnabled(rp_channel_t channel, bool* disabled);
int generate_setOutputEnableSync(bool enable);

int generate_setFrequency(rp_channel_t channel, float frequency, float baseFreq);
int generate_getFrequency(rp_channel_t channel, float* frequency, float baseFreq);
int generate_setWrapCounter(rp_channel_t channel, uint32_t size);
int generate_setTriggerSource(rp_channel_t channel, unsigned short value);
int generate_getTriggerSource(rp_channel_t channel, uint32_t* value);
int generate_setGatedBurst(rp_channel_t channel, uint32_t value);
int generate_getGatedBurst(rp_channel_t channel, uint32_t* value);
int generate_setBurstCount(rp_channel_t channel, uint32_t num);
int generate_getBurstCount(rp_channel_t channel, uint32_t* num);
int generate_setBurstRepetitions(rp_channel_t channel, uint32_t repetitions);
int generate_getBurstRepetitions(rp_channel_t channel, uint32_t* repetitions);
int generate_setBurstDelay(rp_channel_t channel, uint32_t delay);
int generate_getBurstDelay(rp_channel_t channel, uint32_t* delay);
int generate_Trigger(rp_channel_t channel);
int generate_simultaneousTrigger();
int generate_ResetSM();
int generate_ResetChannelSM(rp_channel_t channel);

int generate_writeData(rp_channel_t channel, float* data, int32_t start, uint32_t length);

int generate_setAmplitude(rp_channel_t channel, rp_gen_gain_t gain, rp_gen_load_mode_t mode, float amplitude);
int generate_setDCOffset(rp_channel_t channel, rp_gen_gain_t gain, rp_gen_load_mode_t mode, float offset);
int generate_setAmplitudeAndOffsetOrigin(rp_channel_t channel, rp_gen_gain_t gain, rp_gen_load_mode_t mode);
int generate_getEnableTempProtection(rp_channel_t channel, bool* enable);
int generate_setEnableTempProtection(rp_channel_t channel, bool enable);
int generate_getLatchTempAlarm(rp_channel_t channel, bool* state);
int generate_setLatchTempAlarm(rp_channel_t channel, bool state);
int generate_getRuntimeTempAlarm(rp_channel_t channel, bool* state);

int generate_setBurstLastValue(rp_channel_t channel, rp_gen_gain_t gain, rp_gen_load_mode_t mode, float amplitude);
int generate_setInitGenValue(rp_channel_t channel, rp_gen_gain_t gain, rp_gen_load_mode_t mode, float amplitude);

int generate_SetTriggerDebouncer(uint32_t value);
int generate_GetTriggerDebouncer(uint32_t* value);

int generate_setRandomSeed(rp_channel_t channel, uint32_t seed);
int generate_getRandomSeed(rp_channel_t channel, uint32_t* seed);

int generate_setEnableRandom(rp_channel_t channel, bool enable);
int generate_getEnableRandom(rp_channel_t channel, bool* enable);

int generate_axi_SetEnable(rp_channel_t channel, bool enable);
int generate_axi_GetEnable(rp_channel_t channel, bool* enable);
int generate_axi_SetStartAddress(rp_channel_t channel, uint32_t address);
int generate_axi_SetEndAddress(rp_channel_t channel, uint32_t address);
int generate_axi_SetDecimation(rp_channel_t channel, uint32_t decimation);
int generate_axi_GetDecimation(rp_channel_t channel, uint32_t* decimation);

#endif