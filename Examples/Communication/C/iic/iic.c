/*****************************************************************************/
/**
* @file i2c_test.c
* This is a sample iic read and write application
* @author: Luka Golinar <luka.golinar@redpitaya.com>
*
*
******************************************************************************/
 
/***************************** Include Files *********************************/
 
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
 
/************************** Constant Definitions *****************************/
 
#define I2C_SLAVE_FORCE 		   0x0706
#define I2C_SLAVE    			   0x0703    /* Change slave address            */
#define I2C_FUNCS    			   0x0705    /* Get the adapter functionality */
#define I2C_RDWR    			   0x0707    /* Combined R/W transfer (one stop only)*/
 

#define EEPROM_ADDR            	   0x50
 
/*
 * Page size of the EEPROM. This depends on the type of the EEPROM available
 * on board.
 */
#define PAGESIZE                   32
/* eeprom size on a redpitaya */
#define EEPROMSIZE                 64*1024/8
 

/* Inline functions definition */ 
static int IicRead(char *readBuffer, int offset, int size);
static int IicWrite(char *input_array, int offset, int size);
 
/*
 * File descriptors
 */
int fd; 
 
int main(int argc, char *argv[])
{
    int Status;
	char *readBuffer = (char *)malloc(EEPROMSIZE * sizeof(char));    /* Buffer to hold data read.*/

    char msg[] = "THIS IS A TEST MESSAGE FOR THE I2C PROTOCOL COMMUNICATION WITH A EEPROM. IT WAS WRITTEN FOR A REDPITAYA MEASURMENT TOOL.";
    size_t size = strlen(msg);

    /* Sample offset inside an eeprom */
    int offset = 0x100;

    /*
     * Open the device.
     */
    fd = open("/dev/i2c-0", O_RDWR);

    if(fd < 0)
    {
        printf("Cannot open the IIC device\n");
        return 1;
    }

    Status = ioctl(fd, I2C_SLAVE_FORCE, EEPROM_ADDR);
    if(Status < 0)
    {
        printf("Unable to set the EEPROM address\n");
        return -1;
    }

    /* Write to redpitaya eeprom */
    Status = IicWrite((char *)msg, offset, size);
    if(Status){
        fprintf(stderr, "Cannot Write to EEPROM\n");
        close(fd);
        return -1;
    }
    
    /* Read from redpitaya eeprom */
    Status = IicRead(readBuffer, EEPROM_ADDR, EEPROMSIZE);
    if (Status)
    {
        printf("Cannot Read from EEPROM \n");
        close(fd);
        return 1;
    }
 
    printf("eerprom test successfull.\n");
    
    /* Release allocations */
    close(fd);
    free(readBuffer);

    return 0;
}
 
 
 
/*****************************************************************************/
/**
*
* Read the data from the EEPROM.
*
* @param    read buffer -- input buffer for data storage
* @param    off set     -- eeprom memory space offset
* @param    size        -- size of read data
* @return   iicRead status
*
* @note     None.
*
******************************************************************************/
static int iicRead(char *readBuffer, int offset, int size)
{   
    ssize_t bytesWritten;
    ssize_t bytesRead;
    uint8_t writeBuffer[2];

    /*
     * Load the offset address inside EEPROM where data need to be written. 
     * Supported for BigEndian and LittleEndian CPU's
     */
    writeBuffer[0] = (uint8_t)(offset >> 8);
    writeBuffer[1] = (uint8_t)(offset);

    /* Write the bytes onto the bus */
    bytesWritten = write(fd, writeBuffer, 2);
    if(bytesWritten < 0){
        fprintf(stderr, "EEPROM write address error.\n");
        return -1;
    }

    /*
     * Read the bytes.
     */
    printf ("Performing Read operation.\n");

    /* Read bytes from the bus */
    bytesRead = read(fd, readBuffer, size);
    if(bytesRead < 0){
        fprintf(stderr, "EEPROM read error.\n");
        return -1;
    }

    printf("Read EEPROM Succesful\n");

    return 0;
}


static int iicWrite(char *input_array, int offset, int size){

    /* variable declaration */
    int bytesWritten;
    int writeBytes;
    int index;
    
    /* Check for limits */
    if(size > PAGESIZE){
        writeBytes = PAGESIZE;
    }else{
        writeBytes = size;
    }

    /* Number of needed loops to send all the data.
     * Limit data size per transmission is PAGESIZE */
    int loop = 0;

    while(size > 0){

        /* buffer size is PAGESIZE per transmission */
        uint8_t writeBuffer[32 + 2];

        /*
         * Load the offset address inside EEPROM where data need to be written. 
         * Supported for BigEndian and LittleEndian CPU's
         */
        writeBuffer[0] = (uint8_t)(offset >> 8);
        writeBuffer[1] = (uint8_t)(offset);

        for(index = 0; index < PAGESIZE; index++){
            writeBuffer[index + 2] = input_array[index + (PAGESIZE * loop)];
        }

        // Write the bytes onto the bus.
        bytesWritten = write(fd, writeBuffer, writeBytes + 2);
        // Wait till the EEPROM internally completes the write cycle.
        sleep(2);

        if(bytesWritten != writeBytes+2){
            fprintf(stderr, "Failed to write to EEPROM\n");
            return -1;
        }

        //size = written bytes minus the offset addres of two.
        size -= bytesWritten - 2;
        //Increment offset
        offset += PAGESIZE;

        //Check for limits for the new message
        if(size > PAGESIZE){
            writeBytes = PAGESIZE;
        }else{
            writeBytes = size;
        }

        loop++;
    }

    printf("\nWrite EEPROM Succesful\n");

    return 0;
}
