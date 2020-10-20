#pragma once

#include <memory>
#include "acq.h"
#include "rp.h"

class CCalib {
    public:
        struct DataPass
        {
            int32_t ch1;
            int32_t ch2;
        };

        using Ptr = std::shared_ptr<CCalib>;
        static Ptr Create(COscilloscope::Ptr _acq);

        CCalib(COscilloscope::Ptr _acq);
        CCalib(const CCalib &) = delete;
        CCalib(CCalib &&) = delete;
        ~CCalib();
        
         int resetCalibToZero();
         int resetCalibToFactory();
         int calib(uint16_t _step,float _refdc);
    DataPass getCalibData();
        void restoreCalib();
  

    private:
                    int calib_board(uint16_t _step,float _refdc);
COscilloscope::DataPass getData(int skip_read);       

     COscilloscope::Ptr m_acq;
                    int m_current_step;  
      rp_calib_params_t m_calib_parameters;
      rp_calib_params_t m_calib_parameters_old;
               DataPass m_pass_data;

};