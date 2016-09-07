#include <linux/i2c-dev.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>


//
//pca9557
//reg 0 input port reg read
//reg 1 output port reg write
//reg 2 polarity inversion reg write
//reg 3 configuration 1==in 0==out write
//
int main(int argc, char *argv[]){
	int file;
	int adapter_nr = 0; // probably dynamically determined at boot but lets asume that it is 0 
//	if(argc==2){
//		adapter_nr=atoi(argv[1]);
//	}
	char filename[20];
	for(;adapter_nr<10;adapter_nr++)
	{
		snprintf(filename, 19, "/dev/i2c-%d", adapter_nr);
		file = open(filename, O_RDWR);
		if (file < 0) {
			//ERROR HANDLING;
	    		//exit(1);
			break;
	  	}

		//read gpio expander on 0x1a
		int addr = 0x1a;
		if (ioctl(file, I2C_SLAVE, addr) < 0) {
	    		//ERROR HANDLING;
			exit(1);
	  	}
		// 0x3; //Device control register
		int32_t res;
		//	uint8_t i;
		//	char buf[10];
		//set pins for input
		i2c_smbus_write_word_data(file, 0x3, 0xff);

	       	//read input register
	       	res = i2c_smbus_read_word_data(file, 0);
	       	if (res < 0) {
	               // ERROR HANDLING: i2c transaction failed
		printf("Error reading pins from i2c gpio expander\n");
	       	} else {
	       	        // if 0x14 then add overlay for i2c mux
			switch(res){
				case 0x0a://if 0x0a powerm hamlab 
					printf("HAMLAB PWRM %s %d \n",filename,res);
					//insert adt7470 0x5c overlay or somehow enable this driver 
					//there is no compatible line inside this driver, so it will probably allways stay in kernel, just find a way to set temperature

				break;
				case 0x0b:// if 0x0b powerm eelab  
					//insert adt7470 0x5c overlay or somehow enable this driver
				break;
				case 0x14://if 0x14 hamlab motherboard with i2cmux 2x pca9555 0x200x21
					printf("HAMLAB EXTM %s %d \n",filename,res);
					//insert overlay for i2c mux
					system("bash hamlab_i2cmux.sh");
					printf("i2c expander overlay inserted\n");
					//set init state of relays
					system("../hvlvacdc/main -i -1 1 -2 1");
					system("../laosc/main -o");
				break;
				case 0x15:// if 0x15 eelab motherboard with i2cmux 2x pca9555 0x20 0x21
					//insert overlay for i2c mux
					//set init state of relays
				break;
				case 0x32:// if 0x32 eelab dmm	
					//gpios maybe
				break;
				case 0x50:// if 0x50 eelab bench power supplay
				break;
				case 0x80:// if 0x80 0.8m-10m filters
					printf("HAMLAB C25 %s %d \n",filename,res);
				break;
				case 0x94:// if 0x94 preselector
					printf("HAMLAB C25 %s %d \n",filename,res);
				break;
				case 0xc8:// if 0xc8 power probe
				break;
				default:
					printf("\n");
			}
		}
		if (file) close(file);
	}
}
