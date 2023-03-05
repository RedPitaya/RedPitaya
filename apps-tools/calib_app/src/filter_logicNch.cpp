#include "filter_logicNch.h"
#include <iostream>
#include "common.h"



CFilter_logicNch::Ptr CFilter_logicNch::Create(CCalibMan::Ptr _calib_man)
{
    return std::make_shared<CFilter_logicNch>(_calib_man);
}

CFilter_logicNch::CFilter_logicNch(CCalibMan::Ptr _calib_man):
m_calib_man(_calib_man){
    auto channels = getADCChannels();
    for(auto i = 0u; i< channels;i++){
        m_fl[i] = CFilter_logic::Create(m_calib_man);
    }
}

void CFilter_logicNch::init(){
    auto channels = getADCChannels();
    for(auto i = 0u; i< channels;i++){
        m_fl[i]->init((rp_channel_t)i);
    }
}

void CFilter_logicNch::print(){
    auto channels = getADCChannels();
    for(auto i = 0u; i< channels;i++){
        printf("CH%d\n",i+1);
        m_fl[i]->print();
    }
}

int CFilter_logicNch::setCalibParameters(){
    auto res = false;
    auto channels = getADCChannels();
    for(auto i = 0u; i< channels;i++){
        res |= (m_fl[i]->setCalibParameters() == -1);
    }
    return res ? -1 : 0;
}

void CFilter_logicNch::setCalculatedValue(COscilloscope::DataPassAutoFilterSync item){
    auto channels = getADCChannels();
    for(auto i = 0u; i< channels;i++){
        if (item.valueCH[i].is_valid) m_fl[i]->setCalculatedValue(item.valueCH[i]);
    }
}

int CFilter_logicNch::getCalibCount(){
    int count = m_fl[0]->getCalibCount();
    auto channels = getADCChannels();
    for(auto i = 1u; i< channels;i++){
        auto z = m_fl[i]->getCalibCount();
        if (z > count){
            count = z;
        }
    }
    return count;
}

int CFilter_logicNch::getCalibDone(){
    int count = m_fl[0]->getCalibDone();
    auto channels = getADCChannels();
    for(auto i = 1u; i< channels;i++){
        auto z = m_fl[i]->getCalibDone();
        if (count < z){
            count = z;
        }
    }
    return count;
}

void CFilter_logicNch::removeHalfCalib(){
    auto channels = getADCChannels();
    for(auto i = 0u; i< channels;i++){
        m_fl[i]->removeHalfCalib();
    }
}

int CFilter_logicNch::nextSetupCalibParameters(){
    auto res = true;
    auto channels = getADCChannels();
    for(auto i = 0u; i< channels;i++){
        res &= (m_fl[i]->nextSetupCalibParameters() == -1);
    }
    return res ? -1 : 0;
}

int CFilter_logicNch::calcProgress(){
    auto count = m_fl[0]->calcProgress();
    auto channels = getADCChannels();
    for(auto i = 1u; i< channels;i++){
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
    auto channels = getADCChannels();
    for(auto i = 0u; i< channels;i++){
        m_fl[i]->setCalibRef(_value);
    }
}


void CFilter_logicNch::setCalibMode(int _mode){
    auto channels = getADCChannels();
    for(auto i = 0u; i< channels;i++){
        m_fl[i]->setCalibMode(_mode);
    }
}