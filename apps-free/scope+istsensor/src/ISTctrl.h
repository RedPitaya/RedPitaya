/**
 * $Id: worker.c 881 2013-12-16 05:37:34Z rp_jmenart $
 *
 * @brief Red Pitaya Oscilloscope worker.
 *
 * @Author Jure Menart <juremenart@gmail.com>
 * 
 * @Author: Flavio Ansovini <flavio.ansovini@unige.it>
 * University of Genova - Serizio Supporto Tecnologico
 *         
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */
 
#ifndef __ISTctrl
#define __ISTctrl

#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include "pid.h"

#define DEBUG_MONITOR 0

#define SLOW_DAC_NUM 4
#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)
#define FATAL do { fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", \
  __LINE__, __FILE__, errno, strerror(errno)); exit(1); } while(0)
#define ADC_FULL_RANGE_CNT 0xfff
#define SLOW_DAC_RANGE_CNT 0x9c
#define ADC_POS_RANGE_CNT  0x7ff

int parse_from_argv_par(int a_argc, char **a_argv, double** a_values, ssize_t* a_len);
int parse_from_argv(int a_argc, char **a_argv, unsigned long* a_addr, int* a_type, unsigned long** a_values, ssize_t* a_len);
int parse_from_stdin(unsigned long* a_addr, int* a_type, unsigned long** a_values, ssize_t* a_len);
uint32_t read_value(uint32_t a_addr);
void write_values(unsigned long a_addr, int a_type, unsigned long* a_values, ssize_t a_len);
int fd;

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

float IST_PWR_out[SIGNAL_LENGTH];
int HeatStp,ISTcnt;
float TimeWin;	//in us
FILE *file_ptr;
int fileopen,fileErr;
float ISTsnsAdj;	//about 0.00234;
float ISTmin,ISTmax,ISTadj,ISTfreq,ISTper,ISTlm35;
	
float AmsConversion(ams_t , unsigned int );
void DacWrite(amsReg_t * , double * , ssize_t );
float ISTctrl(void);
void ISTctrl_time(int);
double PID(double delta);
void StopHeat();
void Stop_ISTctrl(rp_app_params_t *);
void ISTctrl_Init(void);
void IST_Initfile(void);
void IST_Closefile(void);
void IST_tempCalib(void);
double getMin(double, double, double);
void swap(double *, double *);

#endif // __ISTctrl

