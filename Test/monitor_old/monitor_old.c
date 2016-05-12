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

#define DEBUG_MONITOR 0

int parse_from_argv_par(int a_argc, char **a_argv, double** a_values, ssize_t* a_len);
int parse_from_argv(int a_argc, char **a_argv, unsigned long* a_addr, int* a_type, unsigned long** a_values, ssize_t* a_len);
int parse_from_stdin(unsigned long* a_addr, int* a_type, unsigned long** a_values, ssize_t* a_len);
uint32_t read_value(uint32_t a_addr);
void write_values(unsigned long a_addr, int a_type, unsigned long* a_values, ssize_t a_len);

void* map_base = (void*)(-1);

const uint32_t c_addrAms=0x40400000;

typedef enum {
	eAmsTemp=0,
	eAmsAI0,
	eAmsAI1,
	eAmsAI2,
	eAmsAI3,
	eAmsAI4,
	eAmsVCCPINT,
	eAmsVCCPAUX,
	eAmsVCCBRAM,
	eAmsVCCINT,
	eAmsVCCAUX,
	eAmsVCCDDR,
	eAmsAO0,
	eAmsAO1,
	eAmsAO2,
	eAmsAO3,
	eSendNum
} ams_t;

const uint8_t amsDesc[eSendNum][20]={
	"Temp(0C-85C)",
	"AI0(0-3.5V)",
	"AI1(0-3.5V)",
	"AI2(0-3.5V)",
	"AI3(0-3.5V)",
	"AI4(5V0)",
	"VCCPINT(1V0)",
	"VCCPAUX(1V8)",
	"VCCBRAM(1V0)",
	"VCCINT(1V0)",
	"VCCAUX(1V8)",
	"VCCDDR(1V5)",
	"AO0(0-1.8V)",
	"AO1(0-1.8V)",
	"AO2(0-1.8V)",
	"AO3(0-1.8V)",
};

#define ADC_FULL_RANGE_CNT 0xfff
#define ADC_POS_RANGE_CNT  0x7ff

#define SLOW_DAC_NUM 4
#define SLOW_DAC_RANGE_CNT 0x9c

typedef struct {
	uint32_t aif[5];
	uint32_t reserved[3];
	uint32_t dac[SLOW_DAC_NUM];
	uint32_t temp;
	uint32_t vccPint;
	uint32_t vccPaux;
	uint32_t vccBram;
	uint32_t vccInt;
	uint32_t vccAux;
	uint32_t vccDddr;
} amsReg_t;

static float AmsConversion(ams_t a_ch, unsigned int a_raw)
{
	float uAdc;
	float val=0;
	switch(a_ch){
		case eAmsAI0:
		case eAmsAI1:
		case eAmsAI2:
		case eAmsAI3:{
			if(a_raw>0x7ff){
				a_raw=0;
			}
			uAdc=(float)a_raw/0x7ff*0.5;
			val=uAdc*(30.0+4.99)/4.99;
		}
		break;
		case eAmsAI4:{
			uAdc=(float)a_raw/ADC_FULL_RANGE_CNT*1.0;
			val=uAdc*(56.0+4.99)/4.99;
		}
		break;
		case eAmsTemp:{
			val=((float)a_raw*503.975) / ADC_FULL_RANGE_CNT - 273.15;
		}
		break;
		case eAmsVCCPINT:
		case eAmsVCCPAUX:
		case eAmsVCCBRAM:
		case eAmsVCCINT:
		case eAmsVCCAUX:
		case eAmsVCCDDR:{
			val=((float)a_raw/ADC_FULL_RANGE_CNT)*3.0;
		}
		break;
		case eAmsAO0:
		case eAmsAO1:
		case eAmsAO2:
		case eAmsAO3:
			val=((float)(a_raw>>16)/SLOW_DAC_RANGE_CNT)*1.8;
		break;
		case eSendNum:
			break;
	}
	return val;
}

static void AmsList(amsReg_t * a_amsReg)
{
	uint32_t i,raw;
	float val;
	printf("#ID\tDesc\t\tRaw\tVal\n");
	for(i=0;i<eSendNum;i++){
		switch(i){
			case eAmsTemp:
			    raw=a_amsReg->temp;
			break;
			case eAmsAI0:
				raw=a_amsReg->aif[0];
			break;
			case eAmsAI1:
				raw=a_amsReg->aif[1];
			break;
			case eAmsAI2:
				raw=a_amsReg->aif[2];
			break;
			case eAmsAI3:
				raw=a_amsReg->aif[3];
				break;
			case eAmsAI4:
				raw=a_amsReg->aif[4];
				break;
			case eAmsVCCPINT:
				raw=a_amsReg->vccPint;
				break;
			case eAmsVCCPAUX:
				raw=a_amsReg->vccPaux;
				break;
			case eAmsVCCBRAM:
				raw=a_amsReg->vccBram;
				break;
			case eAmsVCCINT:
				raw=a_amsReg->vccInt;
				break;
			case eAmsVCCAUX:
				raw=a_amsReg->vccAux;
				break;
			case eAmsVCCDDR:
				raw=a_amsReg->vccDddr;
				break;
			case eAmsAO0:
				raw=a_amsReg->dac[0];
				break;
			case eAmsAO1:
				raw=a_amsReg->dac[1];
				break;
			case eAmsAO2:
				raw=a_amsReg->dac[2];
				break;
			case eAmsAO3:
				raw=a_amsReg->dac[3];
				break;
			case eSendNum:
				break;
		}
		val=AmsConversion(i, raw);
		printf("%d\t%s\t%x\t%.3f\n",i,&amsDesc[i][0],raw,val);
	}
}

static void DacRead(amsReg_t * a_amsReg)
{
	uint32_t i;
	uint32_t raw;
	float val=0;
	for(i=0;i<SLOW_DAC_NUM;i++){
		raw=a_amsReg->dac[i];
		val=AmsConversion(eAmsAO0+i, raw);
		printf("%f\n",val);
	}
}

static void DacWrite(amsReg_t * a_amsReg, double * a_val, ssize_t a_cnt)
{
	uint32_t i;
	for(i=0;i<a_cnt;i++){
		uint32_t dacCnt;
		if(a_val[i]<0){
		   a_val[i]=0;
		}
		if(a_val[i]>1.8){
		   a_val[i]=1.8;
		}
		dacCnt=(a_val[i]/1.8)*SLOW_DAC_RANGE_CNT;
		//dacCnt&=0x9c;
		dacCnt*=256*256; // dacCnt=dacCnt<<16;
		a_amsReg->dac[i]=dacCnt;
	}
}

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
			"\tset slow DAC: -sdac AO0 AO1 AO2 AO3 [V]\n",
                        argv[0], VERSION_STR, REVISION_STR);
		return EXIT_FAILURE;
	}

	if((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) FATAL;

	/* Read from standard input */
	if (strncmp(argv[1], "-ams", 4) == 0) {
		uint32_t addr = c_addrAms;
		amsReg_t* ams=NULL;
		// Map one page
		map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, addr & ~MAP_MASK);
		if(map_base == (void *) -1) FATAL;

		ams = map_base + (addr & MAP_MASK);
		AmsList(ams);

		if (map_base != (void*)(-1)) {
			if(munmap(map_base, MAP_SIZE) == -1) FATAL;
			map_base = (void*)(-1);
		}
	}
	else if (strncmp(argv[1], "-sdac", 5) == 0) {
		uint32_t addr = c_addrAms;
		amsReg_t* ams=NULL;

		double *val = NULL;
		ssize_t val_count = 0;
		parse_from_argv_par(argc, argv, &val, &val_count);

		if(val_count>SLOW_DAC_NUM){
			val_count=SLOW_DAC_NUM;
		}

		// Map one page
		map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, addr & ~MAP_MASK);
		if(map_base == (void *) -1) FATAL;

		ams = map_base + (addr & MAP_MASK);

		if (val_count == 0) {
			DacRead(ams);
		}
		else{
			DacWrite(ams, val, val_count);
		}

		if (map_base != (void*)(-1)) {
			if(munmap(map_base, MAP_SIZE) == -1) FATAL;
			map_base = (void*)(-1);
		}

	}
	else if (strncmp(argv[1], "-", 1) == 0) {
		unsigned long addr;
		unsigned long *val = NULL;
		int access_type = 'w';
		ssize_t val_count = 0;
		while ( parse_from_stdin(&addr, &access_type, &val, &val_count) != -1) {
			if (addr == 0) {
				continue;
			}
			/* Map one page */
			map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, addr & ~MAP_MASK);
			if(map_base == (void *) -1) FATAL;

			if (val_count == 0) {
				read_value(addr);
			}
			else {
				write_values(addr, access_type, val, val_count);
			}
			if (map_base != (void*)(-1)) {
				if(munmap(map_base, MAP_SIZE) == -1) FATAL;
				map_base = (void*)(-1);
			}
#if DEBUG_MONITOR
			printf("addr/type: %lu/%c\n", addr, access_type);

			printf("val (%ld):", val_count);
			for (ssize_t i = 0; i < val_count; ++i) {
				printf("%lu ", val[i]);
			}
			if (val != NULL) {
				free(val);
				val = NULL;
			}
			printf("\n");
#endif
		}
		goto exit;
	}
	/* Read from command line */
	else {
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
#if DEBUG_MONITOR
		printf("addr/type: %lu/%c\n", addr, access_type);

		printf("val (%ld):", val_count);
		for (ssize_t i = 0; i < val_count; ++i) {
			printf("%lu ", val[i]);
		}
		
		if (val != NULL) {
			free(val);
			val = NULL;
		}
		printf("\n");
#endif
	}

exit:

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
	/*
	if (a_len == 1) {
		printf("Written 0x%lX\n", a_values[0]);
	}
	else {
		printf("Written %d values\n", a_len);
	}
	*/
	fflush(stdout);
}

int parse_from_stdin(unsigned long* a_addr, int* a_type, unsigned long** a_values, ssize_t* a_len) {
	char* line = NULL;
	size_t len = 0;
	ssize_t ret = 0;
	ssize_t val_count = 0;

	*a_addr = 0;
	*a_values = calloc(4*1024, sizeof(unsigned long));
	
	while ((ret = getline(&line, &len, stdin)) != -1) {
		if (line[0] == '\n') {
			break;
		}
		{
			char* token;
			token = strtok(line, " \t");
			if (token == NULL) {
				break;
			}
			if (*a_addr == 0) {
				*a_addr = strtoul(token, 0, 0);
				token = strtok(NULL, " \t");
				if (token == NULL) {
					break;
				}
				*a_type = tolower(token[0]);
			}
			else {
				(*a_values)[val_count] = strtoul(token, 0, 0);
				++val_count;
			}

			for (; ; ++val_count) {
				token = strtok(NULL, " \t");
				if (token == NULL)
					break;
				(*a_values)[val_count] = strtoul(token, 0, 0);
			}
		}
	}

	if (line) {
		free(line);
	}

	*a_len = val_count;
	
	if (ret == -1) {
		if (errno != 0) {
			FATAL;
		}
		else if (*a_addr != 0) {
			return val_count;
		}
		return -1;
	}
	else {
		return val_count;
	}
}

int parse_from_argv(int a_argc, char **a_argv, unsigned long* a_addr, int* a_type, unsigned long** a_values, ssize_t* a_len) {

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

int parse_from_argv_par(int a_argc, char **a_argv, double** a_values, ssize_t* a_len) {

	int val_count = 0;
	*a_values = calloc(4*1024, sizeof(double));
	for (int i = 2; i < a_argc; ++i, ++val_count) {
		(*a_values)[val_count] = strtod(a_argv[i], 0);
	}
	*a_len = val_count;
	return 0;
}
