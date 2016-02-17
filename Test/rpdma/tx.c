#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <pthread.h>

#include <unistd.h>

#define STOP_TX 0
#define STOP_RX 1
#define SET_TX 2
#define SET_RX 3
#define START_RX 4
#define START_TX 5
int fd;

int main(int argc, char *argv[])
{
	char buf[1024];
	fd = open("/dev/rpdma", O_RDWR);

	if (fd < 1) {
		printf("Unable to open device file");
		return -1;
	}

	write(fd,buf,(sizeof(buf) / sizeof(buf[0])));

	ioctl(fd,11,0);

	usleep(1000);
//stop
//	ioctl(fd,STOP_TX,0);

	if(fd)close(fd);
	return 0;
}
