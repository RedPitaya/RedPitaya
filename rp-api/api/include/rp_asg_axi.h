/**
 * $Id: $
 *
 * @file rp_asg_axi.h
 * @brief Red Pitaya library API interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#ifndef __RP_ASG_AXI_H
#define __RP_ASG_AXI_H

#include <stdbool.h>
#include <stdint.h>
#include "rp_enums.h"

/** @name Generate
 */
///@{

/**
 * Get reserved memory for DMA mode
 * @param channel Channel index
 * @param enable Enable state
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_GenAxiGetMemoryRegion(uint32_t* _start, uint32_t* _size);

/**
 * Enables AXI mode for the selected channel.
 * @param channel Channel index
 * @param state Set enable
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_GenAxiSetEnable(rp_channel_t channel, bool state);

/**
 * Returns the state of the AXI mode.
 * @param channel Channel index
 * @param state Returns status
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_GenAxiGetEnable(rp_channel_t channel, bool* state);

/**
 * Reserves address space for the generator. Must be done before enabling the mode. Memory size must be a multiple of 0x80
 * @param channel Channel index
 * @param start Starting address
 * @param end End address
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_GenAxiReserveMemory(rp_channel_t channel, uint32_t start, uint32_t end);

/**
 * Frees reserved memory.
 * @param channel Channel index
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_GenAxiReleaseMemory(rp_channel_t channel);

/**
 * Sets the decimation for the generator in AXI mode.
 * You can specify values in the range (1,2,4,8,16-65536)
 * @param channel Channel index
 * @param decimation Decimation values
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_GenAxiSetDecimationFactor(rp_channel_t channel, uint32_t decimation);

/**
 * Returns the set decimation in AXI mode.
 * @param channel Channel index
 * @param decimation Decimation values
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_GenAxiGetDecimationFactor(rp_channel_t channel, uint32_t* decimation);

/**
 * Writes data to a reserved memory area with NUMPY array.
 * @param channel Channel index
 * @param buffer Buffer with signal.
 * @param size The buffer size in samples must match the size of the reserved memory.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_GenAxiWriteWaveform(rp_channel_t channel, float* np_buffer, int size);

/**
 * Writes data to a reserved memory area with NUMPY array.
 * @param channel Channel index
 * @param offset Offset in samples relative to the beginning of the buffer
 * @param buffer Buffer with signal.
 * @param size The buffer size in samples must match the size of the reserved memory.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_GenAxiWriteWaveformOffset(rp_channel_t channel, uint32_t offset, float* np_buffer, int size);

///@}

#endif  //__RP_ASG_AXI_H