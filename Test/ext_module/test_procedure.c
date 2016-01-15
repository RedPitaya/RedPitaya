/**
 * $Id: $
 *
 * @brief Testing procedure for Sensor Extension module
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

#include "redpitaya/rp.h"

int main(int argc, char *argv[]){
    
    
   /* Print error, if rp_Init() function failed */
	if(rp_Init() != RP_OK){
		fprintf(stderr, "Rp api init failed!\n");
	}	
  
     
  
	 //CHEKC button state
	rp_DpinSetDirection(RP_DIO7_N, RP_IN);



    while(1){
    
	int global_c = 0, sum = 0;
	int check[12];
	for (int i = 0; i < 12; i++){
		check[i] = 0;
	}
	
	
    
	printf("Press button to start test procedure\n");

    while(1){

    usleep(50000);
    
    rp_pinState_t button;
    int count;

    rp_DpinGetState(RP_DIO7_N, &button);
 
    if(button==0){ if(++count>10)  

 	{ 
 		printf("Test Procedure is started\n");
 		break; 
 	} }  
 	else{ count=0;  } 
    }




	/* Set PIN value */
	rp_dpin_t pin;
	for(pin = RP_DIO1_P; pin <= RP_DIO1_N; pin++){
		rp_DpinSetDirection(pin, RP_IN);
	}

	rp_DpinSetDirection(RP_DIO0_P, RP_OUT);
	rp_DpinSetState(RP_DIO0_P, 1);
	usleep(1000000);
	rp_DpinSetState(RP_DIO0_P, 0);

	rp_ApinSetValue(RP_AOUT0, 0.5);
	rp_ApinSetValue(RP_AOUT1, 1);

	/* Get DIGITAL pin value */
	rp_dpin_t d_pin = RP_DIO1_P;
	while(d_pin++ < RP_DIO1_N){
		rp_pinState_t state;
		rp_DpinGetState(d_pin, &state);
		if((d_pin % 2 == 0) && state == 0){
			check[global_c++] = 1;
			sum++;
			continue;
		}else if((d_pin % 2 != 0) && state == 1){
			check[global_c++] = 1;
			sum++;
			continue;
		}
		global_c++;
	}

	rp_apin_t a_pin = RP_AOUT3;
	while(a_pin++ < RP_AIN3){
		float a_val;
		rp_ApinGetValue(a_pin, &a_val);
		if(a_pin % 2 == 0){
			if(a_val >= 0.45 && a_val < 0.55){
					check[global_c++] = 1;
					sum++;
					continue;
			}
			global_c++;
		}else if(a_pin % 2 != 0){
			if(a_val > 0.95 && a_val < 1.05){
					check[global_c++] = 1;
					sum++;
					continue;
			}
			global_c++;
		}
	}

	if(sum != 12.0){
		printf("Error on zero state pins...\n\n");
		printf("D2\tD3\tD4\tD5\tD6\tD7\tD8\tD9\tA0\tA1\tA2\tA3\n");
		printf("%.1d\t%.1d\t%.1d\t%.1d\t%.1d\t%.1d\t%.1d\t%.1d\t%.1d\t%.1d\t%.1d\t%.1d\n", 
			check[0], check[1], check[2], check[3], check[4], check[5], check[6], 
			check[7], check[8], check[9], check[10], check[11]);
		//return -1;
		usleep(1000000);
		
	}
	else{

	usleep(1000000);
	printf("Pass!\n");
	
	}
     
    
	//return 0;

    
}
}
