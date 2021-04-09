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
#include <stdint.h>
#include <termios.h>
#include <errno.h>
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

int uart_read(unsigned char *_buffer,int *size){

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
                // Nothing to do
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

int uart_SetSpeed(int _speed){
    if(uart_fd == -1){
        fprintf(stderr, "Failed to open UART.\n");
        return RP_EIU;
    }
   
    if (cfsetspeed(&g_settings, uart_GetSpeedType(_speed))){
        fprintf(stderr, "Error set speed for UART. Errno: %d\n", errno);
        return RP_ESU;
    }

    /* Setting attributes */
    tcflush(uart_fd, TCIFLUSH);
    if (tcsetattr(uart_fd, TCSANOW, &g_settings)){
        fprintf(stderr, "Error set speed for UART. Errno: %d\n", errno);
        return RP_ESU;
    }
    return RP_OK;
}


//  Baud rate:- B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B500000, B576000, B921600, B1000000, B1152000, 
//  B1500000, B2000000, B2500000, B3000000, B3500000, B4000000

int uart_GetSpeedType(int _speed){
    switch(_speed){
        case 1200: return B1200;
        case 2400: return B2400;
        case 4800: return B4800;
        case 9600: return B9600;
        case 19200: return B19200;
        case 38400: return B38400;
        case 57600: return B57600;
        case 115200: return B115200;
        case 230400: return B230400;
        case 576000: return B576000;
        case 912600: return B921600;
        case 1000000: return B1000000;
        case 1152000: return B1152000;
        case 1500000: return B1500000;
        case 2000000: return B2000000;
        case 2500000: return B2500000;
        case 3000000: return B3000000;
        case 3500000: return B3500000;
        case 4000000: return B4000000;
    }
    return -1;
}

int uart_SetBits(rp_uart_bits_size_t _size){
    if(uart_fd == -1){
        fprintf(stderr, "Failed to open UART.\n");
        return RP_EIU;
    }
    int mode = 0;
    switch(_size){
        case RP_UART_CS6: {
            mode = CS6;
            break;
        }
        case RP_UART_CS7: {
            mode = CS7;
            break;
        }
        case RP_UART_CS8: {
            mode = CS8;
            break;
        }
        default:
            return RP_ESU;
    }

    g_settings.c_cflag &= ~CSIZE;
    g_settings.c_cflag |= mode; /* 8 bits */
    /* Setting attributes */
    tcflush(uart_fd, TCIFLUSH);
    if (tcsetattr(uart_fd, TCSANOW, &g_settings)){
        fprintf(stderr, "Error set BitsSize in UART. Errno: %d\n", errno);
        return RP_ESU;
    }
    return RP_OK;
}

int uart_SetParityMode(rp_uart_parity_t mode){
    if(uart_fd == -1){
        fprintf(stderr, "Failed to open UART.\n");
        return RP_EIU;
    }

    switch(mode){
        case RP_UART_NONE: {
            g_settings.c_cflag &= ~(PARENB | PARODD | CMSPAR);
            break;
        }
        case RP_UART_EVEN: {
            g_settings.c_cflag &= ~(PARODD  | CMSPAR);
            g_settings.c_cflag |= PARENB;
            break;
        }
        case RP_UART_ODD: {
            g_settings.c_cflag &= ~CMSPAR;
            g_settings.c_cflag |= PARENB | PARODD ;
            break;
        }
        case RP_UART_SPACE: {
            g_settings.c_cflag |= PARENB | PARODD | CMSPAR;
            break;
        }
        case RP_UART_MARK: {
            g_settings.c_cflag &= ~PARODD;
            g_settings.c_cflag |= PARENB | CMSPAR;
            break;
        }

        default:
            return RP_ESU;
    }
    /* Setting attributes */
    tcflush(uart_fd, TCIFLUSH);
    if (tcsetattr(uart_fd, TCSANOW, &g_settings)){
        fprintf(stderr, "Error set Stop Bits in UART. Errno: %d\n", errno);
        return RP_ESU;
    }
    return RP_OK;
}


int uart_SetStopBits(rp_uart_stop_bits_t mode){
    if(uart_fd == -1){
        fprintf(stderr, "Failed to open UART.\n");
        return RP_EIU;
    }

    g_settings.c_cflag &= ~CSTOPB;
    if (mode == RP_UART_STOP2)
        g_settings.c_cflag |= CSTOPB; /* 8 bits */
    /* Setting attributes */
    tcflush(uart_fd, TCIFLUSH);
    if (tcsetattr(uart_fd, TCSANOW, &g_settings)){
        fprintf(stderr, "Error set Stop Bits in UART. Errno: %d\n", errno);
        return RP_ESU;
    }
    return RP_OK;
}

