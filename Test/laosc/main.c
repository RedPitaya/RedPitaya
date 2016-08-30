#include <linux/i2c-dev.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
//
//pca9555
//reg 0 1 input port reg read
//reg 2 3 output port reg write
//reg 4 5 polarity inversion reg write
//reg 6 7 configuration 1===in 0===out write
//
//0x21   	o11_____o10_____o12_____o13_____o00_____o01_____o02
////	  	trigp	trigs   gpio0   gpio1   led4    led5    led6
//____________________________________________________________________
//osc		y
//____________________________________________________________________
//la			  y
//____________________________________________________________________



uint8_t data[]={0,0};
uint8_t dirty[]={0,0};

static void print_usage(const char *prog)
{
        printf("Usage: %s [-4 -5 -6 -1 -2 -l -o]\n", prog);
        puts(	"  -led4 \n"
		"  -led5 \n"
		"  -led6 \n"
        	"  -gpio1 \n"
 		"  -gpio2 \n"
        	"  -LA in1 \n"
        	"  -Osciloscop trigger \n"
		);
        exit(1);
}
//todo combine register values
static void parse_opts(int argc, char *argv[])
{
	dirty[0]=0;
	dirty[1]=0;
	while (1) {
		static const struct option lopts[] = {
			{ "led4", 0, 0, '4' },
			{ "led5", 0, 0, '5' },
			{ "led6", 0, 0, '6' },
			{ "gpio1", 0, 0, '1' },
			{ "gpio2", 0, 0, '2' },
			{ "la", 0, 0, 'l' },
			{ "osc", 0, 0, 'o' },
			{ NULL, 0, 0, 0 },
		};

		int c/*,t*/;

		c = getopt_long(argc, argv, "45612lo", lopts, NULL);

		if (c == -1)
			break;
	//	t=atoi(optarg);
		switch (c) {

		case '4':// o0.0=1
			data[0]&=~0x1;
			data[0]|=0x1;
			printf("led4\n");
			dirty[0]=1;


		break;
		case '5':// o0.1=1
			data[0]&=~0x2;
                        data[0]|=0x2;
			printf("led5\n");
			dirty[0]=1;
		break;
		case '6':// o0.2=1
                        data[0]&=~0x4;
                        data[0]|=0x4;
			printf("led6\n");
                        dirty[0]=1;
		break;
		case '1':// o1.2=1 
                        data[1]&=~0x4;
                        data[1]|=0x4;
                        printf("gpio1\n");
                        dirty[1]=1;
		break;
                case '2':// o1.3=1
                        data[1]&=~0x8;
                        data[1]|=0x8;
                        printf("gpio2\n");
                        dirty[1]=1;

                break;
                case 'l':// o1.1=0 o1.0=1
                        data[1]&=~0x3;
                        data[1]|=0x1;
                        printf("LA\n");
                        dirty[1]=1;

                break;
                case 'o':// o1.1=1 o1.o=0
                        data[1]&=~0x3;
                        data[1]|=0x2;
                        printf("OSC. tr.\n");
                        dirty[1]=1;

                break;

		default:
			print_usage(argv[0]);
		break;
		}
	}
}

int main(int argc, char *argv[]){
	int file;
	int adapter_nr = 1;

	char filename[20];
	snprintf(filename, 19, "/dev/i2c-%d", adapter_nr);
	file = open(filename, O_RDWR);
	if (file < 0) {
		//ERROR HANDLING;
	   	exit(1);
	 	}
		//read gpio expander on 0x1a
	int addr = 0x20;
	if (ioctl(file, I2C_SLAVE, addr) < 0) {
    		//ERROR HANDLING;
		exit(1);
  	}

//	int32_t res;

	parse_opts(argc, argv);

	//set pins for input
	i2c_smbus_write_word_data(file, 0x6, 0);
	i2c_smbus_write_word_data(file, 0x7, 0);

	parse_opts(argc, argv);

	//send outputs to output registeres
       	if(dirty[0]){
		i2c_smbus_write_word_data(file, 2, data[0]);
		printf("data[0]=%x\n",data[0]);
}
	if(dirty[1]){
		i2c_smbus_write_word_data(file, 3, data[1]);
		printf("data[1]=%x\n",data[1]);
}
	if (file)
		close(file);
}
