#include "calib.h"


CCalib::Ptr CCalib::Create(COscilloscope::Ptr _acq)
{
    return std::make_shared<CCalib>(_acq);
}

CCalib::CCalib(COscilloscope::Ptr _acq):
m_acq(_acq),
m_current_step(-1)
{
   
    
}

CCalib::~CCalib()
{

}

int CCalib::resetCalibToZero(){
    return rp_CalibrationReset();
}

int CCalib::calib(uint16_t _step){
    return calib_board(_step);
}


#ifdef Z10
int CCalib::calib_board(uint16_t _step){
    if (m_current_step == _step) return 0;
    m_current_step = _step;
    switch(_step){
        case 0: {
            resetCalibToZero();
            return 0;
        }
        
    }
    return 0;
}
#endif

#ifdef Z20_250_12
int CCalib::calib_board(uint16_t _step){
    return 0;
}
#endif