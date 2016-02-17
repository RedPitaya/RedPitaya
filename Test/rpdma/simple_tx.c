#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <unistd.h>

#include "rpdma.h"

int main(int argc, char *argv[]) {
    char buf[256];
    int fd;

    // set TX stream size
//    SET_TX_SGMNT_CNT

	printf("rpdma simple write test\n");

	fd = open("/dev/rpdma", O_RDWR);
	if (fd < 1) {
		printf("Unable to open device file");
		return -1;
	}
	write(fd,buf,16*1024);
	usleep(10000);
	ioctl(fd,SIMPLE_TX,0);

	usleep(10000);

    if(fd)close(fd);
	return 0;
}

