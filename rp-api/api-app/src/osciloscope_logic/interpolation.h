#ifndef __INTERPOLATION
#define __INTERPOLATION

#include <math.h>

template<typename T>
inline auto sinc(T x) -> T {
	if (x == 0) {
		return 1.0;
	}
	return sin(x) / x;
}

template<typename T>
inline auto catmullRom(const T _p0,const T _p1,const T _p2, const T _p3, T _u) -> T{
    T point;
	point  = _u * _u * _u * ((-1) * _p0 + 3 * _p1 - 3 * _p2 + _p3) / 2;
	point += _u * _u * ( 2 * _p0 - 5 * _p1 + 4 * _p2 - _p3) / 2;
	point += _u * ((-1) * _p0 + _p2) / 2;
	point += _p1;
    return point;
}

template<typename T>
inline auto bSpline(const T _p0,const T _p1,const T _p2, const T _p3, T _u) -> T{
    T point;
	point  = _u * _u * _u *((-1) * _p0 + 3 * _p1 - 3 * _p2 + _p3) / 6;
	point += _u * _u * (3 * _p0 - 6 * _p1+ 3 * _p2) / 6;
	point += _u * ((-3) * _p0 + 3 * _p2) / 6;
	point += (_p0 + 4 * _p1 + _p2) / 6;
	return point;
}

template<typename T>
inline auto linear(const T ,const T _p1,const T _p2, const T , const T _u) -> T {
    T k = (_p2 - _p1);
    T b = _p1;
    return (k * _u) + b;
}

template<typename T>
inline auto lanczos(const T _p0, const T _p1,const T _p2, const T _p3, const T _p4, T _u) {
	T data[] = {_p0, _p1, _p2 , _p3, _p4};
	int a = 2;
	T result = 0.0;
	for (int k = -a; k <= a; k++) {
        T z = M_PI * (_u - k);
        T sincTerm = sinc<T>(z) * sinc<T>(z/a);
        result += data[k + a] * sincTerm;
    }
	return result;
}


#endif // __INTERPOLATION