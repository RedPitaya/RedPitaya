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

#ifndef __PID_H
#define __PID_H

#include "worker.h"

double kp,ki,kd,Dt,DeltaTemp;
int IST_EN,IST2file;
		
int 	pid_init(void);
int 	pid_reset(void);
int 	pid_constUpdate(rp_app_params_t *params);
float 	pid_update(float);

#endif // __PID_H
