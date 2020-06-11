#pragma once


#ifdef  __cplusplus
extern "C" {
#endif


#define RP_MAX7311_IN1 0x01
#define RP_MAX7311_IN2 0x02

#define RP_MAX7311_OUT1 0x01
#define RP_MAX7311_OUT2 0x02


// Flags for AC DC mode
#define RP_AC_MODE 0x01
#define RP_DC_MODE 0x00

#define RP_ATTENUATOR_1_1  0x01
#define RP_ATTENUATOR_1_20 0x00

#define RP_GAIN_2V  0x00
#define RP_GAIN_10V 0x01

/* If an error occurs then the value will be -1*/

int  rp_initController_C();

/* Sets the AC / DC modes for input.
Corresponds to the switches K1 and K2 on the circuit.
Where K1 corresponds to PIN_0 PIN_1 pins on the chip.
Where K2 corresponds to PIN_2 PIN_3 pins on the chip. */
int  rp_setAC_DC_C(char port,char mode);

/* Sets the attenuator modes for input.
Corresponds to the switches K3 and K4 on the circuit.
Where K3 corresponds to PIN_4 PIN_5 pins on the chip.
Where K4 corresponds to PIN_6 PIN_7 pins on the chip. */
int  rp_setAttenuator_C(char port,char mode);


/* Sets the gain modes for output.
Corresponds to the switches K5 and K6 on the circuit.
Where K5 corresponds to PIN_8 PIN_9 pins on the chip.
Where K6 corresponds to PIN_10 PIN_11 pins on the chip. */
int  rp_setGainOut_C(char port,char mode);

/* Sets the sleep time to switch the relay. The default value is below 1000 ms. */
void rp_setSleepTime_C(unsigned long time);


#ifdef  __cplusplus
}
#endif