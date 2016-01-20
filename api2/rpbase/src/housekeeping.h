/**
 * $Id: $
 *
 * @brief Red Pitaya library housekeeping module interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */



#ifndef __HOUSEKEEPING_H
#define __HOUSEKEEPING_H

#include <stdint.h>
#include <stdbool.h>

// Base Housekeeping address
static const int HOUSEKEEPING_BASE_SIZE = 0x30;

static const uint32_t LED_CONTROL_MASK = 0xFF;
static const uint32_t DIGITAL_LOOP_MASK = 0x1;
static const uint32_t GPIO_MASK = 0xFF;

// Housekeeping structure declaration
typedef struct {
    uint32_t id;            // 0x00
    uint32_t dna_lo;        // 0x04
    uint32_t dna_hi;        // 0x08
    uint32_t digital_loop;  // 0x0c
    uint32_t ex_cd_p;       // 0x10
    uint32_t ex_cd_n;       // 0x14
    uint32_t ex_co_p;       // 0x18
    uint32_t ex_co_n;       // 0x1c
    uint32_t ex_ci_p;       // 0x20
    uint32_t ex_ci_n;       // 0x24
    uint32_t reserved_2;    // 0x28
    uint32_t reserved_3;    // 0x2c
    uint32_t led_control;   // 0x30
} housekeeping_regset_t;

/**
 * Type representing digital input output pins.
 */
typedef enum {
    RP_LED0,       //!< LED 0
    RP_LED1,       //!< LED 1
    RP_LED2,       //!< LED 2
    RP_LED3,       //!< LED 3
    RP_LED4,       //!< LED 4
    RP_LED5,       //!< LED 5
    RP_LED6,       //!< LED 6
    RP_LED7,       //!< LED 7
    RP_DIO0_P,     //!< DIO_P 0
    RP_DIO1_P,     //!< DIO_P 1
    RP_DIO2_P,     //!< DIO_P 2
    RP_DIO3_P,     //!< DIO_P 3
    RP_DIO4_P,     //!< DIO_P 4
    RP_DIO5_P,     //!< DIO_P 5
    RP_DIO6_P,     //!< DIO_P 6
    RP_DIO7_P,	   //!< DIO_P 7
    RP_DIO0_N,     //!< DIO_N 0
    RP_DIO1_N,     //!< DIO_N 1
    RP_DIO2_N,     //!< DIO_N 2
    RP_DIO3_N,     //!< DIO_N 3
    RP_DIO4_N,     //!< DIO_N 4
    RP_DIO5_N,     //!< DIO_N 5
    RP_DIO6_N,     //!< DIO_N 6
    RP_DIO7_N      //!< DIO_N 7
} rp_dpin_t;

/**
 * Type representing pin's high or low state (on/off).
 */
typedef enum {
    RP_LOW, //!< Low state
    RP_HIGH //!< High state
} rp_pinState_t;

/**
 * Type representing pin's input or output direction.
 */
typedef enum {
    RP_IN, //!< Input direction
    RP_OUT //!< Output direction
} rp_pinDirection_t;


int rp_HousekeepingInit(char *dev, rp_handle_uio_t *handle);
int rp_HousekeepingRelease(rp_handle_uio_t *handle);

/**
* Enable or disables digital loop. This internally connect output to input
* @param enable True if you want to enable this feature or false if you want to disable it
* Each rp_GetCalibrationSettings call returns the same cached setting values.
* @return Calibration settings
*/
int rp_EnableDigitalLoop(rp_handle_uio_t *handle, bool enable);


/**
* Gets FPGA Synthesized ID
*/
int rp_IdGetID(rp_handle_uio_t *handle, uint32_t *id);

/**
* Gets FPGA Unique DNA
*/
int rp_IdGetDNA(rp_handle_uio_t *handle, uint64_t *dna);

/**
 * LED methods
 */

int rp_LEDSetState(rp_handle_uio_t *handle, uint32_t state);
int rp_LEDGetState(rp_handle_uio_t *handle, uint32_t *state);

/**
 * GPIO methods
 */

int rp_GPIOnSetDirection(rp_handle_uio_t *handle, uint32_t  direction);
int rp_GPIOnGetDirection(rp_handle_uio_t *handle, uint32_t *direction);
int rp_GPIOnSetState    (rp_handle_uio_t *handle, uint32_t  state);
int rp_GPIOnGetState    (rp_handle_uio_t *handle, uint32_t *state);
int rp_GPIOpSetDirection(rp_handle_uio_t *handle, uint32_t  direction);
int rp_GPIOpGetDirection(rp_handle_uio_t *handle, uint32_t *direction);
int rp_GPIOpSetState    (rp_handle_uio_t *handle, uint32_t  state);
int rp_GPIOpGetState    (rp_handle_uio_t *handle, uint32_t *state);


/** @name Digital Input/Output
 */
///@{

/**
* Sets digital pins to default values. Pins DIO1_P - DIO7_P, RP_DIO0_N - RP_DIO7_N are set al OUTPUT and to LOW. LEDs are set to LOW/OFF
*/
int rp_DpinReset(rp_handle_uio_t *handle);

/**
 * Sets digital input output pin state.
 * @param pin    Digital input output pin.
 * @param state  High/Low state that will be set at the given pin.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_DpinSetState(rp_handle_uio_t *handle, rp_dpin_t pin, rp_pinState_t state);
int rp_DpinGetState(rp_handle_uio_t *handle, rp_dpin_t pin, rp_pinState_t* state);

/**
 * Sets digital input output pin direction. LED pins are already automatically set to the output direction,
 * and they cannot be set to the input direction. DIOx_P and DIOx_N are must set either output or input direction
 * before they can be used. When set to input direction, it is not allowed to write into these pins.
 * @param pin        Digital input output pin.
 * @param direction  In/Out direction that will be set at the given pin.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rp_DpinSetDirection(rp_handle_uio_t *handle, rp_dpin_t pin, rp_pinDirection_t direction);
int rp_DpinGetDirection(rp_handle_uio_t *handle, rp_dpin_t pin, rp_pinDirection_t* direction);

#endif //__HOUSEKEEPING_H
