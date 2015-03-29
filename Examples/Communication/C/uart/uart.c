
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
#include <unistd.h>			//Used for UART
#include <fcntl.h>			//Used for UART
#include <termios.h>		//Used for UART
#include <errno.h>


/* Inline function definition */
static int uart_init();
static int release();
static int uart_read();
static int uart_write();

/* File descriptor definition */
int uart_filestream = -1;

void usage(void){

	char *usage = "Usage:\n\t-First parameter: Input message to be sent.\n";

    fprintf(stderr, "%s", usage);
}

static int uart_init(){

	uart_filestream = open("/dev/ttyPS1", O_RDWR | O_NOCTTY | O_NDELAY);

	if(uart_filestream == -1){
		fprintf(stderr, "Failed to open uart.\n");
		return -1;
	}

	struct termios settings;
	tcgetattr(uart_filestream, &settings);

	/*  CONFIGURE THE UART
	 *  The flags (defined in /usr/include/termios.h - see http://pubs.opengroup.org/onlinepubs/007908799/xsh/termios.h.html):
	 *	Baud rate:- B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B500000, B576000, B921600, B1000000, B1152000, B1500000, B2000000, B2500000, B3000000, B3500000, B4000000
	 *	CSIZE:- CS5, CS6, CS7, CS8
	 *	CLOCAL - Ignore modem status lines
	 * 	CREAD - Enable receiver
	 *	IGNPAR = Ignore characters with parity errors
	 *	ICRNL - Map CR to NL on input (Use for ASCII comms where you want to auto correct end of line characters - don't use for bianry comms!)
	 *	PARENB - Parity enable
	 *	PARODD - Odd parity (else even) */

	/* Set baud rate - default set to 9600Hz */
	speed_t baud_rate = B9600;

	/* Baud rate fuctions
	 * cfsetospeed - Set output speed
	 * cfsetispeed - Set input speed
	 * cfsetspeed  - Set both output and input speed */

	cfsetspeed(&settings, baud_rate);

	settings.c_cflag &= ~PARENB; /* no parity */
	settings.c_cflag &= ~CSTOPB; /* 1 stop bit */
	settings.c_cflag &= ~CSIZE;
	settings.c_cflag |= CS8 | CLOCAL; /* 8 bits */
	settings.c_lflag = ICANON; /* canonical mode */
	settings.c_oflag &= ~OPOST; /* raw output */
	settings.c_cc[VMIN] = 1;
	settings.c_cc[VTIME] = 0;
	
	/* Setting attributes */
	tcflush(uart_filestream, TCIFLUSH);
	tcsetattr(uart_filestream, TCSANOW, &settings);

	return 0;
}

static int uart_read(){

	/* Read some sample data from RX UART */
	
	/* Don't block serial read */
	fcntl(uart_filestream, F_SETFL, FNDELAY); 

	while(1){
		if(uart_filestream == -1){
			fprintf(stderr, "Failed to read from UART.\n");
			return -1;
		}

		unsigned char rx_buffer[256];

		int rx_length = read(uart_filestream, (void*)rx_buffer, 255);

		if (rx_length < 0){

			if(EAGAIN == errno){
				fprintf(stderr, "AGAIN!\n");
				break;
			}else{
				fprintf(stderr, "Error!\n");
				return -1;
			}

		}else if (rx_length == 0){
			fprintf(stderr, "No data waiting\n");

		}else{
			rx_buffer[rx_length] = '\0';
			printf("%i bytes read : %s\n", rx_length, rx_buffer);

		}
	}

	return 0;
}

static int uart_write(char *message){

	/* Write some sample data into UART */
	/* ----- TX BYTES ----- */
	int msg_len = strlen(message);

	int count = 0;
	char tx_buffer[msg_len+1];

	strncpy(tx_buffer, message, msg_len);
	tx_buffer[msg_len++] = 0x0a; //New line numerical value

	if(uart_filestream != -1){
		count = write(uart_filestream, &tx_buffer, (msg_len));
	}
	if(count < 0){
		fprintf(stderr, "UART TX error.\n");
		return -1;
	}
	
	return 0;
}

static int release(){

	tcflush(uart_filestream, TCIFLUSH);
	close(uart_filestream);

	return 0;
}

int main(int argc, char *argv[]){

	char *message = "Redpitaya uart test message.";

	/* uart init */
	if(uart_init() < 0){
		printf("Uart init error.\n");
		return -1;
	}

	/* Sample write */
	if(uart_write(message) < 0){
		printf("Uart write error\n");
		return -1;
	}
	if(uart_read() < 0){
		printf("Uart read error\n");
		return -1;
	}

	/* CLOSING UART */
	release();

	return 0;
}
