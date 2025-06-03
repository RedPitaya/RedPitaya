#pragma once

#define RP_GPIO_IN 0
#define RP_GPIO_OUT 1

int gpio_write(int pin, int value);

int gpio_read(int pin);

int gpio_pin_direction(int pin, int dir);

int gpio_export(int pin);

int gpio_unexport(int pin);
