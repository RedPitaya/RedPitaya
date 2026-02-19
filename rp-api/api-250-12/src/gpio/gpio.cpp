#include "gpio.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <gpiod.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <fstream>
#include <iostream>

#define VALUE_MAX 40
#define MAX_PATH 64
#define LOW 0
#define HIGH 1

#define MSG(...)          \
    if (g_enable_verbous) \
        fprintf(stdout, __VA_ARGS__);
#define MSG_A(...) fprintf(stdout, __VA_ARGS__);

int gpio_write(int pin, int value) {
    struct gpiod_chip* chip;
    struct gpiod_line* line;
    char chip_path[32];
    int ret = 0;

    snprintf(chip_path, sizeof(chip_path), "/dev/gpiochip0");

    chip = gpiod_chip_open(chip_path);
    if (!chip) {
        MSG_A("[rp_gpio] Unable to open GPIO chip %s\n", chip_path);
        return -1;
    }

    line = gpiod_chip_get_line(chip, pin);
    if (!line) {
        MSG_A("[rp_gpio] Unable to get GPIO line %d\n", pin);
        gpiod_chip_close(chip);
        return -1;
    }

    ret = gpiod_line_request_output(line, "rp-gpio", 0);
    if (ret < 0) {
        MSG_A("[rp_gpio] Unable to request GPIO %d as output\n", pin);
        gpiod_line_release(line);
        gpiod_chip_close(chip);
        return -1;
    }

    if (value == LOW) {
        ret = gpiod_line_set_value(line, 0);
        if (ret < 0) {
            MSG_A("[rp_gpio] Unable to set GPIO %d LOW\n", pin);
        }
    } else if (value == HIGH) {
        ret = gpiod_line_set_value(line, 1);
        if (ret < 0) {
            MSG_A("[rp_gpio] Unable to set GPIO %d HIGH\n", pin);
        }
    } else {
        MSG_A("[rp_gpio] Nonvalid pin value requested\n");
        ret = -1;
    }

    gpiod_line_release(line);
    gpiod_chip_close(chip);

    return ret;
}

int gpio_read(int pin) {
    struct gpiod_chip* chip;
    struct gpiod_line* line;
    int value;
    int ret;

    chip = gpiod_chip_open("/dev/gpiochip0");
    if (!chip) {
        MSG_A("[rp_gpio] Unable to open GPIO chip\n");
        return -1;
    }

    line = gpiod_chip_get_line(chip, pin);
    if (!line) {
        MSG_A("[rp_gpio] Unable to get GPIO line %d\n", pin);
        gpiod_chip_close(chip);
        return -1;
    }

    ret = gpiod_line_request_input(line, "rp-gpio");
    if (ret < 0) {
        MSG_A("[rp_gpio] Unable to request GPIO %d as input\n", pin);
        gpiod_line_release(line);
        gpiod_chip_close(chip);
        return -1;
    }

    value = gpiod_line_get_value(line);
    if (value < 0) {
        MSG_A("[rp_gpio] Unable to read value from GPIO %d\n", pin);
        gpiod_line_release(line);
        gpiod_chip_close(chip);
        return -1;
    }

    gpiod_line_release(line);
    gpiod_chip_close(chip);

    return value;
}

int gpio_pin_direction(int pin, int value) {
    struct gpiod_chip* chip;
    struct gpiod_line* line;
    char chip_path[32];
    int ret = 0;

    snprintf(chip_path, sizeof(chip_path), "/dev/gpiochip0");

    chip = gpiod_chip_open(chip_path);
    if (!chip) {
        MSG_A("[rp_gpio] Unable to open GPIO chip %s\n", chip_path);
        return -1;
    }

    line = gpiod_chip_get_line(chip, pin);
    if (!line) {
        MSG_A("[rp_gpio] Unable to get GPIO line %d\n", pin);
        gpiod_chip_close(chip);
        return -1;
    }

    if (value == RP_GPIO_IN) {
        ret = gpiod_line_request_input(line, "rp-gpio");
        if (ret < 0) {
            MSG_A("[rp_gpio] Unable to set GPIO %d as input\n", pin);
        }
    } else if (value == RP_GPIO_OUT) {
        ret = gpiod_line_request_output(line, "rp-gpio", 0);
        if (ret < 0) {
            MSG_A("[rp_gpio] Unable to set GPIO %d as output\n", pin);
        }
    } else {
        MSG_A("[rp_gpio] Nonvalid pin direction requested\n");
        ret = -1;
    }

    gpiod_line_release(line);
    gpiod_chip_close(chip);

    return ret;
}