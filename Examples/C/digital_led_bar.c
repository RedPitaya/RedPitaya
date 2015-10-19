#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <rp.h>

int main(int arc, char **argv){
    // Print error, if rp_Init() function failed
    if(rp_Init() != RP_OK){
        fprintf(stderr, "Rp api init failed!\n");
    }
    rp_dpin_t pin;

    float p = 67;

    // Turning on leds based on parameter p
    for(i = RP_LED0; i <= RP_LED7; i++){
        if(p > (i*(100/8)){
            rp_DpinSetState(pin, RP_HIGH);
        }else{
            rp_DpinSetState(pin, RP_LOW);
        }
    }

    // Releasing resources
    rp_Release();

    return 0;
}
