/* @brief This is a simple application for testing UART communication on a RedPitaya
* @Author Luka Golinar <luka.golinar@redpitaya.com>
*
* (c) Red Pitaya  http://www.redpitaya.com
*
* This part of code is written in C programming language.
* Please visit http://en.wikipedia.org/wiki/C_(programming_language)
* for more details on the language used herein.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rp.h"


int main(int argc, char *argv[]){

    char *buffer = "TEST STRING";
    char rx_buf[255];
    memset(rx_buf,0,255);
    int size = 255;

    printf("Init result: %d\n",rp_UartInit());
    printf("Write result: %d\n", rp_UartWrite((unsigned char*)buffer,strlen(buffer)));
    printf("Read result: %d\n",rp_UartRead(false,(unsigned char*)rx_buf,&size));
    printf("Size: %d (%s)\n",size,rx_buf);
    printf("UnInit result: %d\n",rp_UartRelease());
    return 0;
}