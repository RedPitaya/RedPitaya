#pragma once
#include <vector>

std::vector<int> calcCountCrossZero(float *_buffer, int _size); 
             int findLastMax(float *_buffer,int _size,int _cross);
          float* filterBuffer(float *_buffer,int _size);
          double calculate(float *_buffer, int _size,float _last_max, int _cross1,int _cross2, double &_deviation);