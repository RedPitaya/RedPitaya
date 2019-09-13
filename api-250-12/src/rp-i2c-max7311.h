#pragma once


#ifdef  __cplusplus
extern "C" {
#endif

// I2C Address of device
// MCP4706, MCP4716 & MCP4726 are factory programed for any of 0x60 thru 0x67
//  commonly 0x60
#define MAX7311_DEFAULT_ADDRESS	0x20
#define MAX7311_DEFAULT_DEV     "/dev/i2c-0"

namespace max7311{

#define PIN_0  0x0001
#define PIN_1  0x0002
#define PIN_2  0x0004
#define PIN_3  0x0008
#define PIN_4  0x0010
#define PIN_5  0x0020
#define PIN_6  0x0040
#define PIN_7  0x0080

#define PIN_8  0x0100
#define PIN_9  0x0200
#define PIN_10 0x0400
#define PIN_11 0x0800
#define PIN_12 0x1000
#define PIN_13 0x2000
#define PIN_14 0x4000
#define PIN_15 0x8000

    int  initController(const char *i2c_dev_path,  char address);
    int  initControllerDefault();

    /*Sets the output current value.
    If an error occurs then the value will be -1*/
    int  setPIN(unsigned short pin,bool state);
    
    /*Returns the status value of the port.
    If an error occurs then the value will be -1*/
    int  getPIN(unsigned short pin);

    /*Sets the output current value.
    If an error occurs then the value will be -1*/
    int  setPIN_EX(const char *i2c_dev_path,  char address, unsigned short pin,bool state);
    
    /*Returns the status value of the port.
    If an error occurs then the value will be -1*/
    int  getPIN_EX(const char *i2c_dev_path,  char address, unsigned short pin);
}

namespace rp_max7311{

#define RP_MAX7311_IN1 0x01
#define RP_MAX7311_IN2 0x02

#define RP_MAX7311_OUT1 0x01
#define RP_MAX7311_OUT2 0x02


// Flags for AC DC mode
#define RP_AC_MODE 0x00
#define RP_DC_MODE 0x01

#define RP_ATTENUATOR_1_1  0x00
#define RP_ATTENUATOR_1_20 0x01

#define RP_GAIN_2V  0x00
#define RP_GAIN_10V 0x01

/* If an error occurs then the value will be -1*/
    
    int  rp_initController();

    /* Sets the AC / DC modes for input.
    Corresponds to the switches K1 and K2 on the circuit.
    Where K1 corresponds to PIN_0 PIN_1 pins on the chip.
    Where K2 corresponds to PIN_2 PIN_3 pins on the chip. */
    int  rp_setAC_DC(char port,char mode);
    int  rp_getAC_DC(char port);

    /* Sets the attenuator modes for input.
    Corresponds to the switches K3 and K4 on the circuit.
    Where K3 corresponds to PIN_4 PIN_5 pins on the chip.
    Where K4 corresponds to PIN_6 PIN_7 pins on the chip. */
    int  rp_setAttenuator(char port,char mode);
    int  rp_getAttenuator(char port);


    /* Sets the gain modes for output.
    Corresponds to the switches K5 and K6 on the circuit.
    Where K5 corresponds to PIN_8 PIN_9 pins on the chip.
    Where K6 corresponds to PIN_10 PIN_11 pins on the chip. */
    int  rp_setGainOut(char port,char mode);
    int  rp_getGainOut(char port);

}

#ifdef  __cplusplus
}
#endif