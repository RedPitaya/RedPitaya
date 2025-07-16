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

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string>
//#include <termios.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>

#include "common/version.h"
#include "rp.h"
#include "rp_hw-profiles.h"

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

int parse_from_argv(int a_argc, char** a_argv, unsigned long* a_addr, int* a_type, unsigned long** a_values, ssize_t* a_len);
uint32_t read_value(uint32_t a_addr);
void write_values(unsigned long a_addr, int a_type, unsigned long* a_values, ssize_t a_len);
void set_DAC(float* values, int count);
void showAMS();

void* map_base = (void*)(-1);

int main(int argc, char** argv) {
    int fd = -1;
    int retval = EXIT_SUCCESS;

    if (argc < 2) {
        fprintf(stderr,
                "%s version %s-%s\n"
                "\nUsage:\n"
                "\tread addr: address\n"
                "\twrite addr: address value\n"
                "\tread analog mixed signals: -ams\n"
                "\tset slow DAC: -sdac AO0 AO1 AO2 AO3 [V]\n"
                "\tClock frequency meter: -с\n"
                "\tPrint fpga version: -f\n"
                "\tPrint model name: -n\n"
                "\tPrint model id: -i\n"
                "\tPrint Housekeeping regset: -ph\n"
                "\tPrint Oscilloscope regset: -posc\n"
                "\tPrint Arbitrary Signal Generator regset: -pasg\n"
                "\tPrint Analog Mixed Signals regset: -pams\n"
                "\tPrint Daisy Chain regset: -pdaisy\n"
                "\tReserved memory for DMA: -r\n",

                argv[0], VERSION_STR, REVISION_STR);
        return EXIT_FAILURE;
    }

    std::string key = argv[1];

    if (key == "-sdac") {

        float* val = NULL;
        ssize_t val_count = 0;
        val = (float*)calloc(argc - 2, sizeof(float));
        for (int i = 2; i < argc; ++i, ++val_count) {
            val[val_count] = strtof(argv[i], 0);
        }

        if (val_count > 4) {
            val_count = 4;
        }

        set_DAC(val, val_count);

        free(val);
        return 0;
    }

    if (key == "-ams") {
        showAMS();
        return 0;
    }

    if (key == "-c") {
        rp_InitReset(false);
        uint32_t val = 0;
        auto ret = rp_GetFreqCounter(&val);
        printf("%d\n", val);
        rp_Release();
        return ret;
    }

    if (key == "-r") {
        auto ret = rp_InitReset(false);
        if (ret != RP_OK) {
            fprintf(stderr, "Error init rp api\n");
            return -1;
        }
        uint32_t start, size;
        rp_AcqAxiGetMemoryRegion(&start, &size);
        printf("Reserved memory:\n");
        printf("\tstart:\t0x%X (%d)\n", start, start);
        printf("\tend:\t0x%X (%d)\n", start + size, start + size);
        printf("\tsize:\t0x%X (%d) %d kB\n", size, size, size / 1024);
        rp_Release();
        return 0;
    }

    if (key == "-f") {
        char* modelFPGA = NULL;
        auto ret = rp_HPGetFPGAVersion(&modelFPGA);
        if (ret == RP_HP_OK) {
            printf("%s\n", modelFPGA);
        } else {
            printf("undefined\n");
        }
        return ret;
    }

    if (key == "-n") {
        char* model_name = nullptr;
        auto ret = rp_HPGetModelName(&model_name);
        if (ret == RP_HP_OK) {
            printf("%s\n", model_name);
        } else {
            printf("[Error]\n");
        }
        return ret;
    }

    if (key == "-i") {
        rp_HPeModels_t model;
        auto ret = rp_HPGetModel(&model);
        if (ret == RP_HP_OK) {
            printf("%d\n", model);
        } else {
            printf("-1\n");
        }
        return ret;
    }

    if (key == "-ph") {
        return rp_PrintHouseRegset();
    }

    if (key == "-posc") {
        return rp_PrintOscRegset();
    }

    if (key == "-pasg") {
        return rp_PrintAsgRegset();
    }

    if (key == "-pams") {
        return rp_PrintAmsRegset();
    }

    if (key == "-pdaisy") {
        return rp_PrintDaisyRegset();
    }

    if ((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1)
        FATAL("Error open device");

    /* Read from command line */
    unsigned long addr;
    unsigned long* val = NULL;
    int access_type = 'w';
    ssize_t val_count = 0;
    parse_from_argv(argc, argv, &addr, &access_type, &val, &val_count);

    /* Map one page */
    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, addr & ~MAP_MASK);
    if (map_base == (void*)-1)
        FATAL("Error map memory");

    if (addr != 0) {
        if (val_count == 0) {
            read_value(addr);
        } else {
            write_values(addr, access_type, val, val_count);
        }
    }
    if (map_base != (void*)(-1)) {
        if (munmap(map_base, MAP_SIZE) == -1)
            FATAL("Error unmap memory");
        map_base = (void*)(-1);
    }

    if (map_base != (void*)(-1)) {
        if (munmap(map_base, MAP_SIZE) == -1)
            FATAL("Error unmap memory");
    }
    if (fd != -1) {
        close(fd);
    }

    return retval;
}

uint32_t read_value(uint32_t a_addr) {
    void* virt_addr = map_base + (a_addr & MAP_MASK);
    uint32_t read_result = 0;
    read_result = *((uint32_t*)virt_addr);
    printf("0x%08x\n", read_result);
    fflush(stdout);
    return read_result;
}

void write_values(unsigned long a_addr, int a_type, unsigned long* a_values, ssize_t a_len) {
    void* virt_addr = map_base + (a_addr & MAP_MASK);

    for (ssize_t i = 0; i < a_len; ++i) {
        switch (a_type) {
            case 'b':
                *((unsigned char*)virt_addr) = a_values[i];
                break;
            case 'h':
                *((unsigned short*)virt_addr) = a_values[i];
                break;
            case 'w':
                *((unsigned long*)virt_addr) = a_values[i];
                break;
        }
    }

    fflush(stdout);
}

int parse_from_argv(int a_argc, char** a_argv, unsigned long* a_addr, int* a_type, unsigned long** a_values, ssize_t* a_len) {

    int val_count = 0;

    *a_addr = strtoul(a_argv[1], 0, 0);
    *a_values = (long unsigned int*)calloc(4 * 1024, sizeof(unsigned long));

    //if (a_argc > 2) {
    *a_type = 'w';  //tolower(a_argv[2][0]);
    //}

    for (int i = 2; i < a_argc; ++i, ++val_count) {
        (*a_values)[val_count] = strtoul(a_argv[i], 0, 0);
    }

    *a_len = val_count;
    return 0;
}
