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

#include "rp.h"
#include "rp_hw-profiles.h"


#define PHASE_MIN              -360         // deg
#define PHASE_MAX               360         // deg
#define DUTY_CYCLE_MIN          0           // %
#define DUTY_CYCLE_MAX          100         // %
#define BURST_COUNT_MIN         1
#define BURST_COUNT_MAX         50000
#define BURST_REPETITIONS_MIN   0x1
#define BURST_REPETITIONS_MAX   0x10000     // Used as value-1  0x10000 => 0xFFFF (inf mode)
#define BURST_PERIOD_MIN        1           // us
#define BURST_PERIOD_MAX        500000000   // us

#define CHA_DATA_OFFSET         0x10000
#define CHB_DATA_OFFSET         0x20000
// #define DATA_BIT_LENGTH         14
#define MICRO                   1e6

// Base Generate address
#define GENERATE_BASE_ADDR      0x00200000
#define GENERATE_BASE_SIZE      0x00030000

#define DEBAUNCER_MASK          0xFFFFF     // (20 bit)


typedef struct ch_properties {
    unsigned int amplitudeScale     :14;
    unsigned int                    :2;
    unsigned int amplitudeOffset    :14;
    unsigned int                    :2;
    uint32_t counterWrap; // 0x8
    uint32_t startOffset; // 0xC
    uint32_t counterStep; // 0x10
    unsigned int                    :2;
    uint32_t buffReadPointer        :14;
    unsigned int                    :16;
    uint32_t cyclesInOneBurst;
    uint32_t burstRepetitions;
    uint32_t delayBetweenBurstRepetitions;
} ch_properties_t;


typedef struct asg_axi_state {
    uint32_t trig_recive_read_req_ChA   :1;
    uint32_t first_read_out_ChA         :1;
    uint32_t fifo_read_enable_ChA       :1;
    uint32_t fifo_being_reset_ChA       :1;
    uint32_t                            :12;
    uint32_t trig_recive_read_req_ChB   :1;
    uint32_t first_read_out_ChB         :1;
    uint32_t fifo_read_enable_ChB       :1;
    uint32_t fifo_being_reset_ChB       :1;
    uint32_t                            :12;
} asg_axi_state_t;

typedef struct generate_control_s {
    unsigned int AtriggerSelector   :4;
    unsigned int ASM_WrapPointer    :1;
    unsigned int                    :1;
    unsigned int ASM_reset          :1;
    unsigned int AsetOutputTo0      :1;
    unsigned int AgatedBursts       :1;
    // Work only 250-12 else return 0
    unsigned int AtempProtected     :1;
    unsigned int AlatchedTempAlarm  :1;
    unsigned int AruntimeTempAlarm  :1;
    //
    unsigned int                    :4;

    unsigned int BtriggerSelector   :4;
    unsigned int BSM_WrapPointer    :1;
    unsigned int                    :1;
    unsigned int BSM_reset          :1;
    unsigned int BsetOutputTo0      :1;
    unsigned int BgatedBursts       :1;
    // Work only 250-12 else return 0
    unsigned int BtempProtected     :1;
    unsigned int BlatchedTempAlarm  :1;
    unsigned int BruntimeTempAlarm  :1;
    //
    unsigned int                    :4;

    ch_properties_t properties_chA;
    ch_properties_t properties_chB;
    // NOT WORK with 250-12
    uint32_t     BurstFinalValue_chA; // 0x44
    uint32_t     BurstFinalValue_chB; // 0x48
    uint32_t     cunterStepChALower;  // 0x4C
    uint32_t     cunterStepChBLower;  // 0x50
    /**@brief Trigger debuncer time
    * bits [19:0] Number of ADC clock periods
    * trigger is disabled after activation
    * reset value is decimal 62500
    * or equivalent to 0.5ms
    */
    uint32_t trig_dbc_t:20,:12; // 0x54
    uint32_t reserv1; // 0x58
    uint32_t reserv2; // 0x5C
    uint32_t reserv3; // 0x60
    uint32_t reserv4; // 0x64
    uint32_t initGenValue_chA; // 0x68
    uint32_t initGenValue_chB; // 0x6C
    uint32_t lengthLastValueState_chA; // 0x70
    uint32_t lengthLastValueState_chB; // 0x74

    uint32_t randomSeed_chA; // 0x78
    uint32_t randomSeed_chB; // 0x7C

    uint32_t enableNoise_chA :1,:31; // 0x80
    uint32_t enableNoise_chB :1,:31; // 0x84

    uint32_t reserved[30];

    asg_axi_state_t axi_state;      // 0x100 (R)

    uint32_t enableAXI_ChA :1,:31;  // 0x104 (R/W)

    // Buffer start address
    // Reads are performed in chunks of 16*64 bit.
    // The buffer size must therefore be N*0x80.

    uint32_t axi_start_address_ChA; // 0x108 (R/W)

    // Buffer end address
    // Where the read pointer must pass no further.
    // The last read is performed at
    // [VALUE of this reg]-8 before wrapping around

    uint32_t axi_end_address_ChA;   // 0x10C (R/W)

    uint32_t reserved_1;            // 0x110

    uint32_t enableAXI_ChB :1,:31;  // 0x114 (R/W)

    // Buffer start address
    // Reads are performed in chunks of 16*64 bit.
    // The buffer size must therefore be N*0x80.

    uint32_t axi_start_address_ChB; // 0x118 (R/W)

    // Buffer end address
    // Where the read pointer must pass no further.
    // The last read is performed at
    // [VALUE of this reg]-8 before wrapping around

    uint32_t axi_end_address_ChB;   // 0x11C

    uint32_t axi_error_read_count_ChA;  // 0x120 (R)
    uint32_t axi_transfer_count_ChA;    // 0x124 (R)

    uint32_t axi_error_read_count_ChB;  // 0x128 (R)
    uint32_t axi_transfer_count_ChB;    // 0x12C (R)

    uint32_t axi_decimation_ChA;        // 0x130 (R/W)
    uint32_t axi_decimation_ChB;        // 0x134 (R/W)

} generate_control_t;

int generate_Init();
int generate_Release();

int generate_setOutputDisable(rp_channel_t channel, bool disable);
int generate_getOutputEnabled(rp_channel_t channel, bool *disabled);
int generate_setOutputEnableSync(bool enable);

int generate_setFrequency(rp_channel_t channel, float frequency,float baseFreq);
int generate_getFrequency(rp_channel_t channel, float *frequency,float baseFreq);
int generate_setWrapCounter(rp_channel_t channel, uint32_t size);
int generate_setTriggerSource(rp_channel_t channel, unsigned short value);
int generate_getTriggerSource(rp_channel_t channel, uint32_t *value);
int generate_setGatedBurst(rp_channel_t channel, uint32_t value);
int generate_getGatedBurst(rp_channel_t channel, uint32_t *value);
int generate_setBurstCount(rp_channel_t channel, uint32_t num);
int generate_getBurstCount(rp_channel_t channel, uint32_t *num);
int generate_setBurstRepetitions(rp_channel_t channel, uint32_t repetitions);
int generate_getBurstRepetitions(rp_channel_t channel, uint32_t *repetitions);
int generate_setBurstDelay(rp_channel_t channel, uint32_t delay);
int generate_getBurstDelay(rp_channel_t channel, uint32_t *delay);
int generate_Trigger(rp_channel_t channel);
int generate_simultaneousTrigger();
int generate_ResetSM();
int generate_ResetChannelSM(rp_channel_t channel);

int generate_writeData(rp_channel_t channel, float *data, int32_t start, uint32_t length);

int generate_setAmplitude(rp_channel_t channel, rp_gen_gain_t gain,  float amplitude);
// int generate_getAmplitude(rp_channel_t channel, rp_gen_gain_t gain, float *amplitude);
int generate_setDCOffset(rp_channel_t channel, rp_gen_gain_t gain, float offset);
// int generate_getDCOffset(rp_channel_t channel, rp_gen_gain_t gain, float *offset);
int generate_getEnableTempProtection(rp_channel_t channel, bool *enable);
int generate_setEnableTempProtection(rp_channel_t channel, bool enable);
int generate_getLatchTempAlarm(rp_channel_t channel, bool *state);
int generate_setLatchTempAlarm(rp_channel_t channel, bool  state);
int generate_getRuntimeTempAlarm(rp_channel_t channel, bool *state);

int generate_setBurstLastValue(rp_channel_t channel,rp_gen_gain_t gain, float amplitude);
int generate_setInitGenValue(rp_channel_t channel,rp_gen_gain_t gain, float amplitude);

int generate_SetTriggerDebouncer(uint32_t value);
int generate_GetTriggerDebouncer(uint32_t *value);

int generate_setRandomSeed(rp_channel_t channel, uint32_t seed);
int generate_getRandomSeed(rp_channel_t channel, uint32_t *seed);

int generate_setEnableRandom(rp_channel_t channel, bool enable);
int generate_getEnableRandom(rp_channel_t channel, bool *enable);

int generate_axi_SetEnable(rp_channel_t channel, bool enable);
int generate_axi_GetEnable(rp_channel_t channel, bool *enable);
int generate_axi_SetStartAddress(rp_channel_t channel, uint32_t address);
int generate_axi_SetEndAddress(rp_channel_t channel, uint32_t address);
int generate_axi_SetDecimation(rp_channel_t channel, uint32_t decimation);
int generate_axi_GetDecimation(rp_channel_t channel, uint32_t *decimation);

#endif