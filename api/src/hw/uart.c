/**
 * $Id: $
 *
 * @brief Red Pitaya Uart Module.
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */
#define _BSD_SOURCE
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <stdint.h>
#include <errno.h>
#include "rp_cross.h"
#include "uart.h"


/*  CONFIGURE THE UART
*  The flags (defined in /usr/include/termios.h - see http://pubs.opengroup.org/onlinepubs/007908799/xsh/termios.h.html):
*       Baud rate:- B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B500000, B576000, B921600, B1000000, B1152000, B1500000, B2000000, B2500000, B3000000, B3500000, B4000000
*       CSIZE:- CS5, CS6, CS7, CS8
*       CLOCAL - Ignore modem status lines
*       CREAD - Enable receiver
*       IGNPAR = Ignore characters with parity errors
*       ICRNL - Map CR to NL on input (Use for ASCII comms where you want to auto correct end of line characters - don't use for bianry comms!)
*       PARENB - Parity enable
*       PARODD - Odd parity (else even) */

struct termios g_settings;
/* File descriptor definition */
int uart_fd = -1;

int uart_Init(){
    return uart_InitDevice("/dev/ttyPS1");
}

int uart_InitDevice(char *_device){
    if(uart_fd != -1){
        uart_Release();
    }

    uart_fd = open(_device, O_RDWR | O_NOCTTY | O_NDELAY);

    if(uart_fd == -1){
        fprintf(stderr, "Failed to open UART.\n");
        return RP_EIU;
    }

    tcgetattr(uart_fd, &g_settings);
    return uart_SetDefaultSettings();
}

int uart_SetDefaultSettings(){
    if (uart_fd != -1){
        /* Set baud rate - default set to 9600Hz */
        speed_t baud_rate = B9600;

        /* Baud rate fuctions
        * cfsetospeed - Set output speed
        * cfsetispeed - Set input speed
        * cfsetspeed  - Set both output and input speed */

        cfsetspeed(&g_settings, baud_rate);
        g_settings.c_cflag &= ~PARENB; /* no parity */
        g_settings.c_cflag &= ~CSTOPB; /* 1 stop bit */
        g_settings.c_cflag &= ~CSIZE;
        g_settings.c_cflag |= CS8 | CLOCAL; /* 8 bits */
        g_settings.c_cflag &= ~CRTSCTS; // Disable flow control
        g_settings.c_lflag &= ~ICANON; /* canonical mode */
        g_settings.c_oflag &= ~OPOST; /* raw output */

        /* Setting attributes */
        tcflush(uart_fd, TCIFLUSH);
        tcsetattr(uart_fd, TCSANOW, &g_settings);
        return RP_OK;
    }else{
        fprintf(stderr, "Failed setup settings to UART.\n");
        return RP_EIU;
    }

}

int uart_read(bool _wait_data, unsigned char *_buffer,int *size){

    if (_buffer == NULL || *size <= 0){
        fprintf(stderr, "Failed read from UART.\n");
        return RP_EIPV;
    }

    if (uart_fd != -1){
        fcntl(uart_fd, F_SETFL, FNDELAY);

        while(1){
            if(uart_fd == -1){
                fprintf(stderr, "Failed to read from UART. UART is closed.\n");
                return RP_ERU;
            }

        //    unsigned char rx_buffer[size];

            int rx_length = read(uart_fd, (void*)_buffer, *size);

            if (rx_length < 0){
                /* No data yet avaliable, check again */
                if(errno == EAGAIN){
                    continue;
                /* Error differs */
                }else{
                    fprintf(stderr, "Error read from UART. Errno: %d\n", errno);
                    return RP_ERU;
                }

            }else if (rx_length == 0){
                // Wait data from UART
                if (! _wait_data) {
                    *size = 0;
                    break;
                }
            }else{
                *size = rx_length;
                /* Data get */
                break;
            }
        }
        return RP_OK;
    }else{
        fprintf(stderr, "Failed read from UART.\n");
        return RP_EIU;
    }
}

int uart_write(unsigned char *_buffer, int size){
    int count = 0;
    if (_buffer == NULL || size <= 0){
        fprintf(stderr, "Failed write to UART.\n");
        return RP_EIPV;
    }

    if (uart_fd != -1){
        /* Write some sample data into UART */
        /* ----- TX BYTES ----- */
        if(uart_fd != -1){
            count = write(uart_fd, _buffer, size);
        }else{
            fprintf(stderr, "Failed write to UART.\n");
            return RP_EIU;
        }

        if(count < 0){
            fprintf(stderr, "Failed write to UART.\n");
            return RP_EWU;
        }

        return RP_OK;
    }else{
        fprintf(stderr, "Failed write to UART.\n");
        return RP_EWU;
    }

   

    return 0;
}

int uart_Release(){
    if (uart_fd != -1){
        tcflush(uart_fd, TCIFLUSH);
        close(uart_fd);
        uart_fd = -1;
    }
    return RP_OK;
}

