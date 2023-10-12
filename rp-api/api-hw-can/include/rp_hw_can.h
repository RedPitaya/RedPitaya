/**
 * $Id: $
 *
 * @file rp_hw_can.h
 * @brief Red Pitaya library API hardware interface for CAN
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

#ifndef RP_HW_CAN_H
#define RP_HW_CAN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/** @name Error codes
 *  Various error codes returned by the API.
 */
///@{

#define RP_HW_CAN_OK     0      // Success
#define RP_HW_CAN_ESI    1      // Failed start interface
#define RP_HW_CAN_EST    2      // Failed stop interface
#define RP_HW_CAN_ERI    3      // Failed restart interface
#define RP_HW_CAN_EUI    4      // Unknown interface
#define RP_HW_CAN_EBS    5      // Failed set or get bitrate and sample point
#define RP_HW_CAN_EBT    6      // Failed set or get bittiming
#define RP_HW_CAN_EGF    7      // Failed get clock parameters
#define RP_HW_CAN_EGE    8      // Failed get error counters
#define RP_HW_CAN_ERT    9      // Failed set or get restart time
#define RP_HW_CAN_EGS    10     // Failed get current interface state
#define RP_HW_CAN_ECM    11     // Failed get or set controller mode
#define RP_HW_CAN_ECU    12     // Controller mode not supported
#define RP_HW_CAN_ESO    13     // Failed open socket
#define RP_HW_CAN_ESC    14     // Failed close socket
#define RP_HW_CAN_ESA    15     // Failed. Socket already open
#define RP_HW_CAN_ESB    16     // Failed bind socket
#define RP_HW_CAN_ESN    17     // Failed. Socket not opened
#define RP_HW_CAN_ESD    18     // Failed. Missing data
#define RP_HW_CAN_ESBO   19     // Failed. Send buffer overflow
#define RP_HW_CAN_ESTE   20     // Failed. Timeout reached
#define RP_HW_CAN_ESPE   21     // Failed. Poll error
#define RP_HW_CAN_ESE    22     // Failed. Send error
#define RP_HW_CAN_ESFA   23     // Failed add filter. Filter already present in list
#define RP_HW_CAN_ESFS   24     // Failed apply filter
#define RP_HW_CAN_ESEF   25     // Failed to set error handling
#define RP_HW_CAN_ESR    26     // Failed read frame from socket





///@}

/**
 * CAN interface
 */
typedef enum {
    RP_CAN_0 = 0,   /* can0 interface */
    RP_CAN_1 = 1,   /* can1 interface */
} rp_can_interface_t;

typedef struct {
    uint32_t tq;            /* Time quanta (TQ) in nanoseconds */
    uint32_t prop_seg;      /* Propagation segment in TQs */
    uint32_t phase_seg1;    /* Phase buffer segment 1 in TQs */
    uint32_t phase_seg2;    /* Phase buffer segment 2 in TQs */
    uint32_t sjw;           /* Synchronisation jump width in TQs */
    uint32_t brp;           /* Bit-rate prescaler */
} rp_can_bittiming_t;

typedef struct {
    uint32_t tseg1_min;	    /* Time segement 1 = prop_seg + phase_seg1 */
	uint32_t tseg1_max;
	uint32_t tseg2_min;	    /* Time segement 2 = phase_seg2 */
	uint32_t tseg2_max;
	uint32_t sjw_max;		/* Synchronisation jump width */
	uint32_t brp_min;		/* Bit-rate prescaler */
	uint32_t brp_max;
	uint32_t brp_inc;
} rp_can_bittiming_limits_t;

typedef enum  {
	RP_CAN_STATE_ERROR_ACTIVE   = 0,	/* RX/TX error count < 96 */
	RP_CAN_STATE_ERROR_WARNING  = 1,	/* RX/TX error count < 128 */
	RP_CAN_STATE_ERROR_PASSIVE  = 2,	/* RX/TX error count < 256 */
	RP_CAN_STATE_BUS_OFF        = 3,	/* RX/TX error count >= 256 */
	RP_CAN_STATE_STOPPED        = 4,   	/* Device is stopped */
	RP_CAN_STATE_SLEEPING       = 5,    /* Device is sleeping */
} rp_can_state_t;

typedef enum {
    RP_CAN_MODE_LOOPBACK        = 0x1, 	/* Loopback mode */
    RP_CAN_MODE_LISTENONLY      = 0x2, 	/* Listen-only mode */
    RP_CAN_MODE_3_SAMPLES       = 0x4, 	/* Triple sampling mode */
    RP_CAN_MODE_ONE_SHOT        = 0x8, 	/* One-Shot mode */
    RP_CAN_MODE_BERR_REPORTING  = 0x10, /* Bus-error reporting */
} rp_can_mode_t;

typedef struct {
    uint32_t can_id;
    uint32_t can_id_raw;
    bool     is_extended_format;
    bool     is_error_frame;
    bool     is_remote_request;
    uint8_t  can_dlc;
    uint8_t  data[8];
} rp_can_frame_t;

/**
 * Retrieves the library version number
 * @return Library version
 */
const char* rp_CanGetVersion();

/**
 * Returns textual representation of error code.
 * @param errorCode Error code returned from API.
 * @return Textual representation of error given error code.
 */
const char* rp_CanGetError(int errorCode);

/**
 * Enables FPGA forwarding from CAN controller to GPIO.
 * @param enable  Turns on the mode.
 * @return If the function is successful, the return value is RP_HW_CAN_OK.
 * If the function is unsuccessful, the return value is any of RP_HW_CAN_E* values that indicate an error.
 */
int rp_CanSetFPGAEnable(bool enable);

/**
* Gets the status from the FPGA of the CAN mode status
* @param state  Current state.
* @return If the function is successful, the return value is RP_HW_CAN_OK.
* If the function is unsuccessful, the return value is any of RP_HW_CAN_E* values that indicate an error.
*/
int rp_CanGetFPGAEnable(bool *state);

/**
* Sets the state of the specified interface to UP.
* Before starting the interface or restarting. Be sure to set the bitrate.
* @param interface  Selected interface
* @return If the function is successful, the return value is RP_HW_CAN_OK.
* If the function is unsuccessful, the return value is any of RP_HW_CAN_E* values that indicate an error.
* The function will return an error if the interface is already UP.
*/
int rp_CanStart(rp_can_interface_t interface);

/**
* Sets the state of the specified interface to DOWN
* @param interface  Selected interface
* @return If the function is successful, the return value is RP_HW_CAN_OK.
* If the function is unsuccessful, the return value is any of RP_HW_CAN_E* values that indicate an error.
* The function will return an error if the interface is already DOWN.
*/
int rp_CanStop(rp_can_interface_t interface);

/**
* Restarts the specified interface
* Before starting the interface or restarting. Be sure to set the bitrate
* @param interface  Selected interface
* @return If the function is successful, the return value is RP_HW_CAN_OK.
* If the function is unsuccessful, the return value is any of RP_HW_CAN_E* values that indicate an error.
* The function will return an error if the interface is turned DOWN.
*/
int rp_CanRestart(rp_can_interface_t interface);

/**
* Returns the current state of the CAN interface
* @param interface  Selected interface
* @param state  Current state
* @return If the function is successful, the return value is RP_HW_CAN_OK.
* If the function is unsuccessful, the return value is any of RP_HW_CAN_E* values that indicate an error.
*/
int rp_CanGetState(rp_can_interface_t interface,rp_can_state_t *state);

/**
* Returns the name of state 
* @param state  interface state
* @return If the function is successful, the return name of state.
*/
const char * rp_CanGetStateName(rp_can_state_t state);

/**
* Sets the bitrate for the specified interface. Sample point is set automatically. 
* @param interface  Selected interface
* @param bitRate Bitrate in Hz (bits/second) 
* @return If the function is successful, the return value is RP_HW_CAN_OK.
* If the function is unsuccessful, the return value is any of RP_HW_CAN_E* values that indicate an error.
*/
int rp_CanSetBitrate(rp_can_interface_t interface,uint32_t bitRate);

/**
* Sets the bitrate and sample point for the specified interface.
* @param interface  Selected interface
* @param bitRate Bitrate in Hz (bits/second) 
* @param samplePoint Sample point must be from 0 to 0.999. Sample point in one-tenth of a percent 
* @return If the function is successful, the return value is RP_HW_CAN_OK.
* If the function is unsuccessful, the return value is any of RP_HW_CAN_E* values that indicate an error.
*/
int rp_CanSetBitrateAndSamplePoint(rp_can_interface_t interface,uint32_t bitRate,float samplePoint);

/**
* Shows the real bit-rate in bits/sec and the sample-point in the
* range 0.000..0.999. If the calculation of bit-timing parameters
* is enabled in the kernel (CONFIG_CAN_CALC_BITTIMING=y), the
* bit-timing can be defined by setting the "bitrate" argument.
* Optionally the "sample-point" can be specified. By default it's
* 0.000 assuming CIA-recommended sample-points.
* @param interface  Selected interface
* @param bitRate Bitrate in Hz
* @param samplePoint Sample point {0 to 0.999}
* @return If the function is successful, the return value is RP_HW_CAN_OK.
* If the function is unsuccessful, the return value is any of RP_HW_CAN_E* values that indicate an error.
*/
int rp_CanGetBitrateAndSamplePoint(rp_can_interface_t interface,uint32_t *bitRate,float *samplePoint);

/**
* Shows the time quanta in ns, propagation segment, phase buffer
* segment 1 and 2 and the synchronisation jump width in units of
* tq. They allow to define the CAN bit-timing in a hardware
* independent format as proposed by the Bosch CAN 2.0 spec (see
* chapter 8 of http://www.semiconductors.bosch.de/pdf/can2spec.pdf).
* About bittiming you can read there: https://en.wikipedia.org/wiki/CAN_bus#Bit_timing
* @param interface  Selected interface
* @param bitTiming Pointer to structure
* @return If the function is successful, the return value is RP_HW_CAN_OK.
* If the function is unsuccessful, the return value is any of RP_HW_CAN_E* values that indicate an error.
*/
int rp_CanGetBitTiming(rp_can_interface_t interface, rp_can_bittiming_t *bitTiming);

/**
* Set bittiming settings.
* About bittiming you can read there: https://en.wikipedia.org/wiki/CAN_bus#Bit_timing
* @param interface  Selected interface
* @param bitTiming Pointer to structure
* @return If the function is successful, the return value is RP_HW_CAN_OK.
* If the function is unsuccessful, the return value is any of RP_HW_CAN_E* values that indicate an error.
*/
int rp_CanSetBitTiming(rp_can_interface_t interface, rp_can_bittiming_t bitTiming);

/**
* Shows the bit-timing constants of the CAN controller, here the
* "sja1000". The minimum and maximum values of the time segment 1
* and 2, the synchronisation jump width in units of tq, the
* bitrate pre-scaler and the CAN system clock frequency in Hz.
* These constants could be used for user-defined (non-standard)
* bit-timing calculation algorithms in user-space.
* About bittiming you can read there: https://en.wikipedia.org/wiki/CAN_bus#Bit_timing
* @param interface  Selected interface
* @param bitTiming Pointer to structure
* @return If the function is successful, the return value is RP_HW_CAN_OK.
* If the function is unsuccessful, the return value is any of RP_HW_CAN_E* values that indicate an error.
*/
int rp_CanGetBitTimingLimits(rp_can_interface_t interface, rp_can_bittiming_limits_t *bitTiming);

/**
* Returns the clock value in Hz
* @param interface  Selected interface
* @param freq Clock frequency
* @return If the function is successful, the return value is RP_HW_CAN_OK.
* If the function is unsuccessful, the return value is any of RP_HW_CAN_E* values that indicate an error.
*/
int rp_CanGetClockFreq(rp_can_interface_t interface, uint32_t *freq);

/**
* Returns the number of errors on the bus
* @param interface  Selected interface
* @param tx TX error counter
* @param rx RX error counter
* @return If the function is successful, the return value is RP_HW_CAN_OK.
* If the function is unsuccessful, the return value is any of RP_HW_CAN_E* values that indicate an error.
*/
int rp_CanGetBusErrorCounters(rp_can_interface_t interface, uint16_t *tx, uint16_t *rx);



/**
* Automatic restart delay time. If set to a non-zero value, a
* restart of the CAN controller will be triggered automatically
* in case of a bus-off condition after the specified delay time
* in milliseconds. By default it's off.
* @param interface  Selected interface
* @param ms time in ms
* @return If the function is successful, the return value is RP_HW_CAN_OK.
* If the function is unsuccessful, the return value is any of RP_HW_CAN_E* values that indicate an error.
*/
int rp_CanSetRestartTime(rp_can_interface_t interface, uint32_t ms);

/**
* Returns current settings for restart-ms
* @param interface  Selected interface
* @param ms current value of restart-ms
* @return If the function is successful, the return value is RP_HW_CAN_OK.
* If the function is unsuccessful, the return value is any of RP_HW_CAN_E* values that indicate an error.
*/
int rp_CanGetRestartTime(rp_can_interface_t interface, uint32_t *ms);



/**
* Sets the controller mode
* @param interface  Selected interface
* @param ms time in ms
* @return If the function is successful, the return value is RP_HW_CAN_OK.
* If the function is unsuccessful, the return value is any of RP_HW_CAN_E* values that indicate an error.
*/
int rp_CanSetControllerMode(rp_can_interface_t interface, rp_can_mode_t mode, bool state);

/**
* Checks the status of the selected mode
* @param interface  Selected interface
* @param mode controller mode
* @param state return current state
* @return If the function is successful, the return value is RP_HW_CAN_OK.
* If the function is unsuccessful, the return value is any of RP_HW_CAN_E* values that indicate an error.
*/
int rp_CanGetControllerMode(rp_can_interface_t interface, rp_can_mode_t mode, bool *state);


/**
* Opens a socket connection for the specified interface
* @param interface  Selected interface
* @return If the function is successful, the return value is RP_HW_CAN_OK.
* If the function is unsuccessful, the return value is any of RP_HW_CAN_E* values that indicate an error.
*/
int rp_CanOpen(rp_can_interface_t interface);

/**
* Closes an open connection
* @param interface  Selected interface
* @return If the function is successful, the return value is RP_HW_CAN_OK.
* If the function is unsuccessful, the return value is any of RP_HW_CAN_E* values that indicate an error.
*/
int rp_CanClose(rp_can_interface_t interface);

/**
* Sends the package to the specified address
* @param interface  Selected interface
* @param canId Address of the can device
* @param data Dispatch data. The data must be up to 8 bytes. The rest of the data is ignored
* @param dataSize  The size of the data to be transmitted. From 0 to 8
* @param isExtended Use an extended frame header
* @param rtr Mark frame as remote transmission request
* @param timeout Timeout when sending data. Needed if buffer is full. 0 - timeout is disabled. 
* @return If the function is successful, the return value is RP_HW_CAN_OK.
* If the function is unsuccessful, the return value is any of RP_HW_CAN_E* values that indicate an error.
*/
int rp_CanSend(rp_can_interface_t interface, uint32_t canId, unsigned char *data, uint8_t dataSize, bool isExtended, bool rtr, uint32_t timeout);

/**
* Reads from socket 1 frame 
* @param interface  Selected interface
* @param timeout Timeout when reading data. 0 - timeout is disabled. 
* @param frame Data frame
* @return If the function is successful, the return value is RP_HW_CAN_OK.
* If the function is unsuccessful, the return value is any of RP_HW_CAN_E* values that indicate an error.
*/
int rp_CanRead(rp_can_interface_t interface, uint32_t timeout, rp_can_frame_t *frame);

/**
* Adds another filter to the list of filters. 
* Once all filters have been added, the command to apply filters on the socket must be invoked rp_CanSetFilter.
* A filter matches, when
* <received_can_id> & mask == filter & mask
* @param interface  Selected interface
* @param filter Value of filter
* @param mask Value of mask
* @return If the function is successful, the return value is RP_HW_CAN_OK.
* If the function is unsuccessful, the return value is any of RP_HW_CAN_E* values that indicate an error.
*/
int rp_CanAddFilter(rp_can_interface_t interface, uint32_t filter, uint32_t mask);

/**
* Deletes the specified filter from the filter list.
* @param interface  Selected interface
* @param filter Value of filter
* @param mask Value of mask
* @return If the function is successful, the return value is RP_HW_CAN_OK.
* If the function is unsuccessful, the return value is any of RP_HW_CAN_E* values that indicate an error.
*/
int rp_CanRemoveFilter(rp_can_interface_t interface, uint32_t filter, uint32_t mask);

/**
* Removes all filters from the list.
* To apply the deleted filters, you need to call rp_CanSetFilter
* @param interface  Selected interface
* @param filter Value of filter
* @param mask Value of mask
* @return If the function is successful, the return value is RP_HW_CAN_OK.
* If the function is unsuccessful, the return value is any of RP_HW_CAN_E* values that indicate an error.
*/
int rp_CanClearFilter(rp_can_interface_t interface);

/**
* Applies a list of filters to the socket connection.
* To filter all frames, you must first call: rp_CanClearFilter 
* Important. In order to restore the filter, it is necessary to specify correct masks for frames. Otherwise it will filter on the whole header.
* @param interface  Selected interface
* @param isJoinFilter If True, the filter list is applied to the incoming packet with a logical AND operation. Otherwise, the OR operation will be used. Join mode NOT SUPPORTED driver yet.
* @return If the function is successful, the return value is RP_HW_CAN_OK.
* If the function is unsuccessful, the return value is any of RP_HW_CAN_E* values that indicate an error.
*/
int rp_CanSetFilter(rp_can_interface_t interface, bool isJoinFilter);

/**
* When this mode is enabled, all errors will be converted through data frames with the error frame marking
* @param interface  Selected interface
* @param enable Enables error generation mode.
* @return If the function is successful, the return value is RP_HW_CAN_OK.
* If the function is unsuccessful, the return value is any of RP_HW_CAN_E* values that indicate an error.
*/
int rp_CanShowErrorFrames(rp_can_interface_t interface, bool enable);

#ifdef __cplusplus
}
#endif

#endif // RP_HW_CAN_H
