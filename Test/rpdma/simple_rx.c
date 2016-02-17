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
	printf("rpdma read test\n");

	fd = open("/dev/rpdma", O_RDWR);
	if (fd < 1) {
		printf("Unable to open device file");
		return -1;
	}
    //	write(fd,buf, 20);
    usleep(100);

    map = (unsigned char *)mmap(NULL,4*4*1024,PROT_READ | PROT_WRITE, MAP_SHARED,fd,0);

        if (map==NULL) {
            printf("Failed to mmap\n");
        if(fd)close(fd);
            return -1;
        }

         for(int unsigned l=0;l<4*4*1024;l++)
           map[l]=0;


    ioctl(fd,SIMPLE_RX,0);

	if(read(fd, buf, 1)<0){
           printf("read error\n");
	}else
		printf(".");

	for(int unsigned l=0;l<4*4*1024;l++){
        	printf("%c ",(char)map[l]);
	}
    	if(fd)close(fd);
	return 0;
}

