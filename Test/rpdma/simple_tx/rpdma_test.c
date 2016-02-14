#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <unistd.h>

//ioctl commands
#define STOP_TX 0
#define STOP_RX 1
#define SET_TX 2
#define SET_RX 3
#define START_RX 4
#define START_TX 5
#define SIMPLE 10
#define CYCLIC 0
#define SIMPLE_RX 10
#define SIMPLE_TX 11
int fd;
char buf[256];
unsigned char* map=NULL;


int main(int argc, char *argv[])
{
	int i;
	int dummy;

	printf("rpdma simple write test\n");

	fd = open("/dev/rpdma", O_RDWR);
	if (fd < 1) {
		printf("Unable to open device file");
		return -1;
	}
	write(fd,buf,16*1024 );
	usleep(10000);
	ioctl(fd,SIMPLE_TX,0);

	usleep(10000);

    if(fd)close(fd);
	return 0;
}

