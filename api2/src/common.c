

#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

int FpgaRegDump(uint32_t a_addr, uint32_t * a_data, uint32_t a_len){
	uint32_t addr=a_addr;
	printf("\n\r fpga reg. dump \n\r");
	printf("\n\r index, addr, value\n\r");
	for(int i=0; i<a_len; i++){
		addr+=0x4;
		printf("0x%04d,0x%08x,0x%08x\n\r",i,addr,a_data[i]);
	}
	return RP_OK;
}
