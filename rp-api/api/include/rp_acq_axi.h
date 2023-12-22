/**
 * $Id: $
 *
 * @file rp_acq_axi.h
 * @brief Red Pitaya library API interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef __RP_ACQ_AXI_H
#define __RP_ACQ_AXI_H

#include <stdint.h>
#include <stdbool.h>
#include "rp_enums.h"

/** @name Acquire
 */
///@{

/**
 * Indicates whether the ADC AXI buffer was full of data.
 * @param channel Channel index
 * @param state Returns status
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqAxiGetBufferFillState(rp_channel_t channel, bool* state);

/**
 * Sets the decimation used at acquiring signal for AXI.
 * You can specify values in the range (1,2,4,8,16-65536)
 * @param decimation Decimation values
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqAxiSetDecimationFactor(uint32_t decimation);

/**
 * Gets the decimation used at acquiring signal.
 * @param decimation Decimation values
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqAxiGetDecimationFactor(uint32_t *decimation);
/**
 * Sets the number of decimated data after trigger written into memory.
 * @param channel Channel index
 * @param decimated_data_num Number of decimated data. It must not be higher than the ADC buffer size.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqAxiSetTriggerDelay(rp_channel_t channel, int32_t decimated_data_num);

/**
 * Gets the number of decimated data after trigger written into memory.
 * @param channel Channel index
 * @param decimated_data_num Number of decimated data. It must not be higher than the ADC buffer size.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqAxiGetTriggerDelay(rp_channel_t channel, int32_t *decimated_data_num);

/**
 * Returns current position of AXI ADC write pointer.
 * @param channel Channel index
 * @param pos Write pointer position
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqAxiGetWritePointer(rp_channel_t channel, uint32_t* pos);

/**
 * Returns position of AXI ADC write pointer at time when trigger arrived.
 * @param channel Channel index
 * @param pos Write pointer position
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqAxiGetWritePointerAtTrig(rp_channel_t channel, uint32_t* pos);

/**
 * Get reserved memory for DMA mode
 * @param channel Channel index
 * @param enable Enable state
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqAxiGetMemoryRegion(uint32_t *_start,uint32_t *_size);

/**
 * Sets the AXI enable state.
 * @param channel Channel index
 * @param enable Enable state
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqAxiEnable(rp_channel_t channel, bool enable);

/**
 * Returns the AXI ADC buffer in raw units from specified position and desired size.
 * Output buffer must be at least 'size' long.
 * @param channel Channel A or B for which we want to retrieve the ADC buffer.
 * @param pos Starting position of the ADC buffer to retrieve.
 * @param size Length of the ADC buffer to retrieve. Returns length of filled buffer. In case of too small buffer, required size is returned.
 * @param buffer The output buffer gets filled with the selected part of the ADC buffer.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqAxiGetDataRaw(rp_channel_t channel,  uint32_t pos, uint32_t* size, int16_t* buffer);

/**
 * Returns the AXI ADC buffer in Volt units from specified position and desired size.
 * Output buffer must be at least 'size' long.
 * @param channel Channel A or B for which we want to retrieve the ADC buffer.
 * @param pos Starting position of the ADC buffer to retrieve
 * @param size Length of the ADC buffer to retrieve. Returns length of filled buffer. In case of too small buffer, required size is returned.
 * @param buffer The output buffer gets filled with the selected part of the ADC buffer.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqAxiGetDataV(rp_channel_t channel, uint32_t pos, uint32_t* size, float* buffer);

/**
 * Sets the AXI ADC buffer address and size in samples.
 *
 * @param channel Channel A or B for which we want to set the ADC buffer size.
 * @param address Address of the ADC buffer.
 * @param size Size of the ADC buffer in samples.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqAxiSetBufferSamples(rp_channel_t channel, uint32_t address, uint32_t samples);

/**
 * Sets the AXI ADC buffer address and size in bytes.
 * Buffer size must be a multiple of 2.
 *
 * @param channel Channel A or B for which we want to set the ADC buffer bytes.
 * @param address Address of the ADC buffer.
 * @param size Size of the ADC buffer in samples.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqAxiSetBufferBytes(rp_channel_t channel, uint32_t address, uint32_t size);

///@}

#endif //__RP_ACQ_AXI_H