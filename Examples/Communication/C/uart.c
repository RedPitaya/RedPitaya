
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


void usage(void){

	char *usage = "Usage:\n\t-First parameter: Input message to be sent.\n";

    fprintf(stderr, "%s", usage);
}


int main(int argc, char *argv[]){

	int uart_filestream = -1;

	uart_filestream = open("/dev/ttyPS1", O_RDWR | O_NOCTTY | O_NDELAY);

	if(uart_filestream == -1){
		fprintf(stderr, "Failed to open uart.\n");
		return -1;
	}

	struct termios settings;
	tcgetattr(uart_filestream, &settings);

	//CONFIGURE THE UART
	//The flags (defined in /usr/include/termios.h - see http://pubs.opengroup.org/onlinepubs/007908799/xsh/termios.h.html):
	//	Baud rate:- B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B500000, B576000, B921600, B1000000, B1152000, B1500000, B2000000, B2500000, B3000000, B3500000, B4000000
	//	CSIZE:- CS5, CS6, CS7, CS8
	//	CLOCAL - Ignore modem status lines
	//	CREAD - Enable receiver
	//	IGNPAR = Ignore characters with parity errors
	//	ICRNL - Map CR to NL on input (Use for ASCII comms where you want to auto correct end of line characters - don't use for bianry comms!)
	//	PARENB - Parity enable
	//	PARODD - Odd parity (else even)

	//Set baud rate

	//Default baud set to 9,6kHz
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
	
	//Setting attributes
	tcflush(uart_filestream, TCIFLUSH);
	tcsetattr(uart_filestream, TCSANOW, &settings);



	/* Write some sample data into UART */
	//----- TX BYTES -----
	if(argv[1]){

		int msg_len = strlen((char *)argv[1]);

		int count = 0;
		char tx_buffer[msg_len+1];

		strncpy(tx_buffer, argv[1], msg_len);

		//New line numerical value
		tx_buffer[msg_len++] = 0x0a;

		if(uart_filestream != -1){
			count = write(uart_filestream, &tx_buffer, (msg_len)); //Filestream, bytes to write, number of bytes to write
		}
		if(count < 0){
			fprintf(stderr, "UART TX error.\n");
		}
	}else{
		fprintf(stderr, "Missing first parameter.\n\n");
		usage();
	}

	/* CLOSING UART */
	tcflush(uart_filestream, TCIFLUSH);
	close(uart_filestream);

	return 0;
}
