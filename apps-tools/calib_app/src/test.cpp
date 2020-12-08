#include "acq_math.h"
#include "acq.h"
#include "filter_logic.h"
#include "rp.h"
#include <iostream>

#define DEC 8
int main()
{
  	rp_Init();
	auto acq = COscilloscope::Create(DEC);

    auto calib = CCalibMan::Create(acq);
    auto f_l = CFilter_logic::Create(calib);
    calib->setModeLV_HV( RP_HIGH );
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
            printf("\rPROGRESS: %6d/%6d",f_l->getcCalibDone(), f_l->getCalibCount());
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
            if (f_l->calibPP(d,4) != 0) break;
        }
    }
 //   auto x = acq->getDataAutoFilter();
 //   std::cout <<  " AA = " << x.f_aa << " BB = " << x.f_bb << " PP = " << x.f_pp << " KK = " << x.f_kk << std::endl;
    acq->stop();
    rp_Release();
    return 0;
}