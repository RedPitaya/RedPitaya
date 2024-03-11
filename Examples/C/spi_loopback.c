/* @brief This is a simple application for testing SPI communication on a RedPitaya
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

    int res = rp_SPI_InitDevice("/dev/spidev1.0"); // Init spi api.
    printf("Init result: %d\n",res);

    res = rp_SPI_SetDefaultSettings(); // Set default settings.
    printf("Set default settings: %d\n",res);

    res = rp_SPI_GetSettings(); // Get uart speed.
    printf("Get current settings of spi: %d\n",res);

    rp_spi_mode_t mode;
    res = rp_SPI_GetMode(&mode);
    printf("Get mode: %d ret: %d\n",mode,res);

    res = rp_SPI_SetMode(RP_SPI_MODE_LIST); // Set SPI mode: Low idle level, sample on trailing edge.
    printf("Set mode: %d\n",res);

    rp_spi_cs_mode_t cs_mode;
    res = rp_SPI_GetCSMode(&cs_mode);
    printf("Get CS mode: %d ret: %d\n",cs_mode,res);

    int speed;
    res = rp_SPI_GetSpeed(&speed);
    printf("Get speed: %d ret: %d\n",speed,res);

    res = rp_SPI_SetSpeed(50000000); // Set SPI speed.
    printf("Set speed: %d\n",res);

    res = rp_SPI_SetWordLen(8); // Set word bit size.
    printf("Set word length: %d\n",res);

    res = rp_SPI_SetSettings(); // Apply settings to SPI.
    printf("Set settings: %d\n",res);

    res = rp_SPI_CreateMessage(2); // Create 2 message.
    printf("Set settings: %d\n",res);

    res = rp_SPI_SetBufferForMessage(0,(uint8_t*)buffer,true,strlen(buffer),false); // Set buffer for first message and create RX buffer.
    printf("Set buffers for first msg: %d\n",res);

    res = rp_SPI_SetBufferForMessage(1,0,true,100,false); // Create RX buffer.
    printf("Set buffers for second msg: %d\n",res);

    res = rp_SPI_ReadWrite(); // Pass message to SPI.
    printf("Read/Write to spi: %d\n",res);

    const uint8_t *rx_buffer = 0;
    size_t rx_size = 0;
    res = rp_SPI_GetRxBuffer(0,&rx_buffer,&rx_size); // Get pointer to rx buffer. No need free buffer. Api itself destroy buffer.

    if (rx_buffer)
        printf("Read message: %s (res %d)\n",rx_buffer,res);

    res = rp_SPI_DestoryMessage();

    res = rp_SPI_Release(); // Close spi api.
    printf("UnInit result: %d\n",res);

    return 0;
}
