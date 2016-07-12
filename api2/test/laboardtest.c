/**
 * $Id: $
 *
 * @file logic_generator.c
 * @brief logic generator for testing logic analizer hardware.
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 *
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "redpitaya/rp2.h"
#include "gpio.h"
#include "common.h"
#include "muxctl.h"
int main(int argc, char* argv[])
{
while(1){
	uint32_t tmp=0;
	rp_handle_uio_t pins;
	rp_handle_uio_t leds;
        rp_handle_uio_t mux;
	unsigned char fail=0;

// Initialization of API
	if (rp_GpioOpen("/dev/uio2", &pins) != RP_OK) {
		fprintf(stderr, "Red Pitaya gpio API init failed!\n");
		return -1;
	}
	if (rp_GpioOpen("/dev/uio3", &leds) != RP_OK) {
		fprintf(stderr, "Red Pitaya led API init failed!\n");
		return -1;
	}
	if (rp_MuxctlOpen("/dev/uio1", &mux) != RP_OK) {
                fprintf(stderr, "Red Pitaya mux API init failed!\n");
                return -1;
        }


	rp_GpioSetEnable(&pins, 0x0);
	rp_MuxctlSetGpio(&mux,0x0);

	unsigned int i;
	for (i=0;i<=255;i++){
		rp_GpioSetState(&pins, i);
		usleep(500);
		tmp=0;
		rp_GpioGetState(&pins,&tmp);
		printf("test %u : %u\n",i,(unsigned int)(255-tmp));
		if(255-tmp!=i){
			printf("combination: %d defective\n",i);
			fail=1;
		}
	}

	if(fail==0)
	{
		printf("OK\n");
		rp_GpioSetState(&leds, 0xf);
		usleep(1000000);
		rp_GpioSetState(&leds, 0);
	}
	else
	{
		printf("FAIL\n");
		rp_GpioSetState(&leds, 0xf0);
		usleep(1000000);
		rp_GpioSetState(&leds, 0);
	}
	usleep(10000);
	// Releasing resources
	rp_GpioClose(&leds);
	rp_GpioClose(&pins);
	rp_MuxctlClose(&mux);
	}
return 0;
}
