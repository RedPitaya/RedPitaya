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
	printf("rpdma read test\n");

	fd = open("/dev/rprx", O_RDWR);
	if (fd < 1) {
		printf("Unable to open device file");
		return -1;
	}
	write(fd,buf, 20);
usleep(100000);
ioctl(fd,SIMPLE,0);

	if(read(fd, buf, 1)<0){
            printf("read error\n");
	}else
		printf(".");

	//ioctl(fd,STOP_TX,0);
	//ioctl(fd,STOP_RX,0);
/*
    ioctl(fd,SET_RX, SINGLE);
    ioctl(fd,START_RX,0);

    ioctl(fd,SET_TX, SINGLE);
    ioctl(fd,START_TX,0);
    
    
    usleep(1000000000);
    
    
    ioctl(fd,STOP_RX,0);
	ioctl(fd,STOP_TX,0);
*/
//ioctl(fd,SIMPLE,0);

	map = (unsigned char *)mmap(NULL,4*1024,PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    	if (map==NULL) {
        	printf("Failed to mmap\n");
        	if(fd)close(fd);
        	return -1;
    	}
int l;
	for(l=0;l<4*1024;l++)
		printf("%c ",(char)map[l]);


    if(fd)close(fd);
	return 0;
}

