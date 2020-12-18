#include <cstring>
#include <iostream>
#include <math.h>
#include "acq_math.h"
#include "rp.h"


int16_t convertCnts(int16_t cnts)
{
    int16_t m;

    /* check sign */
    if(cnts & (1 << (ADC_BITS - 1))) {
        /* negative number */
        m = -1 *((cnts ^ ((1 << ADC_BITS) - 1)) + 1);
    } else {
        /* positive number */
        m = cnts;
    }
    /* check limits */
    if(m < (-1 * (1 << (ADC_BITS - 1))))
        m = (-1 * (1 << (ADC_BITS - 1)));
    else if(m > (1 << (ADC_BITS - 1)))
        m = (1 << (ADC_BITS - 1));

    return m;
}

std::vector<int> calcCountCrossZero(float *_buffer, int _size){
    std::vector<int> cross;
    cross.clear();
    for(int i = 0 ; i < _size - 1 ; i ++){
        if ((_buffer[i] < 0 && _buffer[i+1] > 0) || (_buffer[i] > 0 && _buffer[i+1] < 0)){
            cross.push_back(i);
        //    std::cout <<  "\nI = " << i <<  " B[I] " << _buffer[i] << " B[i+1] " << _buffer[i+1] << std::endl;  
        }       
    }    
    return cross;
}

float* filterBuffer(float *_buffer,int _size){
    float *new_buffer = new float[_size];
    std::memcpy(new_buffer,_buffer,_size);    
    float core[] = { 1.0 / 8.0 , 1.0 / 4.0 , 1.0 / 4.0 , 1.0 / 4.0 , 1.0 / 8.0};
    for(int i = 2 ; i < _size - 2 ; i++ ){
        float sum = 0;
        for (int j = -2 ; j <= 2 ; j++ ){
            sum += core[j + 2] * _buffer [i + j];
        }
        new_buffer[i] = sum;
    }
    return new_buffer;
}

int findLastMax(float *_buffer,int _size,int _cross){
    if (_cross == 0) return -1;
    auto f = filterBuffer(_buffer,_size);
    float eps = 0.002;
    int left_pos =  _cross;
    float delta_max = 1;
    while(fabs(f[left_pos - 1] - f[left_pos]) > eps ){
        left_pos--;
        if (left_pos == 0){
            delete f;
            return -1;
        }
    } 
    delete f;
    return left_pos;
}

double calculate(float *_buffer, int _size,float _last_max, int _cross1,int _cross2, double &_deviation){
    if (_cross1 + 1 > _size) return -1;
    _cross1++;
    if (_cross1 >= _cross2) return -1;
    int count = 0;
    double sum = 0;
    auto ch = _buffer;
    auto coff = 1 / _last_max;
    //auto ch = filterBuffer(_buffer,_size);
    float w = 10;
    for(int i = _cross1 ; i < _cross2-1 ; i++){
        if (fabs(sin(1.0/(ch[i] - ch[i+1])) - 1) > 0.05) {
            sum  += fabs(ch[i] * coff - 1)  * w; 
            count ++;
            w--;
            if (w < 1) w =1;
        }
    }
    _deviation = 0;
    for(int i = _cross1; i < _cross2-1 ; i++){
        if (fabs(sin(1.0/(ch[i] - ch[i+1])) - 1) > 0.03) {
            auto z= fabs(ch[i] * coff - ch[i+1] * coff);
             _deviation  +=  z;
        } 
    }    
    _deviation /= 1000.0;
    return sum;
}
