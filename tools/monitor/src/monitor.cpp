/**
 * $Id$
 *
 * @brief Simple program to read/write from/to any location in memory.
 *
 * @Author Crt Valentincic <crt.valentincic@redpitaya.com>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
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

#include <stdexcept>
#include "common/version.h"
#include "rp.h"
#include "rp_hw-profiles.h"

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

uint32_t read_value(uint32_t a_addr);
void read_value(uint32_t a_addr, uint32_t count);
void write_values(uint32_t a_addr, uint32_t a_values);
void set_DAC(float* values, int count);
void showAMS();

void* map_base = (void*)(-1);

struct CommandParams {
    enum Type { SINGLE, RANGE, WRITE, NONE };
    Type type = NONE;
    uint32_t address = 0;
    uint32_t count_or_value = 0;
};

CommandParams parse_args(int argc, char** argv) {
    CommandParams params;

    if (argc < 2) {
        params.type = CommandParams::NONE;
        return params;
    }

    try {
        std::string arg1 = argv[1];

        // Check if arg1 is empty
        if (arg1.empty()) {
            params.type = CommandParams::NONE;
            return params;
        }

        if (argc == 4 && std::string(argv[2]) == "w") {
            if (arg1.find('-') != std::string::npos) {
                params.type = CommandParams::NONE;
                return params;
            }

            params.type = CommandParams::WRITE;
            params.address = std::stoul(arg1, nullptr, 0);
            params.count_or_value = std::stoul(argv[3], nullptr, 0);
            return params;
        }

        size_t dash_pos = arg1.find('-');

        if (dash_pos != std::string::npos) {
            // RANGE format: "addr-count"
            if (dash_pos == 0 || dash_pos == arg1.length() - 1) {
                // Invalid range format (no address or no count)
                params.type = CommandParams::NONE;
                return params;
            }

            params.type = CommandParams::RANGE;
            params.address = std::stoul(arg1.substr(0, dash_pos), nullptr, 0);
            params.count_or_value = std::stoul(arg1.substr(dash_pos + 1), nullptr, 0);

        } else if (argc == 3) {
            if (std::string(argv[2]) == "w") {
                params.type = CommandParams::NONE;
                return params;
            }

            // WRITE format: address value
            params.type = CommandParams::WRITE;
            params.address = std::stoul(arg1, nullptr, 0);
            params.count_or_value = std::stoul(argv[2], nullptr, 0);

        } else if (argc == 2) {
            // SINGLE format: address
            params.type = CommandParams::SINGLE;
            params.address = std::stoul(arg1, nullptr, 0);

        } else {
            params.type = CommandParams::NONE;
        }

    } catch (const std::invalid_argument& e) {
        // Conversion failed due to invalid characters
        params.type = CommandParams::NONE;
    } catch (const std::out_of_range& e) {
        // Number too large for unsigned long
        params.type = CommandParams::NONE;
    }

    return params;
}

int main(int argc, char** argv) {
    int fd = -1;
    int retval = EXIT_SUCCESS;

    if (argc < 2) {
        fprintf(stderr,
                "%s version %s-%s\n"
                "\nUsage:\n"
                "\tread addr: address\n"
                "\tread addr: address-count\n"
                "\twrite addr: address value\n"
                "\twrite addr: address w value\n"
                "\tread analog mixed signals: -ams\n"
                "\tset slow DAC: -sdac AO0 AO1 AO2 AO3 [V]\n"
                "\tClock frequency meter: -c\n"
                "\tPrint fpga version: -f\n"
                "\tPrint DTS version: -d\n"
                "\tPrint model name: -n\n"
                "\tPrint model id: -i\n"
                "\tPrint Housekeeping regset: -ph\n"
                "\tPrint Oscilloscope regset: -posc\n"
                "\tPrint Arbitrary Signal Generator regset: -pasg\n"
                "\tPrint Arbitrary Signal Generator signal from ch1: -pasg_ch1\n"
                "\tPrint Arbitrary Signal Generator signal from ch2: -pasg_ch2\n"
                "\tPrint Analog Mixed Signals regset: -pams\n"
                "\tPrint Daisy Chain regset: -pdaisy\n"
                "\tReserved memory for DMA: -r\n",

                argv[0],
                VERSION_STR,
                REVISION_STR);
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
        const char* modelFPGA = NULL;
        auto ret = rp_HPGetFPGAVersion(&modelFPGA);
        if (ret == RP_HP_OK) {
            printf("%s", modelFPGA);
        } else {
            printf("undefined");
        }
        return ret;
    }

    if (key == "-d") {
        const char* modelDTS = NULL;
        auto ret = rp_HPGetDTSVersion(&modelDTS);
        if (ret == RP_HP_OK) {
            printf("%s", modelDTS);
        } else {
            printf("undefined");
        }
        return ret;
    }

    if (key == "-n") {
        char* model_name = nullptr;
        auto ret = rp_HPGetModelName(&model_name);
        if (ret == RP_HP_OK) {
            printf("%s", model_name);
        } else {
            printf("[Error]");
        }
        return ret;
    }

    if (key == "-i") {
        rp_HPeModels_t model;
        auto ret = rp_HPGetModel(&model);
        if (ret == RP_HP_OK) {
            printf("%d", model);
        } else {
            printf("-1");
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

    if (key == "-pasg_ch1") {
        return rp_PrintAsgChannelData(RP_CH_1);
    }

    if (key == "-pasg_ch2") {
        return rp_PrintAsgChannelData(RP_CH_2);
    }

    if (key == "-pams") {
        return rp_PrintAmsRegset();
    }

    if (key == "-pdaisy") {
        return rp_PrintDaisyRegset();
    }

    if ((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1)
        FATAL("Error open device");

    auto info = parse_args(argc, argv);

    /* Map one page */
    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, info.address & ~MAP_MASK);
    if (map_base == (void*)-1)
        FATAL("Error map memory");

    if (info.type != info.NONE) {
        if (info.type == info.SINGLE) {
            read_value(info.address);
        } else if (info.type == info.RANGE) {
            read_value(info.address, info.count_or_value);
        } else {
            write_values(info.address, info.count_or_value);
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

void read_value(uint32_t a_addr, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        auto a = (a_addr + i * 0x4);
        void* virt_addr = map_base + (a & MAP_MASK);
        uint32_t read_result = 0;
        read_result = *((uint32_t*)virt_addr);
        printf("0x%08x 0x%08x\n", a, read_result);
    }
    fflush(stdout);
}

void write_values(uint32_t a_addr, uint32_t a_values) {
    void* virt_addr = map_base + (a_addr & MAP_MASK);
    *((unsigned long*)virt_addr) = a_values;
}