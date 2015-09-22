/**
 * $Id: $
 *
 * @brief Red Pitaya library Generate module interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
*/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
 
#include "rp.h"
#include "fpga_api.h"


int fpga_load(char *fpga_file){

	int fo, fi;
    int fpga_size;
    struct stat st;

    /* Get FPGA size */
    stat(fpga_file, &st);
    fpga_size = st.st_size;
    char fi_buff[fpga_size];

    fo = open("/dev/xdevcfg", O_WRONLY);
    if(fo < 0){
    	printf("Error opening output file.\n");
    	return RP_EOOR;
    }

    fi = open(fpga_file, O_RDONLY);
    if(fi < 0){
    	printf("Error opening input file.\n");
    	return RP_EOOR;
    }

    if(read(fi, &fi_buff, fpga_size) < 0){
    	printf("Unable to read input file.\n");
    	return RP_EOOR;
    }

    if(write(fo, &fi_buff, fpga_size) < 0){
    	printf("Unable to write to output file.\n");
    	return RP_EOOR;
    }

    /* Closing resource files */
    close(fo);
    close(fi);

	return RP_OK;
}
