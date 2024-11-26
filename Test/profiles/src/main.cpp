/**
 * $Id$
 *
 * @brief Simple program for profile api
 *
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include "common/version.h"
#include "rp_hw-profiles.h"
#include "rp.h"

void usage(char **argv){
	fprintf(stderr,
					"%s version %s-%s\n"
		"\nUsage:\n"
		"\t-p\t: Show current profile\n"
		"\t-pa\t: Show all profiles\n"
		"\t-f\t: Print fpga version\n"
		"\t-n\t: Print model name\n"
		"\t-i\t: Print model id\n"
		"\t-v KEY\t: Print value from profile by key\n",
		argv[0], VERSION_STR, REVISION_STR);

	fprintf(stderr,"\t\tKeys:\n"
				"\t\t\tgpio_n\t: Number of GPIO channels N\n"
				"\t\t\tgpio_p\t: Number of GPIO channels P\n"
	);
}

int main(int argc, char **argv) {
	if(argc < 2) {
		usage(argv);
		return EXIT_FAILURE;
	}
	if (strncmp(argv[1], "-pa", 3) == 0) {
		return rp_HPPrintAll();
	}

	if (strncmp(argv[1], "-p", 2) == 0) {
		return rp_HPPrint();
	}

	if (strncmp(argv[1], "-f", 2) == 0) {
		char *modelFPGA = NULL;
		auto ret = rp_HPGetFPGAVersion(&modelFPGA);
		if (ret == RP_HP_OK){
			printf("%s",modelFPGA);
		}else{
			printf("undefined");
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

	if (strncmp(argv[1], "-v", 2) == 0) {
		if(argc < 3) {
			usage(argv);
		}else{
			if (strncmp(argv[2], "gpio_p", 2) == 0) {
				uint8_t value;
				auto ret = rp_HPGetGPIO_P_Count(&value);
				if (ret == RP_HP_OK){
					printf("%d",value);
				}else{
					printf("[Error]");
					return EXIT_FAILURE;
				}
				return EXIT_SUCCESS;
			}
			if (strncmp(argv[2], "gpio_n", 2) == 0) {
				uint8_t value;
				auto ret = rp_HPGetGPIO_N_Count(&value);
				if (ret == RP_HP_OK){
					printf("%d",value);
				}else{
					printf("[Error]");
					return EXIT_FAILURE;
				}
				return EXIT_SUCCESS;
			}
			usage(argv);
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}