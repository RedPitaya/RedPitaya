

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


/* Init of the control structure */
led_struct_t *led_struct = NULL;
temp_struct_t *temp_struct = NULL;

/** The memory file descriptor used to mmap() the FPGA space */
int control_fd = -1;

/* Program name */
const char *t_argv0 = "LCR control application.";


int control_cleanup(void){
	//We only need to clean-up, if led_struct_t isn't null
	if(led_struct){
		if(munmap(led_struct, LED_BASE_SIZE) < 0){
			fprintf(stderr, "munmap failed %s\n", strerror(errno));
			return -1;
		}
		led_struct = NULL;
	}
	if(temp_struct){
		if(munmap(temp_struct, TEMP_BASE_SIZE) < 0){
			fprintf(stderr, "munmap failed %s\n", strerror(errno));
			return -1;
		}
		temp_struct = NULL;
	}
		
	
	return 0;
}

int control_init(int option){

	/* Page variables used to calculate correct mapping addresses for temperature control */
    void *page_ptr;
    long page_addr, page_off, page_size = sysconf(_SC_PAGESIZE);

    /* Open /dev/mem to access directly system memory */
    control_fd = open("/dev/mem", O_RDWR | O_SYNC);
    /* Error check */
    if(control_fd < 0) {
        fprintf(stderr, " Error opening (/dev/mem): %s\n", strerror(errno));
        return -1;
    }

	if(control_cleanup() < 0){
    	return -1;
    }

	if(option == 1){
		page_addr = LED_BASE_ADDR & (~(page_size-1)); // AB
    	page_off  = LED_BASE_ADDR - page_addr;

    	page_ptr = mmap(NULL, LED_BASE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, control_fd, page_addr);
    	led_struct = page_ptr + page_off;
	}else{
		page_addr = TEMP_BASE_ADDR & (~(page_size-1));
    	page_off = TEMP_BASE_ADDR - page_addr;

    	page_ptr = mmap(NULL, TEMP_BASE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, control_fd, page_addr);
    	temp_struct = page_ptr + page_off;
	}
    
    /* void *mmap(void *addr, size_t length, int prot, int flags,
                  int fd, off_t offset); */
    return 0;
}

void usage(int ret_val){
	char *argv_max = "Too many arguments! LCR control takes exactly 1 argument."
					 "Usage: \n"
					 "\t-Argument 1: Led control\n"
					 "\t-Argument 2: Temperature control.\n";

	if(ret_val == 1){
		fprintf(stderr, argv_max, __TIMESTAMP__, t_argv0);
	}
}


int main(int argc, char *argv[]){
	/* Argument check */
	if(argc > 2){
		usage(0);
	}else if(argc == 1){
		/* Led option*/
		control_init(1);

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
		usleep(10000);
		led_struct->led_control = 0;

	}else{
		/* Init the temperature memory space */
		control_init(2);

		float ret_temp = (float)((temp_struct->temp_control)*503.975) / ADC_FULL_RANGE_CNT;

		printf("Temperature: %.2f°C | %.2f K\n", (ret_temp-273.15), ret_temp );

	}
	control_cleanup();
	
	return 0;
}