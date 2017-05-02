#include "main.h"

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include "version.h"
#include <sys/types.h>
#include <sys/sysinfo.h>


#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

/**********************************************
*                                             *
* This part for LED control is from RP API    *
*                                             *
**********************************************/

// Housekeeping structure declaration
typedef struct housekeeping_control_s {
    uint32_t id;
    uint32_t dna_lo;
    uint32_t dna_hi;
    uint32_t digital_loop;
    uint32_t ex_cd_p;
    uint32_t ex_cd_n;
    uint32_t ex_co_p;
    uint32_t ex_co_n;
    uint32_t ex_ci_p;
    uint32_t ex_ci_n;
    uint32_t reserved_2;
    uint32_t reserved_3;
    uint32_t led_control;
} housekeeping_control_t;

// Base Housekeeping address
static const int HOUSEKEEPING_BASE_ADDR = 0x40000000;
static const int HOUSEKEEPING_BASE_SIZE = 0x30;
static int fd = 0;
static volatile housekeeping_control_t *hk = NULL;

#define ioread32(p) (*(volatile uint32_t *)(p))
#define iowrite32(v,p) (*(volatile uint32_t *)(p) = (v))

int cmn_Init(){
    if (!fd) {
        if((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) { return -1; }
    }
    return 0;
}

int cmn_Release(){
    if (fd) {
        if(close(fd) < 0) { return -1; }
    }
    return 0;
}

int cmn_Map(size_t size, size_t offset, void** mapped){
    if(fd == -1) { return -1; }
    *mapped = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);
    if(mapped == (void *) -1) { return -2; }
    return 0;
}

int cmn_Unmap(size_t size, void** mapped){
    if(fd == -1) { return -1; }
    if((mapped == (void *) -1) || (mapped == NULL)) { return -2; }
    if((*mapped == (void *) -1) || (*mapped == NULL)) { return -3; }
    if(munmap(*mapped, size) < 0){ return -4; }
    *mapped = NULL;
    return 0;
}

//LED only, RP_LED0 = 0
void rp_DpinSetState(unsigned char pin, unsigned char state) {
    uint32_t tmp;
    if ((pin < 8)&&(state<=1)) {
        tmp = ioread32(&hk->led_control);
        iowrite32((tmp & ~(1 << pin)) | ((state << pin) & (1 << pin)), &hk->led_control);
    }
}

unsigned char rp_DpinGetState(unsigned char pin) {
    unsigned char state=0;
    if (pin < 8) {
        state = (ioread32(&hk->led_control) >> pin) & 0x1;
    } 
    return state;
}

/**********************************************
*                                             *
* End of LED control (RP API)                 *
*                                             *
**********************************************/


#define CH_SIGNAL_SIZE_DEFAULT		8
long double cpu_values[4] = {0, 0, 0, 0}; /* reading only user, nice, system, idle */

/* --------------------------------  SIGNALS  ------------------------------ */
CFloatSignal sig("signal", CH_SIGNAL_SIZE_DEFAULT, 0.0f);

/* ------------------------------- PARAMETERS ------------------------------ */
CIntParameter updatePeriod("PERIOD", CBaseParameter::RW, 200, 0, 0, 10000);
CIntParameter timeDelay("time_delay", CBaseParameter::RW, 50000, 0, 0, 100000000);
CFloatParameter cpuLoad("CPU_LOAD", CBaseParameter::RW, 0, 0, 0, 100);
CFloatParameter memoryFree ("FREE_RAM", CBaseParameter::RW, 0, 0, 0, 1e15);
CDoubleParameter counter("COUNTER", CBaseParameter::RW, 1, 0, 1e-12, 1e+12);
CStringParameter text("TEXT", CBaseParameter::RW, "", 10);
CBooleanParameter led0("led0", CBaseParameter::RW, false, 0);
CBooleanParameter led1("led1", CBaseParameter::RW, false, 0);
CBooleanParameter led2("led2", CBaseParameter::RW, false, 0);
CBooleanParameter led3("led3", CBaseParameter::RW, false, 0);
CBooleanParameter led4("led4", CBaseParameter::RW, false, 0);
CBooleanParameter led5("led5", CBaseParameter::RW, false, 0);
CBooleanParameter led6("led6", CBaseParameter::RW, false, 0);
CBooleanParameter led7("led7", CBaseParameter::RW, false, 0);
CBooleanParameter KITT("KITT", CBaseParameter::RW, false, 0);


const char *rp_app_desc(void) {
    return (const char *)"Red Pitaya LED application.\n";
}

int rp_app_init(void) {
    fprintf(stderr, "Loading LED application version %s-%s.\n", VERSION_STR, REVISION_STR);
    CDataManager::GetInstance()->SetParamInterval(updatePeriod.Value());
    CDataManager::GetInstance()->SetSignalInterval(updatePeriod.Value());

    //Init and reset LED
    int res;
    res=cmn_Init();
    if (res!=0){fprintf(stderr, "Init fail %d\n",res);}
    res=cmn_Map(HOUSEKEEPING_BASE_SIZE, HOUSEKEEPING_BASE_ADDR, (void**)&hk);
    if (res!=0){fprintf(stderr, "Map fail %d\n",res);}
    iowrite32(0, &hk->led_control);
    return 0;
}

int rp_app_exit(void) {
    fprintf(stderr, "Unloading LED application version %s-%s.\n", VERSION_STR, REVISION_STR);
    //Release
    int res;
    res=cmn_Unmap(HOUSEKEEPING_BASE_SIZE, (void**)&hk);
    if (res!=0){fprintf(stderr, "Unmap fail %d\n",res);}
    res=cmn_Release();
    if (res!=0){fprintf(stderr, "Release fail %d\n",res);}
    return 0;
}

int rp_set_params(rp_app_params_t *p, int len) {
    return 0;
}

int rp_get_params(rp_app_params_t **p) {
    return 0;
}

int rp_get_signals(float ***s, int *sig_num, int *sig_len) {
    return 0;
}


void UpdateParams(void) {
	CDataManager::GetInstance()->SetParamInterval(updatePeriod.Value());
	CDataManager::GetInstance()->SetSignalInterval(updatePeriod.Value());
   	
    FILE *fp = fopen("/proc/stat","r");
    if(fp)
    {
        long double a[4];
        fscanf(fp,"%*s %Lf %Lf %Lf %Lf",&a[0],&a[1],&a[2],&a[3]);
        fclose(fp);

        long double divider = ((a[0]+a[1]+a[2]+a[3]) - (cpu_values[0]+cpu_values[1]+cpu_values[2]+cpu_values[3]));
        long double loadavg = 100;
        if(divider > 0.01)
        {
            loadavg = ((a[0]+a[1]+a[2]) - (cpu_values[0]+cpu_values[1]+cpu_values[2])) / divider;
        }
        cpuLoad.Value() = (float)(loadavg * 100);
        cpu_values[0]=a[0];cpu_values[1]=a[1];cpu_values[2]=a[2];cpu_values[3]=a[3];
    }
    
    struct sysinfo memInfo;
    sysinfo (&memInfo);
    memoryFree.Value() = (float)memInfo.freeram;
    counter.Value()=counter.Value()+(double)updatePeriod.Value()/1000.0;
    text.Value() = "Hello";
    if (counter.Value()>1000){counter.Value()=0;}
}



void UpdateSignals(void) {
    CDataManager::GetInstance()->SetSignalInterval(updatePeriod.Value());
        
    for(unsigned char i = 0; i < sig.GetSize(); i++) {
        sig[i] = (float) rp_DpinGetState(i);
    }
}


void OnNewParams(void) {
	CDataManager::GetInstance()->UpdateAllParams();
    if (led0.Value()==true){rp_DpinSetState(0, 1);} else {rp_DpinSetState(0, 0);}
    if (led1.Value()==true){rp_DpinSetState(1, 1);} else {rp_DpinSetState(1, 0);}
    if (led2.Value()==true){rp_DpinSetState(2, 1);} else {rp_DpinSetState(2, 0);}
    if (led3.Value()==true){rp_DpinSetState(3, 1);} else {rp_DpinSetState(3, 0);}
    if (led4.Value()==true){rp_DpinSetState(4, 1);} else {rp_DpinSetState(4, 0);}
    if (led5.Value()==true){rp_DpinSetState(5, 1);} else {rp_DpinSetState(5, 0);}
    if (led6.Value()==true){rp_DpinSetState(6, 1);} else {rp_DpinSetState(6, 0);}
    if (led7.Value()==true){rp_DpinSetState(7, 1);} else {rp_DpinSetState(7, 0);}

    //Time expensive algorithms should not be here, but you can test how it behave
    if(KITT.Value()){
        uint32_t tmp = ioread32(&hk->led_control); //Save LED state before KITT
        KITT.Value()=false;
        for(int j=0; j<10; j++){
            for (int i=0; i<8; i++) {
                rp_DpinSetState(i,1);
                usleep(timeDelay.Value());
            }
            for (int i=0; i<8; i++) {
                rp_DpinSetState(i,0);
                usleep(timeDelay.Value());
            }
            for (int i=7; i>=0; i--) {
                rp_DpinSetState(i,1);
                usleep(timeDelay.Value());
            }
            for (int i=7; i>=0; i--) {
                rp_DpinSetState(i,0);
                usleep(timeDelay.Value());
            } 
        }
        iowrite32(tmp, &hk->led_control); //Restore LED state
    }
}

void OnNewSignals(void)
{
	// do something
	//CDataManager::GetInstance()->UpdateAllSignals();
}
