/**
 * $Id: acquire.c 1246 2014-02-22 19:05:19Z ales.bardorfer $
 *
 * @brief Red Pitaya led_control utility.
 *
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#include <fcntl.h>
#include <getopt.h>
#include <linux/i2c-dev.h>
#include <linux/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include "rp_hw-profiles.h"

extern "C" {
#include "rp_hw.h"
}

/** Program name */
const char* g_argv0 = NULL;

/** Minimal number of command line arguments */
#define MINARGS 2

#define EXPANDER_ADDR 0x10

int get_state(bool* state, const char* str) {
    if (strncmp(str, "=Off", 4) == 0) {
        *state = false;
        return 0;
    }
    if (strncmp(str, "=On", 3) == 0) {
        *state = true;
        return 0;
    }

    fprintf(stderr, "Unknown state: %s\n", str);
    return -1;
}

/** Print usage information */
void usage() {
    const char* format =
        "\n"
        "Usage: %s -y[=State] | -r[=State] | -e[=State]   \n"
        "\n"
        "   -y    9 Yellow LED. Responsible for the status of reading the memory card.\n"
        "   -r    Red LED, which is responsible for the heartbeat.\n"
        "   -e    LEDs on ethernet connector.\n"
        "\n"
        "Optional parameter:\n"
        "    State = [Off | On]  Turns LEDs on or off"
        "\n";

    fprintf(stderr, format, g_argv0);
}

bool checkExtensionModuleConnection(bool _muteWarnings) {
    int status;
    int fd;
    char buf[2];

    // Open the device.
    fd = open("/dev/i2c-0", O_RDWR);
    if (fd < 0) {
        if (!_muteWarnings)
            fprintf(stderr, "Cannot open the I2C device: %d\n", fd);
        return false;
    }

    // set slave address
    status = ioctl(fd, I2C_SLAVE_FORCE, EXPANDER_ADDR);
    if (status < 0) {
        close(fd);
        if (!_muteWarnings)
            fprintf(stderr, "Unable to set the I2C address: %d\n", status);
        return false;
    }

    int retVal = read(fd, buf, 1);
    close(fd);
    if (retVal < 0) {
        return false;
    }
    return true;
}

/** Acquire utility main */
int main(int argc, char* argv[]) {
    g_argv0 = argv[0];
    bool y_flag = false;
    bool r_flag = false;
    bool e_flag = false;
    bool z_flag = false;
    bool set_y_state = false;
    bool y_state = false;
    bool set_r_state = false;
    bool r_state = false;
    bool set_e_state = false;
    bool e_state = false;

    if (argc < MINARGS) {
        usage();
        exit(EXIT_FAILURE);
    }

    const char* optstring = "y::r::e::z";

    int ch = -1;
    while ((ch = getopt(argc, argv, optstring)) != -1) {
        switch (ch) {
            case 'z':
                z_flag = true;
                break;
            case 'y':
                y_flag = true;
                if (optarg) {
                    set_y_state = true;
                    if (get_state(&y_state, optarg) != 0) {
                        usage();
                        exit(EXIT_FAILURE);
                    }
                }
                break;

            case 'r':
                r_flag = true;
                if (optarg) {
                    set_r_state = true;
                    if (get_state(&r_state, optarg) != 0) {
                        usage();
                        exit(EXIT_FAILURE);
                    }
                }
                break;

            case 'e':
                e_flag = true;
                if (optarg) {
                    set_e_state = true;
                    if (get_state(&e_state, optarg) != 0) {
                        usage();
                        exit(EXIT_FAILURE);
                    }
                }
                break;
        }
    }

    if (y_flag) {
        if (set_y_state) {
            if (rp_SetLEDMMCState(y_state) != RP_HW_OK) {
                fprintf(stderr, "[Error] Can't set yellow led state\n");
            }
        } else {
            bool state = false;
            if (rp_GetLEDMMCState(&state) == RP_HW_OK) {
                printf("MMC LED state = %d\n", state);
            } else {
                fprintf(stderr, "[Error] Can't get yellow led state\n");
                return -1;
            }
        }
    }

    if (r_flag) {
        if (set_r_state) {
            if (!r_state && rp_HPGetIsE3PresentOrDefault()) {
                if (checkExtensionModuleConnection(true)) {
                    fprintf(stderr, "[WARNING] Turning off this indicator will cause the board to reboot.\n");
                }
            }
            if (rp_SetLEDHeartBeatState(r_state) != RP_HW_OK) {
                fprintf(stderr, "[Error] Can't set red led state\n");
            }
        } else {
            bool state = false;
            if (rp_GetLEDHeartBeatState(&state) == RP_HW_OK) {
                printf("HEARBEAT LED state = %d\n", state);
            } else {
                fprintf(stderr, "[Error] Can't get red led state\n");
                return -1;
            }
        }
    }

    if (e_flag) {
        if (set_e_state) {
            if (rp_SetLEDEthState(e_state) != RP_HW_OK) {
                fprintf(stderr, "[Error] Can't set ethernet led state\n");
            }
        } else {
            bool state = false;
            if (rp_GetLEDEthState(&state) == RP_HW_OK) {
                printf("Ethernet LED state = %d\n", state);
            } else {
                fprintf(stderr, "[Error] Can't get ethernet led state\n");
                return -1;
            }
        }
    }

    if (z_flag) {
        bool rstate = false;
        bool ystate = false;
        bool estate = false;
        if (rp_GetLEDMMCState(&ystate) != RP_HW_OK) {
            fprintf(stderr, "[Error] Can't get yellow led state\n");
            return -1;
        }

        if (rp_GetLEDHeartBeatState(&rstate) != RP_HW_OK) {
            fprintf(stderr, "[Error] Can't get red led state\n");
            return -1;
        }

        if (rp_GetLEDEthState(&estate) != RP_HW_OK) {
            fprintf(stderr, "[Error] Can't get ethernet led state\n");
            return -1;
        }

        if (rstate && ystate && estate) {
            printf("1");
            return 0;
        }

        if (rstate || ystate || estate) {
            printf("2");
            return 0;
        }
        printf("0");
    }

    return 0;
}
