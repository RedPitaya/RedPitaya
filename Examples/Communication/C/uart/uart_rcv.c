

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>			//Used for UART
#include <fcntl.h>			//Used for UART
#include <termios.h>		//Used for UART
#include <errno.h>

int main(int argc, char *argv[]){

	int uart_filestream = -1;

	uart_filestream = open("/dev/ttyPS1",  O_RDWR | O_NOCTTY | O_NDELAY);
	
	if(uart_filestream == -1){
		fprintf(stderr, "Failed to open UART file stream.\n");
		return -1;
	}

	struct termios settings;
	tcgetattr(uart_filestream, &settings);
	speed_t baud_rate = B9600;

	if(cfsetispeed(&settings, baud_rate) == -1){
		fprintf(stderr, "Invalid baud. Check termios.h documentation for a valid baud rate.\n");
		return -1;
	}

	settings.c_cflag &= ~PARENB; /* no parity */
	settings.c_cflag &= ~CSTOPB; /* 1 stop bit */
	settings.c_cflag &= ~CSIZE;
	settings.c_cflag |= CS8 | CLOCAL; /* 8 bits */
	settings.c_lflag = ICANON; /* canonical mode */
	settings.c_oflag &= ~OPOST; /* raw output */

	/* Read some sample data from RX UART */
	//Don't block serial read
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
}
