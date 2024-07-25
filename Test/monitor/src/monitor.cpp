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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

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

#include "common/version.h"
#include "rp_hw-profiles.h"
#include "rp.h"


#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

int parse_from_argv(int a_argc, char **a_argv, unsigned long* a_addr, int* a_type, unsigned long** a_values, ssize_t* a_len);
uint32_t read_value(uint32_t a_addr);
void write_values(unsigned long a_addr, int a_type, unsigned long* a_values, ssize_t a_len);
void set_DAC(float *values,int count);
void showAMS();

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
			"\tread analog mixed signals: -ams\n"
			"\tset slow DAC: -sdac AO0 AO1 AO2 AO3 [V]\n"
			"\tShow current profile: -p\n"
			"\tShow all profiles: -pa\n"
			"\tPrint fpga version: -f\n"
			"\tPrint model name: -n\n"
			"\tPrint model id: -i\n"
			"\tReserved memory for DMA: -r\n",

                        argv[0], VERSION_STR, REVISION_STR);
		return EXIT_FAILURE;
	}


	if (strncmp(argv[1], "-sdac", 5) == 0) {

		float *val = NULL;
		ssize_t val_count = 0;
		val = (float*)calloc(argc-2,sizeof(float));
		for (int i = 2; i < argc; ++i, ++val_count) {
			val[val_count] = strtof(argv[i], 0 );
		}

		if(val_count > 4){
			val_count = 4;
		}

		set_DAC(val,val_count);

		free(val);
		return 0;
	}


	if (strncmp(argv[1], "-ams", 4) == 0) {
		showAMS();
		return 0;
	}

	if (strncmp(argv[1], "-pa", 3) == 0) {
		return rp_HPPrintAll();
	}

	if (strncmp(argv[1], "-p", 2) == 0) {
		return rp_HPPrint();
	}

	if (strncmp(argv[1], "-r", 2) == 0) {
		auto ret = rp_InitReset(false);
		if (ret != RP_OK){
			fprintf(stderr,"Error init rp api\n");
			return -1;
		}
		uint32_t start,size;
		rp_AcqAxiGetMemoryRegion(&start,&size);
		printf("Reserved memory:\n");
		printf("\tstart:\t0x%X (%d)\n",start,start);
		printf("\tend:\t0x%X (%d)\n",start + size,start + size);
		printf("\tsize:\t0x%X (%d) %d kB\n", size,size,size/1024);
		rp_Release();
		return 0;
	}

	if (strncmp(argv[1], "-f", 2) == 0) {
		rp_HPeModels_t model;
		auto ret = rp_HPGetModel(&model);
		switch (model)
		{
			case STEM_125_10_v1_0:
				printf("z10_125");
				break;
			case STEM_125_14_v1_0:
				printf("z10_125");
				break;
			case STEM_125_14_v1_1:
				printf("z10_125");
				break;
			case STEM_125_14_LN_v1_1:
				printf("z10_125");
				break;
			case STEM_125_14_LN_BO_v1_1:
				printf("z10_125");
				break;
			case STEM_125_14_LN_CE1_v1_1:
				printf("z10_125");
				break;
			case STEM_125_14_LN_CE2_v1_1:
				printf("z10_125");
				break;
			case STEM_122_16SDR_v1_0:
				printf("z20_122");
				break;
			case STEM_122_16SDR_v1_1:
				printf("z20_122");
				break;
			case STEM_125_14_Z7020_v1_0:
				printf("z20_125");
				break;
			case STEM_125_14_Z7020_LN_v1_1:
				printf("z20_125");
				break;
			case STEM_125_14_Z7020_4IN_v1_0:
				printf("z20_125_4ch");
				break;
			case STEM_125_14_Z7020_4IN_v1_2:
				printf("z20_125_4ch");
				break;
			case STEM_125_14_Z7020_4IN_v1_3:
				printf("z20_125_4ch");
				break;
			case STEM_250_12_v1_0:
				printf("z20_250_1_0");
				break;
			case STEM_250_12_v1_1:
				printf("z20_250");
				break;
			case STEM_250_12_v1_2:
				printf("z20_250");
				break;
			case STEM_250_12_v1_2a:
				printf("z20_250");
				break;
			case STEM_250_12_v1_2b:
				printf("z20_250");
				break;
			case STEM_250_12_120:
				printf("z20_250");
				break;
			default:
				printf("undefined");
				break;
		}
		return ret;
	}

	if (strncmp(argv[1], "-n", 2) == 0) {
		char *model_name = nullptr;
		auto ret = rp_HPGetModelName(&model_name);
		if (ret == RP_HP_OK){
			printf("%s",model_name);
		}else{
			printf("[Error]");
		}
		return ret;
	}

	if (strncmp(argv[1], "-i", 2) == 0) {
		rp_HPeModels_t model;
		auto ret = rp_HPGetModel(&model);
		if (ret == RP_HP_OK){
			printf("%d",model);
		}else{
			printf("-1");
		}
		return ret;
	}

	if((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) FATAL("Error open device");

	/* Read from command line */
	unsigned long addr;
	unsigned long *val = NULL;
	int access_type = 'w';
	ssize_t val_count = 0;
	parse_from_argv(argc, argv, &addr, &access_type, &val, &val_count);

	/* Map one page */
	map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, addr & ~MAP_MASK);
	if(map_base == (void *) -1) FATAL("Error map memory");

	if (addr != 0) {
		if (val_count == 0) {
			read_value(addr);
		}
		else {
			write_values(addr, access_type, val, val_count);
		}
	}
	if (map_base != (void*)(-1)) {
		if(munmap(map_base, MAP_SIZE) == -1) FATAL("Error unmap memory");
		map_base = (void*)(-1);
	}

	if (map_base != (void*)(-1)) {
		if(munmap(map_base, MAP_SIZE) == -1) FATAL("Error unmap memory");
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
	*a_values = (long unsigned int*)calloc(4*1024, sizeof(unsigned long));

	//if (a_argc > 2) {
		*a_type = 'w';//tolower(a_argv[2][0]);
	//}

	for (int i = 2; i < a_argc; ++i, ++val_count) {
		(*a_values)[val_count] = strtoul(a_argv[i], 0, 0);
	}

	*a_len = val_count;
	return 0;
}
