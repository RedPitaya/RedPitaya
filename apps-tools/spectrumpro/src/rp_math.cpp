#include "rp_math.h"
#include "common.h"

const float __log10f_rng = 0.3010299957f;

const float __log10f_lut[8] = {
    -0.99697286229624,  //p0
    -1.07301643912502,  //p4
    -2.46980061535534,  //p2
    -0.07176870463131,  //p6
    2.247870219989470,  //p1
    0.366547581117400,  //p5
    1.991005185100089,  //p3
    0.006135635201050,  //p7
};

float log10f_neon(float x) {
#ifdef ARCH_ARM
    float dest = 0;
    asm volatile(

        "vdup.f32		d0, d0[0]				\n\t"  //d0 = {x,x};

        //extract exponent
        "vmov.i32		d2, #127				\n\t"  //d2 = 127;
        "vshr.u32		d6, d0, #23				\n\t"  //d6 = d0 >> 23;
        "vsub.i32		d6, d6, d2				\n\t"  //d6 = d6 - d2;
        "vshl.u32		d1, d6, #23				\n\t"  //d1 = d6 << 23;
        "vsub.i32		d0, d0, d1				\n\t"  //d0 = d0 + d1;

        //polynomial:
        "vmul.f32 		d1, d0, d0				\n\t"  //d1 = d0*d0 = {x^2, x^2}
        "vld1.32 		{d2, d3, d4, d5}, [%2]	\n\t"  //q1 = {p0, p4, p2, p6}, q2 = {p1, p5, p3, p7} ;
        "vmla.f32 		q1, q2, d0[0]			\n\t"  //q1 = q1 + q2 * d0[0]
        "vmla.f32 		d2, d3, d1[0]			\n\t"  //d2 = d2 + d3 * d1[0]
        "vmul.f32 		d1, d1, d1				\n\t"  //d1 = d1 * d1 = {x^4, x^4}
        "vmla.f32 		d2, d1, d2[1]			\n\t"  //d2 = d2 + d1 * d2[1]

        //add exponent
        "vdup.32 		d7, %1					\n\t"  //d7 = {rng, rng}
        "vcvt.f32.s32 	d6, d6					\n\t"  //d6 = (float) d6
        "vmla.f32 		d2, d6, d7				\n\t"  //d2 = d2 + d6 * d7

        "vmov.f32 		%0, s4	        		\n\t"  //s0 = s4

        : "r="(dest)
        : "r"(__log10f_rng), "r"(__log10f_lut)
        : "d0", "d1", "q1", "q2", "d6", "d7");
    return dest;
#else
    return log10f(x);
#endif
}

float sqrtf_neon(float x) {
#ifdef ARCH_ARM
    float dest = 0;
    asm volatile(

        //fast invsqrt approx
        "vmov.f32 		d1, d0					\n\t"  //d1 = d0
        "vrsqrte.f32 	d0, d0					\n\t"  //d0 = ~ 1.0 / sqrt(d0)
        "vmul.f32 		d2, d0, d1				\n\t"  //d2 = d0 * d1
        "vrsqrts.f32 	d3, d2, d0				\n\t"  //d3 = (3 - d0 * d2) / 2
        "vmul.f32 		d0, d0, d3				\n\t"  //d0 = d0 * d3
        "vmul.f32 		d2, d0, d1				\n\t"  //d2 = d0 * d1
        "vrsqrts.f32 	d3, d2, d0				\n\t"  //d4 = (3 - d0 * d3) / 2
        "vmul.f32 		d0, d0, d3				\n\t"  //d0 = d0 * d3

        //fast reciporical approximation
        "vrecpe.f32		d1, d0					\n\t"          //d1 = ~ 1 / d0;
        "vrecps.f32		d2, d1, d0				\n\t"          //d2 = 2.0 - d1 * d0;
        "vmul.f32		d1, d1, d2				\n\t"          //d1 = d1 * d2;
        "vrecps.f32		d2, d1, d0				\n\t"          //d2 = 2.0 - d1 * d0;
        "vmul.f32		d0, d1, d2				\n\t"          //d0 = d1 * d2;
        "vmov.f32 		%0, s0	        		        \n\t"  //s0 = s4
        : "r="(dest)::"d0", "d1", "d2", "d3");
    return dest;
#else
    return sqrtf(x);
#endif
}

float indexInLogSpace(int start, int stop, int value) {
    if (value == 0)
        return value;
    double a = start;
    double b = stop;
    double x = log10f_neon(b / a) / (b - a);
    value = (log10f_neon((double)value / b) / x + b);
    return value;
}

float koffInLogSpace(float start, float stop, float value) {
    if (value == 0)
        return value;
    double a = start;
    double b = stop;

    double x = log10f_neon(b / a) / (b - a);
    value = (b / pow(10, x * b) * pow(10, value * x));
    return value;
}

void prepareIndexArray(std::vector<int>* data, int start, int stop, int view_size, int log_mode) {
    float koef = 1;
    float koefMax = 1;
    int pointsNumOrig = stop - start;
    int pointsNum = pointsNumOrig;
    view_size /= 2.0;
    if (pointsNum > view_size) {
        koefMax = (float)pointsNum / view_size;
        koef = (float)pointsNum / view_size;
    }
    data->resize(stop - start);
    // int *idx = new int[stop-start];
    for (size_t i = start, j = 0; i < stop; ++i, ++j) {
        int index = (j / koef);
        if (log_mode == 1) {
            float z = stop - start;
            index = round(indexInLogSpace(1, view_size * 3 + 1, ((float)(j * view_size * 3)) / z + 1));
        }
        (*data)[i - start] = index;
    }
    // return idx;
}

void decimateDataMinMax(CFloatBinarySignal& dest, float* src, int start, int stop, int view_size, int log_mode, int* indexArray) {

    float koef = 1;
    float koefMax = 1;
    int pointsNumOrig = stop - start;
    int pointsNum = pointsNumOrig;
    view_size /= 2.0;
    if (pointsNum > view_size) {
        koefMax = (float)pointsNum / view_size;
        koef = (float)pointsNum / view_size;
    }
    float v_min = 0;
    float v_max = 0;
    std::vector<int> min;
    std::vector<int> max;
    min.reserve(CH_SIGNAL_DATA);
    max.reserve(CH_SIGNAL_DATA);
    int last_index = -1;
    for (size_t i = start, j = 0; i < stop; ++i, ++j) {
        int index = indexArray[i - start];

        if (index != last_index) {
            last_index = index;
            min.push_back(i);
            max.push_back(i);
            v_min = std::numeric_limits<float>::max();
            v_max = std::numeric_limits<float>::lowest();
        }
        if (v_max < src[i]) {
            v_max = src[i];
            max[max.size() - 1] = i;
        }

        if (v_min > src[i]) {
            v_min = src[i];
            min[min.size() - 1] = i;
        }
    }
    dest.Resize(min.size() + max.size());
    for (int i = 0, j = 0; i < min.size() + max.size(); i += 2, j++) {
        dest[i] = min[j] < max[j] ? src[min[j]] : src[max[j]];
        dest[i + 1] = min[j] > max[j] ? src[min[j]] : src[max[j]];
    }
}

void decimateDataFirstN(CFloatBinarySignal& dest, float* src, int start, int stop, int view_size, int log_mode, int* indexArray) {

    int pointsNumOrig = stop - start;
    int pointsNum = pointsNumOrig;
    if (pointsNum > view_size) {
        pointsNum = view_size;
    }

    int size_all = 0;
    int indexes[CH_SIGNAL_DATA];

    int last_index = -1;
    for (size_t i = start, j = 0; i < stop; ++i, ++j) {
        int index = indexArray[i - start];

        if (index != last_index) {
            indexes[size_all++] = i;
            last_index = index;
        }
    }
    dest.Resize(size_all);
    for (size_t i = 0; i < size_all; i++) {
        dest[i] = src[indexes[i]];
    }
}

void decimateDataAvg(CFloatBinarySignal& dest, float* src, int start, int stop, int view_size, int log_mode, int* indexArray) {

    int sum_size = 0;
    float sum_arr[CH_SIGNAL_DATA];
    int sum_arr_count[CH_SIGNAL_DATA];
    int last_index = -1;
    for (size_t i = start, j = 0; i < stop; ++i, ++j) {
        int index = indexArray[i - start];

        if (index != last_index) {
            sum_arr[sum_size] = 0;
            sum_arr_count[sum_size] = 0;
            sum_size++;
            last_index = index;
        }

        sum_arr[sum_size - 1] += src[i];
        sum_arr_count[sum_size - 1]++;
    }
    dest.Resize(sum_size);
    for (size_t i = 0; i < sum_size; i++) {
        if (sum_arr_count[i])
            dest[i] = sum_arr[i] / sum_arr_count[i];
        else
            dest[i] = 0;
    }
}

void decimateDataMax(CFloatBinarySignal& dest, float* src, int start, int stop, int view_size, int log_mode, int* indexArray) {

    float koef = 1;
    float koefMax = 1;
    int pointsNumOrig = stop - start;
    int pointsNum = pointsNumOrig;
    if (pointsNum > view_size) {
        koefMax = (float)pointsNum / view_size;
        koef = (float)pointsNum / view_size;
    }
    float v_max = 0;
    float max[CH_SIGNAL_DATA];
    int all_max = 0;
    int last_index = -1;
    for (size_t i = start, j = 0; i < stop; ++i, ++j) {
        int index = indexArray[i - start];

        if (index != last_index) {
            last_index = index;
            all_max++;
            max[all_max - 1] = src[i];
            ;
        }
        if (max[all_max - 1] < src[i]) {
            max[all_max - 1] = src[i];
        }

        // if (v_min > src[i]) {
        //     v_min = src[i];
        //     min[min.size()-1] = i;
        // }
    }
    dest.Resize(all_max);
    for (int i = 0; i < all_max; i++) {
        dest[i] = max[i];
    }
}

void decimateData(CFloatBinarySignal& dest, float* src, int start, int stop, int view_size, int log_mode, int* indexArray) {
    //    decimateDataMinMax(dest,src,start,stop,view_size,log_mode,indexArray);
    //    decimateDataFirstN(dest,src,start,stop,view_size,log_mode,indexArray);
    //    decimateDataAvg(dest,src,start,stop,view_size,log_mode,indexArray);
    decimateDataMax(dest, src, start, stop, view_size, log_mode, indexArray);
}

// use in waterfall
void decimateDataByMax(CFloatBinarySignal& dest, float* src, int start, int stop, int view_size) {
    float koef = 1;
    int pointsNumOrig = stop - start;
    int pointsNum = pointsNumOrig;
    if (pointsNum > view_size) {
        koef = (float)pointsNum / view_size;
        pointsNum = view_size;
    }
    dest.Resize(pointsNum);
    int last_index = -1;
    for (size_t i = start, j = 0; i < stop; ++i, ++j) {
        int index = (int)(j / koef);
        if (index != last_index) {
            last_index = index;
            dest[index] = std::numeric_limits<float>::lowest();
        }
        if (dest[index] < src[i])
            dest[index] = src[i];
    }
}