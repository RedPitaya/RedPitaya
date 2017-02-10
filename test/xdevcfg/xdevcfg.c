#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

int main(int argc, char *argv[]){
	if (mkfifo("/dev/xdevcfg", S_IRUSR | S_IWUSR | S_IWGRP) == -1 && errno != EEXIST)
		;//return -EEXIST;//errExit("mkfifo %s", "/dev/xdevcfg");
	int c;
	while(1){
		FILE *reader,* writer;
		reader = fopen("/dev/xdevcfg", "r");
		writer = fopen("/lib/firmware/redpitaya/fpga.bit","w");
		if (reader && writer) {
    			while ((c = getc(reader)) != EOF)
        			fputc(c,writer);
    			fclose(reader);
			fclose(writer);
		}
		fflush(writer);
		system("python fpga-bit-to-bin.py -f /lib/firmware/redpitaya/fpga.bit /lib/firmware/redpitaya/fpga.bin");
		system("rmdir /sys/kernel/config/device-tree/overlays/fpga");
		system("mkdir /sys/kernel/config/device-tree/overlays/fpga");
		system("cat fpga.dtbo > /sys/kernel/config/device-tree/overlays/fpga/dtbo");
	}
	return 0;	
}
