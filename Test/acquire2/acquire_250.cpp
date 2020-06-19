#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <sys/param.h>
#include <iostream>
#include "acquire_250.h"

int get_attenuator(int *value, const char *str)
{
    if  (strncmp(str, "20", 2) == 0) {
        *value = 20;
        return 0;
    }
    if  (strncmp(str, "1", 1) == 0)  {
        *value = 1;
        return 0;
    }

    fprintf(stderr, "Unknown attenuator value: %s\n", str);
    return -1;
}

int get_dc_mode(int *value, const char *str)
{
    if  (strncmp(str, "1", 1) == 0) {
        *value = 1;
        return 0;
    }

    if  (strncmp(str, "2", 1) == 0)  {
        *value = 2;
        return 0;
    }

    if  ((strncmp(str, "B", 1) == 0) || (strncmp(str, "b", 1) == 0))  {
        *value = 3;
        return 0;
    }

    fprintf(stderr, "Unknown DC channel value: %s\n", str);
    return -1;
}