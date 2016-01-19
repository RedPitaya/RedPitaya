////////////////////////////////////////////////////////////////////////////////
// Red Pitaya library acquire module interface
// Author: Red Pitaya
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

#ifndef SRC_ACQIRE_H_
#define SRC_ACQIRE_H_

#include <stdint.h>
#include <stdbool.h>

#define RP_MNA 2

#define RP_ACQ_DWI 14

#define RP_ACQ_CTL_RST_MASK 0x1
#define RP_ACQ_CTL_TRG_MASK 0x2
#define RP_ACQ_CTL_ACQ_MASK 0x4

// Base acquire address
static const int ACQUIRE_BASE_SIZE = 0x00100000;

// acquire structure declaration
typedef struct {
    // control/status
    uint32_t ctl;
//    {
//       uint32_t rst :1;
//       uint32_t trg :1;
//       uint32_t acq :1;
//    }
    // configuration
    uint32_t cfg_rng;  // :1;
    // trigger
    uint32_t cfg_sel;  // TWG
    uint32_t cfg_dly;  // u32
    // edge detection
    int32_t  cfg_lvl;  // s14
    uint32_t cfg_hst;  // u14
    // filter/decimation
    uint32_t cfg_byp;  // :1;
    uint32_t cfg_avg;  // :1;
    uint32_t cfg_dec;  // :DWC;
    uint32_t cfg_shr;  // :DWS;
    int32_t  cfg_faa;
    int32_t  cfg_fbb;
    int32_t  cfg_fkk;
    int32_t  cfg_fpp;
} acq_regset_t;

static const uint32_t THRESHOLD_MASK        = 0x3FFF;       // (14 bits)
static const uint32_t HYSTERESIS_MASK       = 0x3FFF;       // (14 bits)
static const uint32_t TRIG_DELAY_MASK       = 0xFFFFFFFF;   // (32 bits)
static const uint32_t WRITE_POINTER_MASK    = 0x3FFF;       // (14 bits)
static const uint32_t EQ_FILTER_AA          = 0x3FFFF;      // (18 bits)
static const uint32_t EQ_FILTER             = 0x1FFFFFF;    // (25 bits)
static const uint32_t RST_WR_ST_MCH_MASK    = 0x2;          // (1st bit)
static const uint32_t TRIG_ST_MCH_MASK      = 0x4;          // (2st bit)
static const uint32_t PRE_TRIGGER_COUNTER   = 0xFFFFFFFF;   // (32 bit)

/* @brief Default filter equalization coefficients */
static const uint32_t GAIN_LO_FILT_AA = 0x7D93;
static const uint32_t GAIN_LO_FILT_BB = 0x437C7;
static const uint32_t GAIN_LO_FILT_PP = 0x2666;
static const uint32_t GAIN_LO_FILT_KK = 0xd9999a;
static const uint32_t GAIN_HI_FILT_AA = 0x4C5F;
static const uint32_t GAIN_HI_FILT_BB = 0x2F38B;
static const uint32_t GAIN_HI_FILT_PP = 0x2666;
static const uint32_t GAIN_HI_FILT_KK = 0xd9999a;

int acq_Init();
int acq_Release();

/**
 * Sets the decimation used at acquiring signal. There is only a set of pre-defined decimation
 * values which can be specified. See the #rp_acq_decimation_t enum values.
 * @param decimation Specify one of pre-defined decimation values
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetDecimation(rp_handle_uio_t *handle, uint32_t decimation);

/**
 * Gets the decimation used at acquiring signal. There is only a set of pre-defined decimation
 * values which can be specified. See the #rp_acq_decimation_t enum values.
 * @param decimation Returns one of pre-defined decimation values which is currently set.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetDecimation(rp_handle_uio_t *handle, uint32_t *decimation);

/**
 * Enables or disables averaging of data between samples.
 * Data between samples can be averaged by setting the averaging flag in the Data decimation register.
 * @param enabled When true, the averaging is enabled, otherwise it is disabled.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetAveraging(rp_handle_uio_t *handle, bool enabled);

/**
 * Returns information if averaging of data between samples is enabled or disabled.
 * Data between samples can be averaged by setting the averaging flag in the Data decimation register.
 * @param enabled Set to true when the averaging is enabled, otherwise is it set to false.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetAveraging(rp_handle_uio_t *handle, bool *enabled);

/**
 * Sets the trigger source used at acquiring signal. When acquiring is started,
 * the FPGA waits for the trigger condition on the specified source and when the condition is met, it
 * starts writing the signal to the buffer.
 * @param source Trigger source.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetTriggerSrc(rp_handle_uio_t *handle, rp_acq_trig_src_t source);

/**
 * Gets the trigger source used at acquiring signal. When acquiring is started,
 * the FPGA waits for the trigger condition on the specified source and when the condition is met, it
 * starts writing the signal to the buffer.
 * @param source Currently set trigger source.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetTriggerSrc(rp_handle_uio_t *handle, rp_acq_trig_src_t *source);

/**
 * Returns the trigger state. Either it is waiting for a trigger to happen, or it has already been triggered.
 * By default it is in the triggered state, which is treated the same as disabled.
 * @param state Trigger state
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetTriggerState(rp_handle_uio_t *handle, rp_acq_trig_state_t* state);

/**
 * Sets the number of decimated data after trigger written into memory.
 * @param decimated_data_num Number of decimated data. It must not be higher than the ADC buffer size.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetTriggerDelay(rp_handle_uio_t *handle, uint32_t value);

/**
 * Returns current number of decimated data after trigger written into memory.
 * @param decimated_data_num Number of decimated data.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetTriggerDelay(rp_handle_uio_t *handle, uint32_t *value);

/**
 * Returns the number of valid data ponts before trigger.
 * @param time_ns number of data points.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetPreTriggerCounter(rp_handle_uio_t *handle, uint32_t* value);

/**
 * Sets the trigger threshold value in volts. Makes the trigger when ADC value crosses this value.
 * @param voltage Threshold value for the channel
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetTriggerLevel(rp_handle_uio_t *handle, float voltage);

/**
 * Gets currently set trigger threshold value in volts
 * @param voltage Current threshold value for the channel
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetTriggerLevel(rp_handle_uio_t *handle, float *voltage);

/**
 * Sets the trigger threshold hysteresis value in volts.
 * Value must be outside to enable the trigger again.
 * @param voltage Threshold hysteresis value for the channel
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetTriggerHyst(rp_handle_uio_t *handle, float voltage);

/**
 * Gets currently set trigger threshold hysteresis value in volts
 * @param voltage Current threshold hysteresis value for the channel
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetTriggerHyst(rp_handle_uio_t *handle, float *voltage);

/**
 * Sets the acquire gain state. The gain should be set to the same value as it is set on the Red Pitaya
 * hardware by the LV/HV gain jumpers. LV = 1V; HV = 20V.
 * @param channel Channel A or B
 * @param state High or Low state
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqSetRange(rp_handle_uio_t *handle, int range);

/**
 * Returns the currently set acquire gain state in the library. It may not be set to the same value as
 * it is set on the Red Pitaya hardware by the LV/HV gain jumpers. LV = 1V; HV = 20V.
 * @param channel Channel A or B
 * @param state Currently set High or Low state in the library.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetRange(rp_handle_uio_t *handle, int* range);

/**
 * Returns current position of ADC write pointer.
 * @param pos Write pointer position
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetWritePointer(rp_handle_uio_t *handle, uint32_t* pos);

/**
 * Returns position of ADC write pointer at time when trigger arrived.
 * @param pos Write pointer position
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetWritePointerAtTrig(rp_handle_uio_t *handle, uint32_t* pos);

/**
 * Starts the acquire. Signals coming from the input channels are acquired and written into memory.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqStart(rp_handle_uio_t *handle);

/**
* Stops the acquire.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_AcqStop(rp_handle_uio_t *handle);

/**
 * Resets the acquire writing state machine.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqReset(rp_handle_uio_t *handle);

/**
 * Returns the latest ADC buffer samples in Volt units.
 * Output buffer must be at least 'size' long.
 * @param channel Channel A or B for which we want to retrieve the ADC buffer.
 * @param size Length of the ADC buffer to retrieve. Returns length of filled buffer. In case of too small buffer, required size is returned.
 * @param buffer The output buffer gets filled with the selected part of the ADC buffer.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_AcqGetData(rp_handle_uio_t *handle, uint32_t* size, int16_t *buffer);


int rp_AcqGetBufSize(rp_handle_uio_t *handle, uint32_t* size);


#endif // SRC_ACQUIRE_H_
