/**
 * $Id: $
 *
 * @brief Red Pitaya library oscilloscope module implementation
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <stdio.h>
#include <sys/types.h>

#include "version.h"
#include "common.h"
#include "oscilloscope.h"

// Base Oscilloscope address
static const int OSC_BASE_ADDR = 0x40100000;
static const int OSC_BASE_SIZE = 0x30000;

// Oscilloscope Channel A input signal buffer offset
#define OSC_CHA_OFFSET 0x10000

// Oscilloscope Channel B input signal buffer offset
#define OSC_CHB_OFFSET 0x20000

// Oscilloscope signal A and B length
#define OSC_SIG_LEN (16*1024)

// Oscilloscope structure declaration
typedef struct osc_control_s {

	/** @brief Offset 0x00 - configuration register
	*
	* Configuration register (offset 0x00):
	* bit [0] - arm_trigger
	* bit [1] - rst_wr_state_machine
	* bits [31:2] - reserved
	*/
	uint32_t conf;

	/** @brief Offset 0x04 - trigger source register
	*
	* Trigger source register (offset 0x04):
	* bits [ 2 : 0] - trigger source:
	* 1 - trig immediately
	* 2 - ChA positive edge
	* 3 - ChA negative edge
	* 4 - ChB positive edge
	* 5 - ChB negative edge
	* 6 - External trigger 0
	* 7 - External trigger 1
	* bits [31 : 3] -reserved
	*/
	uint32_t trig_source;

	/** @brief Offset 0x08 - Channel A threshold register
	*
	* Channel A threshold register (offset 0x08):
	* bits [13: 0] - ChA threshold
	* bits [31:14] - reserved
	*/
	uint32_t cha_thr;

	/** @brief Offset 0x0C - Channel B threshold register
	*
	* Channel B threshold register (offset 0x0C):
	* bits [13: 0] - ChB threshold
	* bits [31:14] - reserved
	*/
	uint32_t chb_thr;

	/** @brief Offset 0x10 - After trigger delay register
	*
	* After trigger delay register (offset 0x10)
	* bits [31: 0] - trigger delay
	* 32 bit number - how many decimated samples should be stored into a buffer.
	* (max 16k samples)
	*/
	uint32_t trigger_delay;

	/** @brief Offset 0x14 - Data decimation register
	*
	* Data decimation register (offset 0x14):
	* bits [16: 0] - decimation factor, legal values:
	* 1, 8, 64, 1024, 8192 65536
	* If other values are written data is undefined
	* bits [31:17] - reserved
	*/
	uint32_t data_dec;

	/** @brief Offset 0x18 - Current write pointer register
	*
	* Current write pointer register (offset 0x18), read only:
	* bits [13: 0] - current write pointer
	* bits [31:14] - reserved
	*/
	uint32_t wr_ptr_cur;

	/** @brief Offset 0x1C - Trigger write pointer register
	*
	* Trigger write pointer register (offset 0x1C), read only:
	* bits [13: 0] - trigger pointer (pointer where trigger was detected)
	* bits [31:14] - reserved
	*/
	uint32_t wr_ptr_trigger;

	/** @brief ChA & ChB hysteresis - both of the format:
	* bits [13: 0] - hysteresis threshold
	* bits [31:14] - reserved
	*/
	uint32_t cha_hystersis;
	uint32_t chb_hystersis;

	/** @brief
	* bits [0] - enable signal average at decimation
	* bits [31:1] - reserved
	*/
	uint32_t other;

	uint32_t reseved; // Empty space...

	/** @brief ChA Equalization filter
	* bits [17:0] - AA coefficient (pole)
	* bits [31:18] - reserved
	*/
	uint32_t cha_filt_aa;

	/** @brief ChA Equalization filter
	* bits [24:0] - BB coefficient (zero)
	* bits [31:25] - reserved
	*/
	uint32_t cha_filt_bb;

	/** @brief ChA Equalization filter
	* bits [24:0] - KK coefficient (gain)
	* bits [31:25] - reserved
	*/
	uint32_t cha_filt_kk;

	/** @brief ChA Equalization filter
	* bits [24:0] - PP coefficient (pole)
	* bits [31:25] - reserved
	*/
	uint32_t cha_filt_pp;

	/** @brief ChB Equalization filter
	* bits [17:0] - AA coefficient (pole)
	* bits [31:18] - reserved
	*/
	uint32_t chb_filt_aa;

	/** @brief ChB Equalization filter
	* bits [24:0] - BB coefficient (zero)
	* bits [31:25] - reserved
	*/
	uint32_t chb_filt_bb;

	/** @brief ChB Equalization filter
	* bits [24:0] - KK coefficient (gain)
	* bits [31:25] - reserved
	*/
	uint32_t chb_filt_kk;

	/** @brief ChB Equalization filter
	* bits [24:0] - PP coefficient (pole)
	* bits [31:25] - reserved
	*/
	uint32_t chb_filt_pp;

	/* ChA & ChB data - 14 LSB bits valid starts from 0x10000 and
	* 0x20000 and are each 16k samples long */
} osc_control_t;


// The FPGA register structure for oscilloscope
static volatile osc_control_t *osc_reg = NULL;

// The FPGA input signal buffer pointer for channel A
static volatile uint32_t *osc_cha = NULL;

// The FPGA input signal buffer pointer for channel B */
static volatile uint32_t *osc_chb = NULL;


static const uint32_t DATA_DEC_MASK = 0xFFFF;
static const uint32_t DATA_AVG_MASK = 0x1;
static const uint32_t TRIG_SRC_MASK = 0x7; // (3 bits)
static const uint32_t START_DATA_WRITE_MASK = 0x1;
static const uint32_t THRESHOLD_MASK = 0x1FFF; // (13 bits)

/**
* general
*/

int osc_Init()
{
	ECHECK(cmn_Init());
	ECHECK(cmn_Map(OSC_BASE_SIZE, OSC_BASE_ADDR, (void**)&osc_reg));
	osc_cha = (uint32_t*)((char*)osc_reg + OSC_CHA_OFFSET);
	osc_chb = (uint32_t*)((char*)osc_reg + OSC_CHB_OFFSET);
	return RP_OK;
}

int osc_Release()
{
	ECHECK(cmn_Unmap(OSC_BASE_SIZE, (void**)&osc_reg));
	osc_cha = NULL;
	osc_chb = NULL;
	ECHECK(cmn_Release());
	return RP_OK;
}


/**
* decimation
*/

int osc_SetDecimation(uint32_t decimation)
{
	return cmn_SetValue(&osc_reg->data_dec, decimation, DATA_DEC_MASK);
}

int osc_GetDecimation(uint32_t* decimation)
{
	return cmn_GetValue(&osc_reg->data_dec, decimation, DATA_DEC_MASK);
}

int osc_SetAveraging(bool enable)
{
	if (enable) {
		return cmn_SetBits(&osc_reg->other, 0x1, DATA_AVG_MASK);
	}
	else {
		return cmn_UnsetBits(&osc_reg->other, 0x1, DATA_AVG_MASK);
	}
}

int osc_GetAveraging(bool* enable)
{
	return cmn_AreBitsSet(osc_reg->other, 0x1, DATA_AVG_MASK, enable);
}


/**
* trigger source
*/

int osc_SetTriggerSource(uint32_t source)
{
	return cmn_SetValue(&osc_reg->trig_source, source, TRIG_SRC_MASK);
}

int osc_GetTriggerSource(uint32_t* source)
{
	return cmn_GetValue(&osc_reg->trig_source, source, TRIG_SRC_MASK);
}

int osc_WriteDataIntoMemory(bool enable)
{
	if (enable) {
		return cmn_SetBits(&osc_reg->conf, 0x1, START_DATA_WRITE_MASK);
	}
	else {
		return cmn_UnsetBits(&osc_reg->conf, 0x1, START_DATA_WRITE_MASK);
	}
}

/**
* Threshold
*/

int osc_SetThresholdChA(uint32_t threshold)
{
	return cmn_SetValue(&osc_reg->cha_thr, threshold, THRESHOLD_MASK);
}

int osc_GetThresholdChA(uint32_t* threshold)
{
	return cmn_GetValue(&osc_reg->cha_thr, threshold, THRESHOLD_MASK);
}

int osc_SetThresholdChB(uint32_t threshold)
{
	return cmn_SetValue(&osc_reg->chb_thr, threshold, THRESHOLD_MASK);
}

int osc_GetThresholdChB(uint32_t* threshold)
{
	return cmn_GetValue(&osc_reg->chb_thr, threshold, THRESHOLD_MASK);
}

