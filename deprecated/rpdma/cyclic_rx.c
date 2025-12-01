#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

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


	ioctl(fd,CYCLIC_RX, 0);

int r;
for(r=0;r<4;r++){
		if(read(fd, buf, 1)<0){
            		printf("read error\n");
		}else{
	    		printf(".");

			map = (unsigned char *)mmap(NULL,4*4*1024,PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    			if (map==NULL) {
        			printf("Failed to mmap\n");
       			if(fd)close(fd);
       	 		return -1;
    			}
		int l;
        for (l=0;l<16*1024;l++){
        		printf("%x ",(char)map[l]);
//                map[l]=0;
		}
	}
}

usleep(10000);
 ioctl(fd,STOP_RX, 0);




    	if(fd)close(fd);
	return 0;
}
