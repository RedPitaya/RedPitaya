
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>

#include "../src/rp.h"



const char HEALTH_OIN_NAME[11][20]={
        "Tempereature",
        "VCCPINT(1V0)",
        "VCCPAUX(1V8)",
        "VCCBRAM(1V0)",
        "VCCINT(1V0)",
        "VCCAUX(1V8)",
        "VCCDDR(1V5)",
        "AO0(0-1.8V)",
        "AO1(0-1.8V)",
        "AO2(0-1.8V)",
        "AO3(0-1.8V)",
};

int main(int argc, char **argv) {

    int result;

    printf("APP START\n");

    printf("Library version: %s\n", rp_GetVersion());

    result = rp_Init();
    printf("Initializing library: %s\n", rp_GetError(result));

    result = rp_AcqSetTriggerLevel (-0.41);
    printf("Set CHA threshold: %s\n", rp_GetError(result));

    float thr;
    result = rp_AcqGetTriggerLevel(&thr);
    printf("Get CHA threshold: %s. Result = %f\n", rp_GetError(result), thr);

    uint32_t pos1;
    result = rp_AcqGetWritePointer (&pos1);
    printf("Get Write Pointer: %s. Result = %d\n", rp_GetError(result), pos1);

    result = rp_AcqStart();
    printf("Acquire start: %s\n", rp_GetError(result));

    usleep(1000 * 1000);



    result = rp_AcqStop();
    printf("Acquire stop: %s\n", rp_GetError(result));

    result = rp_AcqSetTriggerSrc(RP_TRIG_SRC_NOW);
    printf("Acquire Trigger NOW: %s\n", rp_GetError(result));

    uint32_t pos2;
    result = rp_AcqGetWritePointer (&pos2);
    printf("Get Write Pointer: %s. Result = %d\n", rp_GetError(result), pos2);

    usleep(100 * 1000);

    result = rp_AcqGetWritePointer (&pos2);
    printf("Get Write Pointer: %s. Result = %d\n", rp_GetError(result), pos2);

    usleep(100 * 1000);

    result = rp_AcqGetWritePointer (&pos2);
    printf("Get Write Pointer: %s. Result = %d\n", rp_GetError(result), pos2);

    uint32_t size = 20;
    float* buffer = malloc(20 * sizeof(float));
    result = rp_AcqGetLatestDataV (RP_CH_A, &size, buffer);
    printf("Retrieved buffer: %s\n", rp_GetError(result));

    for (int k = 0; k < 20; ++k) {
        printf("%f\n", buffer[k]);
    }

    result = rp_Release();
    printf("Releasing library: %s\n", rp_GetError(result));
    return 0;
}



/*
int main(int argc, char **argv) {

	int result;

	printf("APP START\n");

	printf("Library version: %s\n", rp_GetVersion());

	result = rp_Init();
	printf("Initializing library: %s\n", rp_GetError(result));

    printf("\n---------Testing HEALTH---------\n");
    printf("%s\t%s\t\t%s\n", "#ID", "Desc", "Val");
    float valFloat;
    int i;
    for (i = RP_TEMP_FPGA; i <= RP_VCC_DDR; i++)
    {
        result = rp_HealthGetValue((rp_health_t) i, &valFloat);
        if (result == RP_OK) {
            printf("%d\t%s\t%.3f\n",i, &HEALTH_OIN_NAME[i][0], valFloat);
        } else {
            printf("%d\t%s\tError:%s\n",i, &HEALTH_OIN_NAME[i][0], rp_GetError(result));
        }
    }



    printf("\n---------Testing AMS---------\n");
    uint32_t valInt;
    float min, max;
    result = rp_ApinSetValueRaw(RP_AOUT0, 0x9c);
    printf("Writing Raw %s.\n", rp_GetError(result));
    result = rp_ApinSetValueRaw(RP_AOUT1, 0x57);
    printf("Writing Raw %s.\n", rp_GetError(result));

    result = rp_ApinGetValueRaw(RP_AOUT0, &valInt);
    printf("Reading Raw %s. State on pin: %d is %x. Test: %s.\n", rp_GetError(result), RP_AOUT0, valInt, valInt==0x9c ? "OK" : "FAIL");
    result = rp_ApinGetValueRaw(RP_AOUT1, &valInt);
    printf("Reading Raw %s. State on pin: %d is %x. Test: %s.\n", rp_GetError(result), RP_AOUT1, valInt, valInt==0x57 ? "OK" : "FAIL");

    result = rp_ApinSetValue(RP_AOUT2, 0.3);
    printf("Writing Volts %s.\n", rp_GetError(result));
    result = rp_ApinSetValue(RP_AOUT3, 1.1);
    printf("Writing Volts %s.\n", rp_GetError(result));

    result = rp_ApinGetValue(RP_AOUT2, &valFloat);
    printf("Reading Volts %s. State on pin: %d is %.3f. Test: %s.\n", rp_GetError(result), RP_AOUT2, valFloat, fabs(valFloat-0.3) < 0.1 ? "OK" : "FAIL");
    result = rp_ApinGetValue(RP_AOUT3, &valFloat);
    printf("Reading Volts %s. State on pin: %d is %.3f. Test: %s.\n", rp_GetError(result), RP_AOUT3, valFloat, fabs(valFloat-1.1) < 0.1 ? "OK" : "FAIL");

    result = rp_ApinGetRange(RP_AIN2, &min, &max);
    printf("GetRange%s. Range on pin: %d is: min:%.2f  max:%.2f  Test: %s.\n", rp_GetError(result), RP_AIN1, min, max, fabs(min)< 0.05 && fabs(max-3.5)< 0.1  ? "OK" : "FAIL");
    result = rp_ApinGetRange(RP_AOUT3, &min, &max);
    printf("GetRange%s. Range on pin: %d is: min:%.2f  max:%.2f  Test: %s.\n", rp_GetError(result), RP_AIN1, min, max, fabs(min)< 0.05 && fabs(max-1.8)< 0.1  ? "OK" : "FAIL");


    printf("\n---------Testing LEDs---------\n");

	rp_dpin_t pin = RP_LED1;

	// Try to set LED direction to input (THIS IS INVALID!)
	result = rp_DpinSetDirection(pin, RP_IN);
	printf("PIN1 direction to input: %s\n", rp_GetError(result));

    // Go 10 times through all LEDS
    i = 7*10;
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
*/
