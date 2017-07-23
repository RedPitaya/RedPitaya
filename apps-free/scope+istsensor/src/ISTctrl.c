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
 
#include "ISTctrl.h"

void* map_base_ = (void*)(-1);
const uint32_t c_addrAms = 0x40400000;

float AmsConversion(ams_t a_ch, unsigned int a_raw)
{
	float uAdc;
	float val=0;
	switch(a_ch){
		case eAmsAI0:
		case eAmsAI1:
		case eAmsAI2:
		case eAmsAI3:{
			if(a_raw>0x7ff){
				a_raw=0;
			}
			uAdc=(float)a_raw/0x7ff*0.5;
			val=uAdc*(30.0+4.99)/4.99;
		}
		break;
		case eAmsAI4:{
			uAdc=(float)a_raw/ADC_FULL_RANGE_CNT*1.0;
			val=uAdc*(56.0+4.99)/4.99;
		}
		break;
		case eAmsTemp:{
			val=((float)a_raw*503.975) / ADC_FULL_RANGE_CNT - 273.15;
		}
		break;
		case eAmsVCCPINT:
		case eAmsVCCPAUX:
		case eAmsVCCBRAM:
		case eAmsVCCINT:
		case eAmsVCCAUX:
		case eAmsVCCDDR:{
			val=((float)a_raw/ADC_FULL_RANGE_CNT)*3.0;
		}
		break;
		case eAmsAO0:
		case eAmsAO1:
		case eAmsAO2:
		case eAmsAO3:
			val=((float)(a_raw>>16)/SLOW_DAC_RANGE_CNT)*1.8;
		break;
		case eSendNum:
			break;
	}
	return val;
}

void DacWrite(amsReg_t * a_amsReg, double * a_val, ssize_t a_cnt)
{
	uint32_t i;
	for(i=0;i<a_cnt;i++){
		uint32_t dacCnt;
		if(a_val[i]<0){
		   a_val[i]=0;
		}
		if(a_val[i]>1.8){
		   a_val[i]=1.8;
		}
		dacCnt=(a_val[i]/1.8)*SLOW_DAC_RANGE_CNT;
		//dacCnt&=0x9c;
		dacCnt*=256*256; // dacCnt=dacCnt<<16;
		a_amsReg->dac[i]=dacCnt;
	}
}

void ISTctrl_Init(void)
{
	fileopen = 0;
	HeatStp = 1;
	fileErr = 0;  
	Dt = 0.0001;	//100us of period
	ISTcnt = 0;
	ISTsnsAdj = 0;
	ISTadj = 0;
	ISTmin = 0;
	ISTmax = 0;
	ISTlm35 = 0;
	ISTfreq = 0;
	ISTper = 0;
	IST_tempCalib();
}


void IST_tempCalib(void)
{
	//read ADC 0 ans 1 to make differential mesure
	int i;
	double val[2];
	unsigned int raw;
			
	uint32_t addr = c_addrAms;
	amsReg_t* ams=NULL;
	// Map one page
	map_base_ = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, addr & ~MAP_MASK);	    
	if(map_base_ == (void *) -1) FATAL;	
	
	ams = map_base_ + (addr & MAP_MASK);
	for(i=0;i<2000;i++)
	{
		raw = ams->aif[0];
		val[0]+=AmsConversion(1, raw)*1000;//0.01749 is the conv. value from Volt to °C for the PT1000
		raw = ams->aif[1];
		val[1]+=AmsConversion(2, raw)*100;
		usleep(10);		
	}
		
	ISTsnsAdj = (float)((val[1]/1000)/(val[0]/1000));
	if(isinf(ISTsnsAdj) || isnan(ISTsnsAdj))	//check if it's nan or inf
	{
		ISTsnsAdj = 0;
	}
	munmap(0, MAP_SIZE);
}

void swap(double* a, double* b)
{
	double tmp = *a;
	*b = *a;
	*a = tmp;
}

double getMin(double a, double b, double c)
{	
	if(a>b)
	{
		swap(&a,&b);
	}
	if(b>c)
	{
		swap(&b,&c);
	}
	if(a>b)
	{
		swap(&a,&b);
	}
	return a;
}

float ISTctrl()
{
	//read ADC 0 ans 1 to make differential mesure
	static int filecnt = 0, mv = 0;
	double val[4];
	static double bufVal_0[3],bufVal_1[3];
	unsigned int raw;
	float CtrlMaxTemp = DeltaTemp,toDAC;
			
	uint32_t addr = c_addrAms;
	amsReg_t* ams=NULL;
	// Map one page
	map_base_ = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, addr & ~MAP_MASK);	    
	if(map_base_ == (void *) -1) FATAL;	
	ams = map_base_ + (addr & MAP_MASK);
	raw = ams->aif[0];
	val[0]=AmsConversion(1, raw)*1000*ISTsnsAdj; //0.01749 is the conv. value from Volt to °C for the PT1000
	raw = ams->aif[1];
	val[1]=AmsConversion(2, raw)*100;
	
	//median filter
	bufVal_0[mv] = val[0];
	bufVal_1[mv] = val[1];	
	if(mv<2) 
	{ 
		mv++; 
	}
	else 
	{ 		
		val[0] = getMin(bufVal_0[0],bufVal_0[1],bufVal_0[2]);
		val[1] = getMin(bufVal_1[0],bufVal_1[1],bufVal_1[2]);
		bufVal_0[0] = bufVal_0[1];
		bufVal_0[1] = bufVal_0[2];
		bufVal_1[0] = bufVal_1[1];
		bufVal_1[1] = bufVal_1[2]; 
	
		float RS_LM35_diff = (val[1]-val[0])+ DeltaTemp; //diff measure between LM35 and IST sens		
		float PIDout = pid_update(RS_LM35_diff);
		
		toDAC = (PIDout*1.8/CtrlMaxTemp); //PID control
		if(toDAC>1.8) toDAC = 1.8;
		if(toDAC<0) toDAC = 0;
				
		if(filecnt < 10)	//save at abt 1K Hz, 2ms of period
		{	
			filecnt++;
		}
		else
		{
			filecnt=0;
			ISTlm35 = (float)(val[1]);
			if(IST2file==1 && fileopen==1)// write to the file 
			{
				fprintf(file_ptr, "%f,%f,%f\n", val[0],val[1],toDAC); //IST temp, LM35 temp, Out ctrl
			}
		}
		
		val[0] = toDAC;
		val[1] = 0;//AmsConversion(eAmsAO0+1, ams->dac[1]);
		val[2] = 0;//AmsConversion(eAmsAO0+2, ams->dac[2]);
		val[3] = 0;//AmsConversion(eAmsAO0+3, ams->dac[3]);

		DacWrite(ams, &val[0], 4);
		
		if (map_base_ != (void*)(-1)) {
			if(munmap(map_base_, MAP_SIZE) == -1) FATAL;
			map_base_ = (void*)(-1);
		}
		munmap(0, MAP_SIZE);
	}	
	return (float)toDAC;
}

void Stop_ISTctrl(rp_app_params_t *params)
{	
	if(fileopen==1){ IST_Closefile(); }
	StopHeat();
	params[PID_IST_ENABLE].value = IST_EN;	
}

void StopHeat()
{
	if(IST_EN==0 && HeatStp==0) 
	{ 
		ISTcnt = 0;
		HeatStp = 1;
		double* val;
		amsReg_t* ams=NULL;
		val = calloc(4*1024, sizeof(double));
					
		map_base_ = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, c_addrAms & ~MAP_MASK);
		
		ams = map_base_ + (c_addrAms & MAP_MASK);
		val[0] = 0;
		val[1] = 0;
		val[2] = 0;
		val[3] = 0;

		DacWrite(ams, val, 4);
	}
}

void ISTctrl_time(int delay)
{
	if(IST_EN==1) 
	{ 		
		
		float ISTminTmp = 3.3,ISTmaxTmp;
		float dataTemp;
		HeatStp = 0;
		float ISTctrl_delay = Dt*1000000;	//us
		int iterations = (int)(delay/ISTctrl_delay);
		int dupSample =(int)((TimeWin/ISTctrl_delay)/1024);
		int i;
		static int j = 0;
				
		for(i=0;i<iterations;i++)
		{		
				dataTemp = ISTctrl();
				
				if(j<dupSample)
				{
					j++;
				}
				else
				{		
					j=0;		
					IST_PWR_out[ISTcnt] = dataTemp;
					if(ISTcnt<(SIGNAL_LENGTH-1)) 
					{ 
						ISTcnt++; 
						
						if(IST_PWR_out[ISTcnt] < ISTminTmp) { ISTminTmp = IST_PWR_out[ISTcnt]; }
						if(IST_PWR_out[ISTcnt] > ISTmaxTmp) { ISTmaxTmp = IST_PWR_out[ISTcnt]; }
					}
					else 
					{ 						
						ISTmin = ISTminTmp;
						ISTminTmp = ISTmaxTmp;
						ISTmax = ISTmaxTmp;
						ISTadj = ISTsnsAdj;
						ISTfreq = 10000;
						ISTper = 0.0001;
												
						ISTcnt = 0; 					
					}
				}
				usleep(10);
				
		}	
	}	//enable IST realprobe Controller abt 190uS
	else
	{
		StopHeat();
		usleep(190);
	}
}

void IST_Initfile(void)
{
	if(fileopen==0)
	{		
		int nameCnt=0;
		char filename[64];		
		
		sprintf (filename, "/tmp/ISTctrl_%d.dat", nameCnt);
		while(fopen(filename, "r"))
		{
			nameCnt++;		
			sprintf (filename, "/tmp/ISTctrl_%d.dat", nameCnt);
		}
		
		/* open the file */
		file_ptr = fopen(filename, "w");
		if (file_ptr == NULL) 
		{
			printf("I couldn't open file for writing.\n");
			fileErr = 1;
			exit(0);
		}
		fprintf(file_ptr, "RedPitaya Oscilloscope + IST Realprobe Flow Controller\n");
		fprintf(file_ptr, "The PID controller freq. is 10KHz and the data saving freq. is 1KHz.\n");
		fprintf(file_ptr, "The IST PT100 V/°C conversion value is %f. The following data are:",ISTsnsAdj);
		fprintf(file_ptr, "\nIST realprobe temp (°C), LM35 temp (°C), IST power control (Volt)\n\n");
		fileopen = 1;			
	}
}

void IST_Closefile(void)
{
	if(fileopen==1)
	{
		fileopen = 0;
		/* close the file */
		fclose(file_ptr); 
	}	
}
