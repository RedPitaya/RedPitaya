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
char buf[256];
unsigned char* map=NULL;
int main(int argc, char *argv[])
{
	int i;
	int dummy;

	printf("rpdma read test\n");

	fd = open("/dev/rpdma", O_RDWR);

	if (fd < 1) {
		printf("Unable to open device file");
		return -1;
	}
	//prepair dma
	ioctl(fd,10,0);
	//whait a bit
	usleep(1000);

        if(read(fd, buf, 1)<0){
            printf("read error\n");
	}

	printf(".");
	map = (unsigned char *)mmap(NULL,4*1024,PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    	if (map==NULL) {
        	printf("Failed to mmap\n");
        	if(fd)close(fd);
        	return -1;
    	}
int l;
	for(l=0;l<4*1024;l++)
		printf("%c ",(char)map[l]);
	
        ioctl(fd,STOP_RX,0);

//	ioctl(fd,STOP_TX,0);        

if(fd)close(fd);
	return 0;
}
