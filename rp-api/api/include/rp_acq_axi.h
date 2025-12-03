/**
 * $Id: $
 *
 * @file rp_acq_axi.h
 * @brief Red Pitaya library API interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#ifndef __RP_ACQ_AXI_H
#define __RP_ACQ_AXI_H

#include <stdbool.h>
#include <stdint.h>
#include <span>
#include <vector>
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
 * Sets the decimation used at acquiring signal for AXI.
 * You can specify values in the range (1,2,4,8,16-65536)
 * This channel separation feature works with FPGA support.
 * You can also enable function forwarding via rp_AcqSetSplitTriggerPass if this mode is not available.
 * @param channel Channel A, B, C or D
 * @param decimation Decimation values
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqAxiSetDecimationFactorCh(rp_channel_t channel, uint32_t decimation);

/**
 * Gets the decimation used at acquiring signal.
 * @param decimation Decimation values
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqAxiGetDecimationFactor(uint32_t* decimation);

/**
 * Gets the decimation used at acquiring signal.
 * This channel separation feature works with FPGA support.
 * You can also enable function forwarding via rp_AcqSetSplitTriggerPass if this mode is not available.
 * @param channel Channel A, B, C or D
 * @param decimation Decimation values
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqAxiGetDecimationFactorCh(rp_channel_t channel, uint32_t* decimation);

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
int rp_AcqAxiGetTriggerDelay(rp_channel_t channel, int32_t* decimated_data_num);

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
int rp_AcqAxiGetMemoryRegion(uint32_t* _start, uint32_t* _size);

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
int rp_AcqAxiGetDataRaw(rp_channel_t channel, uint32_t pos, uint32_t* size, int16_t* buffer);

/**
 * The function returns a list of memory areas containing ADC values without copying the data.
 * @param channel Channel A or B for which we want to retrieve the ADC buffer.
 * @param pos Starting position of the ADC buffer to retrieve.
 * @param size Length of the ADC buffer to retrieve.  The value may be larger than the buffer size. Then the data will be returned cyclically over the buffer.
 * @param data List of memory areas containing data.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqAxiGetDataRawDirect(rp_channel_t channel, uint32_t pos, uint32_t size, std::vector<std::span<int16_t>>* data);

/**
 * Returns the AXI ADC buffer in raw units from specified position and desired size.
 * Output buffer must be at least 'size' long.
 * @param channel Channel A or B for which we want to retrieve the ADC buffer.
 * @param pos Starting position of the ADC buffer to retrieve.
 * @param buffer The output buffer gets filled with the selected part of the ADC buffer.
 * @param size Length of the ADC buffer to retrieve. Returns length of filled buffer. In case of too small buffer, required size is returned.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqAxiGetDataRawNP(rp_channel_t channel, uint32_t pos, int16_t* np_buffer, int size);

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
 * Returns the AXI ADC buffer in Volt units from specified position and desired size.
 * Output buffer must be at least 'size' long.
 * @param channel Channel A or B for which we want to retrieve the ADC buffer.
 * @param pos Starting position of the ADC buffer to retrieve
 * @param buffer The output buffer gets filled with the selected part of the ADC buffer.
 * @param size Length of the ADC buffer to retrieve. Returns length of filled buffer. In case of too small buffer, required size is returned.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqAxiGetDataVNP(rp_channel_t channel, uint32_t pos, float* np_buffer, int size);

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

/**
 * Adds a voltage offset when requesting data from AXI buffers. Only affects float and double data types. Raw data remains unchanged.
 * @param channel Channel A, B, C or D
 * @param value Offset value in volts
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqAxiSetOffset(rp_channel_t channel, float value);

/**
 * Returns the offset value.
 * @param channel Channel A, B, C or D
 * @param value Offset value in volts
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqAxiGetOffset(rp_channel_t channel, float* value);

///@}

#endif  //__RP_ACQ_AXI_H