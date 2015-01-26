/* Set analog voltage on slow analog output */

#include <stdio.h>
#include <stdlib.h>

#include <rp.h>

int main(int argv, char **argc){

	/* Print error, if rp_Init() function failed */
	if(rp_Init() != RP_OK){
		fprintf(stderr, "Rp api init failed!\n");
	}
	float value = 1.34;

	int status = rp_ApinSetValue(RP_AOUT2, value);

	if(status != RP_OK){
		printf("Could not set pin voltage.\n");
	}

	rp_Release();
}