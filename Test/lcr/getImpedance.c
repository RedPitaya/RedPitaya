

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "redpitaya/rp.h"

int main(int argc, char **argv){

        /* Print error, if rp_Init() function failed */
        if(rp_Init() != RP_OK){
                fprintf(stderr, "Rp api init failed!\n");
        }

        
        int selected_frequency=1000.0;
        int min_periodes = 8;
        int signal_decimation;
        float wait;

        if      (selected_frequency == 100000.0)            {  wait = 200;    }     
        else if (selected_frequency == 10000.0)             {  wait = 2000;   }    
        else if (selected_frequency == 1000.0)              {  wait = 20000;  }   
        else if (selected_frequency == 100.0)               {  wait = 200000; }

        /*Generate excitation signal*/
        rp_GenReset();
        rp_GenFreq(RP_CH_2, selected_frequency);
        rp_GenAmp(RP_CH_2, 0.2);
        rp_GenOffset(RP_CH_2, 0.25);
        rp_GenWaveform(RP_CH_2, RP_WAVEFORM_SINE);
        rp_GenOutEnable(RP_CH_2);
        usleep(100000);
        
        rp_AcqReset();

        if      (selected_frequency == 100000.0)       {     rp_AcqSetDecimation(RP_DEC_1);     signal_decimation = 1;    }
        else if (selected_frequency == 10000.0)        {     rp_AcqSetDecimation(RP_DEC_8);     signal_decimation = 8;    }
        else if (selected_frequency == 1000.0)         {     rp_AcqSetDecimation(RP_DEC_64);    signal_decimation = 64;   }
        else if (selected_frequency == 100.0)          {     rp_AcqSetDecimation(RP_DEC_1024);  signal_decimation = 1024; }

        uint32_t buff_size = ((min_periodes*125e6)/(selected_frequency*signal_decimation));
        float *buff_in1 = (float *)malloc(buff_size * sizeof(float));
        float *buff_in2 = (float *)malloc(buff_size * sizeof(float));
        
        rp_AcqSetTriggerLevel(0.5);  
        rp_AcqSetTriggerDelay(8192);
        rp_AcqStart();        
        

        rp_AcqSetTriggerSrc(RP_TRIG_SRC_CHA_PE);
        rp_acq_trig_state_t state = RP_TRIG_STATE_TRIGGERED;

        while(1){
                rp_AcqGetTriggerState(&state);
                if(state == RP_TRIG_STATE_TRIGGERED){
                break;
                }
        }
                       
        usleep(wait); 
        rp_AcqGetOldestDataV(RP_CH_1, &buff_size, buff_in1);
        rp_AcqGetOldestDataV(RP_CH_2, &buff_size, buff_in2);
        

        double *U_dut = (double *) malloc( buff_size * sizeof( double ) );
        double *I_dut = (double *) malloc( buff_size * sizeof( double ) );
        
        double *U_dut_sampled_X = (double *) malloc( buff_size * sizeof( double ) );
        double *U_dut_sampled_Y = (double *) malloc( buff_size * sizeof( double ) );
        double *I_dut_sampled_X = (double *) malloc( buff_size * sizeof( double ) );
        double *I_dut_sampled_Y = (double *) malloc( buff_size * sizeof( double ) );
   
        double *X_component_lock_in_U = (double *) malloc( buff_size * sizeof( double ) );
        double *Y_component_lock_in_U = (double *) malloc( buff_size * sizeof( double ) );
        double *X_component_lock_in_I = (double *) malloc( buff_size * sizeof( double ) );
        double *Y_component_lock_in_I = (double *) malloc( buff_size * sizeof( double ) );       
        double d_T=signal_decimation/125E6;

        int i;
        double sum_buff_in1;
        double sum_buff_in2;

        for(i = 0; i < buff_size; i++){

        sum_buff_in1 += buff_in1[i];
        sum_buff_in2 += buff_in2[i];
               
        }
        double mean_buff_in1=sum_buff_in1/buff_size;
        double mean_buff_in2=sum_buff_in2/buff_size;

        float R_shunt=7458.0;
        float PI=3.14;
        double d_angle;

        for(i = 0; i < buff_size; i++){
                       
               U_dut[ i ] = ((buff_in1[i]-mean_buff_in1) - (buff_in2[i]-mean_buff_in2)); 
               I_dut[ i ] = ((buff_in2[i]-mean_buff_in2) / R_shunt);

               d_angle = (i * d_T * selected_frequency * 2 * PI );
               
               U_dut_sampled_X[ i ] = U_dut[ i ] * sin( d_angle );
               U_dut_sampled_Y[ i ] = U_dut[ i ] * sin( d_angle+ ( PI/2 ) );

               I_dut_sampled_X[ i ] = I_dut[ i ] * sin( d_angle );
               I_dut_sampled_Y[ i ] = I_dut[ i ] * sin( d_angle +( PI/2 ) );               
    
        }
    
        /*Trapezoidal method for integration */
        double trapz(double *arrayptr, double d_T, int buff_size) {
        double result = 0;
        int i;
            for (i =0; i < buff_size - 1 ; i++)     { result += ( arrayptr[i] + arrayptr[ i+1 ]); }
            result = (d_T / (double)2) * result;
            return result;
        }


        X_component_lock_in_U[ 1 ] = trapz( U_dut_sampled_X, (double)d_T, buff_size );
        Y_component_lock_in_U[ 1 ] = trapz( U_dut_sampled_Y, (double)d_T, buff_size );
        
        X_component_lock_in_I[ 1 ] = trapz( I_dut_sampled_X, (double)d_T, buff_size );
        Y_component_lock_in_I[ 1 ] = trapz( I_dut_sampled_Y, (double)d_T, buff_size );


        double U_dut_amp;
        double I_dut_amp;
        double Phase_I_dut_amp;
        double Phase_U_dut_amp;

        double Z_abs;
        double Phase_Z_rad;
        double Phase;
        

        /* Calculating voltage amplitude and phase */
        U_dut_amp = (double)2 * (sqrtf( powf( X_component_lock_in_U[ 1 ] , (double)2 ) + powf( Y_component_lock_in_U[ 1 ] , (double)2 )));
        Phase_U_dut_amp = atan2f( Y_component_lock_in_U[ 1 ], X_component_lock_in_U[ 1 ] );

        /* Calculating current amplitude and phase */  
        I_dut_amp = (double)2 * (sqrtf( powf( X_component_lock_in_I[ 1 ], (double)2 ) + powf( Y_component_lock_in_I[ 1 ] , (double)2 ) ) );
        Phase_I_dut_amp = atan2f( Y_component_lock_in_I[1], X_component_lock_in_I[1] );

        
        Phase_Z_rad =  Phase_U_dut_amp - Phase_I_dut_amp;
        Z_abs = U_dut_amp / I_dut_amp; 


        /* Phase has to be limited between Pi and -Pi. */
        if (Phase_Z_rad <=  (-PI) )       { Phase_Z_rad = Phase_Z_rad +(2*PI);  }
        else if ( Phase_Z_rad >= PI )     { Phase_Z_rad = Phase_Z_rad -(2*PI);  }
        else                                { Phase_Z_rad = Phase_Z_rad;            }       
        
        Phase = (double)180*(Phase_Z_rad /((double)2*PI));

        printf("%f\n", Z_abs);
        printf("%f\n", Phase);

        rp_Release();
        return 0;
}

        
