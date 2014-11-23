

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "led_control.h"


/* Init of the led structure */
led_struct_t *led_struct = NULL;

/** The memory file descriptor used to mmap() the FPGA space */
int led_control_fd = -1;


int led_control_cleanup(void){
	//We only need to clean-up, if led_struct_t isn't null
	if(led_struct){
		if(munmap(led_struct, LED_BASE_ADDR) < 0){
			fprintf(stderr, "munmap failed %s\n", strerror(errno));
			return -1;
		}
		led_struct = NULL;
	}
	return 0;
}

int led_control_init(void){

	/* Page variables used to calculate correct mapping addresses */
    void *page_ptr;
    long page_addr, page_off, page_size = sysconf(_SC_PAGESIZE);

    if(led_control_cleanup() < 0){
    	return -1;
    }

    /* Open /dev/mem to access directly system memory */
    led_control_fd = open("/dev/mem", O_RDWR | O_SYNC);
    /* Error check */
    if(led_control_fd < 0) {
        fprintf(stderr, " Error opening (/dev/mem): %s\n", strerror(errno));
        return -1;
    }

    /* Offset */
    page_addr = LED_BASE_ADDR & (~(page_size-1)); // AB
    
    page_off  = LED_BASE_ADDR - page_addr;

    /* void *mmap(void *addr, size_t length, int prot, int flags,
                  int fd, off_t offset); */

    page_ptr = mmap(NULL, LED_BASE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, led_control_fd, page_addr);
    
    /* Set pointers to correct values */
    led_struct = page_ptr + page_off;
    return 0;
}

int usage(void){
	fprintf(NULL, "Led control information.\n");
	return 0;
}


int main(void){
	/* Application init */
	led_control_init();

	/* Lighting up led from 1 to 7 */
	for(int i = 0; i < 9; i++){
		
		led_struct->led_control = pow(2, i);
		usleep(100000);
	}
	/* Lighting up led from 7 to 1 */
	for (int j = 9; j > 0; --j)
	{
		led_struct->led_control = pow(2, j);
		usleep(100000);
	}
	
	return 0;
}