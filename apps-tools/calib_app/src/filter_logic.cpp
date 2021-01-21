#include "filter_logic.h"
#include <iostream>

#define GAIN_LO_FILT_AA 0x7D93
#define GAIN_LO_FILT_BB 0x437C7
#define GAIN_LO_FILT_PP 0x2666
#define GAIN_LO_FILT_KK 0xd9999a
#define GAIN_HI_FILT_AA 0x4205
#define GAIN_HI_FILT_BB 0x2Fbcb
#define GAIN_HI_FILT_PP 0x2666
#define GAIN_HI_FILT_KK 0xd9999a


#define PERCENT_RANGE 20.0
#define PERCENT_MIN 0.001
#define MIN_AA GAIN_LO_FILT_AA * (1.0 - PERCENT_RANGE / 100.0)
#define MAX_AA GAIN_LO_FILT_AA * (1.0 + PERCENT_RANGE / 100.0)
#define MIN_BB GAIN_LO_FILT_BB * (1.0 - PERCENT_RANGE / 100.0)
#define MAX_BB GAIN_LO_FILT_BB * (1.0 + PERCENT_RANGE / 100.0)
#define MIN_AA_HI GAIN_HI_FILT_AA * (1.0 - PERCENT_RANGE / 100.0)
#define MAX_AA_HI GAIN_HI_FILT_AA * (1.0 + PERCENT_RANGE / 100.0)
#define MIN_BB_HI GAIN_HI_FILT_BB * (1.0 - PERCENT_RANGE / 100.0)
#define MAX_BB_HI GAIN_HI_FILT_BB * (1.0 + PERCENT_RANGE / 100.0)
#define MIN_PP 0
#define MAX_PP 0x50000
#define STEPS_AA 2
#define STEPS_BB STEPS_AA

CFilter_logic::Ptr CFilter_logic::Create(CCalibMan::Ptr _calib_man)
{
    return std::make_shared<CFilter_logic>(_calib_man);
}

bool compare(CFilter_logic::GridItem i1, CFilter_logic::GridItem i2) 
{ 
    return ((i1.value_raw  * i1.value_raw + i1.deviationFromAVG * i1.deviationFromAVG) < (i2.value_raw  * i2.value_raw + i2.deviationFromAVG * i2.deviationFromAVG)); 
//    return ((i1.value  * i1.value + i1.deviationFromAVG * i1.deviationFromAVG) < (i2.value  * i2.value + i2.deviationFromAVG * i2.deviationFromAVG)); 
} 

bool IsContain(std::vector<CFilter_logic::GridItem> &_list, CFilter_logic::GridItem i1){
    for (int i = 0 ; i < _list.size() ; i++){
        if (_list[i].aa == i1.aa && _list[i].bb == i1.bb)
            return true;
    }
    return false;
}

CFilter_logic::CFilter_logic(CCalibMan::Ptr _calib_man):
m_calib_man(_calib_man)
{
    m_percent = PERCENT_RANGE;
    m_calibAmpl = 0x1000;
    m_index = 0;
    m_oldcalibAmpl = -1;
    m_calibRef = 0.9;
    m_calibMode = 0;
    m_grid.clear();
}

void CFilter_logic::init(rp_channel_t _ch){
    m_channel = _ch;
    m_index = 0;
    m_percent = PERCENT_RANGE;
    m_calibAmpl = 0x1000;
    m_oldcalibAmpl = -1;
    m_grid.clear();
    auto min_a = MIN_AA;
    auto max_a = MAX_AA;
    auto min_b = MIN_BB;
    auto max_b = MAX_BB;

    if (m_calib_man->getModeLV_HV() == RP_HIGH){
        min_a = MIN_AA_HI;
        max_a = MAX_AA_HI;
        min_b = MIN_BB_HI;
        max_b = MAX_BB_HI;
    }
    for(int i = min_a ; i <= max_a ; i += (max_a - min_a) / STEPS_AA){
        for(int j = min_b ; j <= max_b ; j += (max_b - min_b) / STEPS_BB){
            GridItem item;
            item.aa = i;
            item.bb = j;
            item.value = 0;
            item.value_raw = 0;
            item.calculate = false;
            item.ch = _ch;
            item.deviationFromAVG = 0;
            item.index = m_index++;
            m_grid.push_back(item);
        }
    }
}

void CFilter_logic::setCalibRef(float _value){
    m_calibRef = _value;
}

// 0 - External, 1 - Internal
void CFilter_logic::setCalibMode(int _mode){
    m_calibMode = _mode;
}

void CFilter_logic::print(){
    sort();
    for(int i = 0 ; i < m_grid.size() ; i++){
        printf("%d {AA: %6x, BB: %6x, V: %2.6f, Vraw: %2.6f, D: %3.6f, index: %d}\n",i,m_grid[i].aa,m_grid[i].bb,m_grid[i].value,m_grid[i].value_raw,m_grid[i].deviationFromAVG,m_grid[i].index);
    }
}

int CFilter_logic::setCalibParameters(){
    for(int i = 0 ; i < m_grid.size() ; i++){
        if (m_grid[i].calculate == false) {
            m_calib_man->setCalibValue(m_grid[i].ch == RP_CH_1 ? F_AA_CH1 : F_AA_CH2  ,m_grid[i].aa);
            m_calib_man->setCalibValue(m_grid[i].ch == RP_CH_1 ? F_BB_CH1 : F_BB_CH2  ,m_grid[i].bb);
            m_calib_man->updateCalib();
            m_calib_man->updateAcqFilter(m_grid[i].ch);
//            std::cout  << "Cur step " << i <<  " aa = " << m_grid[i].aa << " bb = " << m_grid[i].bb << std::endl;
            return i;
        }
    }
    return -1;
}

void CFilter_logic::setGoodCalibParameter(){
    sort();
    m_calib_man->setCalibValue(m_lastGood.ch == RP_CH_1 ? F_AA_CH1 : F_AA_CH2  ,m_lastGood.aa);
    m_calib_man->setCalibValue(m_lastGood.ch == RP_CH_1 ? F_BB_CH1 : F_BB_CH2  ,m_lastGood.bb);
    m_calib_man->updateCalib();
    m_calib_man->updateAcqFilter(m_lastGood.ch);
}

int CFilter_logic::getCalibCount(){
    return m_grid.size();
}

int CFilter_logic::getCalibDone(){
    int x = 0;
    for(int i = 0 ; i < m_grid.size() ; i++){
        if (m_grid[i].calculate) x++;
    }
    return x;
}

int CFilter_logic::setCalculatedValue(COscilloscope::DataPassAutoFilter item){
    for(int i = 0 ; i < m_grid.size() ; i++){
        if (m_grid[i].aa == item.f_aa) 
            if (m_grid[i].bb == item.f_bb)
                if (m_grid[i].ch == item.cur_channel)
                    if (!m_grid[i].calculate)
                    {
                        m_grid[i].value = item.calib_value;
                        m_grid[i].value_raw = item.calib_value_raw;                        
                        m_grid[i].calculate = true;
                        m_grid[i].deviationFromAVG = item.deviation;
    //                    std::cout  << "Cur step " << i <<  " aa = " << m_grid[i].aa << " bb = " << m_grid[i].bb << " VALUE " << m_grid[i].value <<  std::endl;
                        return i;
                    }
    }
    return -1;
}

void CFilter_logic::removeHalfCalib(){
    sort();
    m_grid.erase (m_grid.end() - m_grid.size() / 3.0 , m_grid.end());
    if (m_grid.size() > 0) 
        m_lastGood = m_grid[0];
}

CFilter_logic::GridItem GetNewItem(std::vector<CFilter_logic::GridItem> _list,int _index){
    std::vector<CFilter_logic::GridItem> new_grid;
    for(int i = 0 ; i < _list.size() ; i++) {
        if (_list[i].index == _index){
            new_grid.push_back(_list[i]);
        }
    }
    if (new_grid.size() > 0) {
        std::sort(new_grid.begin(), new_grid.end(), compare); 
        return new_grid[0];
    }
    CFilter_logic::GridItem item;
    item.index = -1;
    return item;
}

std::vector<CFilter_logic::GridItem> Split(CFilter_logic::GridItem _item,double _step){   
    std::vector<CFilter_logic::GridItem> new_grid;
    if (_step == 0) return new_grid;
    for(double i = 1.0 - (_step / 100.0) ; i <= 1 + (_step / 100.0) ; i += (_step / 100.0))
        for(double j = 1.0 - (_step / 100.0) ; j <= 1 + (_step / 100.0) ; j += (_step / 100.0))
        {
            CFilter_logic::GridItem new_item = _item;
            new_item.calculate = false;
            new_item.aa *= i;
            new_item.bb *= j;
            new_item.lastValue = new_item.value;
            new_item.lastDeviation = new_item.deviationFromAVG;
            if (!IsContain(new_grid,new_item))
                new_grid.push_back(new_item);
        }
    return new_grid;
}

int CFilter_logic::nextSetupCalibParameters(){
   if (m_percent < PERCENT_MIN) return -1;
   std::vector<GridItem> new_grid;
   std::vector<int64_t>  index_list;
   for(int i = 0 ; i < m_grid.size() ; i++){
       if (std::find(index_list.begin(), index_list.end(), m_grid[i].index) == index_list.end()){
           index_list.push_back(m_grid[i].index);
       }
   }
   m_percent /= STEPS_BB; 
   for(int i = 0 ; i < index_list.size() ; i++){
       auto item = GetNewItem(m_grid,index_list[i]);
       auto split_grid = Split(item,m_percent);
       new_grid.insert(new_grid.end(), split_grid.begin(), split_grid.end());
   }
   m_grid = new_grid;
   std::cout << m_percent << std::endl;
   return  m_percent < PERCENT_MIN ? -1 : 0;
}

void CFilter_logic::sort(){
    std::sort(m_grid.begin(), m_grid.end(), compare); 
}

int CFilter_logic::calcProgress(){
    double p = PERCENT_RANGE;
    double cur = 0;
    double max_range = 0;
    while(p >= PERCENT_MIN){
        max_range++;
        p /= STEPS_BB; 
        if (p >= m_percent) cur++;
    }
    auto progress = cur / max_range * 100.0;
    if (progress < 0) progress = 0;
    if (progress > 100) progress = 100;
    return progress;
};

int CFilter_logic::calibPP(COscilloscope::DataPassAutoFilter item,float _nominal){
    if (item.is_valid == false) return 0;
    if (m_calibAmpl == 0) return -2;     
    if (m_oldcalibAmpl != -1){
        if (m_oldPP != item.f_pp) return 0;
        float dir = 1;
        if (item.ampl > _nominal) dir = -1;

        if ((item.ampl < _nominal && m_oldcalibAmpl > _nominal) || (item.ampl > _nominal && m_oldcalibAmpl < _nominal)){
            dir *= -1;
            m_calibAmpl /= 2;
        }
//        std::cout <<  "A: " << item.ampl << " m_calibAmpl = " << m_calibAmpl << " dir = " << dir << " PP :" << item.f_pp << std::endl;
        item.f_pp += m_calibAmpl * dir;
//        std::cout <<  "New PP :" << item.f_pp << std::endl;
        if (item.f_pp == MIN_PP) return -1;
        if (item.f_pp >= MAX_PP) return -1;
        m_calib_man->setCalibValue(item.cur_channel == RP_CH_1 ? F_AA_CH1 : F_AA_CH2  ,item.f_aa);
        m_calib_man->setCalibValue(item.cur_channel == RP_CH_1 ? F_BB_CH1 : F_BB_CH2  ,item.f_bb);
        m_calib_man->setCalibValue(item.cur_channel == RP_CH_1 ? F_PP_CH1 : F_PP_CH2  ,item.f_pp);
        m_calib_man->setCalibValue(item.cur_channel == RP_CH_1 ? F_KK_CH1 : F_KK_CH2  ,item.f_kk);
        m_calib_man->updateCalib();
        m_calib_man->updateAcqFilter(item.cur_channel);
        if (m_calibAmpl == 0) return -2; 
    }
    m_oldcalibAmpl = item.ampl;
    m_oldPP = item.f_pp;
    return 0;
}
