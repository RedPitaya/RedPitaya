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
#include "rp_hw.h"


int main(int argc, char *argv[]){

    char *buffer = "TEST string";
    char rx_buf[255];
    memset(rx_buf,0,255);
    int size = 255;

    int res = rp_UartInit(); // init uart api
    printf("Init result: %d\n",res);
    
    res = rp_UartSetTimeout(10); // set timeout in 1/10 sec. 10 = 1 sec 
    printf("Set timeout: %d\n",res);
    
    res = rp_UartSetSpeed(115200); // set uart speed
    printf("Set speed: %d\n",res);

    res = rp_UartSetBits(RP_UART_CS8); // set word size
    printf("Set CS8: %d\n",res);

    res = rp_UartSetStopBits(RP_UART_STOP2); // set stop bits
    printf("Set Stop Bits 2: %d\n",res);

    res = rp_UartSetParityMode(RP_UART_MARK); // set parity
    printf("Set Parity Mode: %d\n",res);
    
    res = rp_UartSetSettings(); // apply settings to uart
    printf("Set settings: %d\n",res);
    
    res = rp_UartWrite((unsigned char*)buffer,strlen(buffer)); // write buffer to uart
    printf("Write result: %d\n",res);

    res = rp_UartRead((unsigned char*)rx_buf,&size); // read from uart
    printf("Read result: %d\n",res);   
    printf("Size: %d (%s)\n",size,rx_buf);

    res = rp_UartRelease(); // close uart api
    printf("UnInit result: %d\n",res);
    return 0;
}