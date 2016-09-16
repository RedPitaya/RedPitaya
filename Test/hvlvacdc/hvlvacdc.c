#include <linux/i2c-dev.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
//#include <time.h>
#include <string.h>
//#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
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
uint8_t dirty[2];

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static void pabort(const char *s)
{
        perror(s);
        abort();
}

static void print_usage(const char *prog)
{
        printf("Usage: [-1:2:sih]\n");
        puts(	"  -1 [0-lvac 1-lvdc 2-hvac 3-hvdc] \n"
        	"  -2 [0-lvac 1-lvdc 2-hvac 3-hvdc] \n"
        	"  -s SDR\n"
        	"  -i INSTRUMENTS use with -1  and -2 \n"
		"  -h help\n" );
        exit(1);
}
//todo combine register values
static void parse_opts(int argc, char *argv[])
{
	data[0]=0;
	data[1]=0;
	dirty[0]=0;
	dirty[1]=0;
	while (1) {
		static const struct option lopts[] = {
			{ "sdr", 0, 0, 's' },
			{ "instuments", 0, 0, 'i' },
			{ "ch1", 1, 0, '1' },
			{ "ch2", 1, 0, '2' },
			{ "help", 0, 0, 'h' },
			{ NULL, 0, 0, 0 },
		};

		char c,t;

		c = getopt_long(argc, argv, "si1:2:h", lopts, NULL);

		if (c == -1)
			break;

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
			dirty[0]=1;
			dirty[1]=1;

		break;
		case 'i'://instruments
			//		 O0.2=0  O0.3=1  O0.4=1  O0.5=0  O0.6=1  O0.7=0
			//		 O1.2=0  O1.3=1
			data[0]&=~0x7e;
                        data[1]&=~0x6;
                        data[0]|=0b1011000;
                        data[1]|=0b1000;
			printf("INSTRUMENTS\n");
			dirty[0]=1;
                        dirty[1]=1;
		break;
		case '1':
			//LV AC:	 O1.4=1  O1.5=0  O1.6=0  O1.7=1
			t=atoi(optarg);
			if(t==0){
				data[1]&=~0xf0;
				data[1]|=0x90;
				printf("CH1 LV AC\n");
	                        dirty[1]=1;

			}
			//LV DC:	 O1.4=1  O1.5=0  O1.6=1  O1.7=0
             		else if(t==1){
				data[1]&=~0xf0;
				data[1]|=0x50;
				printf("CH1 LV DC\n");
                                dirty[1]=1;
			}
			//HV AC:	 O1.4=0  O1.5=1  O1.6=0  O1.7=1
                        else if(t==2){
				data[1]&=~0xf0;
				data[1]|=0xa0;
				printf("CH1 HV AC\n");
                        	dirty[1]=1;
			}
			//HV DC:	 O1.4=0  O1.5=1  O1.6=1  O1.7=0
                        else if(t==3){
				data[1]&=~0xf0;
				data[1]|=0x60;
                        	dirty[1]=1;
				printf("CH1 HV DC");
			}
			else {
				printf("error\n");
			}
		break;
		case '2':
			t=atoi(optarg);
			//LV AC:
			//		 O0.0=1  O0.1=0
			//		 O1.0=1  O1.1=0
                        if(t==0){
                        	data[0]&=0x3;
                        	data[1]&=0x3;
				data[0]|=0x1;
                        	data[1]|=0x1;
                        	printf("CH2 LV AC\n");
                        	dirty[0]=1;
                        	dirty[1]=1;
                        }
			//LV DC:
			//		 O0.0=1  O0.1=0
			//		 O1.0=0  O1.1=1
                        else if(t==1){
                        	data[0]&=~0x3;
                        	data[1]&=~0x3;
				data[0]|=0x1;
                        	data[1]|=0x2;
                        	printf("CH2 LV DC\n");
                        	dirty[0]=1;
                        	dirty[1]=1;
                        }
			//HV AC:
			//		 O0.0=0  O0.1=1
			//		 O1.0=1  O1.1=0
                        else if(t==2){
                        	data[0]&=~0x3;
                        	data[1]&=~0x3;
				data[0]|=0x2;
                        	data[1]|=0x1;
                        	printf("CH2 HV AC\n");
                        	dirty[0]=1;
                        	dirty[1]=1;
                        }
			//HV DC:
			//		 O0.0=0  O0.1=1
			//		 O1.0=0  O1.1=1
                        else if(t==3){
                        	data[0]&=~0x3;
                        	data[1]&=~0x3;
				data[0]|=0x2;
                        	data[1]|=0x2;
                        	printf("CH2 HV DC\n");
                        	dirty[0]=1;
                        	dirty[1]=1;
                        }
                        else {
                                printf("error\n");
                        }
		break;
	/*	case 'h':
			print_usage();
		break;
*/
		default :
                        print_usage(argv[0]);
                        break;

		}
	}
}

int main(int argc, char *argv[]){
	int file;
	int adapter_nr = 1;

	char filename[20];
	parse_opts(argc, argv);

	snprintf(filename, 19, "/dev/i2c-%d", adapter_nr);

	file = open(filename, O_RDWR);
	if (file < 0) {
		//ERROR HANDLING;
		pabort("no i2c device file\n");
	   	exit(1);
	 	}
		//read gpio expander on 0x1a
	int addr = 0x20;
	if (ioctl(file, I2C_SLAVE, addr) < 0) {
    		//ERROR HANDLING;
		pabort("ioctl addr fail\n");
		exit(1);
  	}

	//set pins for output
	i2c_smbus_write_word_data(file, 0x6, 0);
	i2c_smbus_write_word_data(file, 0x7, 0);

	//send outputs to output registeres
       	if(dirty[0]){
		i2c_smbus_write_word_data(file, 2, data[0]);
		printf("data[0]=%x\n",data[0]);
		usleep(6000);
		i2c_smbus_write_word_data(file, 2, 0);
	}
	if(dirty[1]){
		i2c_smbus_write_word_data(file, 3, data[1]);
		printf("data[1]=%x\n",data[1]);
		usleep(6000);
		i2c_smbus_write_word_data(file, 3, 0);
	}



	if (file)
		close(file);
}
