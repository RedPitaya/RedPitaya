/* This is a red pitaya test application for deep averagning */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>

#include "rp.h"

int main(int argc, char **argv){

    uint64_t dna;

    if(rp_Init() != RP_OK){
    	fprintf(stderr, "Rp api init failed!\n");
    }

    rp_IdGetDNA (&dna);
    printf("DNA: 0x%016llx\n", dna);
}
