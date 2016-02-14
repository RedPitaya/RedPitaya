#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
//ioctl commands
#define STOP_TX 0
#define STOP_RX 1
#define CYCLIC_TX 2
#define CYCLIC_RX 3
#define SINGLE_RX 10
#define SINGLE_TX 11
#define SIMPLE_RX 18
#define SIMPLE_TX 17
#define SIMPLE 12
#define SET_TX_SEGMENT_CNT 14
#define SET_TX_SEGMENT_SIZE 13
#define SET_RX_SEGMENT_CNT 16
#define SET_RX_SEGMENT_SIZE 15

#define RX_SGMNT_CNT 4
#define RX_SGMNT_SIZE 4*1024
#define TX_SGMNT_CNT 4
#define TX_SGMNT_SIZE  4*1024

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


	ioctl(fd,CYCLIC_RX, 0);

int r;
//for(r=0;r<4;r++)
while(1)
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

		}
	}


usleep(10000);
 ioctl(fd,STOP_RX, 0);




    	if(fd)close(fd);
	return 0;
}
