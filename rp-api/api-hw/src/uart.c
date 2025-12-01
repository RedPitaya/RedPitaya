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
#include <sys/ioctl.h>
#include <stdint.h>
#include <termios.h>
#include <errno.h>
#include "uart.h"
#include "rp_log.h"

#define VMINX 1

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
int g_timeout = 0;
speed_t g_baud_rate = B9600;
rp_uart_bits_size_t g_bit_size = RP_UART_CS8;
rp_uart_parity_t g_parity = RP_UART_NONE;
rp_uart_stop_bits_t g_stop_bit = RP_UART_STOP1;

int uart_GetSpeedType(int _speed);
int uart_ConvertSpeed(int _speed);

int uart_Init()
{
    return uart_InitDevice("/dev/ttyPS1");
}

int uart_InitDevice(char *_device)
{
    if (uart_fd != -1)
    {
        uart_Release();
    }

    uart_fd = open(_device, O_RDWR | O_NOCTTY);

    if (uart_fd == -1)
    {
        ERROR_LOG("Failed to open UART.");
        return RP_HW_EIU;
    }

    tcflush(uart_fd, TCIFLUSH);
    tcflush(uart_fd, TCIOFLUSH);

    tcgetattr(uart_fd, &g_settings);
    return uart_SetSettings();
}

int uart_Timeout(uint8_t deca_sec)
{
    g_timeout = deca_sec;
    return RP_HW_OK;
}

uint8_t uart_GetTimeout()
{
    return g_timeout;
}

int uart_SetSettings()
{
    if (uart_fd != -1)
    {
        /* Set baud rate - default set to 9600Hz */

        int mode = 0;
        switch (g_bit_size)
        {
        case RP_UART_CS6:
        {
            mode = CS6;
            break;
        }
        case RP_UART_CS7:
        {
            mode = CS7;
            break;
        }
        case RP_UART_CS8:
        {
            mode = CS8;
            break;
        }
        default:
            return RP_HW_ESU;
        }

        switch (g_parity)
        {
        case RP_UART_NONE:
        {
            g_settings.c_cflag &= ~(PARENB | PARODD | CMSPAR);
            break;
        }
        case RP_UART_EVEN:
        {
            g_settings.c_cflag &= ~(PARODD | CMSPAR);
            g_settings.c_cflag |= PARENB;
            break;
        }
        case RP_UART_ODD:
        {
            g_settings.c_cflag &= ~CMSPAR;
            g_settings.c_cflag |= PARENB | PARODD;
            break;
        }
        case RP_UART_SPACE:
        {
            g_settings.c_cflag |= PARENB | PARODD | CMSPAR;
            break;
        }
        case RP_UART_MARK:
        {
            g_settings.c_cflag &= ~PARODD;
            g_settings.c_cflag |= PARENB | CMSPAR;
            break;
        }

        default:
            return RP_HW_ESU;
        }

        g_settings.c_cflag &= ~CSTOPB;
        if (g_stop_bit == RP_UART_STOP2)
            g_settings.c_cflag |= CSTOPB; /* 8 bits */

        g_settings.c_cflag &= ~CSIZE;
        g_settings.c_cflag |= mode | CLOCAL | CREAD;           /* 8 bits */
        g_settings.c_cflag &= ~CRTSCTS;                        // Disable flow control
        g_settings.c_iflag &= ~(IXON | IXOFF | IXANY);         // Disable XON/XOFF flow control both input & output
        g_settings.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG); // Non Cannonical mode
        g_settings.c_iflag &= ~ICRNL;
        g_settings.c_oflag &= ~OPOST; /* raw output */

        g_settings.c_lflag = 0;             //  enable raw input instead of canonical,
        g_settings.c_cc[VMIN] = 0;          // Read at least 1 character
        g_settings.c_cc[VTIME] = g_timeout; // Wait indefinetly

        /* Baud rate fuctions
         * cfsetospeed - Set output speed
         * cfsetispeed - Set input speed
         * cfsetspeed  - Set both output and input speed */

        cfsetspeed(&g_settings, g_baud_rate);

        /* Setting attributes */
        tcsetattr(uart_fd, TCSANOW, &g_settings);

        tcflush(uart_fd, TCIFLUSH);
        tcflush(uart_fd, TCIOFLUSH);

        usleep(500000); // 0.5 sec delay

        return RP_HW_OK;
    }
    else
    {
        ERROR_LOG("Failed setup settings to UART.");
        return RP_HW_EIU;
    }
}

int uart_read(unsigned char *_buffer, int *size)
{

    if (_buffer == NULL || *size <= 0)
    {
        ERROR_LOG("Failed read from UART. Buffer is null");
        return RP_HW_EIPV;
    }

    if (uart_fd != -1)
    {
        while (1)
        {
            if (uart_fd == -1)
            {
                ERROR_LOG("Failed to read from UART. UART is closed.");
                return RP_HW_ERU;
            }
            errno = 0;
            int rx_length = read(uart_fd, (void *)_buffer, *size);
            if (rx_length < 0)
            {
                /* No data yet avaliable, check again */
                if (errno == EAGAIN)
                {
                    continue;
                    /* Error differs */
                }
                else
                {
                    ERROR_LOG("Error read from UART. Errno: %d", errno);
                    return RP_HW_ERU;
                }
            }
            else if (rx_length == 0)
            {
                if (g_timeout)
                {
                    return RP_HW_EUTO;
                }
                // Nothing to do
            }
            else
            {
                *size = rx_length;
                /* Data get */
                break;
            }
        }
        return RP_HW_OK;
    }
    else
    {
        ERROR_LOG("Failed read from UART.");
        return RP_HW_EIU;
    }
}

int uart_write(unsigned char *_buffer, int size)
{
    int count = 0;
    if (_buffer == NULL || size <= 0)
    {
        ERROR_LOG("Failed write to UART.");
        return RP_HW_EIPV;
    }

    if (uart_fd != -1)
    {
        /* Write some sample data into UART */
        /* ----- TX BYTES ----- */
        if (uart_fd != -1)
        {
            count = write(uart_fd, _buffer, size);
            TRACE_SHORT("Write %d", count)
        }
        else
        {
            ERROR_LOG("Failed write to UART.");
            return RP_HW_EIU;
        }

        if (count < 0)
        {
            ERROR_LOG("Failed write to UART.");
            return RP_HW_EWU;
        }

        return RP_HW_OK;
    }
    else
    {
        ERROR_LOG("Failed write to UART.");
        return RP_HW_EWU;
    }
    return 0;
}

int uart_Release()
{
    if (uart_fd != -1)
    {
        tcflush(uart_fd, TCIFLUSH);
        close(uart_fd);
        uart_fd = -1;
    }
    return RP_HW_OK;
}

int uart_SetSpeed(int _speed)
{
    g_baud_rate = uart_GetSpeedType(_speed);
    return RP_HW_OK;
}

int uart_GetSpeed()
{
    return uart_ConvertSpeed(g_baud_rate);
}

int uart_SetBits(rp_uart_bits_size_t _size)
{
    g_bit_size = _size;
    return RP_HW_OK;
}

rp_uart_bits_size_t uart_GetBits()
{
    return g_bit_size;
}

int uart_SetParityMode(rp_uart_parity_t mode)
{
    g_parity = mode;
    return RP_HW_OK;
    ;
}

rp_uart_parity_t uart_GetParityMode()
{
    return g_parity;
}

int uart_SetStopBits(rp_uart_stop_bits_t mode)
{
    g_stop_bit = mode;
    return RP_HW_OK;
}

rp_uart_stop_bits_t uart_GetStopBits()
{
    return g_stop_bit;
}

//  Baud rate:- B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B500000, B576000, B921600, B1000000, B1152000,
//  B1500000, B2000000, B2500000, B3000000, B3500000, B4000000

int uart_GetSpeedType(int _speed)
{
    switch (_speed)
    {
    case 1200:
        return B1200;
    case 2400:
        return B2400;
    case 4800:
        return B4800;
    case 9600:
        return B9600;
    case 19200:
        return B19200;
    case 38400:
        return B38400;
    case 57600:
        return B57600;
    case 115200:
        return B115200;
    case 230400:
        return B230400;
    case 576000:
        return B576000;
    case 921600:
        return B921600;
    case 1000000:
        return B1000000;
    case 1152000:
        return B1152000;
    case 1500000:
        return B1500000;
    case 2000000:
        return B2000000;
    case 2500000:
        return B2500000;
    case 3000000:
        return B3000000;
    case 3500000:
        return B3500000;
    case 4000000:
        return B4000000;
    }
    return -1;
}

int uart_ConvertSpeed(int _speed)
{
    switch (_speed)
    {
    case B1200:
        return 1200;
    case B2400:
        return 2400;
    case B4800:
        return 4800;
    case B9600:
        return 9600;
    case B19200:
        return 19200;
    case B38400:
        return 38400;
    case B57600:
        return 57600;
    case B115200:
        return 115200;
    case B230400:
        return 230400;
    case B576000:
        return 576000;
    case B921600:
        return 921600;
    case B1000000:
        return 1000000;
    case B1152000:
        return 1152000;
    case B1500000:
        return 1500000;
    case B2000000:
        return 2000000;
    case B2500000:
        return 2500000;
    case B3000000:
        return 3000000;
    case B3500000:
        return 3500000;
    case B4000000:
        return 4000000;
    }
    return -1;
}
