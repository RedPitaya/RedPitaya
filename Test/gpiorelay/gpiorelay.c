#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>


/*
static void pabort(const char *s)
{
        perror(s);
        abort();
}
*/
static void print_usage(const char *prog)
{
        printf("Usage: [-silt]\n");
        puts(
        	"  -s SDR transmittion\n"
        	"  -i INSTRUMENT generators\n"
        	"  -l LA in1 \n"
        	"  -t External Osciloscop Trigger \n"
	);
        exit(1);
}
//todo combine register values
static void parse_opts(int argc, char *argv[])
{
	int file;

	char c;
	int ret=-1;
	while (1) {
		static const struct option lopts[] = {
			{ "sdr", 0, 0, 's' },
			{ "instuments", 0, 0, 'i' },
			{ "logic", 0, 0, 'l' },
			{ "external osc trigger", 0, 0, 't' },
			{ NULL, 0, 0, 0 },
		};

	

		c = getopt_long(argc, argv, "sitl", lopts, NULL);

		if (c == -1){
			break;
		}

		switch (c) {

			case 's'://sdr
			
			/*file = open("/sys/class/leds/HAMLAB_o1sdr/brightness", O_RDWR);
			if (file < 0) {
				//ERROR HANDLING;
				pabort("no led device file\n");
				exit(1);
			}
			write(file,"1\n",1);
			usleep(6000);
			write(file,"0\n",1);
			close(file);
			
			file = open("/sys/class/leds/HAMLAB_o2sdr/brightness", O_RDWR);
			if (file < 0) {
				//ERROR HANDLING;
				pabort("no led device file\n");
				exit(1);
			}
			write(file,"1\n",1);
			usleep(6000);
			write(file,"0\n",1);
			close(file);*/
			
				ret=system("echo 1 > /sys/class/leds/HAMLAB_o1sdr/brightness");
				usleep(6000);
				ret=system("echo 0 > /sys/class/leds/HAMLAB_o1sdr/brightness");
				
				ret=system("echo 1 > /sys/class/leds/HAMLAB_o2sdr/brightness");
				usleep(6000);
				ret=system("echo 0 > /sys/class/leds/HAMLAB_o2sdr/brightness");
				printf("s");

			break;
			case 'i'://instruments
			/*	file = open("/sys/class/leds/HAMLAB_o1gen/brightness", O_RDWR);
				if (file < 0) {
					//ERROR HANDLING;
					pabort("no led device file\n");
					exit(1);
				}
				write(file,"1\n",1);
				usleep(6000);
				write(file,"0\n",1);
				close(file);
			
				file = open("/sys/class/leds/HAMLAB_o2gen/brightness", O_RDWR);
				if (file < 0) {
					//ERROR HANDLING;
					pabort("no led device file\n");
					exit(1);
				}
				write(file,"1\n",1);
				usleep(6000);
				write(file,"0\n",1);
				close(file);*/
				ret=system("echo 1 > /sys/class/leds/HAMLAB_o1gen/brightness");
				usleep(6000);
				ret=system("echo 0 > /sys/class/leds/HAMLAB_o1gen/brightness");
				
				ret=system("echo 1 > /sys/class/leds/HAMLAB_o2gen/brightness");
				usleep(6000);
				ret=system("echo 0 > /sys/class/leds/HAMLAB_o2gen/brightness");
				printf("i");

			break;
			case 't': //triger
				/*file = open("/sys/class/leds/HAMLAB_trig/brightness", O_RDWR);
				if (file < 0) {
					//ERROR HANDLING;
					pabort("no led device file\n");
					exit(1);
				}
				write(file,"1\n",1);
				usleep(6000);
				write(file,"0\n",1);
				close(file);
			*/
			
				ret=system("echo 1 > /sys/class/leds/HAMLAB_trig/brightness");
				usleep(6000);
				ret=system("echo 0 > /sys/class/leds/HAMLAB_trig/brightness");
				printf("t");

	
			break;
			case 'l':
			
				/*file = open("/sys/class/leds/HAMLAB_logic/brightness", O_RDWR);
				if (file < 0) {
					//ERROR HANDLING;
					pabort("no led device file\n");
					exit(1);
				}
				write(file,"1\n",1);
				usleep(6000);
				write(file,"0\n",1);
				close(file);*/
				ret=system("echo 1 > /sys/class/leds/HAMLAB_logic/brightness");
				usleep(6000);
				ret=system("echo 0 > /sys/class/leds/HAMLAB_logic/brightness");
				printf("l");

			break;
			default:
				print_usage(argv[0]);
			break;

		}
		
	}
	if (ret)printf("something went wrong with leds\n");
        if (file){
                close(file);
	}
}

int main(int argc, char *argv[]){

	parse_opts(argc, argv);

}
