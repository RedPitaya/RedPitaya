/**
 * $Id$
 *
 * @brief Simple program for profile api
 *
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>

#include "common/version.h"
#include "rp.h"
#include "rp_hw-profiles.h"

void usage(char** argv) {
    fprintf(stdout,
            "%s version %s-%s\n"
            "\nUsage:\n"
            "\t-p\t: Show current profile\n"
            "\t-pa\t: Show all profiles\n"
            "\t-f\t: Print fpga version\n"
            "\t-n\t: Print model name\n"
            "\t-i\t: Print model id\n"
            "\t-v KEY\t: Print value from profile by key\n",
            argv[0], VERSION_STR, REVISION_STR);

    fprintf(stdout,
            "\t\tKeys:\n"
            "\t\t\tosc_rate\t: OSC base rate\n"
            "\t\t\tfast_adc_bits\t: HW ADC bits\n"
            "\t\t\tfast_adc_fs\t: HW ADC full scale\n"
            "\t\t\tfast_dac_fs\t: HW DAC full scale\n"
            "\t\t\tis_dac_50ohm\t: Support 50 ohm load mode for DAC\n"
            "\t\t\tis_daisy_clock_sync\t: Synchronization via daisy chain\n"
            "\t\t\tgpio_n\t: Number of GPIO channels N\n"
            "\t\t\tgpio_p\t: Number of GPIO channels P\n"
            "\t\t\te3\t: Availability of E3 connector\n"
            "\t\t\te3_gpio\t: Availability of E3 GPIO\n"
            "\t\t\te3_qspi\t: Availability of E3 QSPI\n");

    fprintf(stdout,
            "\t-t <key>,<key>,...: Print pivot table\n"
            "\t\t<key> - Can be an exact value or as a substring.\n"
            "\t\tKeys:\n");
    rp_HPPrintKeys();
}

int main(int argc, char** argv) {
    if (argc < 2) {
        usage(argv);
        return EXIT_FAILURE;
    }
    if (strncmp(argv[1], "-pa", 3) == 0) {
        return rp_HPPrintAll();
    }

    if (strncmp(argv[1], "-p", 2) == 0) {
        return rp_HPPrint();
    }

    if (strncmp(argv[1], "-t", 2) == 0) {
        if (argc > 2) {
            return rp_HPPrintPivotTable(argv[2]);
        } else {
            usage(argv);
            return EXIT_FAILURE;
        }
    }

    if (strncmp(argv[1], "-f", 2) == 0) {
        const char* modelFPGA = NULL;
        auto ret = rp_HPGetFPGAVersion(&modelFPGA);
        if (ret == RP_HP_OK) {
            printf("%s\n", modelFPGA);
        } else {
            printf("undefined\n");
        }
        return ret;
    }

    if (strncmp(argv[1], "-n", 2) == 0) {
        char* model_name = nullptr;
        auto ret = rp_HPGetModelName(&model_name);
        if (ret == RP_HP_OK) {
            printf("%s\n", model_name);
        } else {
            printf("[Error]\n");
        }
        return ret;
    }

    if (strncmp(argv[1], "-i", 2) == 0) {
        rp_HPeModels_t model;
        auto ret = rp_HPGetModel(&model);
        if (ret == RP_HP_OK) {
            printf("%d\n", model);
        } else {
            printf("-1\n");
        }
        return ret;
    }

    if (strncmp(argv[1], "-v", 2) == 0) {
        if (argc < 3) {
            usage(argv);
        } else {
            std::string key = argv[2];
            if (key == "osc_rate") {
                uint32_t value;
                auto ret = rp_HPGetBaseSpeedHz(&value);
                if (ret == RP_HP_OK) {
                    printf("%d\n", value);
                } else {
                    printf("[Error]\n");
                    return EXIT_FAILURE;
                }
                return EXIT_SUCCESS;
            }
            if (key == "gpio_p") {
                uint8_t value;
                auto ret = rp_HPGetGPIO_P_Count(&value);
                if (ret == RP_HP_OK) {
                    printf("%d\n", value);
                } else {
                    printf("[Error]\n");
                    return EXIT_FAILURE;
                }
                return EXIT_SUCCESS;
            }
            if (key == "gpio_n") {
                uint8_t value;
                auto ret = rp_HPGetGPIO_N_Count(&value);
                if (ret == RP_HP_OK) {
                    printf("%d\n", value);
                } else {
                    printf("[Error]\n");
                    return EXIT_FAILURE;
                }
                return EXIT_SUCCESS;
            }
            if (key == "fast_adc_bits") {
                uint8_t value;
                auto ret = rp_HPGetFastADCBits(&value);
                if (ret == RP_HP_OK) {
                    printf("%d\n", value);
                } else {
                    printf("[Error]\n");
                    return EXIT_FAILURE;
                }
                return EXIT_SUCCESS;
            }
            if (key == "is_dac_50ohm") {
                bool value;
                auto ret = rp_HPGetIsDAC50OhmMode(&value);
                if (ret == RP_HP_OK) {
                    printf("%d\n", value);
                } else {
                    printf("[Error]\n");
                    return EXIT_FAILURE;
                }
                return EXIT_SUCCESS;
            }
            if (key == "is_daisy_clock_sync") {
                bool value;
                auto ret = rp_HPGetIsDaisyChainClockAvailable(&value);
                if (ret == RP_HP_OK) {
                    printf("%d\n", value);
                } else {
                    printf("[Error]\n");
                    return EXIT_FAILURE;
                }
                return EXIT_SUCCESS;
            }
            if (key == "fast_adc_fs") {
                float value;
                auto ret = rp_HPGetHWADCFullScale(&value);
                if (ret == RP_HP_OK) {
                    printf("%f\n", value);
                } else {
                    printf("[Error]\n");
                    return EXIT_FAILURE;
                }
                return EXIT_SUCCESS;
            }
            if (key == "fast_dac_fs") {
                float value;
                auto ret = rp_HPGetHWDACFullScale(&value);
                if (ret == RP_HP_OK) {
                    printf("%f\n", value);
                } else {
                    printf("[Error]\n");
                    return EXIT_FAILURE;
                }
                return EXIT_SUCCESS;
            }
            if (key == "e3") {
                bool value;
                auto ret = rp_HPGetIsE3Present(&value);
                if (ret == RP_HP_OK) {
                    printf("%d\n", value);
                } else {
                    printf("[Error]\n");
                    return EXIT_FAILURE;
                }
                return EXIT_SUCCESS;
            }
            if (key == "e3_gpio") {
                bool value;
                auto ret = rp_HPGetIsE3HighSpeedGPIO(&value);
                if (ret == RP_HP_OK) {
                    printf("%d\n", value);
                } else {
                    printf("[Error]\n");
                    return EXIT_FAILURE;
                }
                return EXIT_SUCCESS;
            }
            if (key == "e3_qspi") {
                bool value;
                auto ret = rp_HPGetIsE3QSPIeMMC(&value);
                if (ret == RP_HP_OK) {
                    printf("%d\n", value);
                } else {
                    printf("[Error]\n");
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