/** @file health.c
 *
 * $Id: main.c 881 2013-12-16 05:37:34Z  $
 *
 * @brief Red Pitaya house keeping fpga.
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
 	house_reg->exp_c_N |= 0x1E; //sets direction of N line bits 4:1 to out

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

int set_transducer_and_led(int curr_probe){
	
	unset_transducer_and_led();
	
	if(curr_probe == 0 ){
		return 0;
	}
	
	house_reg->exp_c_out_N |= 1 << curr_probe;
	house_reg->led_control |= 1 << curr_probe;
	return 0;
}

int unset_transducer_and_led(void){
	
	house_reg->exp_c_out_N &= ~0x1E;
	house_reg->led_control &= ~0x1E;
	return 0;
}



