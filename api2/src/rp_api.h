
#ifndef _RP_API_H_
#define _RP_API_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    RP_DRIVER_VERSION, ///< Version number of Red Pitaya APIs
    RP_USB_VERSION, ///< Type of USB connection to device: 1.1, 2.0 or 3.0
    RP_HARDWARE_VERSION, ///< Hardware version of device
    RP_VARIANT_INFO, ///<  Variant number of device
    RP_BATCH_AND_SERIAL, ///< Batch and serial number of device
    RP_CAL_DATE, ///<  Calibration date of device
    RP_KERNEL_VERSION, ///<  Version of kernel driver
    RP_DIGITAL_HARDWARE_VERSION, ///<  Hardware version of the digital section
    RP_ANALOGUE_HARDWARE_VERSION, ///< Hardware version of the analog section
} RP_INFO;

typedef enum {
    RP_API_OK, ///< The Red Pitaya is functioning correctly
    RP_MAX_UNITS_OPENED, ///< An attempt has been made to open more than RP_MAX_UNITS.
    RP_MEMORY_FAIL, //< Not enough memory could be allocated on the host machine
    RP_NOT_FOUND, ///< No PicoScope could be found
    RP_FW_FAIL, ///< Unable to download firmware
    RP_OPEN_OPERATION_IN_PROGRESS, ///<
    RP_OPERATION_FAILED, ///<
    RP_NOT_RESPONDING, ///< The PicoScope is not responding to commands from the PC
    RP_CONFIG_FAIL, ///< The configuration information in the PicoScope has become corrupt or is missing
    RP_KERNEL_DRIVER_TOO_OLD, ///< The picopp.sys file is too old to be used with the device driver
    RP_EEPROM_CORRUPT, ///< The EEPROM has become corrupt, so the device will use a default setting
    RP_OS_NOT_SUPPORTED, ///< The operating system on the PC is not supported by this driver
    RP_INVALID_HANDLE, ///< There is no device with the handle value passed
    RP_INVALID_PARAMETER, ///< A parameter value is not valid
    RP_INVALID_TIMEBASE, ///< The timebase is not supported or is invalid
    RP_INVALID_VOLTAGE_RANGE, ///< The voltage range is not supported or is invalid
    RP_INVALID_CHANNEL, ///< The channel number is not valid on this device or no channels have been set
    RP_INVALID_TRIGGER_CHANNEL, ///< The channel set for a trigger is not available on this device
    RP_INVALID_CONDITION_CHANNEL, ///< The channel set for a condition is not available on this device
    RP_NO_SIGNAL_GENERATOR, ///< The device does not have a signal generator
    RP_STREAMING_FAILED, ///< Streaming has failed to start or has stopped without user request
    RP_BLOCK_MODE_FAILED, ///< Block failed to start - a parameter may have been set wrongly
    RP_NULL_PARAMETER, ///< A parameter that was required is NULL
    RP_DATA_NOT_AVAILABLE, ///< No data is available from a run block call
    RP_STRING_BUFFER_TOO_SMALL, ///< The buffer passed for the information was too small
    RP_ETS_NOT_SUPPORTED, ///< ETS is not supported on this device
    RP_AUTO_TRIGGER_TIME_TOO_SHORT, ///< The auto trigger time is less than the time it will take to collect the pre-trigger data
    RP_BUFFER_STALL, ///< The collection of data has stalled as unread data would be overwritten
    RP_TOO_MANY_SAMPLES, ///< Number of samples requested is more than available in the current memory
    RP_TOO_MANY_SEGMENTS, ///< Not possible to create number of segments requested
    RP_PULSE_WIDTH_QUALIFIER, ///< A null pointer has been passed in the trigger function or one of the parameters is out of range
    RP_DELAY, ///< One or more of the hold-off parameters are out of range
    RP_SOURCE_DETAILS, ///< One or more of the source details are incorrect
    RP_CONDITIONS, ///< One or more of the conditions are incorrect
    RP_USER_CALLBACK, ///< The driver's thread is currently in the RP...Ready callback function and therefore the action cannot be carried out
    RP_DEVICE_SAMPLING, ///< An attempt is being made to get stored data while streaming. Either stop
    // streaming by calling rpStop, or use rpGetStreamingLatestValues
    RP_NO_SAMPLES_AVAILABLE, ///< ...because a run has not been completed
    RP_SEGMENT_OUT_OF_RANGE, ///< The memory index is out of range
    RP_BUSY, ///< Data cannot be returned yet
    RP_STARTINDEX_INVALID, ///< The start time to get stored data is out of range
    RP_INVALID_INFO, ///< The information number requested is not a valid number
    RP_INFO_UNAVAILABLE, ///< The handle is invalid so no information is available about the device. Only RP_DRIVER_VERSION is available.
    RP_INVALID_SAMPLE_INTERVAL, ///<The sample interval selected for streaming is out of range
    RP_TRIGGER_ERROR, ///<
    RP_MEMORY, ///< Driver cannot allocate memory
    RP_SIG_GEN_PARAM, ///< Incorrect parameter passed to the signal generator
    RP_SHOTS_SWEEPS_WARNING, ///< Conflict between the shots and sweeps parameters sent to the signal generator
    RP_WARNING_EXT_THRESHOLD_CONFLICT, ///< Attempt to set different EXT input thresholds set for signal generator and oscilloscope trigger
    RP_SIGGEN_OUTPUT_OVER_VOLTAGE, ///< The combined peak to peak voltage and the analog offset voltage exceed the allowable voltage the signal generator can produce
    RP_DELAY_NULL, ///< NULL pointer passed as delay parameter
    RP_INVALID_BUFFER, ///< The buffers for overview data have not been set while streaming
    RP_SIGGEN_OFFSET_VOLTAGE, ///< The analog offset voltage is out of range
    RP_SIGGEN_PK_TO_PK, ///< The analog peak to peak voltage is out of range
    RP_CANCELLED, ///< A block collection has been cancelled
    RP_SEGMENT_NOT_USED, ///< The segment index is not currently being used
    RP_INVALID_CALL, ///< The wrong GetValues function has been called for the collection mode in use
    RP_NOT_USED, ///< The function is not available
    RP_INVALID_SAMPLERATIO, ///< The aggregation ratio requested is out of range
    RP_INVALID_STATE, ///< Device is in an invalid state
    RP_NOT_ENOUGH_SEGMENTS, ///< The number of segments allocated is fewer than the number of captures requested
    RP_DRIVER_FUNCTION, ///< You called a driver function while another driver function was still being processed
    RP_RESERVED, ///<
    RP_INVALID_COUPLING, ///< An invalid coupling type was specified in ps3000aSetChannel
    RP_BUFFERS_NOT_SET, ///< An attempt was made to get data before a data buffer was defined
    RP_RATIO_MODE_NOT_SUPPORTED, ///< The selected downsampling mode (used for data reduction) is not allowed
    RP_INVALID_TRIGGER_PROPERTY, ///< An invalid parameter was passed to ps3000aSetTriggerChannelProperties
    RP_INTERFACE_NOT_CONNECTED, ///< The driver was unable to contact the oscilloscope
    RP_SIGGEN_WAVEFORM_SETUP_FAILED, ///< A problem occurred in ps3000aSetSigGenBuiltIn or ps3000aSetSigGenArbitrary
    RP_FPGA_FAIL,
    RP_POWER_MANAGER,
    RP_INVALID_ANALOGUE_OFFSET, ///< An impossible analogue offset value was specified in ps3000aSetChannel
    RP_PLL_LOCK_FAILED, ///< Unable to configure the PicoScope
    RP_ANALOG_BOARD, ///< The oscilloscope's analog board is not detected, or is not connected to thedigital board
    RP_CONFIG_FAIL_AWG, ///< Unable to configure the signal generator
    RP_INITIALISE_FPGA, ///< The FPGA cannot be initialized, so unit cannot be opened
    RP_EXTERNAL_FREQUENCY_INVALID, ///< The frequency for the external clock is not within ±5% of the stated value
    RP_CLOCK_CHANGE_ERROR, ///< The FPGA could not lock the clock signal
    RP_TRIGGER_AND_EXTERNAL_CLOCK_CLASH, ///< You are trying to configure the AUX input as both a trigger and a reference clock
    RP_PWQ_AND_EXTERNAL_CLOCK_CLASH, ///< You are trying to congfigure the AUX input as both a pulse width qualifier and a reference clock
    RP_UNABLE_TO_OPEN_SCALING_FILE, ///< The scaling file set can not be opened.
    RP_MEMORY_CLOCK_FREQUENCY, ///< The frequency of the memory is reporting incorrectly.
    RP_I2C_NOT_RESPONDING, ///< The I2C that is being actioned is not responding to requests.
    RP_NO_CAPTURES_AVAILABLE, ///< There are no captures available and therefore no data can be returned.
    RP_NOT_USED_IN_THIS_CAPTURE_MODE, ///< The capture mode the device is currently running in does not support the current request.
    RP_GET_DATA_ACTIVE, ///< Reserved
    RP_IP_NETWORKED, ///< The device is currently connected via the IP Network socket and thus the call made is not supported.
    RP_INVALID_IP_ADDRESS, ///< An IP address that is not correct has been passed to the driver.
    RP_IPSOCKET_FAILED, ///< The IP socket has failed.
    RP_IPSOCKET_TIMEDOUT, ///< The IP socket has timed out.
    RP_SETTINGS_FAILED, ///< The settings requested have failed to be set.
    RP_NETWORK_FAILED, ///< The network connection has failed.
    RP_WS2_32_DLL_NOT_LOADED, ///< Unable to load the WS2 dll.
    RP_INVALID_IP_PORT, ///< The IP port is invalid
    RP_COUPLING_NOT_SUPPORTED, ///< The type of coupling requested is not supported on the opened device.
    RP_BANDWIDTH_NOT_SUPPORTED, ///< Bandwidth limit is not supported on the opened device.
    RP_INVALID_BANDWIDTH, ///< The value requested for the bandwidth limit is out of range.
    RP_AWG_NOT_SUPPORTED, ///< The arbitrary waveform generator is not supported by the opened device.
    RP_ETS_NOT_RUNNING, ///< Data has been requested with ETS mode set but run block has not been called, or stop has been called.
    RP_SIG_GEN_WHITENOISE_NOT_SUPPORTED, ///< White noise is not supported on the opened device.
    RP_SIG_GEN_WAVETYPE_NOT_SUPPORTED, ///< The wave type requested is not supported by the opened device.
    RP_INVALID_DIGITAL_PORT, ///< A port number that does not evaluate to either RP_DIGITAL_PORT0 or RP_DIGITAL_PORT1 ///< the ports that are supported.
    RP_INVALID_DIGITAL_CHANNEL, ///< The digital channel is not in the range RP_DIGITAL_CHANNEL0 to RP_DIGITAL_CHANNEL15  the digital channels that are supported.
    RP_INVALID_DIGITAL_TRIGGER_DIRECTION, ///< The digital trigger direction is not a valid trigger direction and should be equal
    // in value to one of the RP_DIGITAL_DIRECTION enumerations.
    RP_SIG_GEN_PRBS_NOT_SUPPORTED, ///< Siggen does not generate pseudo-random bit stream.
    RP_ETS_NOT_AVAILABLE_WITH_LOGIC_CHANNELS, ///< When a digital port is enabled, ETS sample mode is not available for use.
    RP_WARNING_REPEAT_VALUE, ///< Not applicable to this device.
    RP_POWER_SUPPLY_CONNECTED, ///< 4-Channel only - The DC power supply is connected.
    RP_POWER_SUPPLY_NOT_CONNECTED, ///< 4-Channel only - The DC power supply isn’t connected.
    RP_POWER_SUPPLY_REQUEST_INVALID, ///< Incorrect power mode passed for current power source.
    RP_POWER_SUPPLY_UNDERVOLTAGE, ///< The supply voltage from the USB source is too low.
    RP_CAPTURING_DATA, ///< The oscilloscope is in the process of capturing data.
    RP_USB3_0_DEVICE_NON_USB3_0_PORT, ///< A USB 3.0 device is connected to a non-USB 3.0 port.
} RP_STATUS;


typedef enum rpDigitalPort {
    RP_DIGITAL_PORT0 = 0x80, // (digital channels 0–7)
    RP_DIGITAL_PORT1 = 0x81  // (digital channels 8–15)
} RP_DIGITAL_PORT;


typedef enum rpChannel {
    RP_CH_AIN1=0,
    RP_CH_AIN2,
    RP_CH_AIN3,
    RP_CH_AIN4,
    RP_CH_DIN
} RP_CHANNEL;

typedef enum rpDigitalChannel
{
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

typedef enum rpDigitalDirection
{
    RP_DIGITAL_DONT_CARE,
    RP_DIGITAL_DIRECTION_LOW,
    RP_DIGITAL_DIRECTION_HIGH,
    RP_DIGITAL_DIRECTION_RISING,
    RP_DIGITAL_DIRECTION_FALLING,
    RP_DIGITAL_DIRECTION_RISING_OR_FALLING,
    RP_DIGITAL_MAX_DIRECTION
} RP_DIGITAL_DIRECTION;


typedef struct tPS3000ADigitalChannelDirections
{
    RP_DIGITAL_CHANNEL channel;
    RP_DIGITAL_DIRECTION direction;
} RP_DIGITAL_CHANNEL_DIRECTIONS;


typedef enum rpRatioMode
{
    RP_RATIO_MODE_NONE, //(downSampleRatio is ignored)
    RP_RATIO_MODE_AGGREGATE,
    RP_RATIO_MODE_AVERAGE,
    RP_RATIO_MODE_DECIMATE
} RP_RATIO_MODE;

typedef enum rpTimeUnits
{
    RP_FS,
    RP_PS,
    RP_NS,
    RP_US,
    RP_MS,
    RP_S
} RP_TIME_UNITS;


typedef void (*rpBlockReady)(RP_STATUS rp_status,
                             void * pParameter);

typedef void (*rpStreamingReady)(int32_t noOfSamples,
                                 uint32_t startIndex,
                                 int16_t overflow,
                                 uint32_t triggerAt,
                                 int16_t triggered,
                                 int16_t autoStop,
                                 void * pParameter);

RP_STATUS rp_OpenUnit(void);

RP_STATUS rp_CloseUnit(void);

RP_STATUS rp_GetUnitInfo(int8_t * string,
                        int16_t stringLength,
                        int16_t * requiredSize,
                        RP_INFO info);

RP_STATUS rp_SetTriggerDigitalPortProperties(RP_DIGITAL_CHANNEL_DIRECTIONS * directions,
                                            int16_t nDirections);


RP_STATUS rp_EnableDigitalPortDataRLE(bool enable);
RP_STATUS rp_SoftwareTrigger(void);
RP_STATUS rp_IsAcquistionComplete();

RP_STATUS rp_GetTimebase(uint32_t timebase,
                        int32_t noSamples,
                        double * timeIntervalNanoseconds,
                        //int16_t oversample,
                        uint32_t * maxSamples
                        //uint32_t segmentIndex
                        );

RP_STATUS rp_SetDataBuffer(RP_CHANNEL channel,
                          int16_t * buffer,
                          int32_t bufferLth,
                          // uint32_t segmentIndex,
                          RP_RATIO_MODE mode);

RP_STATUS rp_RunBlock(uint32_t noOfPreTriggerSamples,
                       uint32_t noOfPostTriggerSamples,
                       uint32_t timebase,
                       // int16_t oversample,
                       double * timeIndisposedMs,
                       //uint32_t segmentIndex,
                       rpBlockReady rpReady,
                       void * pParameter);


RP_STATUS rp_RunStreaming(uint32_t * sampleInterval,
                        RP_TIME_UNITS sampleIntervalTimeUnits,
                        uint32_t maxPreTriggerSamples,
                        uint32_t maxPostTriggerSamples,
                        int16_t autoStop,
                        uint32_t downSampleRatio,
                        RP_RATIO_MODE downSampleRatioMode,
                        uint32_t overviewBufferSize);

RP_STATUS rp_GetTrigPosition(uint32_t * tigger_pos);

RP_STATUS rp_GetValues(uint32_t startIndex,
                      uint32_t * noOfSamples,
                      uint32_t downSampleRatio,
                      RP_RATIO_MODE downSampleRatioMode,
                      //uint32_t segmentIndex,
                      int16_t * overflow);

RP_STATUS rp_GetStreamingLatestValues(rpStreamingReady rpReady,
                                     void * pParameter);

RP_STATUS rp_GetValuesAsync(uint32_t startIndex,
                           uint32_t noOfSamples,
                           uint32_t downSampleRatio,
                           RP_RATIO_MODE downSampleRatioMode,
                           //uint32_t segmentIndex
                           void * lpDataReady,
                           void * pParameter);

RP_STATUS rp_Stop(void);


/** SIGNAL GENERATION  */

typedef enum rpWaveType {
    RP_SG_SINE, ///< sine wave
    RP_SG_SQUARE, ///< square wave
    RP_SG_TRIANGLE, ///< triangle wave
    RP_SG_DC_VOLTAGE, ///< DC voltage
    RP_SG_RAMP_UP, ///< rising sawtooth
    RP_SG_RAMP_DOWN, ///< falling sawtooth
    RP_SG_SINC, ///< sin (x)/x
    RP_SG_GAUSSIAN, ///< Gaussian
    PR_SG_HALF_SINE, ///< half (full-wave rectified) sine
} RP_WAVE_TYPE;

typedef enum rpSweepType{
    RP_SWEEP_UP, ///<
    RP_SWEEP_DOWN, ///<
    RP_SWEEP_UPDOWN, ///<
    RP_SWEEP_DOWNUP, ///<
} RP_SWEEP_TYPE;


typedef enum rpExtraOperationType{
    RP_ES_OFF, ///< normal signal generator operation specified by wavetype.
    RP_WHITENOISE, ///< the signal generator produces white noise and ignores all settings except pkToPk and offsetVoltage.
    RP_PRBS, ///< produces a pseudorandom binary sequence with bit rate specified by the start and stop frequencies.
} RP_EXTRA_OPERATIONS;


typedef enum rpTriggerType{
    RP_SIGGEN_RISING,     ///< trigger on rising edge
    RP_SIGGEN_FALLING,     ///< trigger on falling edge
    RP_SIGGEN_GATE_HIGH,///< run while trigger is high
    RP_SIGGEN_GATE_LOW, ///< run while trigger is low
} RP_SIGGEN_TRIG_TYPE;


typedef enum rpTriggerSource {
    RP_SIGGEN_NONE, ///< run without waiting for trigger
    RP_SIGGEN_SCOPE_TRIG, ///< use scope trigger
    RP_SIGGEN_EXT_IN, ///< use EXT input
    RP_SIGGEN_SOFT_TRIG, ///< wait for software trigger provided by rpSigGenSoftwareControl()
    RP_SIGGEN_TRIGGER_RAW // reserved
} RP_SIGGEN_TRIG_SOURCE;

RP_STATUS rp_DigSigGenOuput(bool enable);

RP_STATUS rp_SigGenSoftwareControl(int16_t state);

RP_STATUS rp_SetSigGenBuiltIn(int32_t offsetVoltage,
                             uint32_t pkToPk,
                             RP_WAVE_TYPE waveType,
                             float startFrequency,
                             float stopFrequency,
                             float increment,
                             float dwellTime,
                             RP_SWEEP_TYPE sweepType,
                             RP_EXTRA_OPERATIONS operation,
                             uint32_t shots,
                             uint32_t sweeps,
                             RP_SIGGEN_TRIG_TYPE triggerType,
                             RP_SIGGEN_TRIG_SOURCE triggerSource,
                             int16_t extInThreshold);

/** DIGITAL SIGNAL GENERATION  */

typedef enum patternType{
    RP_DIG_SIGGEN_PAT_UP_COUNT_8BIT_SEQ_256, ///< counts 8bit
} RP_DIG_SIGGEN_PAT_TYPE;

RP_STATUS rp_DigSigGenSoftwareControl(int16_t state);

RP_STATUS rp_SetDigSigGenBuiltIn(RP_DIG_SIGGEN_PAT_TYPE patternType,
                                double * sample_rate,
                                uint32_t shots,
                                uint32_t delay_between_shots,
                                uint32_t triggerSourceMask);
#endif // _RP_API_H_
