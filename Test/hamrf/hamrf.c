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

uint8_t data[]={0,0};
int addr = 0x20;

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static void pabort(const char *s)
{
        perror(s);
        abort();
}

static void print_usage(const char *prog)
{
        printf("Usage: [-1:2:sih]\n");
        puts(	"  -1 [0-ac 1-dc 2-lv 3-hv] \n"
        	"  -2 [0-ac 1-dc 2-lv 3-hv] \n"
        	"  -s SDR\n"
        	"  -i INSTRUMENTS use with -1  and -2 \n"
		"  -h help\n" );
        exit(1);
}
//todo combine register values
static void parse_opts(int argc, char *argv[])
{
	int file;
	data[0]=0;
	data[1]=0;

	file = open("/dev/i2c-7", O_RDWR);
        if (file < 0) {
                //ERROR HANDLING;
                pabort("no i2c device file\n");
                exit(1);
                }
                //read gpio expander on 0x1a
        if (ioctl(file, I2C_SLAVE, addr) < 0) {
                //ERROR HANDLING;
                pabort("ioctl addr fail\n");
                exit(1);
        }
	i2c_smbus_write_byte_data(file, 0x2, 0);
	i2c_smbus_write_byte_data(file, 0x3, 0);
	i2c_smbus_write_byte_data(file, 0x6, 0);
	i2c_smbus_write_byte_data(file, 0x7, 0);

	while (1) {
		static const struct option lopts[] = {
			{ "sdr", 0, 0, 's' },
			{ "instuments", 0, 0, 'i' },
			{ "ch1", 1, 0, '1' },
			{ "ch2", 1, 0, '2' },
//			{ "help", 0, 0, 'h' },
			{ NULL, 0, 0, 0 },
		};

		char c,t;

		c = getopt_long(argc, argv, "si1:2:", lopts, NULL);

		if (c == -1){
			break;
		}
		//t=atoi(optarg);
		switch (c) {

			case 'i'://instruments
			//so1p
				i2c_smbus_write_byte_data(file, 2, 4);
				usleep(6000);
				i2c_smbus_write_byte_data(file, 2, 0);
			//so2p
				i2c_smbus_write_byte_data(file, 3, 0x40);
				usleep(6000);
				i2c_smbus_write_byte_data(file, 3, 0);

			break;
			case 's'://sdr
			//so1s
				i2c_smbus_write_byte_data(file, 2, 8);
				usleep(6000);
				i2c_smbus_write_byte_data(file, 2, 0);
			//so2s
				i2c_smbus_write_byte_data(file, 3, 0x80);
				usleep(6000);
				i2c_smbus_write_byte_data(file, 3, 0);
			break;
			case '1':
				//dc
				t=atoi(optarg);
				if(t==1){
					i2c_smbus_write_byte_data(file, 2, 0x80);
					usleep(6000);
					i2c_smbus_write_byte_data(file, 2, 0);
				}
				//ac
				else if(t==0){
					i2c_smbus_write_byte_data(file, 2, 0x40);
					usleep(6000);
					i2c_smbus_write_byte_data(file, 2, 0);

				}
				//LV
				else if(t==2){
					i2c_smbus_write_byte_data(file, 2, 0x10);
					usleep(6000);
					i2c_smbus_write_byte_data(file, 2, 0);
				}
				//HV
				else if(t==3){
					i2c_smbus_write_byte_data(file, 2, 0x20);
					usleep(6000);
					i2c_smbus_write_byte_data(file, 2, 0);
				}
				else {
					printf("error\n");
				}
			break;
			case '2':
				t=atoi(optarg);
				//dc
				if(t==1){
					i2c_smbus_write_byte_data(file, 3, 4);
					usleep(6000);
					i2c_smbus_write_byte_data(file, 3, 0);
				}
				//ac
				else if(t==0){
					i2c_smbus_write_byte_data(file, 3, 8);
					usleep(6000);
					i2c_smbus_write_byte_data(file, 3, 0);

				}
				//LV
				else if(t==2){

					i2c_smbus_write_byte_data(file, 3, 0x10);
					usleep(6000);
					i2c_smbus_write_byte_data(file, 3, 0);
				}
				//HV
				else if(t==3){
					i2c_smbus_write_byte_data(file, 3, 0x20);
					usleep(6000);
					i2c_smbus_write_byte_data(file, 3, 0);

				}
				else {
					printf("error\n");
				}
			break;
			default:
				print_usage(argv[0]);
			break;

		}
	}

        if (file){
                close(file);
	}
}

int main(int argc, char *argv[]){

	parse_opts(argc, argv);

}
