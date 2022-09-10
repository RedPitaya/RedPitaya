#include "filter_logicNch.h"
#include <iostream>



CFilter_logicNch::Ptr CFilter_logicNch::Create(CCalibMan::Ptr _calib_man)
{
    return std::make_shared<CFilter_logicNch>(_calib_man);
}

CFilter_logicNch::CFilter_logicNch(CCalibMan::Ptr _calib_man):
m_calib_man(_calib_man)
{
    for(auto i = 0u; i< ADC_CHANNELS;i++){
        m_fl[i] = CFilter_logic::Create(m_calib_man);
    }    
}

void CFilter_logicNch::init(){
    for(auto i = 0u; i< ADC_CHANNELS;i++){
        m_fl[i]->init((rp_channel_t)i);
    }
}

void CFilter_logicNch::print(){
    for(auto i = 0u; i< ADC_CHANNELS;i++){
        printf("CH%d\n",i+1);
        m_fl[i]->print();
    }    
}

int CFilter_logicNch::setCalibParameters(){
    auto res = false;
    for(auto i = 0u; i< ADC_CHANNELS;i++){
        res |= (m_fl[i]->setCalibParameters() == -1);
    }
    return res ? -1 : 0;
}

void CFilter_logicNch::setCalculatedValue(COscilloscope::DataPassAutoFilterSync item){
    for(auto i = 0u; i< ADC_CHANNELS;i++){
        if (item.valueCH[i].is_valid) m_fl[i]->setCalculatedValue(item.valueCH[i]);
    }
}

int CFilter_logicNch::getCalibCount(){
    int count = m_fl[0]->getCalibCount();
    for(auto i = 1u; i < ADC_CHANNELS;i++){
        auto z = m_fl[i]->getCalibCount();
        if (z > count){
            count = z;
        }
    }
    return count;
}

int CFilter_logicNch::getCalibDone(){
    int count = m_fl[0]->getCalibDone();
    for(auto i = 1u; i< ADC_CHANNELS;i++){
        auto z = m_fl[i]->getCalibDone();
        if (count < z){
            count = z;
        }
    }    
    return count;
}

void CFilter_logicNch::removeHalfCalib(){
    for(auto i = 0u; i< ADC_CHANNELS;i++){
        m_fl[i]->removeHalfCalib();
    }    
}

int CFilter_logicNch::nextSetupCalibParameters(){
    auto res = true;
    for(auto i = 0u; i< ADC_CHANNELS;i++){
        res &= (m_fl[i]->nextSetupCalibParameters() == -1);
    }
    return res ? -1 : 0;
}

int CFilter_logicNch::calcProgress(){
    auto count = m_fl[0]->calcProgress();
    for(auto i = 1u; i < ADC_CHANNELS;i++){
        auto z = m_fl[i]->calcProgress();
        if (z < count){
            count = z;
        }
    }
    return count;
}

void CFilter_logicNch::setGoodCalibParameterCh(rp_channel_t ch){
    m_fl[ch]->setGoodCalibParameter();
}

int CFilter_logicNch::calibPPCh(rp_channel_t ch,COscilloscope::DataPassAutoFilterSync item,float _nominal){  
    if (item.valueCH[ch].ampl > 0){
        return m_fl[ch]->calibPP(item.valueCH[ch],_nominal);
    }
    return -1;
}

void CFilter_logicNch::setCalibRef(float _value){
    for(auto i = 0u; i < ADC_CHANNELS;i++){
        m_fl[i]->setCalibRef(_value);
    }
}


void CFilter_logicNch::setCalibMode(int _mode){
    for(auto i = 0u; i < ADC_CHANNELS;i++){
        m_fl[i]->setCalibMode(_mode);
    }    
}