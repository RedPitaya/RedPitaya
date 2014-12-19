
#include <stdio.h>
#include <unistd.h>
#include <math.h>

#include "../src/rp.h"

int main(int argc, char **argv) {

	int result;

	printf("APP START\n");

	printf("Library version: %s\n", rp_GetVersion());

	result = rp_Init();
	printf("Initializing library: %s\n", rp_GetError(result));



    printf("\n---------Testing AMS---------\n");

    float val1, min, max;
    uint32_t val;


    result = rp_ApinSetValueRaw(RP_AOUT0, 0x9c);
    printf("Writing %s\n.", rp_GetError(result));
    result = rp_ApinSetValueRaw(RP_AOUT1, 0x57);
    printf("Writing %s\n.", rp_GetError(result));

    result = rp_ApinGetValueRaw(RP_AOUT0, &val);
    printf("Reading %s. State on pin: %d is %x. Test: %s\n", rp_GetError(result), RP_AIN0, val, val==0x9c ? "OK" : "FAIL");
    result = rp_ApinGetValueRaw(RP_AOUT1, &val);
    printf("Reading %s. State on pin: %d is %x. Test: %s\n", rp_GetError(result), RP_AIN1, val, val==0x57 ? "OK" : "FAIL");

    result = rp_ApinSetValue(RP_AOUT0, 1.8);
    printf("Writing %s\n.", rp_GetError(result));
    result = rp_ApinSetValueRaw(RP_AOUT1, 1);
    printf("Writing %s\n.", rp_GetError(result));

    result = rp_ApinGetValue(RP_AOUT0, &val1);
    printf("Reading %s. State on pin: %d is %.3f. Test: %s\n", rp_GetError(result), RP_AIN0, val1, fabs(val1-1.8) < 0.05 ? "OK" : "FAIL");
    result = rp_ApinGetValue(RP_AOUT1, &val1);
    printf("Reading %s. State on pin: %d is %.3f. Test: %s\n", rp_GetError(result), RP_AIN1, val1, fabs(val1-1.0) < 0.05 ? "OK" : "FAIL");

    result = rp_ApinGetRange(RP_AIN2, &min, &max);
    printf("GetRange%s. Range on pin: %d is: min:%.2f  max:%.2f  Test: %s\n", rp_GetError(result), RP_AIN1, min, max, fabs(min)< 0.05 && fabs(max-3.5)< 0.05  ? "OK" : "FAIL");
    result = rp_ApinGetRange(RP_AOUT3, &min, &max);
    printf("GetRange%s. Range on pin: %d is: min:%.2f  max:%.2f  Test: %s\n", rp_GetError(result), RP_AIN1, min, max, fabs(min)< 0.05 && fabs(max-1.8)< 0.05  ? "OK" : "FAIL");


    printf("\n---------Testing LEDs---------\n");

	rp_dpin_t pin = RP_LED1;

	// Try to set LED direction to input (THIS IS INVALID!)
	result = rp_DpinSetDirection(pin, RP_IN);
	printf("PIN1 direction to input: %s\n", rp_GetError(result));

    // Go 10 times through all LEDS
    int i = 7*10;
	while (i--) {

		result = rp_DpinSetState(pin, RP_LOW);
		printf("Close LED %d: %s\n", pin, rp_GetError(result));

		pin++;
		if (pin > RP_LED7) {
			pin = RP_LED1;
		}
		rp_DpinSetState(pin, RP_HIGH);
		printf("Open LED %d: %s\n", pin, rp_GetError(result));
		usleep(100 * 1000);

	}
    rp_DpinSetState(pin, RP_LOW);




    printf("\n-------Testing digital output-------\n");
    
    // Sets digital pins HIGH and LOW in order DIO7_N - DIO0_N, DIO7_P - DIO0_P.
    // LEDs are indicating which IO is in use. If the top LED is on then the top DIO is in use.
    // For example if LED 3 is on then DIO4_N or DIO4_P is in use.
    pin = RP_DIO7_N;
    while ((pin--) >= RP_DIO0_P) {
        rp_DpinSetState(RP_LED0 + (7-pin % 8), RP_HIGH);                // LED on
        result = rp_DpinSetDirection(pin, RP_OUT);
        printf("Set OUTPUT %d: %s\n", pin, rp_GetError(result));

        result = rp_DpinSetState(pin, RP_HIGH);
        printf("Write HIGH %d: %s\n", pin, rp_GetError(result));
        usleep(3000 * 1000);
        
        result = rp_DpinSetState(pin, RP_LOW);
        printf("Write LOW %d:  %s\n", pin, rp_GetError(result));
        usleep(3000 * 1000);
        rp_DpinSetState(RP_LED0 + (7-pin % 8), RP_LOW);                 // LED off
    }

    printf("\n----------Testing digital input----------\n");
    
    // Reads from digital pins in order DIO7_N - DIO0_N, DIO7_P - DIO0_P.
    // LEDs are indicating which IO is in use. If the top LED is on then the top DIO is in use.
    // For example if LED 3 is on then DIO4_N or DIO4_P is in use.
    rp_pinState_t state;
    pin = RP_DIO7_N;
    while ((pin--) >= RP_DIO0_P) {
        rp_DpinSetState(RP_LED0 + (7-pin % 8), RP_HIGH);                // LED on
        result = rp_DpinSetDirection(pin, RP_IN);
        printf("Set OUTPUT %d: %s\n", pin, rp_GetError(result));

        usleep(3000 * 1000);
        result = rp_DpinGetState(pin, &state);
        printf("Reading %s. State on pin: %d is %s\n", rp_GetError(result), pin, state == RP_HIGH ? "HIGH" : "LOW");
        rp_DpinSetState(RP_LED0 + (7-pin % 8), RP_LOW);                 // LED off
    }

	result = rp_Release();
	printf("Releasing library: %s\n", rp_GetError(result));
    return 0;
}
