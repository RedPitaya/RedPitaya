#include "acq_math.h"
#include "acq.h"
#include "filter_logic.h"
#include "filter_logic2ch.h"
#include "rp.h"
#include <iostream>

#define DEC 8

void test1();
void test2();

int main()
{
  //  test1();
    test2();
    return 0;
}

void test1(){
      	rp_Init();
	auto acq = COscilloscope::Create(DEC);

    auto calib = CCalibMan::Create(acq);
    auto f_l = CFilter_logic::Create(calib);
    calib->setModeLV_HV( RP_LOW );
    f_l->init(RP_CH_1);
    f_l->print();
    acq->start();
    acq->startAutoFilter(DEC);
    acq->updateAcqFilter(RP_CH_1);
    while(1){
        auto d = acq->getDataAutoFilter();
        while (f_l->setCalibParameters() != -1){
        //    auto dp = acq->getDataAutoFilter();
        //    auto cur_index = dp.index;
            
            auto dp = acq->getDataAutoFilter();
            if (dp.is_valid == true) {
                f_l->setCalculatedValue(dp);                
            }else{
                
            }

            //if (f_l->setCalculatedValue(dp)== -1) break;
                
            
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            printf("\rPROGRESS: %6d/%6d",f_l->getCalibDone(), f_l->getCalibCount());
        }
        printf("\n");
    //       std::cout <<  " AA = " << d.f_aa << " BB = " << d.f_bb << " PP = " << d.f_pp << " KK = " << d.f_kk << std::endl;
        f_l->removeHalfCalib();
        printf("CALCULATE\n");
        f_l->print();
        getchar();
        if (f_l->nextSetupCalibParameters() == -1) break;
        printf("SPLIT\n");
        f_l->print();
        printf("======= PROGRESS: %d\n",f_l->calcProgress());

        getchar();
       // break;
    }
    f_l->setGoodCalibParameter();
    

    while(1){
        auto d = acq->getDataAutoFilter();
        
        printf("PP: %d , ampl: %f\n",d.f_pp,d.ampl);
        if (d.ampl > 0){
            if (f_l->calibPP(d,0.9) != 0) break;
        }
    }
 //   auto x = acq->getDataAutoFilter();
 //   std::cout <<  " AA = " << x.f_aa << " BB = " << x.f_bb << " PP = " << x.f_pp << " KK = " << x.f_kk << std::endl;
    acq->stop();

    rp_Release();
}


void test2(){

    rp_Init();
	auto acq = COscilloscope::Create(DEC);
    auto calib = CCalibMan::Create(acq);

    auto f_l = CFilter_logic2ch::Create(calib);
    calib->setModeLV_HV( RP_HIGH );
    	calib->setOffset(RP_CH_1,0);
		calib->setFreq(RP_CH_1,1000);
		calib->setAmp(RP_CH_1,0.9);	
		calib->setOffset(RP_CH_2,0);
		calib->setFreq(RP_CH_2,1000);
		calib->setAmp(RP_CH_2,0.9);	
		calib->setGenType(RP_CH_1,(int)RP_WAVEFORM_SQUARE);
    	calib->setGenType(RP_CH_2,(int)RP_WAVEFORM_SQUARE);  
		calib->enableGen(RP_CH_1,true);
		calib->enableGen(RP_CH_2,true);
    f_l->init();
    f_l->print();
    acq->start();
    acq->startAutoFilter2Ch(DEC);
    acq->updateAcqFilter(RP_CH_1);
    acq->updateAcqFilter(RP_CH_2);
    while(1){
        auto d = acq->getDataAutoFilter2Ch();
        while (f_l->setCalibParameters() != -1){
        //    auto dp = acq->getDataAutoFilter();
        //    auto cur_index = dp.index;
            
            auto dp = acq->getDataAutoFilter2Ch();
            f_l->setCalculatedValue(dp);                
            

            //if (f_l->setCalculatedValue(dp)== -1) break;
                
            
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            printf("\rPROGRESS: %6d/%6d",f_l->getCalibDone(), f_l->getCalibCount());
        }
        printf("\n");
    //       std::cout <<  " AA = " << d.f_aa << " BB = " << d.f_bb << " PP = " << d.f_pp << " KK = " << d.f_kk << std::endl;
        f_l->removeHalfCalib();
        printf("CALCULATE\n");
        f_l->print();
    //    getchar();
        if (f_l->nextSetupCalibParameters() == -1) break;
        printf("SPLIT\n");
        f_l->print();
        printf("======= PROGRESS: %d\n",f_l->calcProgress());

   //     getchar();
       // break;
    }
    f_l->setGoodCalibParameterCh1();
    
    printf("CALIB CH1\n");

    while(1){
        auto d = acq->getDataAutoFilter2Ch();
        
    //    printf("PP: %d , ampl: %f\n",d.valueCH1.f_pp,d.valueCH1.ampl);
        if (f_l->calibPPCh1(d,0.9) != 0) break;
    }
    printf("CALIB CH2\n");
    f_l->setGoodCalibParameterCh2();
     while(1){
        auto d = acq->getDataAutoFilter2Ch();
        
    //    printf("PP: %d , ampl: %f\n",d.valueCH2.f_pp,d.valueCH2.ampl);
        if (f_l->calibPPCh2(d,0.9) != 0) break;
    }
 //   auto x = acq->getDataAutoFilter();
 //   std::cout <<  " AA = " << x.f_aa << " BB = " << x.f_bb << " PP = " << x.f_pp << " KK = " << x.f_kk << std::endl;
    acq->stop();
    rp_Release();
}
