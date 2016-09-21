#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
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



uint8_t data[2];//[]={0,0};
uint8_t dirty[2];//[]={0,0};

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static void pabort(const char *s)
{
	perror(s);
	abort();
}

static void print_usage(const char *prog)
{
        printf("Usage: %s [-4 -5 -6 -1 -2 -l -o -h -0]\n",prog);
        puts(	"  -4 led4 \n"
		"  -5 led5 \n"
		"  -6 led6 \n"
        	"  -1 gpio1 \n"
 		"  -2 gpio2 \n"
        	"  -l LA in1 \n"
        	"  -o Osciloscop trigger \n"
		"  -h help\n"
		"  -0 zero all \n");
        exit(1);
}
//todo combine register values
static void parse_opts(int argc, char *argv[])
{
	int file;
	dirty[0]=0;
	dirty[1]=0;

	file = open("/dev/i2c-8", O_RDWR);
        if (file < 0) {
                //ERROR HANDLING;
                pabort("no i2c device file\n");
                exit(1);
                }
                //read gpio expander on 0x1a
        if (ioctl(file, I2C_SLAVE, 0x21) < 0) {
                //ERROR HANDLING;
                pabort("ioctl addr fail\n");
                exit(1);
        }

        i2c_smbus_write_byte_data(file, 0x6, 0);
        printf("0x0f set for output\n");
        i2c_smbus_write_byte_data(file, 0x7, 0);
        printf("0xf0 set for output\n");

	while (1) {
		static const struct option lopts[] = {
			{ "led4", 0, 0, '4' },
			{ "led5", 0, 0, '5' },
			{ "led6", 0, 0, '6' },
			{ "gpio1", 0, 0, '1' },
			{ "gpio2", 0, 0, '2' },
			{ "la", 0, 0, 'l' },
			{ "osc", 0, 0, 'o' },
			{ "help", 0, 0, 'h' },
			{ "00", 0, 0, '0' },
			{ NULL, 0, 0, 0 },
		};

		char c;

		c = getopt_long(argc, argv, "456120loh", lopts, NULL);

		if (c == -1)
			break;
		switch (c) {

		case '4':// o0.0=1
			data[0]&=~0x1;
			data[0]|=0x1;
			printf("led4\n");
			i2c_smbus_write_byte_data(file, 2, data[0]);
        		printf("data[0]=0x%x\n",data[0]);

			break;
		case '5':// o0.1=1
			data[0]&=~0x2;
                        data[0]|=0x2;
			printf("led5\n");
                        i2c_smbus_write_byte_data(file, 2, data[0]);
                        printf("data[0]=0x%x\n",data[0]);

			break;
		case '6':// o0.2=1
                        data[0]&=~0x4;
                        data[0]|=0x4;
			printf("led6\n");
                        i2c_smbus_write_byte_data(file, 2, data[0]);
                        printf("data[0]=0x%x\n",data[0]);

			break;
		case '1':// o1.2=1
                        data[1]&=~0x4;
                        data[1]|=0x4;
                        printf("gpio1\n");
                        i2c_smbus_write_byte_data(file, 3, data[1]);
                        printf("data[0]=0x%x\n",data[0]);

			break;
                case '2':// o1.3=1
                        data[1]&=~0x8;
                        data[1]|=0x8;
                        printf("gpio2\n");
                        i2c_smbus_write_byte_data(file, 3, data[1]);
                        printf("data[0]=0x%x\n",data[0]);

                	break;
                case '0':// o1.3=1
			data[0]=0x0;
                        data[1]=0x0;
                        printf("gpio2\n");
                        i2c_smbus_write_byte_data(file, 2, data[0]);
			i2c_smbus_write_byte_data(file, 3, data[1]);
                        break;

                case 'l':// o1.1=0 o1.0=1
                        data[1]&=~0x3;
                        data[1]|=0x1;
                        printf("LA\n");

			//send outputs to output registeres
        		i2c_smbus_write_byte_data(file, 3, data[1]);
        		printf("data[1]=0x%x\n",data[1]);
        		usleep(8000);
        		i2c_smbus_write_byte_data(file, 3, 0);
                	break;
                case 'o':// o1.1=1 o1.o=0
                        data[1]&=~0x3;
                        data[1]|=0x2;
                        printf("OSC. tr.\n");

		        //send outputs to output registeres
        		i2c_smbus_write_byte_data(file, 3, data[1]);
        		printf("data[1]=0x%x\n",data[1]);
        		usleep(8000);
        		i2c_smbus_write_byte_data(file, 3, 0);
        	        break;

		default :
			print_usage(argv[0]);
			break;
		}
	}

        if (file){
                close(file);
	}
}

int main(int argc, char *argv[]){
	int file;

	parse_opts(argc, argv);

	file = open("/dev/i2c-8", O_RDWR);
	if (file < 0) {
		//ERROR HANDLING;
		pabort("no i2c device file\n");
	   	exit(1);
	 	}
		//read gpio expander on 0x20

	if (ioctl(file, I2C_SLAVE, 0x21) < 0) {
    		//ERROR HANDLING;
		pabort("ioctl addr fail\n");
		exit(1);
  	}

	//send outputs to output registeres
	i2c_smbus_write_byte_data(file, 2, data[0]);
	printf("data[0]=0x%x\n",data[0]);
	usleep(8000);
	i2c_smbus_write_byte_data(file, 2, 0);
}
