/**
 * $Id$
 *
 * @brief Simple program to read/write from/to any location in memory.
 *
 * @Author Crt Valentincic <crt.valentincic@redpitaya.com>
 *         
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
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
//#include <termios.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdint.h>

#include "version.h"

#define FATAL do { fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", \
  __LINE__, __FILE__, errno, strerror(errno)); exit(1); } while(0)
 
#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

int parse_from_argv(int a_argc, char **a_argv, unsigned long* a_addr, int* a_type, unsigned long** a_values, ssize_t* a_len);
uint32_t read_value(uint32_t a_addr);
void write_values(unsigned long a_addr, int a_type, unsigned long* a_values, ssize_t a_len);

void* map_base = (void*)(-1);

int main(int argc, char **argv) {
	int fd = -1;
	int retval = EXIT_SUCCESS;

	if(argc < 2) {
		fprintf(stderr,
                        "%s version %s-%s\n"
			"\nUsage:\n"
			"\tread addr: address\n"
                        "\twrite addr: address value\n"
			"\tset slow DAC: -sdac AO0 AO1 AO2 AO3 [V]\n",
                        argv[0], VERSION_STR, REVISION_STR);
		return EXIT_FAILURE;
	}

	if((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) FATAL;

	/* Read from command line */
	unsigned long addr;
	unsigned long *val = NULL;
	int access_type = 'w';
	ssize_t val_count = 0;
	parse_from_argv(argc, argv, &addr, &access_type, &val, &val_count);

	/* Map one page */
	map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, addr & ~MAP_MASK);
	if(map_base == (void *) -1) FATAL;
		
	if (addr != 0) {
		if (val_count == 0) {
			read_value(addr);
		}
		else {
			write_values(addr, access_type, val, val_count);
		}
	}
	if (map_base != (void*)(-1)) {
		if(munmap(map_base, MAP_SIZE) == -1) FATAL;
		map_base = (void*)(-1);
	}

	if (map_base != (void*)(-1)) {
		if(munmap(map_base, MAP_SIZE) == -1) FATAL;
	}
	if (fd != -1) {
		close(fd);
	}
	
	return retval;
}

uint32_t read_value(uint32_t a_addr) {
	void* virt_addr = map_base + (a_addr & MAP_MASK);
	uint32_t read_result = 0;
	read_result = *((uint32_t *) virt_addr);
	printf("0x%08x\n", read_result);
	fflush(stdout);
	return read_result;
}

void write_values(unsigned long a_addr, int a_type, unsigned long* a_values, ssize_t a_len) {
	void* virt_addr = map_base + (a_addr & MAP_MASK);

	for (ssize_t i = 0; i < a_len; ++i) {
		switch(a_type) {
			case 'b':
				*((unsigned char *) virt_addr) = a_values[i];
				break;
			case 'h':
				*((unsigned short *) virt_addr) = a_values[i];
				break;
			case 'w':
				*((unsigned long *) virt_addr) = a_values[i];
				break;
		}
	}

	fflush(stdout);
}

int parse_from_argv(int a_argc, char **a_argv, unsigned long* a_addr, 
	int* a_type, unsigned long** a_values, ssize_t* a_len) {

	int val_count = 0;

	*a_addr = strtoul(a_argv[1], 0, 0);
	*a_values = calloc(4*1024, sizeof(unsigned long));

	//if (a_argc > 2) {
		*a_type = 'w';//tolower(a_argv[2][0]);
	//}

	for (int i = 2; i < a_argc; ++i, ++val_count) {
		(*a_values)[val_count] = strtoul(a_argv[i], 0, 0);
	}

	*a_len = val_count;
	return 0;
}


