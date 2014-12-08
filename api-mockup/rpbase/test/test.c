
#include <stdio.h>
#include <unistd.h>


#include "../src/rp.h"

int main(int argc, char **argv) {

	int result;

	printf("APP START\n");

	printf("Library version: %s\n", rp_GetVersion());

	result = rp_Init();
	printf("Initializing library: %s\n", rp_GetError(result));

	rp_dpin_t pin = RP_LED1;

	// Try to set LED direction to input (THIS IS INVALID!)
	result = rp_DpinSetDirection(pin, RP_IN);
	printf("PIN1 direction to input: %s\n", rp_GetError(result));

	while (1) {

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

	result = rp_Release();
	printf("Releasing library: %s\n", rp_GetError(result));

}
