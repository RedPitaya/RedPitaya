/**
 * kratek programcek za test RP Graficnega modula
 * AndrenumBits Trost
 *
 * predelana koda monitor.c (c) Red Pitaya
 * prevedi:  gcc -std=gnu99 ctr.c -o ctr -lm
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdint.h>
#include <math.h>
/*maybe you have to link math.h function
 * go -> properties -> c/c++ build -> settings -> ARM v7 gcc linker -> Librires -> Add m
 */




#define FATAL do { fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", \
  __LINE__, __FILE__, errno, strerror(errno)); exit(1); } while(0)


#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

#define MAP_SIZE_2 4096UL
#define MAP_MASK_2 (MAP_SIZE_2 - 1)

#define MAP_SIZE_3 4096UL
#define MAP_MASK_3 (MAP_SIZE_3 - 1)


void* map_base = (void*)(-1);
void* map_base_2 = (void*)(-1);
void* map_base_3 = (void*)(-1);

/*GPIO address, it can be visible in the Address Editor,
above block design*/


static unsigned long addr;
static unsigned long addr_2;
static unsigned long addr_3;

//addresses for AXI_Gpio
void* led;
void* data_position;
void* data_in;
void* offset;
void* size;


/*draw square*/
void square(int position, int centerX, int centerY, int sideLength)
{
	int column;
	static int row = 0;
	int diffX;
	int diffY;
	short condition = 0;

	column = position % 256;

	if(column == 255){
			if (row < 255) row++;
			else row = 0;
		}

	diffX = abs(centerX - column);
	diffY = abs(centerY - row);
	/*difference is less than half the side length of square,
	-> draw pixel white*/
	condition = ((diffX <= (sideLength/2)) && (diffY <= (sideLength/2)));

	switch(condition){
		case 0:
			*((unsigned long *) data_in) = 0;
			break;
		case 1:
			*((unsigned long *) data_in) = 1;
			break;

	}

}


/*draw circle*/
void circle(int position, int centerX, int centerY, int radius)
{
	int column;
	static int row = 0;
	int diffX;
	int diffY;
	int CalcRadius;


	column = position % 256;

	if(column == 255){
		if (row < 255) row++;
		else row = 0;
	}

	diffX = abs(centerX - column);
	diffY = abs(centerY - row);
	CalcRadius = sqrt(diffX * diffX + diffY * diffY);

	switch(CalcRadius <= radius){
		case 0:
			*((unsigned long *) data_in) = 0;
			break;
		case 1:
			*((unsigned long *) data_in) = 1;
			break;

	}

}


int main(int argc, char **argv) {
	int fd = -1;

	/*picture size and offset on the screen*/
	int sizeX = 0xFF;
	int sizeY = 0xFF;
	int offsetX = 0xFF;
	int offsetY = 0xFF;
	int numBits;
	int positionAndEnable;
	int ledCount;

	if((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) FATAL;

	addr = 0x41200000;  											/* Map one page */
	map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, addr & ~MAP_MASK);

	addr_2 = 0x41220000;
	map_base_2 = mmap(0, MAP_SIZE_2, PROT_READ | PROT_WRITE, MAP_SHARED, fd, addr_2 & ~MAP_MASK_2);

	addr_3 = 0x41210000;
	map_base_3 = mmap(0, MAP_SIZE_3, PROT_READ | PROT_WRITE, MAP_SHARED, fd, addr_3 & ~MAP_MASK_3);

	if(map_base == (void *) -1) FATAL;
	if(map_base_2 == (void *) -1) FATAL;
	if(map_base_3 == (void *) -1) FATAL;

	led = map_base + (addr & MAP_MASK);  							//LED, for controlling if the program is running

	data_position = map_base_2 + (addr_2 & MAP_MASK_2); 			//picture coordinates
	data_in = map_base_2 + ((addr_2 + 0x0008) & MAP_MASK_2); 		//data (black or white pixel)

	offset = map_base_3 + (addr_3 & MAP_MASK_3); 					//offset position on the screen
	size = map_base_3 + ((addr_3 + 0x0008) & MAP_MASK_3);  			//picture size


	/*offset and picture size*/
	*((unsigned long *) offset) = (offsetY << 8) | offsetX;
	*((unsigned long *) size) = (sizeY << 8) | sizeX;


	for(numBits = 0; numBits < 65536; numBits++)
	{
		positionAndEnable = (numBits << 1) | 0b1; 				//data_position must contain also flag that enables writing
		*((unsigned long *) data_position) = positionAndEnable;

//		circle(numBits, 100, 100, 50);
		square(numBits, 100, 100, 50);
	}

	for(ledCount=0; ledCount<3; ledCount++) {

		*((unsigned long *) led) = ledCount;
		sleep(1);
	}

	return EXIT_SUCCESS;
}




