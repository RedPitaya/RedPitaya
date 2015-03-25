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
#define EEPROMSIZE                 64*1024/8
 

/* Inline functions definition */
/* Read eeprom device */ 
static int IicRead(char *readBuffer, int offset, int size);
/* Write to eeprom device */
static int IicWrite(char *input_array, int offset, int size);
 
/*
 * constants declaration
 */
int fd; 
 
int main(int argc, char *argv[])
{
    int Status;
	char *readBuffer = (char *)malloc(EEPROMSIZE * sizeof(char));    /* Buffer to hold data read.*/

    char msg[] = "THIS IS A TEST MESSAGE FOR THE I2C PROTOCOL COMMUNICATION WITH A EEPROM. IT WAS WRITTEN FOR A REDPITAYA.";
    size_t size = strlen(msg);

    //Sample offset.
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

    Status = IicWrite((char *)msg, offset, size);
    if(Status){
        fprintf(stderr, "Cannot Write to EEPROM\n");
        return -1;
    }
    
    Status = IicRead(readBuffer, EEPROM_ADDR, EEPROMSIZE);
    if (Status)
    {
        printf("Cannot Read from EEPROM \n");
        close(fd);
        return 1;
    }
    

//    char command[15];
//    strcpy(command, "touch /tmp/iic_read_data");
//    system(command);

    FILE *file = fopen("iic_read_data.bin", "wb");

    int results = fwrite(readBuffer, 1, EEPROMSIZE, file);

    if(results == EOF){
    	fprintf(stderr, "Failed to write read buffer to file.\n");
    	return -1;
    }
 
    printf("eerprom test successfull \n");
    close(fd);

    free(readBuffer);

    return 0;
}
 
 
 
/*****************************************************************************/
/**
*
* Read the data from the EEPROM.
*
* @param    TestLoops - indicates the number of time to execute the test.
*
* @return   Status of the read from EEPROM
*
* @note     None.
*
******************************************************************************/
static int IicRead(char *readBuffer, int offset, int size)
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

    // Write the bytes onto the bus.
    bytesWritten = write(fd, writeBuffer, 2);
    if(bytesWritten < 0){
        fprintf(stderr, "EEPROM write address error.\n");
    }

    /*
     * Read the bytes.
     */
    printf ("Performing Read operation.\n");

    // Read the bytes from the bus.
    bytesRead = read(fd, readBuffer, size);
    if(bytesRead < 0){
        fprintf(stderr, "EEPROM read error.\n");
    }

    printf("Read EEPROM Succesful\n");
 
    return 0;
}


static int IicWrite(char *input_array, int offset, int size){

    int bytesWritten;
    int writeBytes;

    int index;
    
    //Check for limits
    if(size > PAGESIZE){
        writeBytes = PAGESIZE;
    }else{
        writeBytes = size;
    }

    int loop = 0;

    while(size > 0){

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
