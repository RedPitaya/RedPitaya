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
/** Timeout read from uart */
#define RP_HW_EUTO   14
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
/** Failed to init SPI */
#define RP_HW_EIS    40
/** Failed get settings from SPI */
#define RP_HW_ESGS   41
/** Failed set settings to SPI */
#define RP_HW_ESSS   42
/** Failed SPI read/write */
#define RP_HW_EST    43
/** Failed to init I2C */
#define RP_HW_EIIIC  60
/** Failed to read from I2C */
#define RP_HW_ERIIC  61
/** Failed to write to I2C */
#define RP_HW_EWIIC  62
/** Failed to set slave mode for I2C */
#define RP_HW_ESIIC  63
/** Failed I2C. Buffer is NULL */
#define RP_HW_EBIIC  64

///@}

/**
 * UART Character bits size
 */
typedef enum {
    RP_UART_CS6 = 0,   //!< Set 6 bits (Not supported)
    RP_UART_CS7 = 1,   //!< Set 7 bits
    RP_UART_CS8 = 2    //!< Set 8 bits
} rp_uart_bits_size_t;


/**
 * UART stop bits
 */
typedef enum {
    RP_UART_STOP1 = 0,  //!< Set 1 bit
    RP_UART_STOP2 = 1   //!< Set 2 bits
} rp_uart_stop_bits_t;

/**
 * UART parity mode
 */
typedef enum {
    RP_UART_NONE = 0,      //!< Disable parity check
    RP_UART_EVEN = 1,      //!< Set even mode for parity
    RP_UART_ODD  = 2,      //!< Set odd mode for parity
    RP_UART_MARK = 3,      //!< Set Always 1
    RP_UART_SPACE  = 4     //!< Set Always 0
} rp_uart_parity_t;


/**
 * SPI mode
 */

typedef enum {
    RP_SPI_MODE_LISL = 0,   //!< Low idle level, Sample on leading edge
    RP_SPI_MODE_LIST = 1,   //!< Low idle level, sample on trailing edge
    RP_SPI_MODE_HISL = 2,   //!< High idle level, sample on leading edge
    RP_SPI_MODE_HIST = 3    //!< High idle level, sample on trailing edge
} rp_spi_mode_t;

/**
 * SPI order bit
 */

typedef enum {
    RP_SPI_ORDER_BIT_MSB = 0,    //!< MSB first
    RP_SPI_ORDER_BIT_LSB =1      //!< LSB first
} rp_spi_order_bit_t;

/**
 * SPI state
 */

typedef enum {
    RP_SPI_STATE_NOT   = 0,  //!< Not ready
    RP_SPI_STATE_READY = 1   //!< Ready state bit setted
} rp_spi_state_t;

/** @name General
 */
///@{

/**
 * Opens the UART device (/dev/ttyPS1). Initializes the default settings.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
 */
int rp_UartInit();

/**
* Closes device UART
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_UartRelease();

/**
* Apply current settings to UART
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_UartSetSettings();

/**
* Set timeout for read from uart
* @param deca_sec Time in 1/10 second. Zero value disable timeout.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_UartSetTimeout(uint8_t deca_sec);

/**
* Get timeout value.
* @param value return value
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_UartGetTimeout(uint8_t *value);

/**
* Reading values into the buffer from the UART device
* @param buffer Non-zero buffer for writing data.
* @param size Buffer size. Returns the amount of data read.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_UartRead(unsigned char *buffer, int *size);

/**
* Writes data to UART
* @param buffer The buffer to be written to the UART.
* @param size Buffer size.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_UartWrite(unsigned char *buffer, int size);

/**
* Set speed for the UART.
* @param speed Value of speed
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_UartSetSpeed(int speed);

/**
* Get current setted speed.
* @param value return value
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_UartGetSpeed(int *value);

/**
* Set character size for the UART.
* @param size Value of size
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_UartSetBits(rp_uart_bits_size_t size);

/**
* Get character size of the UART.
* @param value return value
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_UartGetBits(rp_uart_bits_size_t *value);

/**
* Set stop bits size for the UART.
* @param mode Value of size
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_UartSetStopBits(rp_uart_stop_bits_t mode);

/**
* Get stop bits size of the UART.
* @param value return value
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_UartGetStopBits(rp_uart_stop_bits_t *value);

/**
* Set parity check mode for the UART.
* @param mode Value of size
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_UartSetParityMode(rp_uart_parity_t mode);

/**
* Get parity check mode of the UART.
* @param value return value
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_UartGetParityMode(rp_uart_parity_t *value);


/**
* The function returns the on state of the 9 yellow LED indicator.
* @param enable return current state.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_GetLEDMMCState(bool *_enable);

/**
* The function enables or disables the 9 yellow LED indicator
* @param enable Flag enabling LED.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_SetLEDMMCState(bool _enable);

/**
* The function returns the on state of the red LED indicator.
* @param enable return current state.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_GetLEDHeartBeatState(bool *_enable);

/**
* The function enables or disables the red LED indicator
* @param enable Flag enabling LED.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_SetLEDHeartBeatState(bool _enable);

/**
* The function returns the status of indicators on the Ethernet connector.
* @param enable return current state.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_GetLEDEthState(bool *_state);

/**
* The function enables or disables indicators on the the Ethernet connector.
* @param enable Flag enabling LED.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_SetLEDEthState(bool _state);

/**
 * Open the SPI device (/dev/spidev1.0).
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
 */
int rp_SPI_Init();

/**
 * Open the specified SPI device.
 * @param device Path to device.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
 */
int rp_SPI_InitDevice(char *device);

/**
 * Sets the default values for an open SPI device.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
 */
int rp_SPI_SetDefaultSettings();

/**
 * Read settings from SPI device.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
 */
int rp_SPI_GetSettings();

/**
 * Write settings from SPI device.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
 */
int rp_SPI_SetSettings();

/**
* Closes spi device 
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_SPI_Release();

/**
* Get mode of the SPI.
* @param mode mode:
* RP_SPI_MODE_LISL = 0 - Low idle level, Sample on leading edge
* RP_SPI_MODE_LIST = 1 - Low idle level, sample on trailing edge
* RP_SPI_MODE_HISL = 2 - High idle level, sample on leading edge
* RP_SPI_MODE_HIST = 3 - High idle level, sample on trailing edge
* @return If the function is successful, the return value is RP_OK.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_SPI_GetMode(rp_spi_mode_t *mode);

/**
* Set mode for the SPI.
* @param mode mode:
* RP_SPI_MODE_LISL = 0 - Low idle level, Sample on leading edge
* RP_SPI_MODE_LIST = 1 - Low idle level, sample on trailing edge
* RP_SPI_MODE_HISL = 2 - High idle level, sample on leading edge
* RP_SPI_MODE_HIST = 3 - High idle level, sample on trailing edge
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_SPI_SetMode(rp_spi_mode_t mode);

/**
* Get READY state of the SPI.
* @param state state value
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_SPI_GetState(rp_spi_state_t *state);

/**
* Set READY state for the SPI.
* @param state state value
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_SPI_SetState(rp_spi_state_t state);

/**
* Get byte order of the SPI.
* @param order return speed value (Hz)
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_SPI_GetOrderBit(rp_spi_order_bit_t *order);

/**
* Set byte order LSB/MSB for the SPI.
* @param order Byte order mode
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_SPI_SetOrderBit(rp_spi_order_bit_t order);

/**
* Get speed of the SPI.
* @param speed return speed value (Hz)
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_SPI_GetSpeed(int *speed);

/**
* Set speed for the SPI.
* @param speed Value of speed [1...100000000]
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_SPI_SetSpeed(int speed);

/**
* Get "bit per word" of the SPI.
* @param len size of word
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_SPI_GetWordLen(int *len);

/**
* Set "bit per word" for the SPI.
* @param len Word size must be in [7...]
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_SPI_SetWordLen(int len);

/**
* Writes or reads data from the SPI interface.
* @param tx_buffer Buffer for sending data. Can be NULL value
* @param rx_buffer Buffer for receiving data. Can be NULL value
* @param length Buffers length. (RX and TX buffers must be the same length)
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_SPI_ReadWrite(void *tx_buffer, void *rx_buffer, unsigned int length);


/**
 * Set parameters for the I2C.
 * @param device Path to device. Commonly used: /dev/i2c-0.
 * @param addr Device address on the bus in range [0x03-0x77]
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
 */
int rp_I2C_InitDevice(char *device,uint8_t addr);

/**
 * Enables forced bus operation even if the device is in use.
 * @param force Enable force mode
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
 */
int rp_I2C_setForceMode(bool force);

/**
 * Gets the current forced mode setting.
 * @param value Return current mode
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
 */
int rp_I2C_getForceMode(bool *value);

/**
 * Gets the current device address.
 * @param value Return current device address
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
 */
int rp_I2C_getDevAddress(int * address);

/**
 * Read byte from I2C. Used smbus.
 * @param reg Address of register
 * @param value Returns the read value
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
 */
int rp_I2C_Read(uint8_t reg,uint8_t *value);

/**
 * Read word from I2C. Used smbus.
 * @param reg Address of register
 * @param value Returns the read value
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
 */
int rp_I2C_ReadWord(uint8_t reg,uint16_t *value);

/**
 * Read command from I2C. Used smbus.
 * @param value Returns the read value
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
 */
int rp_I2C_ReadCommand(uint8_t *value);

/**
 * Read buffer from I2C. Used smbus.
 * @param reg Address of register
 * @param buffer Buffer pointer
 * @param len Indicates how much data to read, and returns the number of bytes read
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
 */
int rp_I2C_ReadBuffer(uint8_t reg, uint8_t *buffer, int *len);

/**
 * Write byte to I2C. Used smbus.
 * @param reg Address of register
 * @param value Value for write
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
 */
int rp_I2C_Write(uint8_t reg,uint8_t value);

/**
 * Write word to I2C. Used smbus.
 * @param reg Address of register
 * @param value Value for write
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
 */
int rp_I2C_WriteWord(uint8_t reg,uint16_t value);

/**
 * Write command to I2C. Used smbus.
 * @param value Value for write
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
 */
int rp_I2C_WriteCommand(uint8_t value);

/**
 * Write buffer to I2C. Used smbus.
 * @param reg Address of register
 * @param buffer Buffer pointer
 * @param len buffer length
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
 */
int rp_I2C_WriteBuffer(uint8_t reg, uint8_t *buffer, int len);

#ifdef __cplusplus
}
#endif

#endif //__RP_HW_H
