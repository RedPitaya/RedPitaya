#include "filter_logic2ch.h"
#include <iostream>



CFilter_logic2ch::Ptr CFilter_logic2ch::Create(CCalibMan::Ptr _calib_man)
{
    return std::make_shared<CFilter_logic2ch>(_calib_man);
}

CFilter_logic2ch::CFilter_logic2ch(CCalibMan::Ptr _calib_man):
m_calib_man(_calib_man)
{
    m_fl1 = CFilter_logic::Create(m_calib_man);
    m_fl2 = CFilter_logic::Create(m_calib_man);
}

void CFilter_logic2ch::init(){
    m_fl1->init(RP_CH_1);
    m_fl2->init(RP_CH_2);
}

void CFilter_logic2ch::print(){
    printf("CH1\n");
    m_fl1->print();
    printf("CH2\n");
    m_fl2->print();
}

int CFilter_logic2ch::setCalibParameters(){
    auto res1 = m_fl1->setCalibParameters();
    auto res2 = m_fl2->setCalibParameters();
    return (res1 == -1 || res2 == -1) ? -1 : 0;
}

void CFilter_logic2ch::setCalculatedValue(COscilloscope::DataPassAutoFilter2Ch item){
    if (item.valueCH1.is_valid) m_fl1->setCalculatedValue(item.valueCH1);
    if (item.valueCH2.is_valid) m_fl2->setCalculatedValue(item.valueCH2);
    
}

int CFilter_logic2ch::getCalibCount(){
    auto x = m_fl1->getCalibCount();
    auto y = m_fl2->getCalibCount();
    return x < y ? y : x;
}

int CFilter_logic2ch::getCalibDone(){
    auto x = m_fl1->getCalibDone();
    auto y = m_fl2->getCalibDone();
    return x > y ? y : x;
}

void CFilter_logic2ch::removeHalfCalib(){
    m_fl1->removeHalfCalib();
    m_fl2->removeHalfCalib();
}

int CFilter_logic2ch::nextSetupCalibParameters(){
    auto res1 = m_fl1->nextSetupCalibParameters();
    auto res2 = m_fl2->nextSetupCalibParameters();
    return (res1 == -1 && res2 == -1) ? -1 : 0;
}

int CFilter_logic2ch::calcProgress(){
    auto x = m_fl1->calcProgress();
    auto y = m_fl2->calcProgress();
    return x > y ? y : x;
}

void CFilter_logic2ch::setGoodCalibParameterCh1(){
    m_fl1->setGoodCalibParameter();
}

void CFilter_logic2ch::setGoodCalibParameterCh2(){
    m_fl2->setGoodCalibParameter();
}

int CFilter_logic2ch::calibPPCh1(COscilloscope::DataPassAutoFilter2Ch item,float _nominal){  
    if (item.valueCH1.ampl > 0){
        return m_fl1->calibPP(item.valueCH1,_nominal);
    }
    return -1;
}

int CFilter_logic2ch::calibPPCh2(COscilloscope::DataPassAutoFilter2Ch item,float _nominal){  
    if (item.valueCH2.ampl > 0){
        return m_fl2->calibPP(item.valueCH2,_nominal);
    }
    return -1;
}

void CFilter_logic2ch::setCalibRef(float _value){
    m_fl1->setCalibRef(_value);
    m_fl2->setCalibRef(_value); 
}


void CFilter_logic2ch::setCalibMode(int _mode){
    m_fl1->setCalibMode(_mode);
    m_fl2->setCalibMode(_mode); 
}