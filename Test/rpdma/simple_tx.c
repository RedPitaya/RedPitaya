#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <unistd.h>

#include "rpdma.h"

int fd;
char buf[256];
unsigned char* map=NULL;


int main(int argc, char *argv[])
{
	printf("rpdma simple write test\n");

	fd = open("/dev/rprx", O_RDWR);
	if (fd < 1) {
		printf("Unable to open device file");
		return -1;
	}
	write(fd,buf,16*1024 );
	usleep(10000);
//    ioctl(fd,14,4);
//    ioctl(fd,13,1024*4);
 //   usleep(10000);
	ioctl(fd,SINGLE_TX,0);

	usleep(100000);

    if(fd)close(fd);
	return 0;
}

