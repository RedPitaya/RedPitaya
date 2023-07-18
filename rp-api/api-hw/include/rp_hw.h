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
#include <stddef.h>

/** @name Error codes
 *  Various error codes returned by the API.
 */
///@{

/** Success */
#define RP_HW_OK     0
/** Bad alloc */
#define RP_HW_EAL    13
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
/** Failed SPI message not init */
#define RP_HW_ESMI   44
/** Failed index SPI message out of range */
#define RP_HW_ESMO   45
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
    RP_SPI_MODE_LISL = 0,   //!< Clock low idle level, Sample on leading edge
    RP_SPI_MODE_LIST = 1,   //!< Clock low idle level, sample on trailing edge
    RP_SPI_MODE_HISL = 2,   //!< Clock high idle level, sample on leading edge
    RP_SPI_MODE_HIST = 3    //!< Clock high idle level, sample on trailing edge
} rp_spi_mode_t;

/**
 * SPI order bit
 */

typedef enum {
    RP_SPI_ORDER_BIT_MSB = 0,    //!< MSB first
    RP_SPI_ORDER_BIT_LSB = 1      //!< LSB first (Not supported)
} rp_spi_order_bit_t;

/**
 * SPI state
 */

typedef enum {
    RP_SPI_STATE_NOT   = 0,  //!< Not ready
    RP_SPI_STATE_READY = 1   //!< Ready state bit setted (Not supported)
} rp_spi_state_t;

/**
 * SPI CS mode
 */

typedef enum {
    RP_SPI_CS_NORMAL   = 0,  //!< Work by default. Active Low state, Idle - high state
    RP_SPI_CS_HIGH     = 1   //!< Active High state, Idle - low state
} rp_spi_cs_mode_t;


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
* @param _out_value return value
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_UartGetTimeout(uint8_t *_out_value);

/**
* Reading values into the buffer from the UART device
* @param buffer Non-zero buffer for writing data.
* @param _in_out_size Buffer size. Returns the amount of data read.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_UartRead(unsigned char *buffer, int *_in_out_size);

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
* @param _out_value return value
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_UartGetSpeed(int *_out_value);

/**
* Set character size for the UART.
* @param size Value of size
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_UartSetBits(rp_uart_bits_size_t size);

/**
* Get character size of the UART.
* @param _out_value return value
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_UartGetBits(rp_uart_bits_size_t *_out_value);

/**
* Set stop bits size for the UART.
* @param mode Value of size
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_UartSetStopBits(rp_uart_stop_bits_t mode);

/**
* Get stop bits size of the UART.
* @param _out_value return value
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_UartGetStopBits(rp_uart_stop_bits_t *_out_value);

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
int rp_UartGetParityMode(rp_uart_parity_t *_out_value);


/**
* The function returns the on state of the 9 yellow LED indicator.
* @param _out_value return current state.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_GetLEDMMCState(bool *_out_value);

/**
* The function enables or disables the 9 yellow LED indicator
* @param enable Flag enabling LED.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_SetLEDMMCState(bool _enable);

/**
* The function returns the on state of the red LED indicator.
* @param _out_value return current state.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_GetLEDHeartBeatState(bool *_out_value);

/**
* The function enables or disables the red LED indicator
* @param enable Flag enabling LED.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_SetLEDHeartBeatState(bool _enable);

/**
* The function returns the status of indicators on the Ethernet connector.
* @param _out_value return current state.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_GetLEDEthState(bool *_out_value);

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
int rp_SPI_InitDevice(const char *device);

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
 * Creates a message batch in the internal buffer for SPI exchange
 * @param len Number of messages in a batch
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
 */
int rp_SPI_CreateMessage(size_t len);

/**
 * Gets the current number of messages
 * @param _out_value Return number of messages in a batch
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
 */
int rp_SPI_GetMessageLen(size_t *_out_value);

/**
 * Gets a read buffer from the specified messages
 * @param msg Index of msg. Must be less than message length
 * @param _out_buffer Pointer to the data buffer to read. If the buffer was not set in the current message, then the pointer is equal to zero
 * @param _out_len Returns the length of the buffer
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
 */
int rp_SPI_GetRxBuffer(size_t msg,const uint8_t **_out_buffer,size_t *_out_len);

/**
 * Gets a write buffer from the specified messages
 * @param msg Index of msg. Must be less than message length
 * @param _out_buffer Pointer to the data buffer to write. If the buffer was not set in the current message, then the pointer is equal to zero
 * @param _out_len Returns the length of the buffer
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
 */
int rp_SPI_GetTxBuffer(size_t msg,const uint8_t **_out_buffer,size_t *_out_len);

/**
 * Returns chip select reset states. If the value is not set, then within the transmission of the message packet, the state of the chip selection will not be reset.
 * @param msg Index of msg. Must be less than message length
 * @param _out_value Returns the chip select state
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
 */
int rp_SPI_GetCSChangeState(size_t msg,bool *_out_value);

/**
 * Sets the data for the message.
 * @param msg Index of msg. Must be less than message length
 * @param tx_buffer Buffer to send a message. If nothing is sent within this message, then the buffer should be NULL
 * @param init_rx_buffer Initializes a buffer for reading the specified length in the len parameter
 * @param len The length of the buffer for reading and writing, if a buffer for writing is specified, then the buffer for reading will be initialized with the same length.
 * @param cs_change Sets the ability to reset the chip selection between messages. Default value should be false
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
 */
int rp_SPI_SetBufferForMessage(size_t msg,const uint8_t *tx_buffer,bool init_rx_buffer,size_t len, bool cs_change);

/**
 * Deletes messages from the internal buffer
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
 */
int rp_SPI_DestoryMessage();

/**
* Get mode of the SPI.
* @param _out_value mode:
* RP_SPI_MODE_LISL = 0 - Low idle level, Sample on leading edge
* RP_SPI_MODE_LIST = 1 - Low idle level, sample on trailing edge
* RP_SPI_MODE_HISL = 2 - High idle level, sample on leading edge
* RP_SPI_MODE_HIST = 3 - High idle level, sample on trailing edge
* @return If the function is successful, the return value is RP_OK.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_SPI_GetMode(rp_spi_mode_t *_out_value);

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
* @param _out_value state value
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_SPI_GetState(rp_spi_state_t *_out_value);

/**
* Set READY state for the SPI.
* @param state state value
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_SPI_SetState(rp_spi_state_t state);

/**
* Get current settings for CS mode
* @param _out_value return mode value
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_SPI_GetCSMode(rp_spi_cs_mode_t *_out_value);

/**
* Set CS mode in settings.
* @param mode mode value
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_SPI_SetCSMode(rp_spi_cs_mode_t mode);

/**
* Get byte order of the SPI.
* @param _out_value return speed value (Hz)
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_SPI_GetOrderBit(rp_spi_order_bit_t *_out_value);

/**
* Set byte order LSB/MSB for the SPI.
* @param order Byte order mode
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_SPI_SetOrderBit(rp_spi_order_bit_t order);

/**
* Get speed of the SPI.
* @param _out_value return speed value (Hz)
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_SPI_GetSpeed(int * _out_value);

/**
* Set speed for the SPI.
* @param speed Value of speed [1...100000000]
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_SPI_SetSpeed(int speed);

/**
* Get "bit per word" of the SPI.
* @param _out_value size of word
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_SPI_GetWordLen(int * _out_value);

/**
* Set "bit per word" for the SPI.
* @param len Word size must be in [7...]
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_SPI_SetWordLen(int len);

/**
* Writes or reads data from the SPI interface.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
*/
int rp_SPI_ReadWrite();


/**
 * Set parameters for the I2C.
 * @param device Path to device. Commonly used: /dev/i2c-0.
 * @param addr Device address on the bus in range [0x03-0x77]
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
 */
int rp_I2C_InitDevice(const char *device,uint8_t addr);

/**
 * Enables forced bus operation even if the device is in use.
 * @param force Enable force mode
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
 */
int rp_I2C_setForceMode(bool force);

/**
 * Gets the current forced mode setting.
 * @param _out_value Return current mode
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
 */
int rp_I2C_getForceMode(bool *_out_value);

/**
 * Gets the current device address.
 * @param _out_value Return current device address
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
 */
int rp_I2C_getDevAddress(int * _out_value);

/**
 * Read byte from I2C. Used smbus.
 * @param reg Address of register
 * @param _out_value Returns the read value
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
 */
int rp_I2C_SMBUS_Read(uint8_t reg,uint8_t *_out_value);

/**
 * Read word from I2C. Used smbus.
 * @param reg Address of register
 * @param _out_value Returns the read value
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
 */
int rp_I2C_SMBUS_ReadWord(uint8_t reg,uint16_t *_out_value);

/**
 * Read command from I2C. Used smbus.
 * @param _out_value Returns the read value
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
 */
int rp_I2C_SMBUS_ReadCommand(uint8_t *_out_value);

/**
 * Read buffer from I2C. Used smbus.
 * @param reg Address of register
 * @param buffer Buffer pointer
 * @param _in_out_size Indicates how much data to read, and returns the number of bytes read
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
 */
int rp_I2C_SMBUS_ReadBuffer(uint8_t reg, uint8_t *buffer, int *_in_out_size);

/**
 * Write byte to I2C. Used smbus.
 * @param reg Address of register
 * @param value Value for write
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
 */
int rp_I2C_SMBUS_Write(uint8_t reg,uint8_t value);

/**
 * Write word to I2C. Used smbus.
 * @param reg Address of register
 * @param value Value for write
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
 */
int rp_I2C_SMBUS_WriteWord(uint8_t reg,uint16_t value);

/**
 * Write command to I2C. Used smbus.
 * @param value Value for write
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
 */
int rp_I2C_SMBUS_WriteCommand(uint8_t value);

/**
 * Write buffer to I2C. Used smbus.
 * @param reg Address of register
 * @param buffer Buffer pointer
 * @param len buffer length
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
 */
int rp_I2C_SMBUS_WriteBuffer(uint8_t reg, uint8_t *buffer, int len);


/**
 * Read buffer from I2C. Used IOCTL.
 * @param buffer Buffer pointer
 * @param len Indicates how much data to read
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
 */
int rp_I2C_IOCTL_ReadBuffer(uint8_t *buffer, int len);

/**
 * Write buffer to I2C. Used IOCTL.
 * @param buffer Buffer pointer
 * @param len buffer length
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
 */
int rp_I2C_IOCTL_WriteBuffer(uint8_t *buffer, int len);

/**
 * Returns the current CPU temperature
 * @return If the function is successful, the return non
 * If the function is unsuccessful, the return value is any of RP_HW_E* values that indicate an error.
 */
float rp_GetCPUTemperature(uint32_t *raw);

/**
 * Returns the value from analog input AI4. Used to test 5V line power
 * @return If the function is successful, the return non
 * If the function fails, then a non-zero value is returned.
 */
int rp_GetPowerI4(uint32_t *raw,float* value);


/**
 * Returns the value from VCCPINT(1.0V).
 * @return If the function is successful, the return non
 * If the function fails, then a non-zero value is returned.
 */
int rp_GetPowerVCCPINT(uint32_t *raw,float* value);

/**
 * Returns the value from VCCPAUX(1.8V).
 * @return If the function is successful, the return non
 * If the function fails, then a non-zero value is returned.
 */
int rp_GetPowerVCCPAUX(uint32_t *raw,float* value);

/**
 * Returns the value from VCCBRAM(1.0V).
 * @return If the function is successful, the return non
 * If the function fails, then a non-zero value is returned.
 */
int rp_GetPowerVCCBRAM(uint32_t *raw,float* value);

/**
 * Returns the value from VCCINT(1.0V).
 * @return If the function is successful, the return non
 * If the function fails, then a non-zero value is returned.
 */
int rp_GetPowerVCCINT(uint32_t *raw,float* value);

/**
 * Returns the value from VCCAUX(1.8V).
 * @return If the function is successful, the return non
 * If the function fails, then a non-zero value is returned.
 */
int rp_GetPowerVCCAUX(uint32_t *raw,float* value);

/**
 * Returns the value from VCCDDR(1.5V).
 * @return If the function is successful, the return non
 * If the function fails, then a non-zero value is returned.
 */
int rp_GetPowerVCCDDR(uint32_t *raw,float* value);


#ifdef __cplusplus
}
#endif

#endif //__RP_HW_H
