/* This is a red pitaya test application for deep averagning */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>

#include "../../api-mockup/rpbase/src/rp.h"

int main(int argc, char **argv){

	uint32_t buff_size = 16 * 1024;
	printf("Starting RP API\n");
	/* Print error, if rp_Init() function failed */

	if(rp_Init() != RP_OK){
		fprintf(stderr, "Rp api init failed!\n");
	}
	printf("successful start API\n");
	int16_t *cha_buff = (int16_t *)malloc(buff_size * sizeof(int16_t));
	printf("Init deep avg functions.\n");
	rp_DeepAvgStart();
	printf("Init Deep avg counter\n");
	rp_SetDeepAvgCount(10000);
	printf("Init deep avg shift\n");
	rp_SetDeepAvgShift(3);
	printf("Acquire data.\n");
	rp_SetDeepDataSeqLen(100);
	printf("Ret val = %d\n", rp_GetDeepAvgRawData(RP_CH_1, &buff_size, cha_buff));
	printf("Data acquisiton successful!\n");
	int counter = 0;
	uint32_t *deb_time = malloc(100 * sizeof(float));
	for (uint32_t i = 0; i < 10; ++i)
	{
		counter++;
		printf("Inc counter\n");
		rp_GetDeepAvgDebTim(deb_time);
		printf("Getting deb timer\n");
		printf("%d\n", (int)deb_time);
		printf("%f\n", (float)cha_buff[i]);
	}
	printf("%d\n", counter);
	rp_Release();
	free(cha_buff);
	printf("I am the muffin master.\n");

}