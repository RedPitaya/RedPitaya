/**
 * $Id: $
 *
 * @file rp_hw.h
 * @brief Red Pitaya library API hardware interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef RP_HW_H
#define RP_HW_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/** @name Error codes
 *  Various error codes returned by the API.
 */
///@{

/** Success */
#define RP_HW_OK     0
/** Invalid parameter value */
#define RP_HW_EIPV   15
/** Unsupported Feature */
#define RP_HW_EUF    16
/** Failed to init uart */
#define RP_HW_EIU    25
/** Failed read from uart */
#define RP_HW_ERU    26
/** Failed write to uart */
#define RP_HW_EWU    27
/** Failed set settings to uart */
#define RP_HW_ESU    28
/** Failed get settings from uart */
#define RP_HW_EGU    29

///@}

/**
 * UART Character bits size
 */
typedef enum {
    RP_UART_CS6,      //!< Set 6 bits
    RP_UART_CS7,      //!< Set 7 bits
    RP_UART_CS8       //!< Set 8 bits
} rp_uart_bits_size_t;


/**
 * UART stop bits
 */
typedef enum {
    RP_UART_STOP1,      //!< Set 1 bit
    RP_UART_STOP2       //!< Set 2 bits
} rp_uart_stop_bits_t;

/**
 * UART parity mode
 */
typedef enum {
    RP_UART_NONE,      //!< Disable parity check
    RP_UART_EVEN,      //!< Set even mode for parity
    RP_UART_ODD,       //!< Set odd mode for parity
    RP_UART_MARK,      //!< Set Always 1
    RP_UART_SPACE      //!< Set Always 0
} rp_uart_parity_t;

/** @name General
 */
///@{

/**
 * Opens the UART device (/dev/ttyPS1). Initializes the default settings.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_UartInit();

/**
* Closes device UART
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_UartRelease();

/**
* Reading values into the buffer from the UART device
* @param buffer Non-zero buffer for writing data.
* @param size Buffer size. Returns the amount of data read.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_UartRead(unsigned char *buffer, int *size);

/**
* Writes data to UART
* @param buffer The buffer to be written to the UART.
* @param size Buffer size.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_UartWrite(unsigned char *buffer, int size);

/**
* Set speed for the UART.
* @param speed Value of speed
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_UartSpeed(int speed);

/**
* Set character size for the UART.
* @param size Value of size
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_UartSetBits(rp_uart_bits_size_t size);

/**
* Set stop bits size for the UART.
* @param mode Value of size
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_UartSetStopBits(rp_uart_stop_bits_t mode);

/**
* Set parity check mode for the UART.
* @param mode Value of size
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_UartSetParityMode(rp_uart_parity_t mode);


/**
* The function returns the on state of the 9 yellow LED indicator.
* @param enable return current state.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GetLEDMMCState(bool *_enable);

/**
* The function enables or disables the 9 yellow LED indicator
* @param enable Flag enabling LED.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_SetLEDMMCState(bool _enable);

/**
* The function returns the on state of the red LED indicator.
* @param enable return current state.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GetLEDHeartBeatState(bool *_enable);

/**
* The function enables or disables the red LED indicator
* @param enable Flag enabling LED.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_SetLEDHeartBeatState(bool _enable);

/**
* The function returns the status of indicators on the Ethernet connector.
* @param enable return current state.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_GetLEDEthState(bool *_state);

/**
* The function enables or disables indicators on the the Ethernet connector.
* @param enable Flag enabling LED.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rp_SetLEDEthState(bool _state);


#ifdef __cplusplus
}
#endif

#endif //__RP_HW_H
