
#ifndef _RP_API_H_
#define _RP_API_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum rpDigitalChannel {
    RP_DIGITAL_CHANNEL_0,
    RP_DIGITAL_CHANNEL_1,
    RP_DIGITAL_CHANNEL_2,
    RP_DIGITAL_CHANNEL_3,
    RP_DIGITAL_CHANNEL_4,
    RP_DIGITAL_CHANNEL_5,
    RP_DIGITAL_CHANNEL_6,
    RP_DIGITAL_CHANNEL_7,
    RP_DIGITAL_CHANNEL_8,
    RP_DIGITAL_CHANNEL_9,
    RP_DIGITAL_CHANNEL_10,
    RP_DIGITAL_CHANNEL_11,
    RP_DIGITAL_CHANNEL_12,
    RP_DIGITAL_CHANNEL_13,
    RP_DIGITAL_CHANNEL_14,
    RP_DIGITAL_CHANNEL_15,
    RP_MAX_DIGITAL_CHANNELS
} RP_DIGITAL_CHANNEL;

typedef enum rpDigitalDirection {
    RP_DIGITAL_DONT_CARE,
    RP_DIGITAL_DIRECTION_LOW,
    RP_DIGITAL_DIRECTION_HIGH,
    RP_DIGITAL_DIRECTION_RISING,
    RP_DIGITAL_DIRECTION_FALLING,
    RP_DIGITAL_DIRECTION_RISING_OR_FALLING,
    RP_DIGITAL_MAX_DIRECTION
} RP_DIGITAL_DIRECTION;


typedef struct tPS3000ADigitalChannelDirections {
    RP_DIGITAL_CHANNEL channel;
    RP_DIGITAL_DIRECTION direction;
} RP_DIGITAL_CHANNEL_DIRECTIONS;


int rp_OpenUnit();

int rp_CloseUnit();

/**
 * Enable digital port
 *
 * This function will set the individual digital channels' trigger directions. Each trigger
 * direction consists of a channel name and a direction. If the channel is not included in
 * the array of RP_DIGITAL_CHANNEL_DIRECTIONS the driver assumes the
 * digital channel's trigger direction is RP_DIGITAL_DONT_CARE.
 *
 * @param directions      A pointer to an array of structures describing the
 *                         requested properties.
 *                         If directions is NULL, digital triggering is switched off.
 *                         A digital channel that is not included in the array will be set to RP_DIGITAL_DONT_CARE.
 * @param nDirections      The number of digital channel directions being
 *                      passed to the driver.
 *
 */
int rp_SetTriggerDigitalPortProperties(RP_DIGITAL_CHANNEL_DIRECTIONS * directions,
                                            uint16_t nDirections);


int rp_EnableDigitalPortDataRLE(bool enable);

int rp_SoftwareTrigger();
int rp_SetPolarity(uint32_t reg);

int rp_GetTimebase(uint32_t timebase, double * timeIntervalNanoseconds);

/**
 * Set data buffer
 *
 * This function tells the driver where to store the data.
 *
 * @param buffer     The location of the buffer
 * @param size       The size of the buffer array (notice that one sample is 16 bits)
 *
 */
int rp_SetDataBuffer(int16_t * buffer, size_t size);

/**
 *
 * This function tells the driver where to store the data.
 *
 * @param noOfPreTriggerSamples,    The number of samples to return before the trigger event.
 *                                     If no trigger has been set then this argument is ignored and
 *                                     noOfPostTriggerSamples specifies the maximum number of samples to collect.
 * @param noOfPostTriggerSamples      the number of samples to be taken after a trigger event.
 *                                     If no trigger event has been set then this specifies the maximum number of
 *                                     samples to be taken.
 *                                     If a trigger condition has been set, this specifies the number of
 *                                     samples to be taken after a trigger has fired, and the number of samples
 *                                     to be collected is then: noOfPreTriggerSamples + noOfPostTriggerSamples
 * @param timebase                     Timebase
 * @param timeIndisposedS            On exit, the time, in milliseconds, that the scope will spend collecting samples.
 *
 */
int rp_RunBlock(uint32_t noOfPreTriggerSamples,
                uint32_t noOfPostTriggerSamples,
                uint32_t timebase,
                double * timeIndisposedMs);

int rp_GetTrigPosition(uint32_t * tigger_pos);

/**
 * This function returns block-mode data, with or without down-sampling, starting at the
 * specified sample number. It is used to get the stored data from the driver after data
 * collection has stopped.
 *
 * This function tells the driver where to store the data.
 *
 * @param noOfSamples          On entry, the number of samples required. On exit, the actual number retrieved.
 *                          The number of samples retrieved will not be more than the number requested,
 *                          and the data retrieved starts at startIndex.
 */
int rp_GetValues(uint32_t * noOfSamples);

/**
 * Stops the scope device from sampling data. If this function is called
 * before a trigger event occurs, the oscilloscope may not contain valid data.
 * Always call this function after the end of a capture to ensure that the scope is ready for the next capture.
 */
int rp_Stop(void);


#endif // _RP_API_H_
