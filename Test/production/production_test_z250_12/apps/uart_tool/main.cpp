#include <errno.h>
#include <fcntl.h> 
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int
set_interface_attribs (int fd, int speed, int parity)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                fprintf (stderr, "error %d from tcgetattr", errno);
                return -1;
        }

        cfsetospeed (&tty, speed);
        cfsetispeed (&tty, speed);

        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
        // disable IGNBRK for mismatched speed tests; otherwise receive break
        // as \000 chars
        tty.c_iflag &= ~IGNBRK;         // disable break processing
        tty.c_lflag = 0;                // no signaling chars, no echo,
                                        // no canonical processing
        tty.c_oflag = 0;                // no remapping, no delays
        tty.c_cc[VMIN]  = 0;            // read doesn't block
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

        tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                        // enable reading
        tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
        tty.c_cflag |= parity;
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CRTSCTS;

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
        {
                fprintf (stderr, "error %d from tcsetattr", errno);
                return -1;
        }
        return 0;
}

void
set_blocking (int fd, int should_block)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                fprintf (stderr, "error %d from tggetattr", errno);
                return;
        }

        tty.c_cc[VMIN]  = should_block ? 1 : 0;
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
                fprintf (stderr, "error %d setting term attributes", errno);
}

/* use omega UART1 */
const char *portname = "/dev/ttyPS1";

int uartFd = -1; 

void uart_writestr(const char* string) {
	write(uartFd, string, strlen(string));
}	

void uart_write(void* data, size_t len) {
	write(uartFd, data, len); 
}

ssize_t uart_read(void* buffer, size_t charsToRead) {
	return read(uartFd, buffer, charsToRead); 
}

int uart_open(const char* port, int baud, int blocking) {
	uartFd = open (port, O_RDWR | O_NOCTTY | O_SYNC);
	if (uartFd < 0)
	{
			fprintf (stderr, "error %d opening %s: %s", errno, port, strerror (errno));
			return -1;
	}
	set_interface_attribs (uartFd, baud, 0);  // set speed, 8n1 (no parity)
	set_blocking (uartFd, blocking); //set blocking mode
	//printf("Port %s opened.\n", port); 
	return 1;
}

int main(int argc, char* argv[] ) {

    if (argc  < 2){
        printf("Missing command for test board\n");
        return -1;
    } 

    bool showresult = !(argc > 2 && strcmp(argv[2], "-s")==0);
    

	if(!uart_open(portname, B115200, 1)) 
		return -1;
    char str[255];
    sprintf(str,"%s\r\n",argv[1]);
    uart_writestr(str);
    usleep(100 * 1000);    
    char buff[255];
    memset(buff,0,255);
    uart_read(buff,255);
    if (showresult)
     printf("%s",buff);
	
	return 0;
}