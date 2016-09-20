#include <i2c-dev.h>
#include <smbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
//#include <time.h>
#include <string.h>
//#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include <sys/ioctl.h>
//
//pca9555
//reg 0 1 input port reg read
//reg 2 3 output port reg write
//reg 4 5 polarity inversion reg write
//reg 6 7 configuration 1===in 0===out write
//

//0x20__________p13_____p12_____p03_____p02_____p04_____p05_____p06_____p07_____p17_____p16_____p10_____p11_____p15_____p14_____p01_____p00____
//		so1p	so1s	so2p	so2s	out1s	out1p	out2s	out2p	acdc1p	acdc1s  acdc2s	acdc2p	att1p	att1s	att2p	att2s
//_____________________________________________________________________________________________________________________________________________
//sdr mode		y		y		y		y
//_____________________________________________________________________________________________________________________________________________
//instrument	y		y		y		y
//_____________________________________________________________________________________________________________________________________________
//hv ch1													y
//_____________________________________________________________________________________________________________________________________________
//hv ch2															y
//_____________________________________________________________________________________________________________________________________________
//lv ch1														y
//_____________________________________________________________________________________________________________________________________________
//lv ch2																y
//_____________________________________________________________________________________________________________________________________________
//ac ch1									y
//_____________________________________________________________________________________________________________________________________________
//ac ch2											y
//_____________________________________________________________________________________________________________________________________________
//dc ch1										y
//_____________________________________________________________________________________________________________________________________________
//dc ch2												y
//_____________________________________________________________________________________________________________________________________________
//osc trigger
//_____________________________________________________________________________________________________________________________________________
//la in1
//_____________________________________________________________________________________________________________________________________________




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
        puts(	"  -1 [0-lv 1-hv] \n"
        	"  -2 [0-lv 1-hv] \n"
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

	file = open("/dev/i2c-8", O_RDWR);
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

	i2c_smbus_write_byte_data(file, 0x6, 0);
        printf("0x0f set for output\n");
	i2c_smbus_write_byte_data(file, 0x7, 0);
	printf("0xf0 set for output\n");

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
			printf("no argument break out of loop\n");
			break;
		}
		//t=atoi(optarg);
		switch (c) {

		case 's'://sdr
			//		O0.2=1  O0.3=0  O0.4=0  O0.5=1  O0.6=0  O0.7=1
			//		O1.2=1  O1.3=0
			data[0]&=~0x7e;
			data[1]&=~0x6;
			data[0]|=0b10100100;
			data[1]|=0b100;
			printf("SRD\n");
			        i2c_smbus_write_byte_data(file, 2, data[0]);
		        printf("data[0]=0x%x\n",data[0]);
       			usleep(8000);
        		i2c_smbus_write_byte_data(file, 2, 0);

        		i2c_smbus_write_byte_data(file, 3, data[1]);
        		printf("data[1]=0x%x\n",data[1]);
        		usleep(8000);
        		i2c_smbus_write_byte_data(file, 3, 0);

		break;
		case 'i'://instruments
			//		 O0.2=0  O0.3=1  O0.4=1  O0.5=0  O0.6=1  O0.7=0
			//		 O1.2=0  O1.3=1
			data[0]&=~0x7e;
                        data[1]&=~0x6;
                        data[0]|=0b1011000;
                        data[1]|=0b1000;
			printf("INSTRUMENTS\n");
        		i2c_smbus_write_byte_data(file, 2, data[0]);
        		printf("data[0]=0x%x\n",data[0]);
        		usleep(8000);
        		i2c_smbus_write_byte_data(file, 2, 0);

        		i2c_smbus_write_byte_data(file, 3, data[1]);
        		printf("data[1]=0x%x\n",data[1]);
        		usleep(8000);
        		i2c_smbus_write_byte_data(file, 3, 0);

		break;
		case '1':
			//LV:	 O1.4=1  O1.5=0
			t=atoi(optarg);
			if(t==0){
				data[1]&=~0x30;
				data[1]|=0x20;
				printf("CH1 LV\n");
        			i2c_smbus_write_byte_data(file, 3, data[1]);
        			printf("data[1]=0x%x\n",data[1]);
        			usleep(8000);
        			i2c_smbus_write_byte_data(file, 3, 0);
			}
			//HV:	 O1.4=0  O1.5=1
                        else if(t==1){
				data[1]&=~0x30;
				data[1]|=0x10;
				printf("CH1 HV\n");
        			i2c_smbus_write_byte_data(file, 3, data[1]);
        			printf("data[1]=%x\n",data[1]);
        			usleep(8000);
        			i2c_smbus_write_byte_data(file, 3, 0);

			}
			else {
				printf("error\n");
			}
		break;
		case '2':
			t=atoi(optarg);
			//LV:
			//		 O0.0=1  O0.1=0
			if(t==0){
                        	data[0]&=0x3;
				data[0]|=0x1;
                        	printf("CH2 LV\n");
        			i2c_smbus_write_byte_data(file, 2, data[0]);
        			printf("data[0]=0x%x\n",data[0]);
        			usleep(8000);
        			i2c_smbus_write_byte_data(file, 2, 0);

                        }
			//HV:
			//		 O0.0=0  O0.1=1
			else if(t==1){
                        	data[0]&=~0x3;
				data[0]|=0x2;
                        	printf("CH2 HV\n");
        			i2c_smbus_write_byte_data(file, 2, data[0]);
        			printf("data[0]=0x%x\n",data[0]);
        			usleep(8000);
        			i2c_smbus_write_byte_data(file, 2, 0);
                        }
                        else {
                                printf("error\n");
                        }
		break;
		default:
                        print_usage(argv[0]);
                break;

		}
		printf("loop\n");
	}

        if (file){
                close(file);
	}
}

int main(int argc, char *argv[]){

	parse_opts(argc, argv);

}
