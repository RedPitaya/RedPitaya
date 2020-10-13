#pragma once

#include <memory>
#include "acq.h"

class CCalib {
    public:

        using Ptr = std::shared_ptr<CCalib>;
        static Ptr Create(COscilloscope::Ptr _acq);

        CCalib(COscilloscope::Ptr _acq);
        CCalib(const CCalib &) = delete;
        CCalib(CCalib &&) = delete;
        ~CCalib();
        
        int resetCalibToZero();
        int calib(uint16_t _step);
    private:
        int calib_board(uint16_t _step);
        
COscilloscope::Ptr m_acq;
        int        m_current_step;     
};