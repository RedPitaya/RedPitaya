/**
 * $Id: worker.c 881 2013-12-16 05:37:34Z rp_jmenart $
 *
 * @brief Red Pitaya Oscilloscope worker.
 *
 * @Author Jure Menart <juremenart@gmail.com>
 * 
 * @Author: Flavio Ansovini <flavio.ansovini@unige.it>
 * University of Genova - Serizio Supporto Tecnologico
 *         
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "pid.h"
/**
 * GENERAL DESCRIPTION:
 *
 */

static double lastDelta = 0;
static double PIDint = 0;

int pid_init(void)
{
	kp = 0.2;
	ki = 0.05;
	kd = 0;
	DeltaTemp = 8; //10°C between LM35 and IST realprobe flow sensor
	lastDelta = 0;
	PIDint = 0;
	
	return 0;
}


/*----------------------------------------------------------------------------------*/
/** @brief Cleanup PID COntroller module
 *
 * The Function reset the integral factor and the last deltaTemp to zero 
 */
int pid_reset(void)
{
	lastDelta = 0;
	PIDint = 0;
	
	return 0;
}

//int pid_constUpdate(int t_en, int t_rs, double t_dt, double t_kp, double t_ki, double t_kd)//rp_app_params_t *params)
int pid_constUpdate(rp_app_params_t *params)
{
		IST_EN = params[PID_IST_ENABLE].value;	
		IST2file = params[IST_SAVE2FILE].value;	
		if(params[PID_IST_DT].value!=0) { DeltaTemp =  params[PID_IST_DT].value; }
		else { params[PID_IST_DT].value = DeltaTemp; }
		if(params[PID_IST_KP].value!=0) { kp = params[PID_IST_KP].value; }
		else { params[PID_IST_KP].value = kp; }
		if(params[PID_IST_KI].value!=0) { ki = params[PID_IST_KI].value; }
		else { params[PID_IST_KI].value = ki; }
		if(params[PID_IST_KD].value!=0) { kd = params[PID_IST_KD].value; }
		else { params[PID_IST_KD].value = kd; }
		
		if(IST2file==1) 
		{
			if(fileopen==0)	{ IST_Initfile(); }
			else
			{
				if(fileErr==1) 
				{
					fileopen = 0;
					IST2file = 0;
					params[IST_SAVE2FILE].value = IST2file;
				} 
			}
		}
		else
		{
			if(fileopen==1)	{IST_Closefile();}
		}
		return 0;
}

/*----------------------------------------------------------------------------------*/
/**
 * @brief Update PID Controller module towards actual settings.
 *
 * The function update the PID values and return the control output
 */
float pid_update(float delta)
{		
	float out,deriv;
	
	PIDint += ((delta - lastDelta) + lastDelta)*Dt;
	deriv = (atan2f((delta - lastDelta),Dt)/M_2_PI);
	
	out = delta*kp + PIDint*ki + deriv*kd;
	lastDelta = delta;
	
	return out;	
}

