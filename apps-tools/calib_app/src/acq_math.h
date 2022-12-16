#pragma once
#include <vector>
#include <stdint.h>

// auto convertCnts(int16_t cnts) -> int16_t;
auto calcCountCrossZero(float *_buffer, int _size) -> std::vector<int>;
auto findLastMax(float *_buffer,int _size,int _cross) -> int;
auto filterBuffer(float *_buffer,int _size) -> float*;
auto calculate(float *_buffer, int _size,float _last_max, int _cross1,int _cross2, double &_deviation) -> double;