/** @file health.c
 *
 * $Id: main.c 881 2013-12-16 05:37:34Z rp_jmenart $
 *
 * @brief Red Pitaya Main module.
 * @author Luka Golinar <luka.golinar@gmail.com>
 * @copyright Red Pitaya  http://www.redpitaya.com
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <sys/mman.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <unistd.h>
 #include <fcntl.h>
 #include <math.h>

 #include "house_kp.h"

 /* Global variables */
 house_t *house_reg = NULL;
 int house_fd = -1;

 int house_init(void){

 	if(house_release() < -1){
 		printf("Mapping already mapped resources.\n");
 		return -1;
 	}

 	void *page_ptr;
 	long page_addr, page_off, page_size = sysconf(_SC_PAGESIZE);

 	/* Open device */
 	house_fd = open("/dev/mem", O_RDWR | O_SYNC);
 	if(house_fd < -1){
 		printf("Error opening /dev/mem.\n");
 		return -1;
 	}

 	/* Calculate correct page address */
 	page_addr = HK_BASE_ADDR & (~(page_size-1));
 	page_off = HK_BASE_ADDR - page_addr;

 	/* Mmap physical address into logical */
 	page_ptr = mmap(NULL, HK_BASE_SIZE, PROT_READ |
 		PROT_WRITE, MAP_SHARED, house_fd, page_addr);


 	if((void *)page_ptr == MAP_FAILED){
 		printf("Error mmaping physical structure.\n");
 		/* CALL CLEANUP */
 		return -1;
 	}

 	house_reg = page_ptr + page_off;

 	return 0;
 }


 int house_release(void){

 	if(house_reg){
 		if(munmap(house_reg, HK_BASE_SIZE) < 0){
 			printf("Error unmaping.\n");
 			return -1;
 		}
 		house_reg = NULL;
 	}

 	if(house_fd >= 1){
 		close(house_fd);
 		house_fd = -1;
 	}

 	return 0;
}

int set_led(int led){

	house_reg->led_control = 0;

	uint32_t reg_val = 0;
	switch(led){
		case 1:
			reg_val = 2;
			break;
		case 2:
			reg_val = 4;
			break;
		case 3:
			reg_val = 8;
			break;
		case 4:
			reg_val = 16;
			break;
		case 5:
			reg_val = 32;
			break;
		case 6:
			reg_val = 64;
			break;
		case 7:
			reg_val = 128;
			break;
	}

	house_reg->led_control = reg_val;
	return 0;
}



